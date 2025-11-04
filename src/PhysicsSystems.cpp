#include "PhysicsSystems.h"

#include "JoltUtils.h"
#include "MathUtils.h"
#include "Phases.h"
#include "PhysicsComponents.h"
#include "RenderComponents.h"
#include "TransformComponents.h"

#include <flecs.h>
#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>
#include <Jolt/Physics/Character/Character.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>

#include <iostream>

#include "InputComponents.h"
#include "spdlog/spdlog.h"


using namespace JPH::literals;

res::PhysicsSystems::PhysicsSystems(flecs::world& world)
{
    world.module<PhysicsSystems>();

    const auto on_tick_phase = world.lookup(OnTickPhaseName.data());

    assert(on_tick_phase != 0 && "Tick Phase not found!");

    world.observer<PhysicsHandleComponent>("Initialize Physics System")
         .event(flecs::OnAdd)
         .each([](flecs::entity e, PhysicsHandleComponent& handle)
         {
             JPH::RegisterDefaultAllocator();

             JPH::Trace = TraceImpl;
             JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = AssertFailedImpl);

             JPH::Factory::sInstance = new JPH::Factory();

             JPH::RegisterTypes();

             handle.tempAllocator = std::make_unique<JPH::TempAllocatorImpl>(10 * 1024 * 1024);
             handle.jobSystem = std::make_unique<JPH::JobSystemThreadPool>(
                 JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers,
                 std::thread::hardware_concurrency() - 1);

             constexpr JPH::uint cMaxBodies = 65536;
             constexpr JPH::uint cNumBodyMutexes = 0;
             constexpr JPH::uint cMaxBodyPairs = 65536;
             constexpr JPH::uint cMaxContactConstraints = 10240;

             handle.broadPhaseLayerInterface = std::make_unique<BPLayerInterfaceImpl>();
             handle.objectVsBroadPhaseLayerFilter = std::make_unique<ObjectVsBroadPhaseLayerFilterImpl>();
             handle.objectVsObjectLayerFilter = std::make_unique<ObjectLayerPairFilterImpl>();

             handle.physicsSystem = std::make_unique<JPH::PhysicsSystem>();
             handle.physicsSystem->Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints,
                                        *handle.broadPhaseLayerInterface, *handle.objectVsBroadPhaseLayerFilter,
                                        *handle.objectVsObjectLayerFilter);

             handle.bodyInterface = &handle.physicsSystem->GetBodyInterface();
         });

    world.observer<PhysicsHandleComponent>("Deinitialize Physics System")
         .event(flecs::OnRemove)
         .each([](flecs::entity e, PhysicsHandleComponent& handle)
         {
             JPH::UnregisterTypes();
             delete JPH::Factory::sInstance;
             JPH::Factory::sInstance = nullptr;
         });

    world.observer<ModelComponent, PhysicsBodyIdComponent, MatrixComponent, MeshColliderComponent>(
             "Create StaticMesh Body")
         .event(flecs::OnAdd)
         .each([&world](const ModelComponent& modelComponent, PhysicsBodyIdComponent& bodyIdHolder,
                        MatrixComponent& matrixComponent,
                        const MeshColliderComponent& meshColliderComponent)
             {
                 JPH::StaticCompoundShapeSettings staticCompoundShapeSettings{};
                 AssembleStaticCompoundShape(staticCompoundShapeSettings, modelComponent);
                 JPH::ShapeSettings::ShapeResult compoundShapeResult = staticCompoundShapeSettings.Create();
                 if (compoundShapeResult.HasError())
                 {
                     spdlog::error("Failed to create a compound shape!");
                 }

                 JPH::ShapeRefC meshShape = compoundShapeResult.Get();
                 auto entityPosition = GetPositionFromMatrix(matrixComponent.matrix);
                 auto entityRotation = QuaternionNormalize(QuaternionFromMatrix(matrixComponent.matrix));
                 JPH::RVec3 bodyPosition{entityPosition.x, entityPosition.y, entityPosition.z};
                 JPH::Quat bodyRotation{entityRotation.x, entityRotation.y, entityRotation.z, entityRotation.w};
                 JPH::BodyCreationSettings bodySettings{
                     meshShape, bodyPosition, bodyRotation, JPH::EMotionType::Static,
                     PhysicsObjectLayers::NON_MOVING
                 };

                 auto& handle = world.get<PhysicsHandleComponent>();
                 JPH::Body* body = handle.bodyInterface->CreateBody(bodySettings);
                 handle.bodyInterface->AddBody(body->GetID(), JPH::EActivation::DontActivate);
                 bodyIdHolder.bodyID = body->GetID();
             }
         );

    world.observer<PhysicsBodyIdComponent>("Clear BodyID")
         .event(flecs::OnRemove)
         .each([&world](PhysicsBodyIdComponent& body_id_holder)
         {
             if (body_id_holder.bodyID.IsInvalid())
             {
                 spdlog::error("Body Id is invalid!");
                 return;
             }
             auto& handle = world.get<PhysicsHandleComponent>();
             handle.bodyInterface->RemoveBody(body_id_holder.bodyID);
             handle.bodyInterface->DestroyBody(body_id_holder.bodyID);
         });

    world.observer<const RigidbodySphereComponent, PhysicsBodyIdComponent>("Create Physics Ball")
         .event(flecs::OnAdd)
         .each([&world](const RigidbodySphereComponent& rigidbody_sphere, PhysicsBodyIdComponent& body_id_holder)
         {
             if (body_id_holder.bodyID.IsInvalid())
             {
                 spdlog::error("Body Id is invalid!");
                 return;
             }
             auto& handle = world.get<PhysicsHandleComponent>();
             constexpr float kSphereRadius = 0.5f;
             constexpr float kInitialHeight = 5.0f;
             constexpr float kRestitution = 1.0f;
             constexpr float kFriction = 0.0f;
             constexpr float kInitialVelocityY = -1.0f;
             
             JPH::BodyCreationSettings sphere_settings(new JPH::SphereShape(kSphereRadius),
                                                      JPH::RVec3(0.0_r, kInitialHeight, 0.0_r),
                                                      JPH::Quat::sIdentity(),
                                                      JPH::EMotionType::Dynamic,
                                                      PhysicsObjectLayers::MOVING);
             sphere_settings.mRestitution = kRestitution;
             sphere_settings.mFriction = kFriction;
             body_id_holder.bodyID = handle.bodyInterface->CreateAndAddBody(
                 sphere_settings, JPH::EActivation::Activate);

             // Now you can interact with the dynamic body, in this case we're going to give it a velocity.
             // (note that if we had used CreateBody then we could have set the velocity straight on the body before adding it to the physics system)
             handle.bodyInterface->SetLinearVelocity(body_id_holder.bodyID, JPH::Vec3(0.0f, kInitialVelocityY, 0.0f));
         });

    world.observer<const CharacterControllerComponent, PhysicsBodyIdComponent>("Create Character Capsule")
         .event(flecs::OnSet)
         .each([&world](const CharacterControllerComponent& character_capsule, PhysicsBodyIdComponent& body_id_holder)
         {
             auto& handle = world.get<PhysicsHandleComponent>();

             constexpr float kMaxSlopeAngle = 45.0f;
             constexpr float kCharacterFriction = 0.5f;
             
             JPH::RefConst capsule_shape = JPH::RotatedTranslatedShapeSettings(
                                              JPH::Vec3(
                                                  0, 0.5f * character_capsule.characterHeight +
                                                  character_capsule.characterRadius,
                                                  0),
                                              JPH::Quat::sIdentity(),
                                              new JPH::CapsuleShape(0.5f * character_capsule.characterHeight,
                                                                    character_capsule.characterRadius)).
                                          Create().Get();

             JPH::Ref character_settings = new JPH::CharacterSettings();
             character_settings->mMaxSlopeAngle = kMaxSlopeAngle;
             character_settings->mLayer = PhysicsObjectLayers::MOVING;
             character_settings->mShape = capsule_shape;
             character_settings->mFriction = kCharacterFriction;
             character_settings->mSupportingVolume = JPH::Plane(JPH::Vec3::sAxisY(), -character_capsule.characterRadius);
             auto character = new JPH::Character(character_settings, JPH::Vec3::sZero(), JPH::Quat::sIdentity(), 0,
                                                 handle.physicsSystem.get());
             body_id_holder.bodyID = character->GetBodyID();
             character->AddToPhysicsSystem(JPH::EActivation::Activate);
         });


    world.system<const GravityComponent, PhysicsBodyIdComponent>("Apply Gravity")
         .kind(on_tick_phase)
         .each([&world](const GravityComponent& gravity_component, PhysicsBodyIdComponent& body_id_holder)
         {
             if (body_id_holder.bodyID.IsInvalid())
             {
                 spdlog::error("Body Id is invalid! System: Apply Gravity");
                 return;
             }
             auto& handle = world.get<PhysicsHandleComponent>();

             auto current_position = handle.bodyInterface->GetCenterOfMassPosition(body_id_holder.bodyID);
             handle.bodyInterface->MoveKinematic(body_id_holder.bodyID,
                                                 current_position + (gravity_component.gravityForce * GetFrameTime()),
                                                 JPH::Quat::sIdentity(), GetFrameTime());
         });

    world.system<const MovementInputComponent, const CharacterControllerComponent, const PhysicsBodyIdComponent>(
             "Apply Character Movement Input")
         .kind(on_tick_phase)
         .each([&world](const MovementInputComponent& movement_input_component,
                        const CharacterControllerComponent& character_component,
                        const PhysicsBodyIdComponent& body_id_holder)
         {
             if (body_id_holder.bodyID.IsInvalid())
             {
                 spdlog::error("Body Id is invalid! System:Move Character with keys");
                 return;
             }
             auto& handle = world.get<PhysicsHandleComponent>();
             constexpr float kMovementSpeed = 5.0f;
             auto movement = JPH::Vec3(movement_input_component.input.x * kMovementSpeed, 0.0f,
                                       movement_input_component.input.y * kMovementSpeed);
             handle.bodyInterface->SetLinearVelocity(body_id_holder.bodyID, movement);
         });


    world.system("Run Physics Simulation")
         .kind(on_tick_phase)
         .run([&world](flecs::iter& it)
         {
             auto& handle = world.get<PhysicsHandleComponent>();
             constexpr int kCollisionSteps = 1;
             handle.physicsSystem->Update(GetFrameTime(), kCollisionSteps, handle.tempAllocator.get(),
                                          handle.jobSystem.get());
         });

    world.system<const PhysicsBodyIdComponent, MatrixComponent>("Move Physics Body")
         .kind(on_tick_phase)
         .each([&world](const PhysicsBodyIdComponent& body_id_component, MatrixComponent& matrix)
         {
             if (body_id_component.bodyID.IsInvalid())
             {
                 spdlog::error("Body Id is invalid! system: Move Physics Body");
                 return;
             }
             auto& handle = world.get<PhysicsHandleComponent>();
             auto position = handle.bodyInterface->GetCenterOfMassPosition(body_id_component.bodyID);
             matrix.matrix = MatrixTranslate(position.GetX(), position.GetY(), position.GetZ());
         });
}

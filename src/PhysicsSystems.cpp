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

#include "spdlog/spdlog.h"


using namespace JPH::literals;

res::PhysicsSystems::PhysicsSystems(flecs::world& world)
{
    world.module<PhysicsSystems>();

    const auto onTickPhase = world.lookup(OnTickPhaseName.data());

    assert(onTickPhase != 0 && "Tick Phase not found!");

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
         .each([&world](PhysicsBodyIdComponent& bodyIdHolder)
         {
             if (bodyIdHolder.bodyID.IsInvalid())
             {
                 spdlog::error("Body Id is invalid!");
                 return;
             }
             auto& handle = world.get<PhysicsHandleComponent>();
             handle.bodyInterface->RemoveBody(bodyIdHolder.bodyID);
             handle.bodyInterface->DestroyBody(bodyIdHolder.bodyID);
         });

    world.observer<const RigidbodySphereComponent, PhysicsBodyIdComponent>("Create Physics Ball")
         .event(flecs::OnAdd)
         .each([&world](const RigidbodySphereComponent& body, PhysicsBodyIdComponent& bodyIdHolder)
         {
             if (bodyIdHolder.bodyID.IsInvalid())
             {
                 spdlog::error("Body Id is invalid!");
                 return;
             }
             auto& handle = world.get<PhysicsHandleComponent>();
             JPH::BodyCreationSettings sphereSettings(new JPH::SphereShape(0.5f),
                                                      JPH::RVec3(0.0_r, 5.0_r, 0.0_r),
                                                      JPH::Quat::sIdentity(),
                                                      JPH::EMotionType::Dynamic,
                                                      PhysicsObjectLayers::MOVING);
             sphereSettings.mRestitution = 1.0f;
             sphereSettings.mFriction = 0.0f;
             bodyIdHolder.bodyID = handle.bodyInterface->CreateAndAddBody(
                 sphereSettings, JPH::EActivation::Activate);

             // Now you can interact with the dynamic body, in this case we're going to give it a velocity.
             // (note that if we had used CreateBody then we could have set the velocity straight on the body before adding it to the physics system)
             handle.bodyInterface->SetLinearVelocity(bodyIdHolder.bodyID, JPH::Vec3(0.0f, -1.0f, 0.0f));
         });

    world.observer<const CharacterControllerComponent, PhysicsBodyIdComponent>("Create Character Capsule")
         .event(flecs::OnSet)
         .each([&world](const CharacterControllerComponent& characterCapsule, PhysicsBodyIdComponent& bodyIdHolder)
         {
             auto& handle = world.get<PhysicsHandleComponent>();

             JPH::RefConst capsuleShape = JPH::RotatedTranslatedShapeSettings(
                                              JPH::Vec3(
                                                  0, 0.5f * characterCapsule.characterHeight +
                                                  characterCapsule.characterRadius,
                                                  0),
                                              JPH::Quat::sIdentity(),
                                              new JPH::CapsuleShape(0.5f * characterCapsule.characterHeight,
                                                                    characterCapsule.characterRadius)).
                                          Create().Get();

             JPH::Ref characterSettings = new JPH::CharacterSettings();
             characterSettings->mMaxSlopeAngle = 45.0f;
             characterSettings->mLayer = PhysicsObjectLayers::MOVING;
             characterSettings->mShape = capsuleShape;
             characterSettings->mFriction = 0.5f;
             characterSettings->mSupportingVolume = JPH::Plane(JPH::Vec3::sAxisY(), -characterCapsule.characterRadius);
             auto character = new JPH::Character(characterSettings, JPH::Vec3::sZero(), JPH::Quat::sIdentity(), 0,
                                                 handle.physicsSystem.get());
             bodyIdHolder.bodyID = character->GetBodyID();
             character->AddToPhysicsSystem(JPH::EActivation::Activate);
         });


    world.system<const GravityComponent, PhysicsBodyIdComponent>("Apply Gravity")
         .kind(onTickPhase)
         .each([&world](const GravityComponent& gravityComponent, PhysicsBodyIdComponent& bodyIdHolder)
         {
             if (bodyIdHolder.bodyID.IsInvalid())
             {
                 spdlog::error("Body Id is invalid! System: Apply Gravity");
                 return;
             }
             auto& handle = world.get<PhysicsHandleComponent>();

             auto currentPos = handle.bodyInterface->GetCenterOfMassPosition(bodyIdHolder.bodyID);
             handle.bodyInterface->MoveKinematic(bodyIdHolder.bodyID,
                                                 currentPos + (gravityComponent.gravityForce * GetFrameTime()),
                                                 JPH::Quat::sIdentity(), GetFrameTime());
         });

    world.system<const CharacterControllerComponent, const PhysicsBodyIdComponent>("Move character with keys")
         .kind(onTickPhase)
         .each([&world](const CharacterControllerComponent& characterComponent,
                        const PhysicsBodyIdComponent& bodyIdHolder)
         {
             if (bodyIdHolder.bodyID.IsInvalid())
             {
                 spdlog::error("Body Id is invalid! System:Move Character with keys");
                 return;
             }
             auto& handle = world.get<PhysicsHandleComponent>();
             auto movement = JPH::Vec3((float)IsKeyDown(KEY_W) * 10.0f - (float)IsKeyDown(KEY_S) * 10.0f, 0.0f, 0.0f);
             handle.bodyInterface->SetLinearVelocity(bodyIdHolder.bodyID, movement);
         });


    world.system("Run Physics Simulation")
         .kind(onTickPhase)
         .run([&world](flecs::iter& it)
         {
             auto& handle = world.get<PhysicsHandleComponent>();
             handle.physicsSystem->Update(GetFrameTime(), 1, handle.tempAllocator.get(),
                                          handle.jobSystem.get());
         });

    world.system<const PhysicsBodyIdComponent, MatrixComponent>("Move Physics Body")
         .kind(onTickPhase)
         .each([&world](const PhysicsBodyIdComponent& bodyIdComponent, MatrixComponent& matrix)
         {
             if (bodyIdComponent.bodyID.IsInvalid())
             {
                 spdlog::error("Body Id is invalid! system: Move Physics Body");
                 return;
             }
             auto& handle = world.get<PhysicsHandleComponent>();
             auto pos = handle.bodyInterface->GetCenterOfMassPosition(bodyIdComponent.bodyID);
             matrix.matrix = MatrixTranslate(pos.GetX(), pos.GetY(), pos.GetZ());
         });
}

#include "PhysicsSystems.h"

#include <iostream>

#include <flecs.h>
#include <Jolt/Jolt.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Character/Character.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/RegisterTypes.h>
#include <spdlog/spdlog.h>

#include "InputComponents.h"
#include "JoltUtils.h"
#include "MathUtils.h"
#include "Phases.h"
#include "PhysicsComponents.h"
#include "RenderComponents.h"
#include "TransformComponents.h"


using namespace JPH::literals;

res::PhysicsSystems::PhysicsSystems(flecs::world& world)
{
    world.module<PhysicsSystems>();

    const auto on_tick_phase = world.lookup(kTickPhaseName.data());

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

             handle.temp_allocator = std::make_unique<JPH::TempAllocatorImpl>(10 * 1024 * 1024);
             handle.job_system = std::make_unique<JPH::JobSystemThreadPool>(
                 JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers,
                 std::thread::hardware_concurrency() - 1);

             constexpr JPH::uint kMaxBodies = 65536;
             constexpr JPH::uint kNumBodyMutexes = 0;
             constexpr JPH::uint kMaxBodyPairs = 65536;
             constexpr JPH::uint kMaxContactConstraints = 10240;

             handle.broad_phase_layer_interface = std::make_unique<BPLayerInterfaceImpl>();
             handle.object_vs_broad_phase_layer_filter = std::make_unique<ObjectVsBroadPhaseLayerFilterImpl>();
             handle.object_vs_object_layer_filter = std::make_unique<ObjectLayerPairFilterImpl>();

             handle.physics_system = std::make_unique<JPH::PhysicsSystem>();
             handle.physics_system->Init(kMaxBodies, kNumBodyMutexes, kMaxBodyPairs, kMaxContactConstraints,
                                        *handle.broad_phase_layer_interface, *handle.object_vs_broad_phase_layer_filter,
                                        *handle.object_vs_object_layer_filter);

             handle.body_interface = &handle.physics_system->GetBodyInterface();
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
         .each([&world](const ModelComponent& model_component, PhysicsBodyIdComponent& body_id_holder,
                        MatrixComponent& matrix_component,
                        const MeshColliderComponent& mesh_collider)
             {
                 JPH::StaticCompoundShapeSettings static_compound_shape_settings{};
                 AssembleStaticCompoundShape(static_compound_shape_settings, model_component);
                 JPH::ShapeSettings::ShapeResult compound_shape_result = static_compound_shape_settings.Create();
                 if (compound_shape_result.HasError())
                 {
                     spdlog::error("Failed to create a compound shape!");
                 }

                 JPH::ShapeRefC mesh_shape = compound_shape_result.Get();
                 auto entity_position = GetPositionFromMatrix(matrix_component.matrix);
                 auto entity_rotation = QuaternionNormalize(QuaternionFromMatrix(matrix_component.matrix));
                 JPH::RVec3 body_position{entity_position.x, entity_position.y, entity_position.z};
                 JPH::Quat body_rotation{entity_rotation.x, entity_rotation.y, entity_rotation.z, entity_rotation.w};
                 JPH::BodyCreationSettings body_settings{
                     mesh_shape, body_position, body_rotation, JPH::EMotionType::Static,
                     PhysicsObjectLayers::NON_MOVING
                 };

                 auto& handle = world.get<PhysicsHandleComponent>();
                 JPH::Body* body = handle.body_interface->CreateBody(body_settings);
                 handle.body_interface->AddBody(body->GetID(), JPH::EActivation::DontActivate);
                 body_id_holder.body_id = body->GetID();
             }
         );

    world.observer<PhysicsBodyIdComponent>("Clear BodyID")
         .event(flecs::OnRemove)
         .each([&world](PhysicsBodyIdComponent& body_id_holder)
         {
             if (body_id_holder.body_id.IsInvalid())
             {
                 spdlog::error("Body Id is invalid!");
                 return;
             }
             auto& handle = world.get<PhysicsHandleComponent>();
             handle.body_interface->RemoveBody(body_id_holder.body_id);
             handle.body_interface->DestroyBody(body_id_holder.body_id);
         });

    world.observer<const RigidbodySphereComponent, PhysicsBodyIdComponent>("Create Physics Ball")
         .event(flecs::OnAdd)
         .each([&world](const RigidbodySphereComponent& rigidbody_sphere, PhysicsBodyIdComponent& body_id_holder)
         {
             if (body_id_holder.body_id.IsInvalid())
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
             body_id_holder.body_id = handle.body_interface->CreateAndAddBody(
                 sphere_settings, JPH::EActivation::Activate);

             // Now you can interact with the dynamic body, in this case we're going to give it a velocity.
             // (note that if we had used CreateBody then we could have set the velocity straight on the body before adding it to the physics system)
             handle.body_interface->SetLinearVelocity(body_id_holder.body_id, JPH::Vec3(0.0f, kInitialVelocityY, 0.0f));
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
                                                  0, 0.5f * character_capsule.character_height +
                                                  character_capsule.character_radius,
                                                  0),
                                              JPH::Quat::sIdentity(),
                                              new JPH::CapsuleShape(0.5f * character_capsule.character_height,
                                                                    character_capsule.character_radius)).
                                          Create().Get();

             JPH::Ref character_settings = new JPH::CharacterSettings();
             character_settings->mMaxSlopeAngle = kMaxSlopeAngle;
             character_settings->mLayer = PhysicsObjectLayers::MOVING;
             character_settings->mShape = capsule_shape;
             character_settings->mFriction = kCharacterFriction;
             character_settings->mSupportingVolume = JPH::Plane(JPH::Vec3::sAxisY(), -character_capsule.character_radius);
             auto character = new JPH::Character(character_settings, JPH::Vec3::sZero(), JPH::Quat::sIdentity(), 0,
                                                 handle.physics_system.get());
             body_id_holder.body_id = character->GetBodyID();
             character->AddToPhysicsSystem(JPH::EActivation::Activate);
         });


    world.system<const GravityComponent, PhysicsBodyIdComponent>("Apply Gravity")
         .kind(on_tick_phase)
         .each([&world](const GravityComponent& gravity_component, PhysicsBodyIdComponent& body_id_holder)
         {
             if (body_id_holder.body_id.IsInvalid())
             {
                 spdlog::error("Body Id is invalid! System: Apply Gravity");
                 return;
             }
             auto& handle = world.get<PhysicsHandleComponent>();

             auto current_position = handle.body_interface->GetCenterOfMassPosition(body_id_holder.body_id);
             handle.body_interface->MoveKinematic(body_id_holder.body_id,
                                                 current_position + (gravity_component.gravity_force * GetFrameTime()),
                                                 JPH::Quat::sIdentity(), GetFrameTime());
         });

    world.system<const MovementInputComponent, const CharacterControllerComponent, const PhysicsBodyIdComponent>(
             "Apply Character Movement Input")
         .kind(on_tick_phase)
         .each([&world](const MovementInputComponent& movement_input_component,
                        const CharacterControllerComponent& character_component,
                        const PhysicsBodyIdComponent& body_id_holder)
         {
             if (body_id_holder.body_id.IsInvalid())
             {
                 spdlog::error("Body Id is invalid! System:Move Character with keys");
                 return;
             }
             auto& handle = world.get<PhysicsHandleComponent>();
             constexpr float kMovementSpeed = 5.0f;
             auto movement = JPH::Vec3(movement_input_component.input.x * kMovementSpeed, 0.0f,
                                       movement_input_component.input.y * kMovementSpeed);
             handle.body_interface->SetLinearVelocity(body_id_holder.body_id, movement);
         });


    world.system("Run Physics Simulation")
         .kind(on_tick_phase)
         .run([&world](flecs::iter& it)
         {
             auto& handle = world.get<PhysicsHandleComponent>();
             constexpr int kCollisionSteps = 1;
             handle.physics_system->Update(GetFrameTime(), kCollisionSteps, handle.temp_allocator.get(),
                                          handle.job_system.get());
         });

    world.system<const PhysicsBodyIdComponent, MatrixComponent>("Move Physics Body")
         .kind(on_tick_phase)
         .each([&world](const PhysicsBodyIdComponent& body_id_component, MatrixComponent& matrix)
         {
             if (body_id_component.body_id.IsInvalid())
             {
                 spdlog::error("Body Id is invalid! system: Move Physics Body");
                 return;
             }
             auto& handle = world.get<PhysicsHandleComponent>();
             auto position = handle.body_interface->GetCenterOfMassPosition(body_id_component.body_id);
             matrix.matrix = MatrixTranslate(position.GetX(), position.GetY(), position.GetZ());
         });
}

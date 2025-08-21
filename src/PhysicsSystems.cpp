#include "PhysicsSystems.h"

#include "PhysicsComponents.h"
#include "JoltUtils.h"
#include "MathUtils.h"
#include "RenderComponents.h"
#include "TransformComponents.h"
#include "Phases.h"

#include <flecs.h>
#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>

#include <iostream>

#include "Jolt/Physics/Character/Character.h"
#include "Jolt/Physics/Collision/Shape/CapsuleShape.h"
#include "Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h"

using namespace JPH::literals;

res::PhysicsSystems::PhysicsSystems(flecs::world& world)
{
    world.module<PhysicsSystems>();

    world.observer<sPhysicsHandle>("Initialize Physics System")
         .event(flecs::OnAdd)
         .each([](flecs::entity e, sPhysicsHandle& handle)
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

    world.observer<sPhysicsHandle>("Deinitialize Physics System")
         .event(flecs::OnRemove)
         .each([](flecs::entity e, sPhysicsHandle& handle)
         {
             JPH::UnregisterTypes();
             delete JPH::Factory::sInstance;
             JPH::Factory::sInstance = nullptr;
         });

    world.observer<cModel, cPhysicsBodyID, cMatrix, cMeshCollider>("Create StaticMesh Body")
         .event(flecs::OnAdd)
         .each([&world](const cModel& modelComponent, cPhysicsBodyID& bodyIdHolder, cMatrix& matrixComponent,
                        const cMeshCollider& meshColliderComponent)
             {
                 JPH::StaticCompoundShapeSettings staticCompoundShapeSettings{};
                 AssembleStaticCompoundShape(staticCompoundShapeSettings, modelComponent);
                 JPH::ShapeSettings::ShapeResult compoundShapeResult = staticCompoundShapeSettings.Create();
                 if (compoundShapeResult.HasError())
                 {
                     std::cout << "Failed to create compound shape!" << "\n";
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

                 auto& handle = world.get<sPhysicsHandle>();
                 JPH::Body* body = handle.bodyInterface->CreateBody(bodySettings);
                 handle.bodyInterface->AddBody(body->GetID(), JPH::EActivation::DontActivate);
                 bodyIdHolder.bodyID = body->GetID();
             }
         );

    world.observer<cPhysicsBodyID>("Clear BodyID")
         .event(flecs::OnRemove)
         .each([&world](cPhysicsBodyID& bodyIdHolder)
         {
             if (bodyIdHolder.bodyID.IsInvalid())
             {
                 return;
             }
             auto& handle = world.get<sPhysicsHandle>();
             handle.bodyInterface->RemoveBody(bodyIdHolder.bodyID);
             handle.bodyInterface->DestroyBody(bodyIdHolder.bodyID);
         });

    world.observer<const cPhysicsBall, cPhysicsBodyID>("Create Physics Ball")
         .event(flecs::OnAdd)
         .each([&world](const cPhysicsBall& body, cPhysicsBodyID& bodyIdHolder)
         {
             auto& handle = world.get<sPhysicsHandle>();
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

    world.observer<const cCharacterCapsule, cPhysicsBodyID>("Create Character Capsule")
         .event(flecs::OnAdd)
         .each([&world](const cCharacterCapsule& characterCapsule, cPhysicsBodyID& bodyIdHolder)
         {
             //TODO: write a Character
             // auto& handle = world.get<sPhysicsHandle>();
             //
             // auto capsuleShape = JPH::RotatedTranslatedShapeSettings(JPH::Vec3(0, 0.5f * 2 + 1, 0),
             //                                                         JPH::Quat::sIdentity(),
             //                                                         new JPH::CapsuleShape(0.5f * 2, 1)).Create().Get();
             //
             // JPH::CharacterSettings characterSettings{};
             // characterSettings.mMaxSlopeAngle = 45.0f;
             // characterSettings.mLayer = PhysicsObjectLayers::MOVING;
             // characterSettings.mShape = capsuleShape;
             // characterSettings.mFriction = 0.5f;
             // characterSettings.mSupportingVolume = JPH::Plane(JPH::Vec3::sAxisY(),);
         });

    const auto onTickPhase = world.lookup(OnTickPhaseName.data());

    world.system<const cGravity, cPhysicsBodyID>("Apply Gravity")
         .kind(onTickPhase)
         .each([&world](const cGravity& gravityComponent, cPhysicsBodyID& bodyIdHolder)
         {
             auto& handle = world.get<sPhysicsHandle>();

             auto currentPos = handle.bodyInterface->GetCenterOfMassPosition(bodyIdHolder.bodyID);
             handle.bodyInterface->MoveKinematic(bodyIdHolder.bodyID,
                                                 currentPos + (gravityComponent.gravityForce * GetFrameTime()),
                                                 JPH::Quat::sIdentity(), GetFrameTime());
         });

    world.system("Run Physics Simulation")
         .kind(onTickPhase)
         .run([&world](flecs::iter& it)
         {
             auto& handle = world.get<sPhysicsHandle>();
             handle.physicsSystem->Update(GetFrameTime(), 1, handle.tempAllocator.get(),
                                          handle.jobSystem.get());
         });

    world.system<const cPhysicsBodyID, cMatrix>("Move Physics Body")
         .kind(onTickPhase)
         .each([&world](const cPhysicsBodyID& pb, cMatrix& matrix)
         {
             if (pb.bodyID.IsInvalid())
             {
                 std::cout << "body id is invalid" << "\n";
                 return;
             }
             auto& handle = world.get<sPhysicsHandle>();
             auto pos = handle.bodyInterface->GetCenterOfMassPosition(pb.bodyID);
             matrix.matrix = MatrixTranslate(pos.GetX(), pos.GetY(), pos.GetZ());
         });
}

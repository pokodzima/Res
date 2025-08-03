#pragma once
#include "PhysicsComponents.h"
#include "JoltUtils.h"

#include <flecs.h>
#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>

#include <iostream>


using namespace JPH::literals;

namespace res
{
    struct PhysicsSystems
    {
        explicit PhysicsSystems(flecs::world& world)
        {
            world.module<PhysicsSystems>();

            world.observer<sPhysicsHandle>()
                 .event(flecs::OnAdd)
                 .each([](flecs::entity e, sPhysicsHandle& handle)
                 {
                     std::cout << "Start Physics..." << "\n";

                     JPH::RegisterDefaultAllocator();

                     JPH::Trace = TraceImpl;
                     JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = AssertFailedImpl);

                     JPH::Factory::sInstance = new JPH::Factory();

                     JPH::RegisterTypes();


                     handle.tempAllocator = std::make_unique<JPH::TempAllocatorImpl>(10 * 1024 * 1024);
                     //JPH::TempAllocatorImpl tempAllocator(10 * 1024 * 1024);
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

            world.observer<sPhysicsHandle>()
                 .event(flecs::OnRemove)
                 .each([](flecs::entity e, sPhysicsHandle& handle)
                 {
                     std::cout << "Destroy Physics..." << "\n";

                     JPH::UnregisterTypes();
                     delete JPH::Factory::sInstance;
                     JPH::Factory::sInstance = nullptr;
                 });

            world.observer<cStaticPhysicsBody>()
                 .event(flecs::OnAdd)
                 .each([&world](cStaticPhysicsBody& body)
                 {
                     auto& handle = world.get<sPhysicsHandle>();
                     JPH::BoxShapeSettings floorShapeSettings(JPH::Vec3(100.0f, 1.0f, 100.0f));
                     //floorShapeSettings.SetEmbedded();
                     JPH::ShapeSettings::ShapeResult floorShapeResult = floorShapeSettings.Create();
                     JPH::ShapeRefC floorShape = floorShapeResult.Get();
                     if (floorShapeResult.HasError())
                     {
                         std::cout << "ERROR shape";
                         return;
                     }
                     JPH::BodyCreationSettings floorSettings(floorShape, JPH::RVec3(0.0_r, -1.0_r, 0.0_r),
                                                             JPH::Quat::sIdentity(), JPH::EMotionType::Static,
                                                             PhysicsObjectLayers::NON_MOVING);

                     if (handle.bodyInterface == nullptr)
                     {
                         std::cout << "NO Body Interface, ERROR";
                         return;
                     }
                     JPH::Body* floor = handle.bodyInterface->CreateBody(floorSettings);

                     handle.bodyInterface->AddBody(floor->GetID(), JPH::EActivation::DontActivate);
                     body.bodyID = floor->GetID();
                 });

            world.observer<cStaticPhysicsBody>()
                 .event(flecs::OnRemove)
                 .each([&world](cStaticPhysicsBody& body)
                 {
                     if (body.bodyID.IsInvalid())
                     {
                         return;
                     }
                     auto& handle = world.get<sPhysicsHandle>();
                     handle.bodyInterface->RemoveBody(body.bodyID);
                     handle.bodyInterface->DestroyBody(body.bodyID);
                 });

            world.observer<cPhysicsBall>()
                 .event(flecs::OnAdd)
                 .each([&world](cPhysicsBall& body)
                 {
                     auto& handle = world.get<sPhysicsHandle>();
                     JPH::BodyCreationSettings sphere_settings(new JPH::SphereShape(0.5f),
                                                               JPH::RVec3(0.0_r, 5.0_r, 0.0_r),
                                                               JPH::Quat::sIdentity(), JPH::EMotionType::Dynamic,
                                                               PhysicsObjectLayers::MOVING);
                     sphere_settings.mRestitution = 1.0f;
                     sphere_settings.mFriction = 0.0f;
                     body.bodyID = handle.bodyInterface->CreateAndAddBody(
                         sphere_settings, JPH::EActivation::Activate);

                     // Now you can interact with the dynamic body, in this case we're going to give it a velocity.
                     // (note that if we had used CreateBody then we could have set the velocity straight on the body before adding it to the physics system)
                     handle.bodyInterface->SetLinearVelocity(body.bodyID, JPH::Vec3(0.0f, -1.0f, 0.0f));
                 });

            world.observer<cPhysicsBall>()
                 .event(flecs::OnRemove)
                 .each([&world](cPhysicsBall& body)
                 {
                     if (body.bodyID.IsInvalid())
                     {
                         std::cout << "body id is invalid" << "\n";
                         return;
                     }
                     auto& handle = world.get<sPhysicsHandle>();
                     handle.bodyInterface->RemoveBody(body.bodyID);
                     handle.bodyInterface->DestroyBody(body.bodyID);
                 });


            world.system("Run Physics Simulation")
                 .kind(OnTick)
                 .run([&world](flecs::iter& it)
                 {
                     auto& handle = world.get<sPhysicsHandle>();
                     handle.physicsSystem->Update(1.0f / 60.0f, 1, handle.tempAllocator.get(),
                                                  handle.jobSystem.get());
                 });

            world.system<const cPhysicsBall, cMatrix>()
                 .kind(OnTick)
                 .each([&world](const cPhysicsBall& pb, cMatrix& matrix)
                 {
                     if (pb.bodyID.IsInvalid())
                     {
                         std::cout << "body id is invalid" << "\n";
                         return;
                     }
                     auto& handle = world.get<sPhysicsHandle>();
                     auto pos = handle.bodyInterface->GetCenterOfMassPosition(pb.bodyID);
                     matrix.matrix = MatrixTranslate(pos.GetX(), pos.GetY(), pos.GetZ());
                     std::cout << pos.GetX() << "\n";
                 });
        }
    };
}

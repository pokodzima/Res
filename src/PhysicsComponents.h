#pragma once
#include "JoltUtils.h"

#include <flecs.h>
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSystem.h>


namespace res
{
    struct PhysicsHandleComponent
    {
        std::unique_ptr<BPLayerInterfaceImpl> broadPhaseLayerInterface;
        std::unique_ptr<ObjectVsBroadPhaseLayerFilterImpl> objectVsBroadPhaseLayerFilter;
        std::unique_ptr<ObjectLayerPairFilterImpl> objectVsObjectLayerFilter;
        std::unique_ptr<JPH::PhysicsSystem> physicsSystem;
        std::unique_ptr<JPH::TempAllocatorImpl> tempAllocator;
        std::unique_ptr<JPH::JobSystemThreadPool> jobSystem;
        JPH::BodyInterface* bodyInterface{nullptr};
    };

    struct RigidbodySphereComponent
    {
    };

    struct PhysicsBodyIdComponent
    {
        JPH::BodyID bodyID{};
    };

    struct MeshColliderComponent
    {
    };

    struct CharacterControllerComponent
    {
        float characterHeight = 2.0f;
        float characterRadius = 0.5f;
    };

    struct GravityComponent
    {
        JPH::Vec3 gravityForce = JPH::Vec3(0.0f, -9.8f, 0.0f);
    };

    struct PhysicsComponents
    {
        explicit PhysicsComponents(flecs::world& world)
        {
            world.module<PhysicsComponents>();

            world.add<PhysicsHandleComponent>();
        }
    };
}

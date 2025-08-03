#pragma once
#include "JoltUtils.h"

#include <flecs.h>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyInterface.h>

#include "Jolt/Core/JobSystemThreadPool.h"
#include "Jolt/Physics/PhysicsSystem.h"


namespace res
{
    struct sPhysicsHandle
    {
        std::unique_ptr<BPLayerInterfaceImpl> broadPhaseLayerInterface;
        std::unique_ptr<ObjectVsBroadPhaseLayerFilterImpl> objectVsBroadPhaseLayerFilter;
        std::unique_ptr<ObjectLayerPairFilterImpl> objectVsObjectLayerFilter;
        std::unique_ptr<JPH::PhysicsSystem> physicsSystem;
        std::unique_ptr<JPH::TempAllocatorImpl> tempAllocator;
        std::unique_ptr<JPH::JobSystemThreadPool> jobSystem;
        JPH::BodyInterface* bodyInterface;
    };

    struct cStaticPhysicsBody
    {
        JPH::BodyID bodyID;
    };

    struct cPhysicsBall
    {
        JPH::BodyID bodyID;
    };

    struct PhysicsComponents
    {
        explicit PhysicsComponents(flecs::world& world)
        {
            world.module<PhysicsComponents>();

            world.add<sPhysicsHandle>();
        }
    };
}

#pragma once

#include "JoltUtils.h"

#include <flecs.h>
#include <Jolt/Jolt.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/PhysicsSystem.h>


namespace res
{
    struct PhysicsHandleComponent
    {
        std::unique_ptr<BPLayerInterfaceImpl> broad_phase_layer_interface;
        std::unique_ptr<ObjectVsBroadPhaseLayerFilterImpl> object_vs_broad_phase_layer_filter;
        std::unique_ptr<ObjectLayerPairFilterImpl> object_vs_object_layer_filter;
        std::unique_ptr<JPH::PhysicsSystem> physics_system;
        std::unique_ptr<JPH::TempAllocatorImpl> temp_allocator;
        std::unique_ptr<JPH::JobSystemThreadPool> job_system;
        JPH::BodyInterface* body_interface{nullptr};
    };

    struct RigidbodySphereComponent
    {
    };

    struct PhysicsBodyIdComponent
    {
        JPH::BodyID body_id{};
    };

    struct MeshColliderComponent
    {
    };

    struct CharacterControllerComponent
    {
        float character_height = 2.0f;
        float character_radius = 0.5f;
    };

    struct GravityComponent
    {
        JPH::Vec3 gravity_force = JPH::Vec3(0.0f, -9.8f, 0.0f);
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

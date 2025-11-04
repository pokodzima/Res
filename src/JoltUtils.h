#pragma once

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Geometry/IndexedTriangle.h>

#include <cstdarg>
#include <cstdio>
#include <iostream>

namespace JPH
{
    class StaticCompoundShapeSettings;
}

namespace res
{
    struct ModelComponent;

    namespace PhysicsObjectLayers
    {
        static constexpr JPH::ObjectLayer NON_MOVING = 0;
        static constexpr JPH::ObjectLayer MOVING = 1;
        static constexpr JPH::ObjectLayer NUM_LAYERS = 2;
    }

    namespace BroadPhaseLayers
    {
        static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
        static constexpr JPH::BroadPhaseLayer MOVING(1);
        static constexpr JPH::uint NUM_LAYERS = 2;
    }

    static void TraceImpl(const char* format, ...)
    {
        // Format the message
        va_list list;
        va_start(list, format);
        char buffer[1024];
        vsnprintf(buffer, sizeof(buffer), format, list);
        va_end(list);

        // Print to the TTY
        std::cout << buffer << "\n";
    }

#ifdef JPH_ENABLE_ASSERTS
    // Callback for asserts, connect this to your own assert handler if you have one
    static bool AssertFailedImpl(const char* expression, const char* message, const char* file, JPH::uint line)
    {
        // Print to the TTY
        std::cout << file << ":" << line << ": (" << expression << ") " << (message != nullptr ? message : "")
            << "\n";

        // Breakpoint
        return true;
    };
#endif // JPH_ENABLE_ASSERTS

    class ObjectLayerPairFilterImpl final : public JPH::ObjectLayerPairFilter
    {
    public:
        [[nodiscard]] bool ShouldCollide(JPH::ObjectLayer layer1, JPH::ObjectLayer layer2) const override;
    };

    class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
    {
    public:
        BPLayerInterfaceImpl():
            object_to_broad_phase_{}
        {
            object_to_broad_phase_[PhysicsObjectLayers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
            object_to_broad_phase_[PhysicsObjectLayers::MOVING] = BroadPhaseLayers::MOVING;
        }

        [[nodiscard]] JPH::uint GetNumBroadPhaseLayers() const override
        {
            return BroadPhaseLayers::NUM_LAYERS;
        }

        [[nodiscard]] JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer layer) const override
        {
            JPH_ASSERT(layer < PhysicsObjectLayers::NUM_LAYERS);
            return object_to_broad_phase_[layer];
        }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
        [[nodiscard]] const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer layer) const override
        {
            switch (static_cast<JPH::BroadPhaseLayer::Type>(layer))
            {
            case static_cast<JPH::BroadPhaseLayer::Type>(BroadPhaseLayers::NON_MOVING): return "NON_MOVING";
            case static_cast<JPH::BroadPhaseLayer::Type>(BroadPhaseLayers::MOVING): return "MOVING";
            default:
                JPH_ASSERT(false);
                return "INVALID";
            }
        }
#endif

    private:
        JPH::BroadPhaseLayer object_to_broad_phase_[PhysicsObjectLayers::NUM_LAYERS];
    };

    class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter
    {
    public:
        [[nodiscard]] bool ShouldCollide(JPH::ObjectLayer layer1, JPH::BroadPhaseLayer layer2) const override;
    };


    void PopulateJoltVertices(const float* raylib_vertices, int vertex_count, JPH::VertexList& jolt_vertices);
    void PopulateJoltTriangles(const unsigned short* raylib_indices, int triangle_count,
                               JPH::IndexedTriangleList& jolt_triangles);
    void AssembleStaticCompoundShape(JPH::StaticCompoundShapeSettings& shape_settings, const ModelComponent& model_component);
}

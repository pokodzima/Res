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

    static void TraceImpl(const char* inFMT, ...)
    {
        // Format the message
        va_list list;
        va_start(list, inFMT);
        char buffer[1024];
        vsnprintf(buffer, sizeof(buffer), inFMT, list);
        va_end(list);

        // Print to the TTY
        std::cout << buffer << "\n";
    }

#ifdef JPH_ENABLE_ASSERTS
    // Callback for asserts, connect this to your own assert handler if you have one
    static bool AssertFailedImpl(const char* inExpression, const char* inMessage, const char* inFile, JPH::uint inLine)
    {
        // Print to the TTY
        std::cout << inFile << ":" << inLine << ": (" << inExpression << ") " << (inMessage != nullptr ? inMessage : "")
            << "\n";

        // Breakpoint
        return true;
    };
#endif // JPH_ENABLE_ASSERTS

    class ObjectLayerPairFilterImpl final : public JPH::ObjectLayerPairFilter
    {
    public:
        [[nodiscard]] bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::ObjectLayer inLayer2) const override;
    };

    class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
    {
    public:
        BPLayerInterfaceImpl():
            mObjectToBroadPhase{}
        {
            mObjectToBroadPhase[PhysicsObjectLayers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
            mObjectToBroadPhase[PhysicsObjectLayers::MOVING] = BroadPhaseLayers::MOVING;
        }

        [[nodiscard]] JPH::uint GetNumBroadPhaseLayers() const override
        {
            return BroadPhaseLayers::NUM_LAYERS;
        }

        [[nodiscard]] JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override
        {
            JPH_ASSERT(inLayer < PhysicsObjectLayers::NUM_LAYERS);
            return mObjectToBroadPhase[inLayer];
        }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
        [[nodiscard]] const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override
        {
            switch (static_cast<JPH::BroadPhaseLayer::Type>(inLayer))
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
        JPH::BroadPhaseLayer mObjectToBroadPhase[PhysicsObjectLayers::NUM_LAYERS];
    };

    class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter
    {
    public:
        [[nodiscard]] bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override;
    };


    void PopulateJoltVertices(const float* rlVertices, int vertexCount, JPH::VertexList& joltVertices);
    void PopulateJoltTriangles(const unsigned short* rlIndices, int triangleCount,
                               JPH::IndexedTriangleList& joltTriangles);
    void AssembleStaticCompoundShape(JPH::StaticCompoundShapeSettings& shapeSettings, const ModelComponent& modelComponent);
}

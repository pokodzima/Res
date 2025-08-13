#pragma once
#include <Jolt/Jolt.h>
#include <Jolt/Math/Float3.h>
#include <Jolt/Geometry/IndexedTriangle.h>

namespace res
{
    struct cModel;
}

namespace JPH
{
    class StaticCompoundShapeSettings;
}

namespace flecs
{
    struct world;
}

namespace res
{
    struct PhysicsSystems
    {
        explicit PhysicsSystems(flecs::world& world);
    };

    void PopulateJoltVertices(const float* rlVertices, int vertexCount, JPH::VertexList& joltVertices);
    void PopulateJoltTriangles(const unsigned short* rlIndices, int triangleCount,
                               JPH::IndexedTriangleList& joltTriangles);
    void AssembleStaticCompoundShape(JPH::StaticCompoundShapeSettings& shapeSettings, const cModel& modelComponent);
}

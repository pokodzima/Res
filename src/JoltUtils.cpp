#include "JoltUtils.h"

#include "RenderComponents.h"

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>

bool res::ObjectLayerPairFilterImpl::ShouldCollide(JPH::ObjectLayer inLayer1, JPH::ObjectLayer inLayer2) const
{
    switch (inLayer1)
    {
    case PhysicsObjectLayers::NON_MOVING:
        return inLayer2 == PhysicsObjectLayers::MOVING;
    case PhysicsObjectLayers::MOVING:
        return true;
    default:
        JPH_ASSERT(false);
        return false;
    }
}

bool res::ObjectVsBroadPhaseLayerFilterImpl::ShouldCollide(JPH::ObjectLayer inLayer1,
                                                           JPH::BroadPhaseLayer inLayer2) const
{
    switch (inLayer1)
    {
    case PhysicsObjectLayers::NON_MOVING:
        return inLayer2 == BroadPhaseLayers::MOVING;
    case PhysicsObjectLayers::MOVING:
        return true;
    default:
        JPH_ASSERT(false);
        return false;
    }
}

void res::PopulateJoltVertices(const float* rlVertices, const int vertexCount, JPH::VertexList& joltVertices)
{
    std::vector<float> vertexBuffer{};

    for (int vertexIndex = 0; vertexIndex < vertexCount * 3; ++vertexIndex)
    {
        if (vertexBuffer.size() < 3)
        {
            vertexBuffer.push_back(rlVertices[vertexIndex]);
        }
        if (vertexBuffer.size() == 3)
        {
            JPH::Float3 joltVertex{};

            joltVertex.x = vertexBuffer[0];
            joltVertex.y = vertexBuffer[1];
            joltVertex.z = vertexBuffer[2];

            joltVertices.push_back(joltVertex);

            vertexBuffer.clear();
        }
    }
}

void res::PopulateJoltTriangles(const unsigned short* rlIndices, const int triangleCount,
                                JPH::IndexedTriangleList& joltTriangles)
{
    for (int triangleIndex = 0; triangleIndex < triangleCount; ++triangleIndex)
    {
        auto index1 = rlIndices[triangleIndex * 3];
        auto index2 = rlIndices[triangleIndex * 3 + 1];
        auto index3 = rlIndices[triangleIndex * 3 + 2];
        JPH::IndexedTriangle joltTriangle{index1, index2, index3};
        joltTriangles.push_back(joltTriangle);
    }
}

void res::AssembleStaticCompoundShape(JPH::StaticCompoundShapeSettings& shapeSettings, const cModel& modelComponent)
{
    for (int meshIndex = 0; meshIndex < modelComponent.model.meshCount; ++meshIndex)
    {
        auto rlVertices = modelComponent.model.meshes[meshIndex].vertices;
        auto vertexCount = modelComponent.model.meshes[meshIndex].vertexCount;
        JPH::VertexList joltVertices{};
        PopulateJoltVertices(rlVertices, vertexCount, joltVertices);

        auto rlIndices = modelComponent.model.meshes[meshIndex].indices;
        auto triangleCount = modelComponent.model.meshes[meshIndex].triangleCount;
        JPH::IndexedTriangleList joltTriangleList{};
        PopulateJoltTriangles(rlIndices, triangleCount, joltTriangleList);

        JPH::MeshShapeSettings meshShapeSettings{
            joltVertices, joltTriangleList
        };

        JPH::ShapeSettings::ShapeResult shapeResult = meshShapeSettings.Create();

        if (shapeResult.HasError())
        {
            std::cout << "Failed to create shape!" << "\n";
        }
        shapeSettings.AddShape(JPH::Vec3::sZero(), JPH::Quat::sIdentity(),
                               shapeResult.Get());
    }
}

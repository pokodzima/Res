#include "JoltUtils.h"

#include "RenderComponents.h"

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>
#include <spdlog/spdlog.h>

#include <utility>


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
    if (!rlVertices || vertexCount <= 0)
    {
        spdlog::error("Error populating triangles");
        return;
    }

    joltVertices.reserve(joltVertices.size() + static_cast<size_t>(vertexCount));

    for (int i = 0; i < vertexCount; ++i)
    {
        const int base = i * 3;
        JPH::Float3 v{rlVertices[base + 0], rlVertices[base + 1], rlVertices[base + 2]};
        joltVertices.push_back(v);
    }
}

void res::PopulateJoltTriangles(const unsigned short* rlIndices, const int triangleCount,
                                JPH::IndexedTriangleList& joltTriangles)
{
    if (!rlIndices || triangleCount <= 0)
    {
        spdlog::error("Error populating triangles");
        return;
    }

    joltTriangles.reserve(joltTriangles.size() + static_cast<size_t>(triangleCount));
    for (int i = 0; i < triangleCount; ++i)
    {
        const int base = i * 3;
        const auto i0 = rlIndices[base + 0];
        const auto i1 = rlIndices[base + 1];
        const auto i2 = rlIndices[base + 2];
        joltTriangles.emplace_back(i0, i1, i2);
    }
}

void res::AssembleStaticCompoundShape(JPH::StaticCompoundShapeSettings& shapeSettings, const ModelComponent& modelComponent)
{
    for (int meshIndex = 0; meshIndex < modelComponent.model.meshCount; ++meshIndex)
    {
        const auto& mesh = modelComponent.model.meshes[meshIndex];

        JPH::VertexList joltVertices{};
        PopulateJoltVertices(mesh.vertices, mesh.vertexCount, joltVertices);

        JPH::IndexedTriangleList joltTriangleList{};
        PopulateJoltTriangles(mesh.indices, mesh.triangleCount, joltTriangleList);

        JPH::MeshShapeSettings meshShapeSettings{
            std::move(joltVertices), std::move(joltTriangleList)
        };

        JPH::ShapeSettings::ShapeResult shapeResult = meshShapeSettings.Create();
        if (shapeResult.HasError())
        {
            spdlog::error("Error Creating shape!", shapeResult.GetError());
            continue;
        }
        shapeSettings.AddShape(JPH::Vec3::sZero(), JPH::Quat::sIdentity(),
                               shapeResult.Get());
    }
}

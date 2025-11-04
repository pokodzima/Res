#include "JoltUtils.h"

#include "RenderComponents.h"

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>
#include <spdlog/spdlog.h>

#include <utility>


bool res::ObjectLayerPairFilterImpl::ShouldCollide(JPH::ObjectLayer layer1, JPH::ObjectLayer layer2) const
{
    switch (layer1)
    {
    case PhysicsObjectLayers::NON_MOVING:
        return layer2 == PhysicsObjectLayers::MOVING;
    case PhysicsObjectLayers::MOVING:
        return true;
    default:
        JPH_ASSERT(false);
        return false;
    }
}

bool res::ObjectVsBroadPhaseLayerFilterImpl::ShouldCollide(JPH::ObjectLayer layer1,
                                                           JPH::BroadPhaseLayer layer2) const
{
    switch (layer1)
    {
    case PhysicsObjectLayers::NON_MOVING:
        return layer2 == BroadPhaseLayers::MOVING;
    case PhysicsObjectLayers::MOVING:
        return true;
    default:
        JPH_ASSERT(false);
        return false;
    }
}

void res::PopulateJoltVertices(const float* raylib_vertices, const int vertex_count, JPH::VertexList& jolt_vertices)
{
    if (!raylib_vertices || vertex_count <= 0)
    {
        spdlog::error("Error populating vertices");
        return;
    }

    jolt_vertices.reserve(jolt_vertices.size() + static_cast<size_t>(vertex_count));

    constexpr int kVertexComponents = 3;
    for (int i = 0; i < vertex_count; ++i)
    {
        const int base = i * kVertexComponents;
        JPH::Float3 vertex{raylib_vertices[base + 0], raylib_vertices[base + 1], raylib_vertices[base + 2]};
        jolt_vertices.push_back(vertex);
    }
}

void res::PopulateJoltTriangles(const unsigned short* raylib_indices, const int triangle_count,
                                JPH::IndexedTriangleList& jolt_triangles)
{
    if (!raylib_indices || triangle_count <= 0)
    {
        spdlog::error("Error populating triangles");
        return;
    }

    jolt_triangles.reserve(jolt_triangles.size() + static_cast<size_t>(triangle_count));
    constexpr int kIndicesPerTriangle = 3;
    for (int i = 0; i < triangle_count; ++i)
    {
        const int base = i * kIndicesPerTriangle;
        const auto index0 = raylib_indices[base + 0];
        const auto index1 = raylib_indices[base + 1];
        const auto index2 = raylib_indices[base + 2];
        jolt_triangles.emplace_back(index0, index1, index2);
    }
}

void res::AssembleStaticCompoundShape(JPH::StaticCompoundShapeSettings& shape_settings, const ModelComponent& model_component)
{
    for (int mesh_index = 0; mesh_index < model_component.model.meshCount; ++mesh_index)
    {
        const auto& mesh = model_component.model.meshes[mesh_index];

        JPH::VertexList jolt_vertices{};
        PopulateJoltVertices(mesh.vertices, mesh.vertexCount, jolt_vertices);

        JPH::IndexedTriangleList jolt_triangle_list{};
        PopulateJoltTriangles(mesh.indices, mesh.triangleCount, jolt_triangle_list);

        JPH::MeshShapeSettings mesh_shape_settings{
            std::move(jolt_vertices), std::move(jolt_triangle_list)
        };

        JPH::ShapeSettings::ShapeResult shape_result = mesh_shape_settings.Create();
        if (shape_result.HasError())
        {
            spdlog::error("Error creating shape: {}", shape_result.GetError());
            continue;
        }
        shape_settings.AddShape(JPH::Vec3::sZero(), JPH::Quat::sIdentity(),
                               shape_result.Get());
    }
}

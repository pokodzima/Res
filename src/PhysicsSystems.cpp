#include "PhysicsSystems.h"

#include "PhysicsComponents.h"
#include "JoltUtils.h"
#include "MathUtils.h"
#include "RenderComponents.h"
#include "TransformComponents.h"
#include "Phases.h"

#include <flecs.h>
#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>

#include <iostream>

using namespace JPH::literals;

res::PhysicsSystems::PhysicsSystems(flecs::world& world)
{
    world.module<PhysicsSystems>();

    world.observer<sPhysicsHandle>("Initialize Physics System")
         .event(flecs::OnAdd)
         .each([](flecs::entity e, sPhysicsHandle& handle)
         {
             JPH::RegisterDefaultAllocator();

             JPH::Trace = TraceImpl;
             JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = AssertFailedImpl);

             JPH::Factory::sInstance = new JPH::Factory();

             JPH::RegisterTypes();

             handle.tempAllocator = std::make_unique<JPH::TempAllocatorImpl>(10 * 1024 * 1024);
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

    world.observer<sPhysicsHandle>("Deinitialize Physics System")
         .event(flecs::OnRemove)
         .each([](flecs::entity e, sPhysicsHandle& handle)
         {
             JPH::UnregisterTypes();
             delete JPH::Factory::sInstance;
             JPH::Factory::sInstance = nullptr;
         });

    world.observer<cModel, cPhysicsBodyID, cMatrix, cMeshCollider>("Create StaticMesh Body")
         .event(flecs::OnAdd)
         .each([&world](const cModel& modelComponent, cPhysicsBodyID& bodyIdHolder, cMatrix& matrixComponent,
                        const cMeshCollider& meshColliderComponent)
             {
                 JPH::StaticCompoundShapeSettings staticCompoundShapeSettings{};
                 AssembleStaticCompoundShape(staticCompoundShapeSettings, modelComponent);
                 JPH::ShapeSettings::ShapeResult compoundShapeResult = staticCompoundShapeSettings.Create();
                 if (compoundShapeResult.HasError())
                 {
                     std::cout << "Failed to create compound shape!" << "\n";
                 }

                 JPH::ShapeRefC meshShape = compoundShapeResult.Get();
                 auto entityPosition = GetPositionFromMatrix(matrixComponent.matrix);
                 auto entityRotation = QuaternionNormalize(QuaternionFromMatrix(matrixComponent.matrix));
                 JPH::RVec3 bodyPosition{entityPosition.x, entityPosition.y, entityPosition.z};
                 JPH::Quat bodyRotation{entityRotation.x, entityRotation.y, entityRotation.z, entityRotation.w};
                 JPH::BodyCreationSettings bodySettings{
                     meshShape, bodyPosition, bodyRotation, JPH::EMotionType::Static,
                     PhysicsObjectLayers::NON_MOVING
                 };

                 auto& handle = world.get<sPhysicsHandle>();
                 JPH::Body* body = handle.bodyInterface->CreateBody(bodySettings);
                 handle.bodyInterface->AddBody(body->GetID(), JPH::EActivation::DontActivate);
                 bodyIdHolder.bodyID = body->GetID();
             }
         );

    world.observer<cPhysicsBodyID>("Clear BodyID")
         .event(flecs::OnRemove)
         .each([&world](cPhysicsBodyID& bodyIdHolder)
         {
             if (bodyIdHolder.bodyID.IsInvalid())
             {
                 return;
             }
             auto& handle = world.get<sPhysicsHandle>();
             handle.bodyInterface->RemoveBody(bodyIdHolder.bodyID);
             handle.bodyInterface->DestroyBody(bodyIdHolder.bodyID);
         });

    world.observer<const cPhysicsBall, cPhysicsBodyID>()
         .event(flecs::OnAdd)
         .each([&world](const cPhysicsBall& body, cPhysicsBodyID& bodyIdHolder)
         {
             auto& handle = world.get<sPhysicsHandle>();
             JPH::BodyCreationSettings sphere_settings(new JPH::SphereShape(0.5f),
                                                       JPH::RVec3(0.0_r, 5.0_r, 0.0_r),
                                                       JPH::Quat::sIdentity(),
                                                       JPH::EMotionType::Dynamic,
                                                       PhysicsObjectLayers::MOVING);
             sphere_settings.mRestitution = 1.0f;
             sphere_settings.mFriction = 0.0f;
             bodyIdHolder.bodyID = handle.bodyInterface->CreateAndAddBody(
                 sphere_settings, JPH::EActivation::Activate);

             // Now you can interact with the dynamic body, in this case we're going to give it a velocity.
             // (note that if we had used CreateBody then we could have set the velocity straight on the body before adding it to the physics system)
             handle.bodyInterface->SetLinearVelocity(bodyIdHolder.bodyID, JPH::Vec3(0.0f, -1.0f, 0.0f));
         });

    auto onTickPhase = world.lookup(OnTickPhaseName.data());

    world.system("Run Physics Simulation")
         .kind(onTickPhase)
         .run([&world](flecs::iter& it)
         {
             auto& handle = world.get<sPhysicsHandle>();
             handle.physicsSystem->Update(1.0f / 60.0f, 1, handle.tempAllocator.get(),
                                          handle.jobSystem.get());
         });

    world.system<const cPhysicsBodyID, cMatrix>("Move Physics Body")
         .kind(onTickPhase)
         .each([&world](const cPhysicsBodyID& pb, cMatrix& matrix)
         {
             if (pb.bodyID.IsInvalid())
             {
                 std::cout << "body id is invalid" << "\n";
                 return;
             }
             auto& handle = world.get<sPhysicsHandle>();
             auto pos = handle.bodyInterface->GetCenterOfMassPosition(pb.bodyID);
             matrix.matrix = MatrixTranslate(pos.GetX(), pos.GetY(), pos.GetZ());
         });
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

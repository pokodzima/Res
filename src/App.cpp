#include "App.h"

#include "DebugSystems.h"
#include "RenderComponents.h"
#include "RenderSystems.h"
#include "TransformComponents.h"
#include "TransformSystems.h"
#include "Phases.h"
#include "PhysicsSystems.h"
#include "PhysicsComponents.h"

#include <flecs.h>
#include <raylib.h>
#include <raymath.h>


namespace res
{
    App::App()
        : mWindow{1920 / 2, 1080 / 2, 60, "Res"}
    {
        CreatePhases(mWorld);

        mWorld.import<RenderSystems>();
        mWorld.import<DebugSystems>();
        mWorld.import<TransformSystems>();
        mWorld.import<PhysicsSystems>();
        mWorld.import<PhysicsComponents>();

        auto roomEntity = mWorld.entity("Room");
        roomEntity.add<cRenderable>().set<cModel>({LoadModel("assets/roomV2.glb")});
        roomEntity.add<cMeshCollider>().add<cPhysicsBodyID>().set<cMatrix>({MatrixIdentity()});


        auto cameraEntity = mWorld.entity("Camera");
        auto cameraTransform = MatrixInvert(MatrixLookAt(Vector3{5.6, 3.3, -5.3}, Vector3{0.3, 0.6, -2},
                                                         Vector3{0, 1, 0}));
        cameraEntity.set<cMatrix>({cameraTransform});
        cameraEntity.set<cCamera>({
            Vector3{5.6, 3.3, -5.3},
            Vector3{0.3, 0.6, -2},
            Vector3{0, 1, 0},
            60,
            CAMERA_PERSPECTIVE
        });

        auto sphere = mWorld.entity("Ball");
        sphere.add<cPhysicsBall>();
        sphere.add<cMatrix>();
        sphere.add<cRenderable>();
        sphere.add<cRlSphere>();
        sphere.add<cPhysicsBodyID>();
    }

    void App::Run()
    {
        while (!WindowShouldClose())
        {
            mWorld.progress();
        }
    }
}

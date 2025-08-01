#include "App.h"

#include "DebugSystems.h"
#include "RenderComponents.h"
#include "RenderSystems.h"
#include "TransformComponents.h"
#include "Phases.h"

#include "raylib.h"
#include "raymath.h"


namespace res
{
    App::App()
        : mWindow{1920 / 2, 1080 / 2, 60, "Res"}
    {
        mWorld.import<flecs::stats>();
        mWorld.import<Phases>();
        mWorld.import<RenderSystems>();
        mWorld.import<DebugSystems>();
        mWorld.set<flecs::Rest>({});


        auto roomEntity = mWorld.entity("Room");
        roomEntity.add<cRenderable>().set<cModel>({LoadModel("assets/room.glb")});

        auto cameraEntity = mWorld.entity("Camera");
        auto cameraStartMatrix = MatrixLookAt(Vector3{5.6, 3.3, -5.3}, Vector3{0.3, 0.6, -2}, Vector3{0, 1, 0});
        auto cameraRotation = QuaternionFromMatrix(cameraStartMatrix);
        cameraEntity.set<cPosition>({5.6, 3.3, -5.3});
        cameraEntity.set<cRotation>({cameraRotation.x, cameraRotation.y, cameraRotation.z, cameraRotation.w});
        cameraEntity.set<cScale>({1, 1, 1});
        cameraEntity.set<cMatrix>({cameraStartMatrix});
        cameraEntity.set<cCamera>({
            Vector3{5.6, 3.3, -5.3}, Vector3{0.3, 0.6, -2}, Vector3{0, 1, 0}, 60, CAMERA_PERSPECTIVE
        });
    }

    void App::Run()
    {
        while (!WindowShouldClose())
        {
            // ReSharper disable once CppExpressionWithoutSideEffects
            mWorld.progress();
        }
    }
}

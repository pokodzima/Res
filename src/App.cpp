#include "App.h"

#include "DebugSystems.h"
#include "RenderComponents.h"
#include "RenderSystems.h"
#include "TransformComponents.h"
#include "TransformSystems.h"
#include "Phases.h"

#include "raylib.h"
#include "raymath.h"


namespace res
{
    App::App()
        : mWindow{1920 / 2, 1080 / 2, 60, "Res"}
    {
        mWorld.import<Phases>();
        mWorld.import<RenderSystems>();
        mWorld.import<DebugSystems>();
        mWorld.import<TransformSystems>();

        auto roomEntity = mWorld.entity("Room");
        roomEntity.add<cRenderable>().set<cModel>({LoadModel("assets/room.glb")});


        auto cameraEntity = mWorld.entity("Camera");
        auto cameraTransform = MatrixInvert(MatrixLookAt(Vector3{5.6, 3.3, -5.3}, Vector3{0.3, 0.6, -2},
                                                         Vector3{0, 1, 0}));
        cameraEntity.set<cMatrix>({cameraTransform});
        cameraEntity.set<cCamera>({
            Vector3Zero(),
            Vector3Zero(),
            Vector3Zero(),
            60,
            CAMERA_PERSPECTIVE
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

#include "App.h"
#include "raylib.h"
#include "raymath.h"
#include "rlImGui.h"
#include "imgui.h"
#include "RenderComponents.h"

namespace res
{
    App::App()
        :mWindow{1920/2,1080/2,60,"Res"}
    {

        mRenderModelSystem = mWorld.system<const cRenderable, const cModel>()
                                   .each([](const cRenderable& r, const cModel& mh)
                                   {
                                       DrawModel(mh.model, Vector3Zero(), 0.01f, WHITE);
                                   });

        auto roomEntity = mWorld.entity("Room");

        roomEntity.add<cRenderable>().set<cModel>({LoadModel("assets/room.glb")});
    }

    void App::Run()
    {
        Camera3D camera{Vector3{5.6, 3.3, -5.3}, Vector3{0.3, 0.6, -2}, Vector3{0, 1, 0}, 60, CAMERA_PERSPECTIVE};

        while (!WindowShouldClose())
        {
            //UpdateCamera(&camera, CAMERA_FREE);

            BeginDrawing();

            ClearBackground(WHITE);

            BeginMode3D(camera);

            mRenderModelSystem.run();

            EndMode3D();

            rlImGuiBegin();

            ImGui::Text("%f,%f,%f", camera.position.x, camera.position.y, camera.position.z);
            rlImGuiEnd();

            EndDrawing();
        }
    }
}

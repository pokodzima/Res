#include "App.h"

#include "HelperComponents.h"
#include "raylib.h"
#include "raymath.h"
#include "rlImGui.h"
#include "imgui.h"
#include "RenderComponents.h"
#include "TransformComponents.h"

namespace res
{
    App::App()
        : mWindow{1920 / 2, 1080 / 2, 60, "Res"}
    {
        flecs::entity OnTick = mWorld.entity()
                                     .add(flecs::Phase)
                                     .depends_on(flecs::OnUpdate);

        flecs::entity OnPreRender = mWorld.entity()
                                          .add(flecs::Phase)
                                          .depends_on(OnTick);

        flecs::entity OnRender = mWorld.entity()
                                       .add(flecs::Phase)
                                       .depends_on(OnPreRender);

        flecs::entity OnPreRender3D = mWorld.entity()
                                            .add(flecs::Phase)
                                            .depends_on(OnRender);

        flecs::entity OnRender3D = mWorld.entity()
                                         .add(flecs::Phase)
                                         .depends_on(OnPreRender3D);

        flecs::entity OnPostRender3D = mWorld.entity()
                                             .add(flecs::Phase)
                                             .depends_on(OnRender3D);

        flecs::entity OnRender2D = mWorld.entity()
                                         .add(flecs::Phase)
                                         .depends_on(OnRender3D);

        flecs::entity OnPostRender = mWorld.entity()
                                           .add(flecs::Phase)
                                           .depends_on(OnRender2D);


        mWorld.system("BeginRender")
              .kind(OnPreRender)
              .run([](flecs::iter& it)
              {
                  BeginDrawing();
                  ClearBackground(WHITE);
              });

        mWorld.system<const cCamera>("BeginRender3D")
              .kind(OnPreRender3D)
              .each([](const cCamera& c)
              {
                  const auto camera = c.raylibCamera;
                  BeginMode3D(camera);
              });

        mWorld.system("EndRender3D")
              .kind(OnPostRender3D)
              .run([](flecs::iter& it)
              {
                  EndMode3D();
              });

        mWorld.system<const cCamera>("OnRenderImGUI")
              .kind(OnRender2D)
              .each([](const cCamera& c)
              {
                  rlImGuiBegin();
                  const auto camera = c.raylibCamera;
                  ImGui::Text("%f,%f,%f", camera.position.x, camera.position.y, camera.position.z);
                  rlImGuiEnd();
              });

        mWorld.system("EndRender")
              .kind(OnPostRender)
              .run([](flecs::iter& it)
              {
                  EndDrawing();
              });

        mRenderModelSystem = mWorld.system<const cRenderable, const cModel>()
                                   .kind(OnRender3D)
                                   .each([](const cRenderable& r, const cModel& mh)
                                   {
                                       DrawModel(mh.model, Vector3Zero(), 0.01f, WHITE);
                                   });


        auto roomEntity = mWorld.entity("Room");
        roomEntity.add<cRenderable>().set<cModel>({LoadModel("assets/room.glb")});

        auto cameraEntity = mWorld.entity("Camera");
        auto cameraStartMatrix = MatrixLookAt(Vector3{5.6, 3.3, -5.3}, Vector3{0.3, 0.6, -2}, Vector3{0, 1, 0});
        auto cameraRotation = QuaternionFromMatrix(cameraStartMatrix);
        cameraEntity.set<cPosition>({5.6, 3.3, -5.3});
        cameraEntity.set<cRotation>({cameraRotation.x, cameraRotation.y, cameraRotation.z});
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

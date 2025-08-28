#include "InputSystems.h"

#include "InputComponents.h"
#include "CommonComponents.h"
#include "Phases.h"

#include <flecs.h>
#include <raymath.h>

#include <cassert>


res::InputSystems::InputSystems(flecs::world& world)
{
    world.module<InputSystems>();

    auto onTickPhase = world.lookup(OnTickPhaseName.data());

    assert(onTickPhase != 0 && "OnTickPhase not found");

    world.system<MovementInputComponent, const PlayerComponent>("Populate Player Movement Input")
         .kind(onTickPhase)
         .each([](MovementInputComponent& movementInputComponent, const PlayerComponent& playerComponent)
         {
             movementInputComponent.input.x = static_cast<float>(IsKeyDown(KEY_W)) * 1.0f - static_cast<float>(
                 IsKeyDown(KEY_S)) * 1.0f;

             movementInputComponent.input.y = static_cast<float>(IsKeyDown(KEY_D)) * 1.0f - static_cast<float>(
                 IsKeyDown(KEY_A)) * 1.0f;

             movementInputComponent.input = Vector2Normalize(movementInputComponent.input);
         });
}

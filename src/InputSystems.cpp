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

    auto on_tick_phase = world.lookup(OnTickPhaseName.data());

    assert(on_tick_phase != 0 && "OnTickPhase not found");

    world.system<MovementInputComponent, const PlayerComponent>("Populate Player Movement Input")
         .kind(on_tick_phase)
         .each([](MovementInputComponent& movement_input, const PlayerComponent& player)
         {
             movement_input.input.x = static_cast<float>(IsKeyDown(KEY_W)) - static_cast<float>(
                 IsKeyDown(KEY_S));

             movement_input.input.y = static_cast<float>(IsKeyDown(KEY_D)) - static_cast<float>(
                 IsKeyDown(KEY_A));

             movement_input.input = Vector2Normalize(movement_input.input);
         });
}

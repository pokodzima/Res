#include "InputSystems.h"

#include "CommonComponents.h"
#include "InputComponents.h"
#include "Phases.h"

#include <cassert>

#include <flecs.h>
#include <raymath.h>


res::InputSystems::InputSystems(flecs::world& world)
{
    world.module<InputSystems>();

    auto on_tick_phase = world.lookup(kTickPhaseName.data());

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

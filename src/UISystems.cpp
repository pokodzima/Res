#include "UISystems.h"

#include "CommonComponents.h"
#include "Phases.h"
#include "UIComponents.h"

#include <flecs.h>
#include <raylib.h>


res::UISystems::UISystems(flecs::world& world)
{
    world.module<UISystems>();

    auto on_render_2d_phase = world.lookup(res::kRender2DPhaseName.data());

    world.system<const TextComponent, const Position2dComponent, const Renderable2dComponent, const TextElementComponent
                 , const ColorComponent>(
             "Render UI Text")
         .kind(on_render_2d_phase)
         .each([](const TextComponent& text_component, const Position2dComponent& position_component,
                  const Renderable2dComponent& renderable_component, const TextElementComponent& element_component,
                  const ColorComponent& color_component)
         {
             DrawText(text_component.text.c_str(), static_cast<int>(position_component.x), 
                      static_cast<int>(position_component.y),
                      element_component.font_size,
                      color_component.color);
         });
}

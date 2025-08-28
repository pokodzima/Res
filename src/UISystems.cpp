#include "UISystems.h"

#include "CommonComponents.h"
#include "Phases.h"
#include "UIComponents.h"

#include <flecs.h>
#include <raylib.h>


res::UISystems::UISystems(flecs::world& world)
{
    world.module<UISystems>();

    auto OnRender2DPhase = world.lookup(res::OnRender2DPhaseName.data());

    world.system<const TextComponent, const Position2dComponent, const Renderable2dComponent, const TextElementComponent
                 , const ColorComponent>(
             "Render UI Text")
         .kind(OnRender2DPhase)
         .each([](const TextComponent& textComponent, const Position2dComponent& positionComponent,
                  const Renderable2dComponent& renderableComponent, const TextElementComponent& elementComponent,
                  const ColorComponent& colorComponent)
         {
             DrawText(textComponent.textString.c_str(), (int)positionComponent.x, (int)positionComponent.y,
                      elementComponent.fontSize,
                      colorComponent.color);
         });
}

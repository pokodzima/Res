#include "UISystems.h"

#include "DataComponents.h"

#include <flecs.h>

#include "Phases.h"
#include "raylib.h"
#include "UIComponents.h"

res::UISystems::UISystems(flecs::world& world)
{
    world.module<UISystems>();

    auto OnRender2DPhase = world.lookup(res::OnRender2DPhaseName.data());

    world.system<const TextComponent, const cPosition2D, const cRenderable2D, const cTextElement, const ColorComponent>(
             "Render UI Text")
         .kind(OnRender2DPhase)
         .each([](const TextComponent& textComponent, const cPosition2D& positionComponent,
                  const cRenderable2D& renderableComponent, const cTextElement& elementComponent,
                  const ColorComponent& colorComponent)
         {
             DrawText(textComponent.textString.c_str(), (int)positionComponent.x, (int)positionComponent.y,
                      elementComponent.fontSize,
                      colorComponent.color);
         });
}

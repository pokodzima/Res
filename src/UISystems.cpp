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

    world.system<const cText, const cPosition2D, const cRenderable2D, const cTextElement, const cColor>(
             "Render UI Text")
         .kind(OnRender2DPhase)
         .each([](const cText& textComponent, const cPosition2D& positionComponent,
                  const cRenderable2D& renderableComponent, const cTextElement& elementComponent,
                  const cColor& colorComponent)
         {
             DrawText(textComponent.textString.c_str(), (int)positionComponent.x, (int)positionComponent.y,
                      elementComponent.fontSize,
                      colorComponent.color);
         });
}

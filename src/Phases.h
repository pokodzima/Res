#pragma once

#include <string>

namespace flecs
{
    struct world;
}

namespace res
{
    static constexpr std::string_view OnBeginPhaseName = "OnBeginPhase";
    static constexpr std::string_view OnTickPhaseName = "OnTickPhase";
    static constexpr std::string_view OnPostTickPhaseName = "OnPostTickPhase";
    static constexpr std::string_view OnPreRenderPhaseName = "OnPreRenderPhase";
    static constexpr std::string_view OnRenderPhaseName = "OnRenderPhase";
    static constexpr std::string_view OnPreRender3DPhaseName = "OnPreRender3DPhase";
    static constexpr std::string_view OnRender3DPhaseName = "OnRender3DPhase";
    static constexpr std::string_view OnPostRender3DPhaseName = "OnPostRender3DPhase";
    static constexpr std::string_view OnRender2DPhaseName = "OnRender2DPhase";
    static constexpr std::string_view OnPostRenderPhaseName = "OnPostRenderPhase";


    void CreatePhases(flecs::world& world);
}

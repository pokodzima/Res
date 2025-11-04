#pragma once

#include <string>

namespace flecs
{
    struct world;
}

namespace res
{
    static constexpr std::string_view kBeginPhaseName = "OnBeginPhase";
    static constexpr std::string_view kTickPhaseName = "OnTickPhase";
    static constexpr std::string_view kPostTickPhaseName = "OnPostTickPhase";
    static constexpr std::string_view kPreRenderPhaseName = "OnPreRenderPhase";
    static constexpr std::string_view kRenderPhaseName = "OnRenderPhase";
    static constexpr std::string_view kPreRender3DPhaseName = "OnPreRender3DPhase";
    static constexpr std::string_view kRender3DPhaseName = "OnRender3DPhase";
    static constexpr std::string_view kPostRender3DPhaseName = "OnPostRender3DPhase";
    static constexpr std::string_view kRender2DPhaseName = "OnRender2DPhase";
    static constexpr std::string_view kPostRenderPhaseName = "OnPostRenderPhase";


    void CreatePhases(flecs::world& world);
}

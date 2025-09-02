#pragma once
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <d3d9.h>
#include <d3d11.h>
#include "imgui/imgui.h"

#include <plugin.h>

#define MOD_NAME "Carcols Editor"

enum class eRenderer {
    Dx8,
    Dx9,
    Dx10,
    Dx11,
    Unknown,
};

extern eRenderer gRenderer;
extern bool gEditorVisible;
extern void* gD3dDevice;

inline ImVec2 CalcSize(short count, bool spacing) {
    if (count == 1) {
        spacing = false;
    }

    float x = ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x - ImGui::GetStyle().ItemSpacing.x * (spacing ? count : 1);
    return ImVec2(x / count, ImGui::GetFrameHeight() * 1.15f);
}

inline ImVec2 CalcSizeFrame(const char* text) {
    return ImVec2(ImGui::CalcTextSize(text).x + 2 * ImGui::GetStyle().ItemSpacing.x, ImGui::GetFrameHeight());
}
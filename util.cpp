#include "util.h"

ImVec2 Util::CalcSize(short count, bool spacing) {
    if (count == 1) {
        spacing = false;
    }

    float x = ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x - ImGui::GetStyle().ItemSpacing.x * (spacing ? count : 1);
    return ImVec2(x / count, ImGui::GetFrameHeight() * 1.15f);
}

ImVec2 Util::CalcFrameSize(const char* text) {
    return ImVec2(ImGui::CalcTextSize(text).x + 2 * ImGui::GetStyle().ItemSpacing.x, ImGui::GetFrameHeight());
}
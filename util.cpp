#include "util.h"
#include "defines.h"
#include <fstream>
#include <map>
#include <vector>
#include <filesystem>

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

bool Util::ColorCombo(const char* label, int* pOutId) {
    bool changed = false;
    const int id = *pOutId;
    const CRGBA c = CVehicleModelInfo::ms_vehicleColourTable[id];
    const ImVec4 v = ImVec4(c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, 1.0f); // Inlined ToVec4

    ImGui::PushID(label);
    const std::string preview = std::string("ID ") + std::to_string(id);

    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x / 1.5f);
    if (ImGui::BeginCombo("##combo", preview.c_str())) {
        ImGuiListClipper clipper;
        clipper.Begin(MAX_COLORS);
        while (clipper.Step()) {
            for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i) {
                ImGui::PushID(i);
                const CRGBA ci = CVehicleModelInfo::ms_vehicleColourTable[i];
                ImGui::ColorButton("##c", ImVec4(ci.r / 255.0f, ci.g / 255.0f, ci.b / 255.0f, 1.0f), ImGuiColorEditFlags_NoTooltip, ImVec2(20, 20)); // Inlined ToVec4
                ImGui::SameLine();
                bool sel = (id == i);
                if (ImGui::Selectable(std::to_string(i).c_str(), sel)) {
                    *pOutId = i;
                    changed = true;
                }
                ImGui::PopID();
            }
        }
        ImGui::EndCombo();
    }
    ImGui::SameLine();
    ImGui::ColorButton("##prev", v, ImGuiColorEditFlags_NoTooltip, ImVec2(28, 18));
    ImGui::PopID();
    return changed;
}

extern std::map<int, std::string> store;
void Util::GenerateCarcol() {
    const std::string carcolPath = GAME_PATH("data/carcols.dat");
    const std::string backupPath = carcolPath + ".backup";

    if (std::filesystem::exists(carcolPath)) {
        std::filesystem::rename(carcolPath, backupPath);
    }

    std::ofstream out(carcolPath, std::ios::trunc);
    out << "# Carcols.dat file generated using CarcolsEditor by Grinch_\n\ncol\n";

    for (int i = 0; i < MAX_COLORS; i++) {
        const CRGBA col = CVehicleModelInfo::ms_vehicleColourTable[i];
        out << (int)col.r << ", " << (int)col.g << ", " << (int)col.b << "\n";
    }

    out << "end\n\ncar4\n";

    for (int id = 0; id <= MAX_VEHICLE_ID; id++) {
        CBaseModelInfo* base = CModelInfo::GetModelInfo(id);
        if (base && (int)base > 0xFFFF && base->GetModelType() == MODEL_INFO_VEHICLE) {
            auto* pInfo = reinterpret_cast<CVehicleModelInfo*>(base);
            auto it = store.find(id);
            const std::string& name = (it != store.end()) ? it->second : std::to_string(id);

            out << name << ", ";
            for (int i = 0; i < pInfo->m_nNumColorVariations; ++i) {
                out << (int)pInfo->m_anPrimaryColors[i] << ", "
                    << (int)pInfo->m_anSecondaryColors[i] << ", "
                    << (int)pInfo->m_anTertiaryColors[i] << ", "
                    << (int)pInfo->m_anQuaternaryColors[i];
                if (i != pInfo->m_nNumColorVariations - 1) out << ", ";
            }
            out << "\n";
        }
    }

    out << "end" << std::endl;
}
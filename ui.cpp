#include "pch.h"
#include <CVehicleModelInfo.h>
#include "d3dhook.h"
#include <fstream>

int MAX_COLORS = 128;
int MAX_COLORS_PER_VEHICLE = 8;
int MAX_VEHICLE_ID = 20000;

void ShowColorPaletteTab() {
    ImGui::BeginChild(__FUNCTION__);
    ImGui::Columns(2, nullptr, false);
    for (int i = 0; i < MAX_COLORS; ++i) {
        CRGBA& color = CVehicleModelInfo::ms_vehicleColourTable[i];
        float col[3] = { color.r / 255.0f, color.g / 255.0f, color.b / 255.0f };

        ImGui::SetNextItemWidth(CalcSize(2, true).x);
        if (ImGui::ColorEdit3(std::format("Color {}", i).c_str(), col, ImGuiColorEditFlags_NoInputs)) {
            color.r = static_cast<unsigned char>(col[0] * 255);
            color.g = static_cast<unsigned char>(col[1] * 255);
            color.b = static_cast<unsigned char>(col[2] * 255);
        }
        ImGui::NextColumn();
    }

    ImGui::Columns(1);
    ImGui::EndChild();
}

void ShowCurrentVehicleColorsTab(CVehicleModelInfo* pModelInfo) {
    ImGui::NewLine();
    ImGui::Text("Available Variations");
    ImGui::Separator();

    ImGui::Columns(3, NULL, false);
    ImGui::Text("Primary");
    ImGui::NextColumn();
    ImGui::Text("Secondary");
    ImGui::NextColumn();
    ImGui::Text("Actions");
    ImGui::Columns(1);

    ImGui::Columns(3, NULL, false);

    for (size_t i = 0; i < pModelInfo->m_nNumColorVariations; i++) {
        unsigned char& primaryId = pModelInfo->m_anPrimaryColors[i];
        unsigned char& secondaryId = pModelInfo->m_anSecondaryColors[i];

        CRGBA primaryColor = CVehicleModelInfo::ms_vehicleColourTable[primaryId];
        CRGBA secondaryColor = CVehicleModelInfo::ms_vehicleColourTable[secondaryId];

        ImVec4 primaryVec = { primaryColor.r / 255.0f, primaryColor.g / 255.0f, primaryColor.b / 255.0f, 1.0f };
        ImVec4 secondaryVec = { secondaryColor.r / 255.0f, secondaryColor.g / 255.0f, secondaryColor.b / 255.0f, 1.0f };

        // Primary
        ImGui::ColorButton(std::format("##Primary{}", i).c_str(), primaryVec, ImGuiColorEditFlags_NoTooltip, ImVec2(40, 20));
        ImGui::SameLine();
        ImGui::Text("ID: %d", primaryId);

        // Secondary
        ImGui::NextColumn();
        ImGui::ColorButton(std::format("##Secondary{}", i).c_str(), secondaryVec, ImGuiColorEditFlags_NoTooltip, ImVec2(40, 20));
        ImGui::SameLine();
        ImGui::Text("ID: %d", secondaryId);

        // Actions
        ImGui::NextColumn();
        if (ImGui::Button(std::format("Delete##{}", i).c_str(), ImVec2(60, 20))) {
            for (int j = i; j < pModelInfo->m_nNumColorVariations - 1; ++j) {
                pModelInfo->m_anPrimaryColors[j] = pModelInfo->m_anPrimaryColors[j + 1];
                pModelInfo->m_anSecondaryColors[j] = pModelInfo->m_anSecondaryColors[j + 1];
            }
            --pModelInfo->m_nNumColorVariations;
            --i;
        }
        ImGui::NextColumn();
    }

    ImGui::Columns(1);

    ImGui::Separator();

    // Add new entry
    static int newPrimary = 0;
    static int newSecondary = 0;

    ImGui::Text("Add New Variation");
    ImGui::Spacing();

    if (pModelInfo->m_nNumColorVariations < MAX_COLORS_PER_VEHICLE) {
        if (ImGui::BeginCombo("Primary", std::format("ID {}##PrimaryLabel", newPrimary).c_str())) {
            for (int i = 0; i < MAX_COLORS; ++i) {
                CRGBA color = CVehicleModelInfo::ms_vehicleColourTable[i];
                ImVec4 colorVec = { color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, 1.0f };

                ImGui::PushID(i);
                ImGui::ColorButton("##ColorPreview", colorVec, ImGuiColorEditFlags_NoTooltip, ImVec2(20, 20));
                ImGui::SameLine();
                if (ImGui::Selectable(std::format("ID {}", i).c_str(), newPrimary == i)) {
                    newPrimary = i;
                }
                ImGui::PopID();
            }
            ImGui::EndCombo();
        }

        if (ImGui::BeginCombo("Secondary", std::format("ID {}##SecondaryLabel", newSecondary).c_str())) {
            for (int i = 0; i < MAX_COLORS; ++i) {
                CRGBA color = CVehicleModelInfo::ms_vehicleColourTable[i];
                ImVec4 colorVec = { color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, 1.0f };

                ImGui::PushID(1000 + i);
                ImGui::ColorButton("##ColorPreview", colorVec, ImGuiColorEditFlags_NoTooltip, ImVec2(20, 20));
                ImGui::SameLine();
                if (ImGui::Selectable(std::format("ID {}", i).c_str(), newSecondary == i)) {
                    newSecondary = i;
                }
                ImGui::PopID();
            }
            ImGui::EndCombo();
        }

        ImGui::Spacing();

        if (ImGui::Button("Add Color Variation")) {
            int idx = pModelInfo->m_nNumColorVariations;
            pModelInfo->m_anPrimaryColors[idx] = static_cast<unsigned char>(newPrimary);
            pModelInfo->m_anSecondaryColors[idx] = static_cast<unsigned char>(newSecondary);
            ++pModelInfo->m_nNumColorVariations;
        }
    }
    else {
        ImGui::Text("Color slots full");
    }
}
void GenerateCarcol() {
    std::ofstream out(GAME_PATH("carcols.dat"), std::ios::trunc);
    out << "# Carcols.dat file generated using CarcolsEditor by Grinch_\n\ncol\n";

    for (int i = 0; i < MAX_COLORS; i++) {
        CRGBA col = CVehicleModelInfo::ms_vehicleColourTable[i];
        out << (int)col.r << ", " << (int)col.g << ", " << (int)col.b << "\n";
    }
    out << "end\ncar\n";

    for (int id = 0; id <= MAX_VEHICLE_ID; id++) {
        CBaseModelInfo* modelInfo = CModelInfo::GetModelInfo(id);
        if (modelInfo && (int)modelInfo > 0xFFFF && modelInfo->GetModelType() == MODEL_INFO_VEHICLE) {
            CVehicleModelInfo* pInfo = reinterpret_cast<CVehicleModelInfo*>(modelInfo);
            const char* name = (const char*)CModelInfo::GetModelInfo(id) + 0x32;

            if (name && std::isprint(name[0])) {
                out << name << ", ";
                for (int i = 0; i < pInfo->m_nNumColorVariations; i++) {
                    out << (int)pInfo->m_anPrimaryColors[i] << ", " << (int)pInfo->m_anSecondaryColors[i] << ", ";
                }
                out << "\n";
            }
        }
    }
    out << "end" << std::endl;
}

void ShowSettingsTab() {
    ImGui::InputInt("Carcol colors", &MAX_COLORS);
    ImGui::InputInt("Colors per vehicle", &MAX_COLORS_PER_VEHICLE);
    ImGui::InputInt("Max Veh ID", &MAX_VEHICLE_ID);
}

void CarcolsEditorUI() {
    if (plugin::KeyPressed(VK_F6)) {
        gEditorVisible = !gEditorVisible;
    }

    D3dHook::SetMouseState(gEditorVisible);

    if (!gEditorVisible || !ImGui::Begin(MOD_NAME"by Grinch_", &gEditorVisible, ImGuiWindowFlags_NoCollapse))
        return;

    ImVec2 sz = CalcSize(2, true);
    if (ImGui::Button("Reload carcols.dat", sz)) {
        plugin::Call<0x5B6890>();
    }
    ImGui::SameLine();
    if (ImGui::Button("Generate carcols.dat", sz)) {
        GenerateCarcol();
    }
    ImGui::NewLine();
    if (ImGui::BeginTabBar("MainTab")) {
        // Palette Tab
        if (ImGui::BeginTabItem("Palette")) {
            ShowColorPaletteTab();
            ImGui::EndTabItem();
        }

        // Current Vehicle Tab
        CVehicle* pVeh = FindPlayerVehicle();
        if (pVeh) {
            CVehicleModelInfo* pModelInfo = static_cast<CVehicleModelInfo*>(CModelInfo::GetModelInfo(pVeh->m_nModelIndex));
            if (pModelInfo && ImGui::BeginTabItem("CurrentVehicle")) {
                ShowCurrentVehicleColorsTab(pModelInfo);
                ImGui::EndTabItem();
            }
        }

        if (ImGui::BeginTabItem("Settings")) {
            ShowSettingsTab();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

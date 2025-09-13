#include "pch.h"
#include <CVehicleModelInfo.h>
#include "renderhook.h"
#include "util.h"
#include <imgui/imgui_internal.h>
#include "defines.h"
#include <CHud.h>

extern bool gEditorVisible;

static void ShowColorPaletteTab() {
    ImGui::Spacing();
    ImGui::BeginChild("PaletteChild");

    if (ImGui::BeginTable("ColorTable", 4,
        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
        ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY,
        ImVec2(0, 0)))
    {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 70.0f);
        ImGui::TableSetupColumn("Color", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 70.0f);
        ImGui::TableSetupColumn("Color", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();

        for (int i = 0; i < MAX_COLORS; i += 2) {
            ImGui::TableNextRow();

            CRGBA& c1 = CVehicleModelInfo::ms_vehicleColourTable[i];
            float col1[3] = { c1.r / 255.0f, c1.g / 255.0f, c1.b / 255.0f };

            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Col %03d", i);

            ImGui::TableSetColumnIndex(1);
            ImGui::PushID(i);
            if (ImGui::ColorEdit3("##Color", col1, ImGuiColorEditFlags_NoInputs)) {
                c1.r = static_cast<unsigned char>(col1[0] * 255.0f);
                c1.g = static_cast<unsigned char>(col1[1] * 255.0f);
                c1.b = static_cast<unsigned char>(col1[2] * 255.0f);
            }
            ImGui::PopID();

            if (i + 1 < MAX_COLORS) {
                CRGBA& c2 = CVehicleModelInfo::ms_vehicleColourTable[i + 1];
                float col2[3] = { c2.r / 255.0f, c2.g / 255.0f, c2.b / 255.0f };

                ImGui::TableSetColumnIndex(2);
                ImGui::Text("Col %03d", i + 1);

                ImGui::TableSetColumnIndex(3);
                ImGui::PushID(i + 1);
                if (ImGui::ColorEdit3("##Color", col2, ImGuiColorEditFlags_NoInputs)) {
                    c2.r = static_cast<unsigned char>(col2[0] * 255.0f);
                    c2.g = static_cast<unsigned char>(col2[1] * 255.0f);
                    c2.b = static_cast<unsigned char>(col2[2] * 255.0f);
                }
                ImGui::PopID();
            }
        }

        ImGui::EndTable();
    }

    ImGui::EndChild();
}


static void ShowCurrentVehicleColorsTab(CVehicleModelInfo* pModelInfo) {
    ImVec2 btnSz = Util::CalcSize(2, false);
    ImGui::Spacing();

    if (ImGui::Button("Add New", btnSz)) {
        if (pModelInfo->m_nNumColorVariations < MAX_COLORS_PER_VEHICLE) {
            const int idx = pModelInfo->m_nNumColorVariations++;
            pModelInfo->m_anPrimaryColors[idx] = 0;
            pModelInfo->m_anSecondaryColors[idx] = 0;
            pModelInfo->m_anTertiaryColors[idx] = 0;
            pModelInfo->m_anQuaternaryColors[idx] = 0;
        }
        else {
            CHud::SetHelpMessage("Limit reached", false, false, false);
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Remove all", btnSz)) {
        pModelInfo->m_nNumColorVariations = 0;
    }

    ImGui::Spacing();
    ImGui::Text("Used Variations: %d/%d", pModelInfo->m_nNumColorVariations, MAX_COLORS_PER_VEHICLE);
    ImGui::Spacing();

    ImGui::BeginChild("CC");
    if (ImGui::BeginTable("VehicleColorsTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 40.0f);
        ImGui::TableSetupColumn("Primary / Secondary");
        ImGui::TableSetupColumn("Tertiary / Quaternary");
        ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 50.0f);
        ImGui::TableHeadersRow();

        for (int i = 0; i < pModelInfo->m_nNumColorVariations; ++i) {
            ImGui::PushID(i);
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%d", i);

            ImGui::TableSetColumnIndex(1);
            auto drawStackedSlot = [&](const char* label, unsigned char* pId) {
                int id = *pId;
                Util::ColorCombo(label, &id);
                *pId = static_cast<unsigned char>(id);
                };
            drawStackedSlot(("P##" + std::to_string(i)).c_str(), &pModelInfo->m_anPrimaryColors[i]);
            drawStackedSlot(("S##" + std::to_string(i)).c_str(), &pModelInfo->m_anSecondaryColors[i]);

            ImGui::TableSetColumnIndex(2);
            drawStackedSlot(("T##" + std::to_string(i)).c_str(), &pModelInfo->m_anTertiaryColors[i]);
            drawStackedSlot(("Q##" + std::to_string(i)).c_str(), &pModelInfo->m_anQuaternaryColors[i]);

            ImGui::TableSetColumnIndex(3);
            if (ImGui::Button("Del")) {
                for (int j = i; j < pModelInfo->m_nNumColorVariations - 1; ++j) {
                    pModelInfo->m_anPrimaryColors[j] = pModelInfo->m_anPrimaryColors[j + 1];
                    pModelInfo->m_anSecondaryColors[j] = pModelInfo->m_anSecondaryColors[j + 1];
                    pModelInfo->m_anTertiaryColors[j] = pModelInfo->m_anTertiaryColors[j + 1];
                    pModelInfo->m_anQuaternaryColors[j] = pModelInfo->m_anQuaternaryColors[j + 1];
                }
                --pModelInfo->m_nNumColorVariations;
                --i;
            }

            ImGui::PopID();
        }
        ImGui::EndTable();
    }
    ImGui::EndChild();
}

static void ShowSettingsTab() {
    ImGui::Spacing();
    
    ImGui::BeginChild("SettingsPanel", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysUseWindowPadding);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 8));
    ImGui::Columns(2, nullptr, false);

    ImGui::Text("Palette size:");
    ImGui::NextColumn();
    ImGui::SetNextItemWidth(-1);
    ImGui::InputInt("##PaletteSize", &MAX_COLORS);
    ImGui::NextColumn();

    ImGui::Text("Max variations per vehicle:");
    ImGui::NextColumn();
    ImGui::SetNextItemWidth(-1);
    ImGui::InputInt("##MaxVariations", &MAX_COLORS_PER_VEHICLE);
    ImGui::NextColumn();

    ImGui::Text("Max vehicle ID:");
    ImGui::NextColumn();
    ImGui::SetNextItemWidth(-1);
    ImGui::InputInt("##MaxVehicleID", &MAX_VEHICLE_ID);

    ImGui::Columns(1);
    ImGui::PopStyleVar();

    ImGui::EndChild();
}


void CarcolsEditorUI() {
    RenderHook::SetCursorVisible(gEditorVisible);

    ImGui::SetNextWindowSize(ImVec2(450, 800), ImGuiCond_Once);
    if (!gEditorVisible || !ImGui::Begin(MOD_NAME " by Grinch_", &gEditorVisible, ImGuiWindowFlags_NoCollapse)) {
        return;
    }

    if (ImGui::Button("Reload carcols.dat", Util::CalcSize(2))) {
        plugin::Call<0x5B6890>();
    }
    ImGui::SameLine();
    if (ImGui::Button("Generate carcols.dat", Util::CalcSize(2))) {
        Util::GenerateCarcol();
    }
    ImGui::Spacing();

    if (ImGui::BeginTabBar("MainTab", ImGuiTabBarFlags_Reorderable)) {
        if (ImGui::BeginTabItem("Color Palette")) {
            ShowColorPaletteTab();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Current Vehicle")) {
            CVehicle* pVeh = FindPlayerVehicle();
            if (pVeh) {
                auto* pModelInfo = static_cast<CVehicleModelInfo*>(CModelInfo::GetModelInfo(pVeh->m_nModelIndex));
                ShowCurrentVehicleColorsTab(pModelInfo);
            }
            else {
                ImGui::Text("Player NOT inside a vehicle.");
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Settings")) {
            ShowSettingsTab();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::End();
}

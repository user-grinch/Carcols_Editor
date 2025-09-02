#include "pch.h"
#include "d3dhook.h"

extern void CarcolsEditorUI();

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        uint gameVer = plugin::GetGameVersion();

        if (gameVer == GAME_10US_HOODLUM || gameVer == GAME_10US_COMPACT) {
            plugin::Events::initGameEvent += []() {
                D3dHook::Init(CarcolsEditorUI);
            };
        }
        else {
            MessageBox(nullptr, "Unknown game version. GTA SA v1.0 US Hoodlum or Compact is required.", MOD_NAME, MB_ICONERROR);
        }
    }

    return TRUE;
}


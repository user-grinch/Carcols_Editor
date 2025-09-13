#include "pch.h"
#include "renderhook.h"
#include <map>

extern void CarcolsEditorUI();

std::map<int, std::string> store;

using sscanf_t = int(__cdecl*)(const char*, const char*, ...);
sscanf_t ogsscanf;
int __cdecl sscanfHook(const char* line, const char* fmt,
    int& model, char* modelName, char* txdName, char* v16, char* vehicleName,
    char* a2, char* v25, char* v18, short& v20, unsigned char& v22,
    unsigned int& v21, int& v13, float& v15, float& v14, int& v19)
{
    int result = ogsscanf(
        line, "%d %s %s %s %s %s %s %s %d %d %x %d %f %f %d",
        &model, modelName, txdName, v16, vehicleName, a2, v25, v18,
        &v20, &v22, &v21, &v13, &v15, &v14, &v19);

    store[model] = std::string(modelName);
    return result;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        uint gameVer = plugin::GetGameVersion();

        if (gameVer == GAME_10US_HOODLUM || gameVer == GAME_10US_COMPACT) {
            plugin::Events::initRwEvent += []() {
                ogsscanf = plugin::patch::TranslateCallOffset<sscanf_t>(0x5B6FC7);
                plugin::patch::ReplaceFunctionCall(0x5B6FC7, sscanfHook);
            };
            
            plugin::Events::initGameEvent += []() {
                RenderHook::Init(CarcolsEditorUI);
            };

            plugin::Events::processScriptsEvent += []() {
                static bool wasDown = false;
                bool isDown = plugin::KeyPressed(VK_F8);
                if (wasDown && !isDown) {
                    gEditorVisible = !gEditorVisible;
                }
                wasDown = isDown;
            };
        }
        else {
            MessageBox(nullptr, "Unknown game version. GTA SA v1.0 US Hoodlum or Compact is required.", MOD_NAME, MB_ICONERROR);
        }
    }

    return TRUE;
}


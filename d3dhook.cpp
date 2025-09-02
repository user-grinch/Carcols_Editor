#include "pch.h"
#include "d3dhook.h"
#include "kiero/kiero.h"
#include "kiero/minhook/MinHook.h"
#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool D3dHook::GetMouseState() {
    return mouseShown;
}

void D3dHook::SetMouseState(bool state) {
    mouseShown = state;
}

LRESULT D3dHook::hkWndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);

    if (ImGui::GetIO().WantTextInput) {
        plugin::Call<0x53F1E0>();
        return 1;
    }

    return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

HRESULT D3dHook::hkReset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters) {
    ImGui_ImplDX9_InvalidateDeviceObjects();

    return oReset(pDevice, pPresentationParameters);
}

BOOL CALLBACK D3dHook::hkSetCursorPos(int x, int y) {
    if (ImGui::GetIO().MouseDrawCursor) {
        return true;
    }
    return oSetCursorPos(x, y);
}

BOOL CALLBACK D3dHook::hkShowCursor(bool flag) {
    if (ImGui::GetIO().MouseDrawCursor) {
        return oShowCursor(TRUE);
    }
    return oShowCursor(flag);
}

void D3dHook::ProcessFrame(void* ptr) {
    ImGuiIO& io = ImGui::GetIO();
    static bool init;

    if (init) {
        ProcessMouse();
        ImGui_ImplWin32_NewFrame();
        if (gRenderer == eRenderer::Dx9) {
            ImGui_ImplDX9_NewFrame();
        }
        else {
            ImGui_ImplDX11_NewFrame();
        }

        ImGui::NewFrame();

        if (pCallbackFunc != nullptr) {
            pCallbackFunc();
        }
     
        io.MouseDrawCursor = mouseShown;

        ImGui::EndFrame();
        ImGui::Render();

        if (gRenderer == eRenderer::Dx9) {
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
        }
        else {
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        }
    }
    else {
        init = true;
        ImGui_ImplWin32_Init(RsGlobal.ps->window);

        plugin::patch::Nop(0x00531155, 5); // shift trigger fix

        if (gRenderer == eRenderer::Dx9) {
            gD3dDevice = ptr;
            ImGui_ImplDX9_Init(reinterpret_cast<IDirect3DDevice9*>(ptr));
        }
        else {
            // for dx11 device ptr is swapchain
            reinterpret_cast<IDXGISwapChain*>(ptr)->GetDevice(__uuidof(ID3D11Device), &ptr);
            gD3dDevice = ptr;
            ID3D11DeviceContext* context;
            reinterpret_cast<ID3D11Device*>(ptr)->GetImmediateContext(&context);

            ImGui_ImplDX11_Init(reinterpret_cast<ID3D11Device*>(ptr), context);
        }

        ImGui_ImplWin32_EnableDpiAwareness();

        io.IniFilename = MOD_NAME;
        io.LogFilename = MOD_NAME;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
        oWndProc = (WNDPROC)SetWindowLongPtr(RsGlobal.ps->window, GWL_WNDPROC, (LRESULT)hkWndProc);
    }
}

HRESULT D3dHook::hkEndScene(IDirect3DDevice9* pDevice) {
    ProcessFrame(pDevice);
    return oEndScene(pDevice);
}

HRESULT D3dHook::hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags) {
    ProcessFrame(pSwapChain);
    return oPresent(pSwapChain, SyncInterval, Flags);
}

void D3dHook::ProcessMouse() {
    // Disable player controls for controllers
    bool bMouseDisabled = false;
    bool isController = plugin::patch::Get<BYTE>(0xBA6818);

    if (gEditorVisible && isController && (mouseShown || bMouseDisabled)) {

        CPlayerPed* player = FindPlayerPed();
        CPad* pad = player ? player->GetPadFromPlayer() : NULL;

        if (pad) {
            if (mouseShown) {
                bMouseDisabled = true;
                pad->DisablePlayerControls = true;
            }
            else {
                bMouseDisabled = false;
                pad->DisablePlayerControls = false;
            }
        }
    }

    static bool mouseState = false;
    if (mouseState != mouseShown) {
        if (mouseShown) {

            plugin::patch::SetUChar(0x6194A0, 0xC3); // psSetMousePos
            plugin::patch::Nop(0x541DD7, 5); // don't call CPad::UpdateMouse()

            // Fix bug with radio switching
            plugin::patch::SetUChar(0x4EB731, 0xEB); // jz -> jmp, skip mouse checks
            plugin::patch::SetUChar(0x4EB75A, 0xEB); // jz -> jmp, skip mouse checks
        }
        else {

            plugin::patch::SetUChar(0x6194A0, 0xE9);
            
            plugin::patch::SetRaw(0x541DD7, (char*)"\xE8\xE4\xD5\xFF\xFF", 5);
            plugin::patch::SetUChar(0x4EB731, 0x74); // jz
            plugin::patch::SetUChar(0x4EB75A, 0x74); // jz
        }

        // Need to update pads before resting values
        CPad::UpdatePads();
        CPad::NewMouseControllerState.x = 0;
        CPad::NewMouseControllerState.y = 0;

        CPad::ClearMouseHistory();
        if (CPad::GetPad(0)) {
            CPad::GetPad(0)->NewState.DPadUp = 0;
            CPad::GetPad(0)->OldState.DPadUp = 0;
            CPad::GetPad(0)->NewState.DPadDown = 0;
            CPad::GetPad(0)->OldState.DPadDown = 0;
        }
        mouseState = mouseShown;
    }
}

bool D3dHook::Init(std::function<void()> pCallback) {
    static bool hookInjected;
    if (hookInjected) {
        return false;
    }

    if (!ImGui::GetCurrentContext()) {
        ImGui::SetCurrentContext(ImGui::CreateContext());
    }

    MH_Initialize();
    PVOID pSetCursorPos = GetProcAddress(GetModuleHandle("user32.dll"), "SetCursorPos");
    PVOID pShowCursor = GetProcAddress(GetModuleHandle("user32.dll"), "ShowCursor");
    MH_CreateHook(pSetCursorPos, hkSetCursorPos, reinterpret_cast<LPVOID*>(&oSetCursorPos));
    MH_EnableHook(pSetCursorPos);

    /*
        Must check for d3d9 first!
        Seems to crash with nvidia geforce experience overlay
        if anything else is checked before d3d9
    */
    if (init(kiero::RenderType::D3D9) == kiero::Status::Success) {
        gRenderer = eRenderer::Dx9;
        kiero::bind(16, (void**)&oReset, hkReset);
        kiero::bind(42, (void**)&oEndScene, hkEndScene);
        pCallbackFunc = pCallback;
        hookInjected = true;
    }
    else {
        if (init(kiero::RenderType::D3D11) == kiero::Status::Success) {
            gRenderer = eRenderer::Dx11;
            kiero::bind(8, (void**)&oPresent, hkPresent);
            pCallbackFunc = pCallback;
            hookInjected = true;
        }
    }

    return hookInjected;
}

void D3dHook::Shutdown() {
    pCallbackFunc = nullptr;
    SetWindowLongPtr(RsGlobal.ps->window, GWL_WNDPROC, (LRESULT)oWndProc);
    if (gRenderer == eRenderer::Dx9) {
        ImGui_ImplDX9_Shutdown();
    }
    else {
        ImGui_ImplDX11_Shutdown();
    }
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    kiero::shutdown();
}
#include "pch.h"
#include "d3dhook.h"
#include "kiero/kiero.h"
#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_impl_win32.h"
#include <CMenuManager.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool D3dHook::IsCursorVisible() {
    return bMouseVisible;
}

void D3dHook::SetCursorVisible(bool visible) {
    bMouseVisible = visible;
}

LRESULT D3dHook::WndProcHook(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);

    if (ImGui::GetIO().WantTextInput) {
        plugin::Call<0x53F1E0>();
        return 1;
    }
    return CallWindowProc(ogWndProc, hWnd, uMsg, wParam, lParam);
}

HRESULT D3dHook::ResetHook(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* params) {
    ImGui_ImplDX9_InvalidateDeviceObjects();
    return ogReset(device, params);
}

float UpdateScaling(HWND hwnd) {
    float scale = std::min(plugin::screen::GetScreenHeight() / 1080, plugin::screen::GetScreenWidth() / 1920);
    ImGui::GetStyle().ScaleAllSizes(std::ceil(scale));
    return scale * 20.0f;
}

HRESULT D3dHook::EndSceneHook(IDirect3DDevice9* device) {
    static bool imguiInitialized = false;

    if (!imguiInitialized) {
        InitImGui(device);
        imguiInitialized = true;
    }

    ImGuiIO& io = ImGui::GetIO();
    if (!FrontEndMenuManager.m_bMenuActive) {
        ProcessMouse();

        ImGui_ImplWin32_NewFrame();
        ImGui_ImplDX9_NewFrame();
        ImGui::NewFrame();

        ImGui::PushFont(NULL, UpdateScaling(RsGlobal.ps->window));

        if (renderFn) {
            renderFn();
        }

        ImGui::PopFont();

        io.MouseDrawCursor = bMouseVisible;

        ImGui::EndFrame();
        ImGui::Render();
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
    }
    else {
        bool temp = bMouseVisible;
        io.MouseDrawCursor = bMouseVisible = false;
        ProcessMouse();
        bMouseVisible = temp;
    }
    return ogEndScene(device);
}

void D3dHook::InitImGui(IDirect3DDevice9* device) {
    ImGui::CreateContext();
    ImGui_ImplWin32_Init(RsGlobal.ps->window);
    ImGui_ImplDX9_Init(device);
    ImGui_ImplWin32_EnableDpiAwareness();

    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = NULL;
    io.LogFilename = NULL;
    io.FontDefault = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 20.0f);
    ogWndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(RsGlobal.ps->window, GWL_WNDPROC, reinterpret_cast<LONG_PTR>(WndProcHook)));
    plugin::patch::Nop(0x00531155, 5); // shift trigger fix
}

void D3dHook::ShutdownImGui() {
    if (ogWndProc) {
        SetWindowLongPtr(RsGlobal.ps->window, GWL_WNDPROC, reinterpret_cast<LONG_PTR>(ogWndProc));
        ogWndProc = nullptr;
    }
    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void D3dHook::ProcessMouse() {
    static bool lastMouseState = false;

    bool isController = plugin::patch::Get<BYTE>(0xBA6818);
    if (gEditorVisible && isController && bMouseVisible) {
        if (CPlayerPed* player = FindPlayerPed()) {
            if (CPad* pad = player->GetPadFromPlayer()) {
                pad->DisablePlayerControls = bMouseVisible;
            }
        }
    }

    if (lastMouseState == bMouseVisible)
        return;

    if (bMouseVisible) {
        plugin::patch::SetUChar(0x6194A0, 0xC3);        // psSetMousePos
        plugin::patch::Nop(0x541DD7, 5);                // disable UpdateMouse
        plugin::patch::SetUChar(0x4EB731, 0xEB);        // skip mouse checks
        plugin::patch::SetUChar(0x4EB75A, 0xEB);
    }
    else {
        plugin::patch::SetUChar(0x6194A0, 0xE9);
        plugin::patch::SetRaw(0x541DD7, (void*)"\xE8\xE4\xD5\xFF\xFF", 5);
        plugin::patch::SetUChar(0x4EB731, 0x74);        // restore jz
        plugin::patch::SetUChar(0x4EB75A, 0x74);
    }

    CPad::UpdatePads();
    CPad::NewMouseControllerState.x = 0;
    CPad::NewMouseControllerState.y = 0;
    CPad::ClearMouseHistory();

    if (auto pad = CPad::GetPad(0)) {
        pad->NewState.DPadUp = pad->OldState.DPadUp = 0;
        pad->NewState.DPadDown = pad->OldState.DPadDown = 0;
    }

    lastMouseState = bMouseVisible;
}

void D3dHook::Init(std::function<void()> callback) {
    if (bInitialized)
        return;

    if (kiero::init(kiero::RenderType::D3D9) == kiero::Status::Success) {
        kiero::bind(16, reinterpret_cast<void**>(&ogReset), ResetHook);
        kiero::bind(42, reinterpret_cast<void**>(&ogEndScene), EndSceneHook);
    }

    renderFn = std::move(callback);
    bInitialized = true;
}

void D3dHook::Shutdown() {
    if (!bInitialized)
        return;

    renderFn = nullptr;
    ShutdownImGui();
    kiero::shutdown();

    bInitialized = false;
}

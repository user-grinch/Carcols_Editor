#pragma once
#include "pch.h"
#include <functional>

class D3dHook {
private:
    using EndSceneFn = HRESULT(CALLBACK*)(IDirect3DDevice9*);
    using ResetFn = HRESULT(CALLBACK*)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);

    static inline WNDPROC          ogWndProc = nullptr;
    static inline EndSceneFn       ogEndScene = nullptr;
    static inline ResetFn          ogReset = nullptr;
    static inline bool             bMouseVisible = false;
    static inline bool             bInitialized = false;
    static inline std::function<void()> renderFn = nullptr;

    // Hooks
    static LRESULT CALLBACK WndProcHook(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static HRESULT CALLBACK EndSceneHook(IDirect3DDevice9* device);
    static HRESULT CALLBACK ResetHook(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* params);

    static void ProcessMouse();
    static void InitImGui(IDirect3DDevice9* device);
    static void ShutdownImGui();
    static void RefreshFonts();

public:
    static bool IsCursorVisible();
    static void SetCursorVisible(bool visible);

    static void Init(std::function<void()> callback);
    static void Shutdown();
};

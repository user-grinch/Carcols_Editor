#pragma once
#include "pch.h"
#include <functional>

class D3dHook {
private:
    using f_EndScene = HRESULT(CALLBACK*)(IDirect3DDevice9*);
    using f_Present = HRESULT(CALLBACK*)(IDXGISwapChain*, UINT, UINT);
    using f_Reset = HRESULT(CALLBACK*)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);
    using f_SetCursorPos = BOOL(CALLBACK*)(int, int);
    using f_ShowCursor = BOOL(CALLBACK*)(bool);

    static inline WNDPROC oWndProc;
    static inline f_Present oPresent;
    static inline f_EndScene oEndScene;
    static inline f_SetCursorPos oSetCursorPos;
    static inline f_ShowCursor oShowCursor;
    static inline f_Reset oReset;
    static inline bool mouseShown;
    static inline std::function<void()> pCallbackFunc = nullptr;

    static void CALLBACK ProcessFrame(void* ptr);
    static LRESULT CALLBACK hkWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static void ProcessMouse();

    static BOOL CALLBACK hkSetCursorPos(int, int);
    static BOOL CALLBACK hkShowCursor(bool);

    // DirectX9
    static HRESULT CALLBACK hkEndScene(IDirect3DDevice9* pDevice);
    static HRESULT CALLBACK hkReset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters);

    // DirectX11, Renderhook
    static HRESULT CALLBACK hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);

public:

    D3dHook() = delete;
    D3dHook(D3dHook const&) = delete;
    void operator=(D3dHook const&) = delete;

    // Returns the current mouse visibility state
    static bool GetMouseState();

    // Injects game hooks & stuff
    static bool Init(std::function<void()> pCallback);

    // Sets the current mouse visibility state
    static void SetMouseState(bool state);

    // Cleans up hooks
    static void Shutdown();
};
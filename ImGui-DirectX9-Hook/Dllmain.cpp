#include <d3d9.h>
#include "GuiWindow.h"
#include "MinHook/include/MinHook.h"
#pragma comment (lib, "d3d9.lib")

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
typedef HRESULT(WINAPI* Reset)(LPDIRECT3DDEVICE9 Direct3Device9, D3DPRESENT_PARAMETERS* pPresentationParameters);
typedef HRESULT(WINAPI* EndScene)(LPDIRECT3DDEVICE9 Direct3Device9);
HRESULT WINAPI Hook_Reset(LPDIRECT3DDEVICE9 Direct3Device9, D3DPRESENT_PARAMETERS* pPresentationParameters);
HRESULT WINAPI Hook_EndScene(LPDIRECT3DDEVICE9 Direct3Device9);
LRESULT WINAPI Hook_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

Reset Original_Reset;
EndScene Original_EndScene;
WNDPROC Original_WndProc;
HMODULE g_hInstance;
HANDLE g_hEndEvent;
LPVOID g_lpVirtualTable;
GuiWindow* g_GuiWindow;

void InitHook()
{
#if defined _M_IX86
    DWORD* lpVTable = (DWORD*)g_lpVirtualTable;
    lpVTable = (DWORD*)lpVTable[0];
#elif defined _M_X64
    DWORD64* lpVTable = (DWORD64*)g_lpVirtualTable;
    lpVTable = (DWORD64*)lpVTable[0];
#endif
    Original_Reset = (Reset)lpVTable[16];
    Original_EndScene = (EndScene)lpVTable[42];
    Original_WndProc = (WNDPROC)::SetWindowLongPtr(g_GuiWindow->hWnd, GWLP_WNDPROC, (LONG_PTR)Hook_WndProc);

    MH_Initialize();

    // Reset
    LPVOID lpTarget = (LPVOID)lpVTable[16];
    MH_CreateHook(lpTarget, &Hook_Reset, (void**)&Original_Reset);
    MH_EnableHook(lpTarget);

    // EndScene
    lpTarget = (LPVOID)lpVTable[42];
    MH_CreateHook(lpTarget, &Hook_EndScene, (void**)&Original_EndScene);
    MH_EnableHook(lpTarget);
}

void ReleaseHook()
{
    ::SetWindowLongPtr(g_GuiWindow->hWnd, GWLP_WNDPROC, (LONG_PTR)Original_WndProc);
    MH_DisableHook(MH_ALL_HOOKS);

    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    ::SetEvent(g_hEndEvent);
}

LRESULT WINAPI Hook_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_KEYDOWN:
        if (wParam == VK_INSERT)
            g_GuiWindow->bShowMenu = !g_GuiWindow->bShowMenu;
        break;

    case WM_DESTROY:
        ReleaseHook();
        break;
    }

    if (g_GuiWindow->bShowMenu && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
        return true;

    return ::CallWindowProc(Original_WndProc, hWnd, uMsg, wParam, lParam);
}

inline static void InitImGui(LPDIRECT3DDEVICE9 lpDirect3Device9)
{
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
    io.Fonts->AddFontFromFileTTF(g_GuiWindow->FontPath, 20.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;

    ImGuiStyle& style = ImGui::GetStyle();
    style.ButtonTextAlign.y = 0.46f;
    style.WindowBorderSize = 0.0f;
    style.WindowRounding = 0.0f;
    style.WindowPadding.x = 0.0f;
    style.WindowPadding.y = 0.0f;
    style.FrameRounding = 0.0f;
    style.FrameBorderSize = 0.0f;
    style.FramePadding.x = 0.0f;
    style.FramePadding.y = 0.0f;
    style.ChildRounding = 0.0f;
    style.ChildBorderSize = 0.0f;
    style.GrabRounding = 0.0f;
    style.GrabMinSize = 8.0f;
    style.PopupBorderSize = 0.0f;
    style.PopupRounding = 0.0f;
    style.ScrollbarRounding = 0.0f;
    style.TabBorderSize = 0.0f;
    style.TabRounding = 0.0f;
    style.DisplaySafeAreaPadding.x = 0.0f;
    style.DisplaySafeAreaPadding.y = 0.0f;
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_FrameBg] = ImColor(0, 74, 122, 100).Value;
    style.Colors[ImGuiCol_FrameBgHovered] = ImColor(0, 74, 122, 175).Value;
    style.Colors[ImGuiCol_FrameBgActive] = ImColor(0, 74, 122, 255).Value;
    style.Colors[ImGuiCol_TitleBg] = ImColor(0, 74, 122, 255).Value;
    style.Colors[ImGuiCol_TitleBgActive] = ImColor(0, 74, 122, 255).Value;

    ImGui_ImplWin32_Init(g_GuiWindow->hWnd);
    ImGui_ImplDX9_Init(lpDirect3Device9);
}

HRESULT WINAPI Hook_EndScene(LPDIRECT3DDEVICE9 lpDirect3Device9)
{
    static bool g_bImGuiInit = false;

    if (!g_bImGuiInit)
    {
        InitImGui(lpDirect3Device9);
        g_bImGuiInit = true;
    }
    else if (g_GuiWindow->UIStatus & GuiWindow::Detach)
    {
        ReleaseHook();
        return Original_EndScene(lpDirect3Device9);
    }

    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::ShowDemoWindow();
    //g_GuiWindow->Update();

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

    return Original_EndScene(lpDirect3Device9);
}

HRESULT WINAPI Hook_Reset(LPDIRECT3DDEVICE9 lpDirect3Device9, D3DPRESENT_PARAMETERS* pPresentationParameters)
{
    ImGui_ImplDX9_InvalidateDeviceObjects();

    HRESULT hResult = Original_Reset(lpDirect3Device9, pPresentationParameters);

    ImGui_ImplDX9_CreateDeviceObjects();

    return hResult;
}

DWORD WINAPI Start(LPVOID lpParameter)
{
    g_hInstance = (HMODULE)lpParameter;
    g_hEndEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    g_GuiWindow = new GuiWindow();
    g_GuiWindow->Init();

    WNDCLASSEX windowClass{};
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = ::DefWindowProc;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = ::GetModuleHandle(NULL);
    windowClass.hIcon = NULL;
    windowClass.hCursor = NULL;
    windowClass.hbrBackground = NULL;
    windowClass.lpszMenuName = NULL;
    windowClass.lpszClassName = "DirectX9";
    windowClass.hIconSm = NULL;

    ::RegisterClassEx(&windowClass);
    HWND hWnd = ::CreateWindow(
        windowClass.lpszClassName,
        "DirectX9Window",
        WS_OVERLAPPEDWINDOW,
        0,
        0,
        100,
        100,
        NULL,
        NULL,
        windowClass.hInstance,
        NULL);

    LPDIRECT3D9 lpDirect3D9 = ::Direct3DCreate9(D3D_SDK_VERSION);
    if (!lpDirect3D9)
        return -1;

    D3DPRESENT_PARAMETERS params{};
    params.AutoDepthStencilFormat = D3DFMT_UNKNOWN;
    params.BackBufferWidth = 0;
    params.BackBufferHeight = 0;
    params.BackBufferCount = 0;
    params.BackBufferFormat = D3DFMT_UNKNOWN;
    params.EnableAutoDepthStencil = 0;
    params.Flags = NULL;
    params.FullScreen_RefreshRateInHz = 0;
    params.hDeviceWindow = hWnd;
    params.MultiSampleType = D3DMULTISAMPLE_NONE;
    params.MultiSampleQuality = NULL;
    params.PresentationInterval = 0;
    params.SwapEffect = D3DSWAPEFFECT_DISCARD;
    params.Windowed = 1;

    LPDIRECT3DDEVICE9 lpDirect3Device9;
    if (SUCCEEDED(lpDirect3D9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &params, &lpDirect3Device9)))
    {
        g_lpVirtualTable = lpDirect3Device9;
        InitHook();

        lpDirect3Device9->Release();
        lpDirect3D9->Release();
    }
    ::DestroyWindow(hWnd);
    ::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);

    if (g_hEndEvent)
        ::WaitForSingleObject(g_hEndEvent, INFINITE);
    ::FreeLibraryAndExitThread(g_hInstance, 0);

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        if (::GetModuleHandleA("d3d9.dll") == NULL)
            return false;

        ::DisableThreadLibraryCalls(hModule);
        ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Start, hModule, 0, NULL);
        break;

    case DLL_PROCESS_DETACH:
        MH_Uninitialize();
        break;
    }

    return true;
}

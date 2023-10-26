#include <d3d9.h>
#include "mainwindow.h"
#include "detours.h"
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "detours.lib")

typedef HRESULT(WINAPI* Reset)(LPDIRECT3DDEVICE9 Direct3Device9, D3DPRESENT_PARAMETERS* pPresentationParameters);
typedef HRESULT(WINAPI* EndScene)(LPDIRECT3DDEVICE9 Direct3Device9);
HRESULT WINAPI HK_Reset(LPDIRECT3DDEVICE9 Direct3Device9, D3DPRESENT_PARAMETERS* pPresentationParameters);
HRESULT WINAPI HK_EndScene(LPDIRECT3DDEVICE9 Direct3Device9);
LRESULT WINAPI HK_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

Reset Original_Reset;
EndScene Original_EndScene;
WNDPROC Original_WndProc;
HWND g_mainWindow;
HMODULE g_hHinstance;
HANDLE g_hEndEvent;
DWORD64* g_methodsTable;
GuiWindow* g_GuiWindow;
bool g_ImGuiInit = false;

void InitHook()
{
#if defined _M_IX86
    DWORD* DVTable = (DWORD*)g_methodsTable;
    DVTable = (DWORD*)DVTable[0];
#elif defined _M_X64
    DWORD64* DVTable = (DWORD64*)g_methodsTable;
    DVTable = (DWORD64*)DVTable[0];
#endif

    Original_Reset = (Reset)DVTable[16];
    Original_EndScene = (EndScene)DVTable[42];

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(LPVOID&)Original_Reset, HK_Reset);
    DetourAttach(&(LPVOID&)Original_EndScene, HK_EndScene);
    DetourTransactionCommit();
}

void ReleaseHook()
{
    SetWindowLongPtr(g_mainWindow, GWLP_WNDPROC, (LONG_PTR)Original_WndProc);

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourDetach(&(LPVOID&)Original_Reset, HK_Reset);
    DetourDetach(&(LPVOID&)Original_EndScene, HK_EndScene);
    DetourTransactionCommit();

    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    SetEvent(g_hEndEvent);
}

LRESULT WINAPI HK_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_KEYDOWN:
        if (wParam == VK_M)
            g_GuiWindow->showMenu = !g_GuiWindow->showMenu;
        break;

    case WM_DESTROY:
        ReleaseHook();
        break;
    }

    if (g_GuiWindow->showMenu && ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
        return true;

    return CallWindowProc(Original_WndProc, hwnd, uMsg, wParam, lParam);
}

inline void InitImGui(LPDIRECT3DDEVICE9 Direct3Device9)
{
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
    io.Fonts->AddFontFromFileTTF(g_GuiWindow->fontPath, 20.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;

    ImGuiStyle& Style = ImGui::GetStyle();
    Style.ButtonTextAlign.y = 0.46f;
    Style.WindowBorderSize = 0.0f;
    Style.WindowRounding = 0.0f;
    Style.WindowPadding.x = 0.0f;
    Style.WindowPadding.y = 0.0f;
    Style.FrameRounding = 0.0f;
    Style.FrameBorderSize = 0.0f;
    Style.FramePadding.x = 0.0f;
    Style.FramePadding.y = 0.0f;
    Style.ChildRounding = 0.0f;
    Style.ChildBorderSize = 0.0f;
    Style.GrabRounding = 0.0f;
    Style.GrabMinSize = 8.0f;
    Style.PopupBorderSize = 0.0f;
    Style.PopupRounding = 0.0f;
    Style.ScrollbarRounding = 0.0f;
    Style.TabBorderSize = 0.0f;
    Style.TabRounding = 0.0f;
    Style.DisplaySafeAreaPadding.x = 0.0f;
    Style.DisplaySafeAreaPadding.y = 0.0f;
    Style.Colors[ImGuiCol_WindowBg] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
    Style.Colors[ImGuiCol_ChildBg] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
    Style.Colors[ImGuiCol_PopupBg] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
    Style.Colors[ImGuiCol_FrameBg] = ImColor(0, 74, 122, 100).Value;
    Style.Colors[ImGuiCol_FrameBgHovered] = ImColor(0, 74, 122, 175).Value;
    Style.Colors[ImGuiCol_FrameBgActive] = ImColor(0, 74, 122, 255).Value;
    Style.Colors[ImGuiCol_TitleBg] = ImColor(0, 74, 122, 255).Value;
    Style.Colors[ImGuiCol_TitleBgActive] = ImColor(0, 74, 122, 255).Value;

    ImGui_ImplWin32_Init(g_mainWindow);
    ImGui_ImplDX9_Init(Direct3Device9);
    Original_WndProc = (WNDPROC)SetWindowLongPtr(g_mainWindow, GWLP_WNDPROC, (LONG_PTR)HK_WndProc);

    g_ImGuiInit = true;
}

HRESULT WINAPI HK_EndScene(LPDIRECT3DDEVICE9 Direct3Device9)
{
    if (!g_ImGuiInit)
    {
        D3DDEVICE_CREATION_PARAMETERS params;
        Direct3Device9->GetCreationParameters(&params);
        g_mainWindow = params.hFocusWindow;

        InitImGui(Direct3Device9);
    }
    else if (g_GuiWindow->windowStatus & WindowStatus::Exit)
    {
        ReleaseHook();
        return Original_EndScene(Direct3Device9);
    }

    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::ShowDemoWindow();
    //g_GuiWindow->Update();

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

    return Original_EndScene(Direct3Device9);
}

HRESULT WINAPI HK_Reset(LPDIRECT3DDEVICE9 Direct3Device9, D3DPRESENT_PARAMETERS* pPresentationParameters)
{
    ImGui_ImplDX9_InvalidateDeviceObjects();

    HRESULT hResult = Original_Reset(Direct3Device9, pPresentationParameters);

    ImGui_ImplDX9_CreateDeviceObjects();

    return hResult;
}


DWORD WINAPI Start(LPVOID lpParameter)
{
    g_hHinstance = (HMODULE)lpParameter;
    g_hEndEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    g_GuiWindow = new GuiWindow();
    //g_GuiWindow->Init();

    WNDCLASSEX windowClass;
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = DefWindowProc;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = GetModuleHandle(NULL);
    windowClass.hIcon = NULL;
    windowClass.hCursor = NULL;
    windowClass.hbrBackground = NULL;
    windowClass.lpszMenuName = NULL;
    windowClass.lpszClassName = "DirectX9";
    windowClass.hIconSm = NULL;

    ::RegisterClassEx(&windowClass);
    HWND hwnd = ::CreateWindow(windowClass.lpszClassName, "DirectX9Window", WS_OVERLAPPEDWINDOW, 0, 0, 100, 100, NULL, NULL, windowClass.hInstance, NULL);

    LPDIRECT3D9 direct3D9 = Direct3DCreate9(D3D_SDK_VERSION);
    if (!direct3D9)
        return 0xF;

    D3DPRESENT_PARAMETERS params{};
    params.AutoDepthStencilFormat = D3DFMT_UNKNOWN;
    params.BackBufferWidth = 0;
    params.BackBufferHeight = 0;
    params.BackBufferCount = 0;
    params.BackBufferFormat = D3DFMT_UNKNOWN;
    params.EnableAutoDepthStencil = 0;
    params.Flags = NULL;
    params.FullScreen_RefreshRateInHz = 0;
    params.hDeviceWindow = hwnd;
    params.MultiSampleType = D3DMULTISAMPLE_NONE;
    params.MultiSampleQuality = NULL;
    params.PresentationInterval = 0;
    params.SwapEffect = D3DSWAPEFFECT_DISCARD;
    params.Windowed = 1;

    LPDIRECT3DDEVICE9 direct3Device9;
    if (!direct3D9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &params, &direct3Device9))
    {
        g_methodsTable = (DWORD64*)direct3Device9;

        InitHook();

        direct3Device9->Release();
        direct3D9->Release();
    }
    ::DestroyWindow(hwnd);
    ::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);

    ::WaitForSingleObject(g_hEndEvent, INFINITE);
    ::FreeLibraryAndExitThread(g_hHinstance, 0);

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
        ReleaseHook();
        break;
    }

    return true;
}
#pragma once
#include <Windows.h>
#include <string>
#include "ImGui/imgui.h"
#include "ImGui/imgui_internal.h"
#include "Imgui/imgui_impl_dx9.h"
#include "Imgui/imgui_impl_win32.h"

#define AUTHORINFO          "Build.20xx.xx.xx\nby l4kkS41"

#define WINDOWNAME          "ImGui Window"
#define MAJORVERSION        0
#define MINORVERSION        0
#define REVISIONVERSION     1

#define TARGETCLASS         "gfx_test"
#define TARGETWINDOW        "Renderer: [DirectX9], Input: [Window Messages], 64 bits"
#define TARGETMODULE        "GFXTest64.exe"

#define WIDTH               600
#define HEIGHT              400

#define VK_M                0x4D

enum WindowStatus : DWORD
{
    Normal      = 0,
    Repaint     = 1 << 0,
    Message     = 1 << 1,
    Exiting     = 1 << 2,
    End         = 1 << 3
};

class GuiWindow
{
public:
    char*       fontPath;
    char*       windowName;
    HWND        mainWindow;
    HANDLE      hProcess;
    HMODULE     hModule;
    LPBYTE      baseAddress;
    LPBYTE      lpBuffer;
    DWORD       windowStatus;
    ImVec2      startPostion;
    bool        crossHair;
    bool        showMenu;

    GuiWindow();
    ~GuiWindow();

    void Init();
    void Update();
    void OnRepaint();
    void Button_OnExiting();
    void Toggle_CrossHair(const bool& isEnable);
};

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
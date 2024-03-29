#pragma once
#include <Windows.h>
#include <string>
#include "ImGui/imgui.h"
#include "Imgui/imgui_impl_dx9.h"
#include "Imgui/imgui_impl_win32.h"
#include "Imgui/imgui_internal.h"

#define AUTHORINFO          "Build.20xx.xx.xx\nby l4kkS41"

#define WINDOWNAME          "ImGui Window"
#define MAJORVERSION        1
#define MINORVERSION        0
#define REVISIONVERSION     0

#define WIDTH               600
#define HEIGHT              400

#if defined _M_IX86
#define TARGETCLASS         "gfx_test"
#define TARGETWINDOW        "Renderer: [DirectX9], Input: [Window Messages], 32 bits"
#define TARGETMODULE        "GFXTest32.exe"
#elif defined _M_X64
#define TARGETCLASS         "gfx_test"
#define TARGETWINDOW        "Renderer: [DirectX9], Input: [Window Messages], 64 bits"
#define TARGETMODULE        "GFXTest64.exe"
#endif

class GuiWindow
{
public:
    enum GuiStatus : DWORD
    {
        Normal = 0,
        Reset = 1 << 0,
        Exit = 1 << 1,
        Detach = 1 << 2
    };

    HANDLE      hProcess;
    HMODULE     hModule;
    HWND        Hwnd;
    PCHAR       FontPath;
    PCHAR       WindowName;
    LPBYTE      ModuleAddress;
    LPBYTE      lpBuffer;
    ImVec2      StartPostion;
    DWORD       UIStatus;
    bool        bCrossHair;
    bool        bShowMenu;

    GuiWindow();
    ~GuiWindow();

    void Init();
    void Update();
    
    void Button_Exit();
    void ResetWindow();
    void Toggle_CrossHair(const bool& isEnable);
};
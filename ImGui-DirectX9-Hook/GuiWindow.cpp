#include "GuiWindow.h"

GuiWindow::GuiWindow()
{
    // 申请内存
    this->lpBuffer = (LPBYTE)::VirtualAlloc(NULL, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    ::memset(this->lpBuffer, 0xCC, 0x1000);

    // 加载中文字体
    this->FontPath = new char[MAX_PATH] {};
    ::GetEnvironmentVariableA("WINDIR", this->FontPath, MAX_PATH);
    ::strcat_s(this->FontPath, MAX_PATH, "\\Fonts\\msyh.ttc");

    // 窗口名称
    std::string strText;
    strText = strText.append(WINDOWNAME).append(" v") + std::to_string(MAJORVERSION) + std::string().append(".") + std::to_string(MINORVERSION) + std::string().append(".") + std::to_string(REVISIONVERSION);
    size_t nLength = strText.length() + 1;
    this->WindowName = new char[nLength] {};
    ::memcpy(this->WindowName, strText.c_str(), strText.length());

    // 窗口设置
    this->StartPostion = ImVec2(0.0f, 0.0f);
    this->UIStatus = GuiStatus::Reset;

    // 功能菜单
    this->bShowMenu = true;
    this->bCrossHair = false;
}

GuiWindow::~GuiWindow()
{
    if (this->lpBuffer)
        ::VirtualFree(this->lpBuffer, 0, MEM_RELEASE);

    delete[] this->FontPath;
    delete[] this->WindowName;
}

void GuiWindow::Init()
{
    do
    {
        this->Hwnd = FindWindow(TARGETCLASS, TARGETWINDOW);
        this->hProcess = ::GetCurrentProcess();
        this->hModule = GetModuleHandle(TARGETMODULE);
        Sleep(200);
    } while (this->Hwnd == NULL || this->hProcess == NULL || this->hModule == NULL);

    this->ModuleAddress = (LPBYTE)this->hModule;
}

void GuiWindow::Update()
{
    if (this->bShowMenu)
    {
        const ImGuiWindowFlags windowflags =
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse |
            ImGuiWindowFlags_NoSavedSettings;
        ImGui::Begin(this->WindowName, nullptr, windowflags);
        if (this->UIStatus & GuiStatus::Reset)
            ResetWindow();
        
        ImVec2 windowPostion = ImGui::GetWindowPos();
        ImGui::Text(this->WindowName);
        if (ImGui::CloseButton(0x1000, ImVec2(windowPostion.x + WIDTH - 20.0f, windowPostion.y)))
            this->UIStatus |= GuiStatus::Exit;

        if (ImGui::Checkbox(u8"绘制准星", &this->bCrossHair))
            Toggle_CrossHair(this->bCrossHair);

        const std::string authorInfo = std::string(AUTHORINFO);
        ImVec2 textSize = ImGui::CalcTextSize(authorInfo.c_str());
        ImGui::SetCursorPosY(HEIGHT - textSize.y);
        ImGui::Text(authorInfo.c_str());

        const std::string hotKey = std::string(u8"INSERT显示/隐藏界面");
        textSize = ImGui::CalcTextSize(hotKey.c_str());
        ImGui::SetCursorPos(ImVec2(WIDTH - textSize.x, HEIGHT - textSize.y));
        ImGui::Text(hotKey.c_str());

        if (this->UIStatus & GuiStatus::Exit)
            this->Button_Exit();

        ImGui::End();
    }

    ImDrawList* drawList = ImGui::GetBackgroundDrawList();
    if (this->bCrossHair)
    {
        ImVec2 displaySize = ImGui::GetIO().DisplaySize;

        drawList->AddLine(
            ImVec2(displaySize.x / 2 - 30.0f, displaySize.y / 2),
            ImVec2(displaySize.x / 2 + 30.0f, displaySize.y / 2),
            ImColor(0, 255, 0, 255));
        drawList->AddLine(
            ImVec2(displaySize.x / 2, displaySize.y / 2 - 30.0f),
            ImVec2(displaySize.x / 2, displaySize.y / 2 + 30.0f),
            ImColor(0, 255, 0, 255));
    }
}

void GuiWindow::Button_Exit()
{
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
    ImGui::SetCursorPos(ImVec2(0, 0));
    ImGui::BeginChildFrame(0x2000, ImVec2(WIDTH, HEIGHT));
    ImGui::SetCursorPos(ImVec2(0, 0));
    ImGui::BeginChild("Exiting", ImVec2(WIDTH, HEIGHT));

    std::string strText = std::string(u8"是否退出程序？");
    ImVec2 textSize = ImGui::CalcTextSize(strText.c_str());
    ImGui::SetCursorPos(ImVec2((WIDTH - textSize.x) * 0.5f, HEIGHT * 0.382f - textSize.y * 0.5f));
    ImGui::Text(strText.c_str());

    ImGui::SetCursorPos(ImVec2(WIDTH * 0.5f - 120, HEIGHT * 0.618f));
    if (ImGui::Button(u8"确认", ImVec2(100.0f, 50.0f)))
        this->UIStatus |= GuiStatus::Detach;

    ImGui::SetCursorPos(ImVec2(WIDTH * 0.5f + 20, HEIGHT * 0.618f));
    if (ImGui::Button(u8"取消", ImVec2(100.0f, 50.0f)))
        this->UIStatus &= ~GuiStatus::Exit;

    ImGui::EndChild();
    ImGui::EndChildFrame();
    ImGui::PopStyleColor(2);
}

void GuiWindow::ResetWindow()
{
    ImGui::SetWindowPos(this->StartPostion);
    ImGui::SetWindowSize(ImVec2(WIDTH, HEIGHT));
    this->UIStatus &= ~GuiStatus::Reset;
}

void GuiWindow::Toggle_CrossHair(const bool& isEnable)
{
    this->bCrossHair = isEnable;
}
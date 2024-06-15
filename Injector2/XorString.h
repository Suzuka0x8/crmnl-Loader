#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>

#include <iostream>
#include <Windows.h>

// Data
static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#include "imgui_internal.h"

#include <D3DX11tex.h>
#pragma comment(lib, "D3DX11.lib")

#include "background.h"
#include "user_circle.h"
#include "inter.h"
#include "ico_pack.h"

#include "color.h"

using namespace std;

ImFont* ico = nullptr;
ImFont* ico_2 = nullptr;
ImFont* ico_minimize = nullptr;
ImFont* tab_text;
ImFont* tab_text1;
ImFont* tab_text2;
ImFont* tab_text3;
ImFont* ico_logo;
ImFont* ico_subtab;

float slider_blur = 3.f;

namespace var {
    bool checkbox = true;
    bool checkboxIn = false;
    bool esp = false;
    bool box = false;
    bool name = false;
    bool hp = false;
    bool ar = false;
    bool am = false;
    bool esp1 = false;
    bool box1 = false;
    bool name1 = false;
    bool hp1 = false;
    bool ar1 = false;
    bool am1 = false;
    int slider_intager = 50;
    float slider_float = 0.5f;

    bool boundbox1 = false;
    bool boundbox = false;
    bool bindbox = false;

    float slider_float1 = 0.0f;
    float slider_float2 = 1.0f;
    float slider_float3 = 90.0f;
    int selectedItem3 = 0;
    const char* items3_eng[] = { "Selected 0", "Selected 1", "Selected 2", "Selected 3" };
    bool multi_items_count[5];
    const char* multi_items[5] = { "One", "Two", "Three", "Four", "Five" };
    float color_edit0[4] = { 0.70f, 0.80f, 0.90f, 1.000f };
    float color_edit1[4] = { 0.70f, 0.80f, 0.90f, 1.000f };
    float color_edit2[4] = { 218 / 255.f, 96 / 255.f, 21 / 255.f, 255 / 255.f};
    float color_edit3[4] = { 0.70f, 0.80f, 0.90f, 1.000f };
    float color_edit4[4] = { 0.70f, 0.80f, 0.90f, 1.000f };

    static const char* items[]{ "Default", "Triangles", "3D Box" };
    int selectedItem = 0;
    static char input[64] = { "" };
    char input2[64] = { "" };
    int key0;
    int key1;
    int key2;
}

static float tab_alpha = 0.f;
static float tab_add;
static int active_tab = 0;
int tabs = 0;
int sub_tabs = 0;

bool menu = true;

bool espkkkk = true;
float dpi_scale = 1.0f;

void CustomStyleColor()
{
    ImGuiStyle& s = ImGui::GetStyle();

    s.Colors[ImGuiCol_WindowBg] = ImColor(60, 65, 80, 60);
    s.Colors[ImGuiCol_ChildBg] = ImColor(20, 20, 20, 255);
    s.Colors[ImGuiCol_PopupBg] = ImColor(26, 26, 26, 255);
    s.Colors[ImGuiCol_Text] = ImColor(120, 120, 120, 255);
    s.Colors[ImGuiCol_TextDisabled] = ImColor(100, 100, 100, 255);
    s.Colors[ImGuiCol_Border] = ImColor(28, 28, 28, 255);
    s.Colors[ImGuiCol_TextSelectedBg] = ImColor(25, 22, 33, 100);

    s.Colors[ImGuiCol_ScrollbarGrab] = ImColor(24, 24, 24, 255);
    s.Colors[ImGuiCol_ScrollbarGrabHovered] = ImColor(24, 24, 24, 255);
    s.Colors[ImGuiCol_ScrollbarGrabActive] = ImColor(24, 24, 24, 255);

    s.WindowBorderSize = 0;
    s.WindowPadding = ImVec2(0, 0);
    s.WindowRounding = 5.f;
    s.PopupBorderSize = 0.f;
    s.PopupRounding = 5.f;
    s.ChildRounding = 7;
    s.ChildBorderSize = 1.f;
    s.FrameBorderSize = 1.0f;
    s.ScrollbarSize = 3.0f;
    s.FrameRounding = 5.f;
    s.ItemSpacing = ImVec2(0, 20);
    s.ItemInnerSpacing = ImVec2(10, 0);
}

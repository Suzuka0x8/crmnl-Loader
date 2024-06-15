#include "main.h"
#include <map>
static bool animated_background;
int key0;
int key1;
float color_edit[4] = { 64 / 255.f, 77 / 255.f, 236 / 255.f, 190 / 255.f };
ID3D11ShaderResourceView* dr = nullptr;
void Particles()
{
    ImVec2 screen_size = { (float)GetSystemMetrics(SM_CXSCREEN), (float)GetSystemMetrics(SM_CYSCREEN) };

    static ImVec2 partile_pos[100];
    static ImVec2 partile_target_pos[100];
    static float partile_speed[100];
    static float partile_radius[100];

    for (int i = 1; i < 50; i++)
    {
        if (partile_pos[i].x == 0 || partile_pos[i].y == 0)
        {
            partile_pos[i].x = rand() % (int)screen_size.x + 1;
            partile_pos[i].y = 15.f;
            partile_speed[i] = 1 + rand() % 25;
            partile_radius[i] = rand() % 4;

            partile_target_pos[i].x = rand() % (int)screen_size.x;
            partile_target_pos[i].y = screen_size.y * 2;
        }

        partile_pos[i] = ImLerp(partile_pos[i], partile_target_pos[i], ImGui::GetIO().DeltaTime * (partile_speed[i] / 60));

        if (partile_pos[i].y > screen_size.y)
        {
            partile_pos[i].x = 0;
            partile_pos[i].y = 0;
        }

        ImGui::GetWindowDrawList()->AddCircleFilled(partile_pos[i], partile_radius[i], ImColor(255, 255, 255, 255));
    }

}

void espkk()
{
    ImGui::SetNextWindowSize(ImVec2(430, 790));
    ImGui::Begin("espkk", &espkkkk, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground);
    {
        const auto& p = ImGui::GetWindowPos();

        ImGuiStyle& s = ImGui::GetStyle();
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
        D3DX11_IMAGE_LOAD_INFO info; ID3DX11ThreadPump* pump{ nullptr };
        ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(0 + p.x, 0 + p.y), ImVec2(855 + p.x, 790 + p.y), ImGui::GetColorU32(colors::main_color), s.WindowRounding);

        ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(0 + p.x, 759 + p.y), ImVec2(855 + p.x, 790 + p.y), ImGui::GetColorU32(colors::lite_color), s.WindowRounding);
        ImGui::GetForegroundDrawList()->AddText(tab_text, 16, ImVec2(110 + p.x, 762 + p.y), ImColor(255, 255, 255, 255), "UC:hefanccc");
        if (dr == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, dragon, sizeof(dragon), &info, pump, &dr, 0);
        ImGui::SetCursorPosX(25);
        ImGui::SetCursorPosY(30);
        ImGui::BeginChild("ESP Preview", ImVec2(380, 720), true);
        {
            ImVec2 pos = ImGui::GetWindowPos();
            ImDrawList* draw = ImGui::GetWindowDrawList();
           draw->AddImageRounded(dr, ImVec2(pos.x+80  /*position X*/, pos.y+80 /*position Y*/),ImVec2(pos.x + 330 /*size X*/ /*position X*/, pos.y + 530 /*size Y*/ /*position Y*/),ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255, 255) /*color*/, 10 /*rounding*/);
            if (var::esp)
            {
                if (var::box)
                {
                    ImGui::SetCursorPos(ImVec2(50, 70));
                    ImVec2 pos1 = ImGui::GetCursorScreenPos();
                    draw->AddRect(ImVec2(pos1.x, pos1.y), ImVec2(pos1.x + 280, pos1.y + 480), ImColor(255, 255, 255, 255), 0.0f, 0.0f, 1.f);
                }
                if (var::hp)
                {
                    ImGui::SetCursorPos(ImVec2(42, 70));
                    ImVec2 pos1 = ImGui::GetCursorScreenPos();
                    draw->AddRectFilled(ImVec2(pos1.x, pos1.y), ImVec2(pos1.x + 4, pos1.y + 480), ImColor(132, 103, 209, 255), 0.0f);
                }
                if (var::ar)
                {
                    ImGui::SetCursorPos(ImVec2(334, 70));
                    ImVec2 pos1 = ImGui::GetCursorScreenPos();
                    draw->AddRectFilled(ImVec2(pos1.x, pos1.y), ImVec2(pos1.x + 4, pos1.y + 480), ImColor(210, 146, 6, 255), 0.0f);
                }
                if (var::am)
                {
                    ImGui::SetCursorPos(ImVec2(50, 556));
                    ImVec2 pos1 = ImGui::GetCursorScreenPos();
                    draw->AddRectFilled(ImVec2(pos1.x, pos1.y), ImVec2(pos1.x + 280, pos1.y + 4), ImColor(135, 106, 210, 255), 0.0f);
                }
                if (var::name)
                {
                    ImGui::SetCursorPos(ImVec2(170, 50));
                    ImVec2 pos1 = ImGui::GetCursorScreenPos();
                    draw->AddText(ImVec2(pos1.x, pos1.y), ImColor(255, 255, 255, 255), "PLAYER NAME ");
                }
            }
        }ImGui::EndChild();

        ImGui::PopStyleColor();

        if (animated_background)
            Particles();
    }
    ImGui::End();
}

struct keybind_state
{
    ImVec4 mode_color = ImVec4(1.f, 1.f, 1.f, 1.f);
    ImVec4 key_color = ImVec4(1.f, 1.f, 1.f, 1.f);
    float alpha = 0.f;
};


inline ImColor main_color(218, 96, 21, 255);
inline ImVec4 color_particle(0.7f, 0.f, 0.f, 1.f);
inline ImColor background_color(24, 24, 24, 255);

inline ImVec4 second_color(0.09f, 0.09f, 0.09f, 1.f);


inline ImVec2 frame_size = ImVec2(605, 65);

inline float anim_speed = 8.f;

inline bool draw_grind;
const char* themes[]{ "Head","Chest", "Legs", "Hands" };

static int combo = 0;
inline float pos_offset;
inline bool size_change;


static int keybind = 0;
static int iSubTabs = 0;
// Main code
int main(int, char**)
{
    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Dear ImGui DirectX11 Example", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, nullptr, nullptr, wc.hInstance, nullptr);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    io.Fonts->AddFontFromMemoryTTF(&inter, sizeof inter, 16 * dpi_scale, NULL, io.Fonts->GetGlyphRangesCyrillic());
    tab_text1 = io.Fonts->AddFontFromMemoryTTF(&inter, sizeof inter, 12 * dpi_scale, NULL, io.Fonts->GetGlyphRangesCyrillic());
    tab_text2 = io.Fonts->AddFontFromMemoryTTF(&inter, sizeof inter, 24 * dpi_scale, NULL, io.Fonts->GetGlyphRangesCyrillic());
    tab_text3 = io.Fonts->AddFontFromMemoryTTF(&inter, sizeof inter, 40 * dpi_scale, NULL, io.Fonts->GetGlyphRangesCyrillic());
    ico = io.Fonts->AddFontFromMemoryTTF(&icon, sizeof icon, 25 * dpi_scale, NULL, io.Fonts->GetGlyphRangesCyrillic());
    ico_2 = io.Fonts->AddFontFromMemoryTTF(&Menuicon, sizeof Menuicon, 20 * dpi_scale, NULL, io.Fonts->GetGlyphRangesCyrillic());
    ico_subtab = io.Fonts->AddFontFromMemoryTTF(&icon, sizeof icon, 35 * dpi_scale, NULL, io.Fonts->GetGlyphRangesCyrillic());
    ico_logo = io.Fonts->AddFontFromMemoryTTF(&icon, sizeof icon, 31 * dpi_scale, NULL, io.Fonts->GetGlyphRangesCyrillic());
    tab_text = io.Fonts->AddFontFromMemoryTTF(&inter, sizeof inter, 19 * dpi_scale, NULL, io.Fonts->GetGlyphRangesCyrillic());
    ico_minimize = io.Fonts->AddFontFromMemoryTTF(&icon, sizeof icon, 27 * dpi_scale, NULL, io.Fonts->GetGlyphRangesCyrillic());
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    // Our state
    bool show_demo_window = false;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // Handle window resize (we don't resize directly in the WM_SIZE handler)
        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_ResizeWidth = g_ResizeHeight = 0;
            CreateRenderTarget();
        }

        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        {

            CustomStyleColor();


            D3DX11_IMAGE_LOAD_INFO info; ID3DX11ThreadPump* pump{ nullptr };

            if (bg == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, background, sizeof(background), &info, pump, &bg, 0);

            ImGui::GetBackgroundDrawList()->AddImage(bg, ImVec2(0, 0), ImVec2(1920, 1080), ImVec2(0, 0), ImVec2(1, 1), ImColor(100, 100, 100, 255));

            if (us == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, user, sizeof(user), &info, pump, &us, 0);

            ImGui::SetNextWindowSize(ImVec2(855* dpi_scale, 790* dpi_scale));
            ImGui::Begin("Menu", &menu, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground);
            {

                const auto& p = ImGui::GetWindowPos();

                ImGuiStyle& s = ImGui::GetStyle();

                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
                ImGui::BeginChild("G-Tab", ImVec2(173 * dpi_scale, 790 * dpi_scale), false);
                {
                    ImGui::GetForegroundDrawList()->AddText(tab_text3, 20 * dpi_scale, ImVec2(20 * dpi_scale + p.x, 12 * dpi_scale + p.y), ImColor(255, 255, 255, 255), "Unknowncheats");
                    ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(0 + p.x, 0 + p.y), ImVec2(273 * dpi_scale + p.x, 790 * dpi_scale + p.y), ImGui::GetColorU32(colors::Tab_Child), s.WindowRounding);

                    ImGui::SetCursorPosY(60);

                    ImGui::SetWindowFontScale(dpi_scale);
                    if (ImGui::Tab("H", "Aimbot","Legit,Trigger,Rage", 0 == tabs, ImVec2(150 * dpi_scale, 42 * dpi_scale))) tabs = 0;
                    if (ImGui::Tab("G", "Changer", "Inventory & Profile", 1 == tabs, ImVec2(150 * dpi_scale, 42 * dpi_scale))) tabs = 1;
                    if (ImGui::Tab("F", "Visuals", "Player,World,Glow", 2 == tabs, ImVec2(150 * dpi_scale, 42 * dpi_scale))) tabs = 2;
                    if (ImGui::Tab("E", "Misc", "Other settings", 3 == tabs, ImVec2(150 * dpi_scale, 42 * dpi_scale))) tabs = 3;
                    if (ImGui::Tab("D", "Binds", "Use keyboard to on", 4 == tabs, ImVec2(150 * dpi_scale, 42 * dpi_scale))) tabs = 4;
                    if (ImGui::Tab("C", "Minigames", "Snake", 5 == tabs, ImVec2(150 * dpi_scale, 42 * dpi_scale))) tabs = 5;
                    if (ImGui::Tab("B", "Config", "Manage your configs", 6 == tabs, ImVec2(150 * dpi_scale, 42 * dpi_scale))) tabs = 6;
                    if (ImGui::Tab("A", "Chat", "Chat box", 7 == tabs, ImVec2(150 * dpi_scale, 42 * dpi_scale))) tabs = 7;
                    if (ImGui::Tab("I", "Search", "Search for cheats", 8 == tabs, ImVec2(150 * dpi_scale, 42 * dpi_scale))) tabs = 8;

                }ImGui::EndChild();


                ImGui::PopStyleColor();

                ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(0 + p.x, 0 + p.y), ImVec2(855 * dpi_scale + p.x, 790 * dpi_scale + p.y), ImGui::GetColorU32(colors::main_color), s.WindowRounding);

                ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(0 + p.x, 755 * dpi_scale + p.y), ImVec2(855 * dpi_scale + p.x, 755 * dpi_scale + p.y), ImGui::GetColorU32(colors::lite_color), s.WindowRounding);

             //   ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(0 + p.x, 0 + p.y), ImVec2(705 + p.x, 60 + p.y), ImGui::GetColorU32(colors::lite_color), s.WindowRounding);

                
                ImGui::GetForegroundDrawList()->AddText(tab_text3, 16 * dpi_scale, ImVec2(10 * dpi_scale + p.x, 765 * dpi_scale + p.y), ImColor(255, 255, 255, 255), "Uid:1337");
                ImGui::GetForegroundDrawList()->AddText(tab_text3, 16 * dpi_scale, ImVec2(790 * dpi_scale + p.x, 765 * dpi_scale + p.y), ImColor(255, 255, 255, 255), "Release");
                tab_alpha = ImClamp(tab_alpha + (7.f * ImGui::GetIO().DeltaTime * (tabs == active_tab ? 1.f : -1.f)), 0.f, 1.f);
                tab_add = ImClamp(tab_add + (std::round(50.f) * ImGui::GetIO().DeltaTime * (tabs == active_tab ? 1.f : -1.f)), 0.f, 1.f);

                if (tab_alpha == 0.f && tab_add == 0.f) active_tab = tabs;


                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, tab_alpha * s.Alpha);

                ImGui::SetCursorPos(ImVec2(203 * dpi_scale, 30 * dpi_scale));

                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
                ImGui::BeginChild("General", ImVec2(717 * dpi_scale, 720 * dpi_scale), false);
                {
                    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetColorU32(colors::lite_color));
                    switch (active_tab) {

                    case 0:
                    {

                        ImGui::BeginChildPos("", ImVec2(620 * dpi_scale, 100 * dpi_scale));
                        {
                        ImGui::GetForegroundDrawList()->AddText(tab_text3, 26 * dpi_scale, ImVec2(475 * dpi_scale + p.x, 55 * dpi_scale + p.y), ImColor(255, 255, 255, 255), "Aimbot");
                        ImGui::GetForegroundDrawList()->AddText(tab_text3, 16 * dpi_scale, ImVec2(405 * dpi_scale + p.x, 85 * dpi_scale + p.y), ImColor(255, 255, 255, 255), "Play like a pro and stay unroticed");
                        }
                        ImGui::EndChild();
                        ImGui::SetCursorPosY(120 * dpi_scale);
                        ImGui::BeginChildPos("AimBot", ImVec2(300 * dpi_scale, 580 * dpi_scale));
                        {
                            ImGui::SetWindowFontScale(dpi_scale);
                            ImGui::Checkbox("Enable Ragebot", &var::checkbox);
                          //  ImGui::BindBox("Enable Legitbot", &var::checkbox, "Enables legitbot aimbot", &keybind);
                            ImGui::SliderInt("Field of view", &var::slider_intager, 0, 100, "%d%%", 0);
                        //    ImGui::ColorBox("Draw Bullet Trace", &var::checkboxIn, "Adds visual effects to bullet", &(float)*var::color_edit2);
                         //   ImGui::ColorEdit4("Color Picker", &(float)*var::color_edit2, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs);
                          //  ImGui::BindBox("Enable Legitbot", &var::checkbox, "Enables legitbot aimbot", &keybind);
                            ImGui::Combo("Bones selection", &combo, themes, IM_ARRAYSIZE(themes), 6);
                            static bool multi[5];
                            const char* multicombo_items[] = { "Selected 1", "Selected 2", "Selected 3", "Selected 4", "Selected 5" };
                            ImGui::MultiCombo("MultiCombo", multi, multicombo_items, IM_ARRAYSIZE(multicombo_items));

                            ImGui::Keybind("Binderbox", &key0, true);
                        }
                        ImGui::EndChild();
                        ImGui::SetCursorPos(ImVec2(320 * dpi_scale, 120 * dpi_scale));
                        ImGui::BeginChildPos("Misc", ImVec2(300 * dpi_scale, 580 * dpi_scale));
                        {
                            ImGui::SetWindowFontScale(dpi_scale);
                            static bool checboxes[40];
                            for (int i = 0; i < 40; i++)
                            {
                                ImGui::Text("Test1337:UI BY:hefanccc", checboxes);
                            }
                        }
                        ImGui::EndChild();

                    }
                    break;
                    case 2: {
                        ImGui::BeginChildPos("", ImVec2(620 * dpi_scale, 100 * dpi_scale));
                        {
                            ImGui::GetForegroundDrawList()->AddText(tab_text3, 26 * dpi_scale, ImVec2(475 * dpi_scale + p.x, 55 * dpi_scale + p.y), ImColor(255, 255, 255, 255), "Visuals");
                            ImGui::GetForegroundDrawList()->AddText(tab_text3, 16 * dpi_scale, ImVec2(425 * dpi_scale + p.x, 85 * dpi_scale + p.y), ImColor(255, 255, 255, 255), "Painting on Object Overlay");
                        }
                        ImGui::EndChild();
                        ImGui::SetCursorPosY(120 * dpi_scale);
                        ImGui::BeginChildPos("Esp", ImVec2(300 * dpi_scale, 580 * dpi_scale));
                        {
                            ImGui::SetWindowFontScale(dpi_scale);
                            ImGui::Checkbox("Enable Esp", &var::esp);
                            ImGui::Checkbox("Enable 2Dbox", &var::box);
                            ImGui::Checkbox("Enable Health bar", &var::hp);
                            ImGui::Checkbox("Enable Armor bar", &var::ar);
                            ImGui::Checkbox("Enable Ammo bar", &var::am);
                            ImGui::Checkbox("Enable Player name", &var::name);
                            if (&var::esp)
                                espkk();
                            ImGui::Combo("Bones selection", &combo, themes, IM_ARRAYSIZE(themes), 24);
                            ImGui::Keybind("Binderbox", &key0, true);
                            ImGui::SliderInt("Field of view", &var::slider_intager, 0, 100, "%d%%", 0);
                            ImGui::ColorEdit4("Colorpicker##0", (float*)&color_edit, ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs);

                            
                        }
                        ImGui::EndChild();
                        ImGui::SetCursorPos(ImVec2( 320 * dpi_scale, 120 * dpi_scale));
                        ImGui::BeginChildPos("Misc", ImVec2(300 * dpi_scale, 580 * dpi_scale));
                        {
                            ImGui::SetWindowFontScale(dpi_scale);
                            ImGui::Checkbox("Enable Esp", &var::esp1);
                            ImGui::Checkbox("Enable 2Dbox", &var::box1);
                            ImGui::Checkbox("Enable Health bar", &var::hp1);
                            ImGui::Checkbox("Enable Armor bar", &var::ar1);
                            ImGui::Checkbox("Enable Ammo bar", &var::am1);
                            ImGui::Checkbox("Enable Player name", &var::name1);
                            ImGui::Combo("Bones selection", &combo, themes, IM_ARRAYSIZE(themes), 6);
                            ImGui::Keybind("Binderbox", &key1, true);
                            ImGui::SliderInt("Field of view", &var::slider_intager, 0, 100, "%d%%", 0);

                        }
                        ImGui::EndChild();
                    }
                    break;
                    case 3: {
                        ImGui::BeginChildPos("", ImVec2(620 * dpi_scale, 100 * dpi_scale));
                        {
                            ImGui::GetForegroundDrawList()->AddText(tab_text3, 26 * dpi_scale, ImVec2(450 * dpi_scale + p.x, 55 * dpi_scale + p.y), ImColor(255, 255, 255, 255), "Miscellaneous");
                            ImGui::GetForegroundDrawList()->AddText(tab_text3, 16 * dpi_scale, ImVec2(390 * dpi_scale + p.x, 85 * dpi_scale + p.y), ImColor(255, 255, 255, 255), "Modify menu games and other functions");
                        }
                        ImGui::EndChild();
                        ImGui::SetCursorPosY(120 * dpi_scale);
                        ImGui::BeginChildPos("Misc", ImVec2(620 * dpi_scale, 580 * dpi_scale));
                        {ImGui::SetWindowFontScale(dpi_scale);
                            ImGui::Checkbox("Render animated background", &animated_background);
                            ImGui::SliderFloat("DPI", &dpi_scale, 1.0f, 1.2f, "%.3f", 0);


                        }
                        ImGui::EndChild();
                    }
                    break;
                    case 7:
                    {
                        ImGui::BeginChildPos("", ImVec2(620 * dpi_scale, 100 * dpi_scale));
                        {
                            ImGui::GetForegroundDrawList()->AddText(tab_text3, 26 * dpi_scale, ImVec2(475 * dpi_scale + p.x, 55 * dpi_scale + p.y), ImColor(255, 255, 255, 255), "ChatBox");
                            ImGui::GetForegroundDrawList()->AddText(tab_text3, 16 * dpi_scale, ImVec2(385 * dpi_scale + p.x, 85 * dpi_scale + p.y), ImColor(255, 255, 255, 255), "Chat online with other subscribed users");
                        }
                        ImGui::EndChild();
                        ImGui::SetCursorPosY(120 * dpi_scale);
                        ImGui::BeginChildPos("  ", ImVec2(620 * dpi_scale, 490 * dpi_scale));
                        {ImGui::SetWindowFontScale(dpi_scale);
                            static bool checboxes[40];
                            for (int i = 0; i < 40; i++)
                            {
                                ImGui::Text("Test1337:UI BY:hefanccc",checboxes);
                            }

                        }
                        ImGui::EndChild();
                        ImGui::BeginChild(" ", ImVec2(620 * dpi_scale, 70 * dpi_scale),false);
                        {ImGui::SetWindowFontScale(dpi_scale);
                            ImGui::SetCursorPosX(10 * dpi_scale);
                            ImGui::InputText("Enter chat content here", var::input, 64);
                            ImGui::SetCursorPos(ImVec2(500 * dpi_scale, 30 * dpi_scale));
                            ImGui::Button("Sending chat");
                        }
                        ImGui::EndChild();
                    }
                    break;
                    case 8:
                    {
                        ImGui::BeginChildPos("Search here for what you want", ImVec2(620 * dpi_scale, 700 * dpi_scale));
                        {ImGui::SetWindowFontScale(dpi_scale);
                            ImGui::GetForegroundDrawList()->AddText(ico, 30 * dpi_scale, ImVec2(770 * dpi_scale + p.x, 85 * dpi_scale + p.y), ImColor(93, 93, 93, 255), "I");
                            ImGui::SetCursorPosY(30 * dpi_scale);
                            ImGui::SetNextItemWidth(540 * dpi_scale);
                            ImGui::InputText(" ", var::input, 64);

                        }
                        ImGui::EndChild();
                    }
                    break;
                    }
                    ImGui::PopStyleColor(1);

                    ImGui::Spacing();

                    ImGui::EndChild();

                    ImGui::PopStyleColor(1);
                }
                if (animated_background)
                    Particles();
                ImGui::PopStyleVar(1);

            }
            ImGui::End();
        }
        ImGui::Render();
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0); // Present with vsync
        //g_pSwapChain->Present(0, 0); // Present without vsync
    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

#include "main_window.h"
#include "app.h"
#include "world.h"
#include "entity/entity.h"
#include "entity/components/c_camera.h"
#include "gameplay/camera_controller.h"

MainWindow::MainWindow(uint32_t width, uint32_t height)
    : Window(width, height)
{
}

MainWindow::~MainWindow()
{
}

void MainWindow::set_current_app(Application* app)
{
    _app = app;
}

bool show_demo_window = true;
void MainWindow::draw_ui()
{
    if (!_app)
        return;

    if (show_demo_window)
        ImGui::ShowDemoWindow(&show_demo_window);
    return;

    World* world = _app->get_world();
    CameraController* camera_controller = _app->get_camera_controller();

    ImGui::Begin("Settings");
    if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None))
    {
        if (ImGui::BeginTabItem("Camera"))
        {
            std::vector<Entity*>& cameras = world->get_camera_entities();
            for (int i = 0; i < cameras.size(); ++i)
            {
                CCamera* c_camera = cameras[i]->get_component<CCamera>();
                if (ImGui::TreeNode(cameras[i]->get_name().c_str()))
                {
                    bool is_display = enum_has_flags(c_camera->get_usage(), ViewUsageFlags::Display);
                    if (ImGui::Checkbox("Current display", &is_display))
                    {
                        Entity* controlled_camera = camera_controller->get_camera();
                        if (cameras[i] != controlled_camera && is_display)
                        {
                            camera_controller->set_camera(cameras[i]);

                            CCamera* controlled_c_camera = controlled_camera->get_component<CCamera>();
                            ViewUsageFlags uasge = controlled_c_camera->get_usage() & ~ViewUsageFlags::Display;
                            controlled_c_camera->set_uasge((ViewUsageFlags)uasge);

                            uasge = c_camera->get_usage() | ViewUsageFlags::Display;
                            c_camera->set_uasge((ViewUsageFlags)uasge);
                        }
                    }
                    ImGui::TreePop();
                }
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}
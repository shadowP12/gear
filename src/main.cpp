#include "app.h"

int main()
{
    ApplicationSetting app_setting;
    app_setting.window_pos_x = 400;
    app_setting.window_pos_y = 150;

    Application app;
    app.setup(app_setting);
    while (!app.should_close()) {
        app.run();
    }
    app.exit();
    return 0;
}
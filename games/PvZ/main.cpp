// ==============================
// CoCo PvZ — Plants vs Zombies Modifier 入口
// ==============================

#include "framework/OverlayApp.h"
#include "PvZFeature.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    coco::OverlayApp app;

    // 注册 PvZ 插件
    app.RegisterFeature(std::make_unique<coco::PvZFeature>(app.Logger()));

    // 启动主循环
    return app.Run(hInstance, nCmdShow);
}

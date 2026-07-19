// ==============================
// CoCo RPG — RPG Modifier 入口（模板）
// ==============================

#include "framework/OverlayApp.h"
#include "RPGFeature.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    coco::OverlayApp app;
    app.RegisterFeature(std::make_unique<coco::RPGFeature>());
    return app.Run(hInstance, nCmdShow);
}

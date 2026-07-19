// ==============================
// CoCo FPS — FPS Modifier 入口（模板）
// ==============================

#include "framework/OverlayApp.h"
#include "FPSFeature.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    coco::OverlayApp app;
    app.RegisterFeature(std::make_unique<coco::FPSFeature>());
    return app.Run(hInstance, nCmdShow);
}

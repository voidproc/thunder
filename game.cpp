#include "game.h"
#include "scene.h"
#include "scenemgr.h"


// 画面の解像度
const Size SCREEN_SIZE{ 200, 150 };

// 画面を描画する際の拡大率
const double SCALE = 3.0;

// グローバル変数： シーン／キャラ管理
SceneMgr gSceneMgr;


void initGraphics()
{
    Window::Resize(SCREEN_SIZE * (int)SCALE);
    Graphics2D::SetBlendState(BlendState::Default);
    Graphics2D::SetSamplerState(SamplerState::ClampPoint);
}

void Main()
{
    Window::SetTitle(L"THUNDER #Siv3DGameJam");

    // 普段の描画先：最後にSCALE倍する
    RenderTexture render_texture{ SCREEN_SIZE, Palette::Black };

    initGraphics();

    FontAsset::Register(L"title", 14, L"Times New Roman", FontStyle::BitmapBoldItalic);
    FontAsset::Register(L"sys",    6, L"Arial",       FontStyle::BitmapBold);
    FontAsset::Register(L"over",  10, L"Verdana",     FontStyle::BitmapBold);
    FontAsset::Register(L"wave",   8, L"Arial Black", FontStyle::Bitmap);
    FontAsset::Register(L"clear",  8, L"Verdana",     FontStyle::BitmapBold);

    TextureAsset::Register(L"a", L"Asset/a.png");

    gSceneMgr.add(new SceneTitle());

	while (System::Update()) {
        #ifdef _DEBUG
        // デバッグ表示
        ClearPrint();
        PutText(Profiler::FPS()).at(10, 440);
        #endif // _DEBUG

        // シーンを更新
        gSceneMgr.update();

        // シーンがなくなったらプログラム終了
        if (gSceneMgr.empty()) { break; }

        // シーンを描画
        Graphics2D::SetRenderTarget(render_texture);
        gSceneMgr.draw();

        // 拡大表示
        Graphics2D::SetRenderTarget(Graphics::GetSwapChainTexture());
        render_texture.scale(SCALE).draw();

        gSceneMgr.postUpdate();
    }
}

void addScene(Scene* scene)
{
    gSceneMgr.add(scene);
}

void eraseScene(SceneType scene_erase)
{
    gSceneMgr.erase(scene_erase);
}

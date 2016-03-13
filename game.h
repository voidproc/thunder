#pragma once

// 画面の解像度
extern const Size SCREEN_SIZE;

// 画面を描画する際の拡大率
extern const double SCALE;

// シーン操作
class Scene;
enum class SceneType;
void addScene(Scene* scene);
void eraseScene(SceneType scene_erase);

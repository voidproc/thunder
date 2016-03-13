#pragma once

class Actor;

// 稲妻のようなギザギザの線を描画する
void drawLightning(const Vec2& p0, const Vec2& p1, const int div, const Color& color = Palette::White);

// アクターからアクターを狙う角度 [deg]
const double aimAngle(Actor* src, Actor* dst);

// なにか方向キーが押されているか
const bool isAnyArrowKeyClicked();

// 画面外かどうか
const bool isOffscreen(const Vec2& pos);

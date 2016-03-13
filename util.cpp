#include "util.h"
#include "actor.h"
#include "game.h"


void drawLightning(const Vec2& p0, const Vec2& p1, const int div, const Color& color)
{
    // 始点と終点をdiv分割した線分
    Vec2 vec_part = (p1 - p0).normalized() * (p1.distanceFrom(p0) / div);

    // 各点の位置を決める：支店と終点以外はランダムに動かしギザギザにする
    vector<Vec2> joints(div+1);
    joints[0] = p0;
    joints[div] = p1;
    for (int i = 1; i < div; i++) {
        joints[i] = p0 + vec_part * i + RandomVec2(8.0);
    }

    for (int i = 1; i < div+1; i++) {
        Line(joints[i - 1], joints[i]).draw(Random(1, 2), color);
    }
}

// アクターからアクターを狙う角度 [deg]
const double aimAngle(Actor* src, Actor* dst)
{
    Vec2 v = dst->pos_ - src->pos_;
    return Degrees(atan2(v.x, -v.y));
}

const bool isAnyArrowKeyClicked()
{
    return Input::KeyLeft.clicked || Input::KeyRight.clicked || Input::KeyUp.clicked || Input::KeyDown.clicked;
}

const bool isOffscreen(const Vec2& pos)
{
    return !Rect(-32, -32, SCREEN_SIZE.x + 32 * 2, SCREEN_SIZE.y + 32 * 2).intersects(pos.asPoint());
}

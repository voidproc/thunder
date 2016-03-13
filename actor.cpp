#include "actor.h"
#include "actormgr.h"
#include "game.h"
#include "util.h"

// Player
// パワー最大値
const int POWER_MAX = 10000;
// パワー消費量[/frame]
const int POWER_USE = 40;
// パワー使用可能最低値（これ以上低いとパワー0になる）
const int POWER_USE_LIM = 1000;
// パワー自動回復量[/frame]
const int POWER_AUTO_RECOVER = 3;
// ライフ最大値
const int LIFE_MAX = 100;
// ダメージ時無敵時間[frame]
const int DAMAGE_INV_TIME = 80;


// 引数として
// (1) Actorの現在のフレームカウンタ
// (2) { 開始フレーム, 終了フレーム }のペアのvector
// を与えると、現在(2)のうちどのフレーム範囲にいるのか（vectorのインデックス）を戻り値に返す
// 良い名前が浮かばない．
const int currentActorBehaviorFrameRange(const int frame, vector<pair<int, int>>& ranges)
{
    int index = 0;
    for (auto& range : ranges) {
        if (frame >= range.first && frame <= range.second) {
            break;
        }
        index++;
    }
    return index;
}

// キャラのステータス設定
unordered_map<ActorType, Actor::Attr> Actor::attr_ = {
    //                       life, score, collision
    { ActorType::Player,    {  100,     0, true, { 0, 0, 30, 20 } } },
    { ActorType::Lightning, {    1,     0, true, { 0, 0,  6,  0 } } },
    { ActorType::Enemy2,    {    1,   100, true, { 0, 0,  8,  8 } } },
    { ActorType::Enemy3,    {    1,   100, true, { 0, 0, 12, 12 } } },
    { ActorType::Enemy4,    {   10,   500, true, { 0, 0, 16, 16 } } },
    { ActorType::Enemy5,    {   20,  1000, true, { 0, 0, 16, 16 } } },
    { ActorType::Enemy6,    {  230, 10000, true, { 0, 0, 32, 32 } } },
    { ActorType::Bullet1,   {    1,    50, true, { 0, 0,  2,  2 } } },
    { ActorType::Bullet2,   { 9999,  9999, true, { 0, 0,  2,  2 } } },
    { ActorType::FxExplode, {    1,     0, false,{ 0, 0,  0,  0 } } },
    { ActorType::FxRatio,   {    1,     0, false,{ 0, 0,  0,  0 } } },
};

unordered_map<ActorType, std::function<void(Actor*, ActorMgrMap*)>> Actor::funcUpdate_ = {
    {
        ActorType::Enemy2,
        [](Actor* actor, ActorMgrMap* mgr)
        {
            // 左右から現れ等速移動する
            vector<pair<int, int>> frame_ranges{ { 0, 0 }, { 1, 9999 }, };
            const int index = currentActorBehaviorFrameRange(actor->frame(), frame_ranges);

            switch (index) {
            case 0:
                actor->speed_ = 1.5;
                actor->angle_ = 80.0;
                if (actor->pos_.x > SCREEN_SIZE.x / 2)
                    actor->angle_ = -80.0;
                break;
            case 1:
                break;
            }
        }
    },
    {
        ActorType::Enemy3,
        [](Actor* actor, ActorMgrMap* mgr)
        {
            // 風船
            vector<pair<int, int>> frame_ranges{ { 0, 0 },{ 1, 9999 }, };
            const int index = currentActorBehaviorFrameRange(actor->frame(), frame_ranges);

            switch (index) {
            case 0:
                actor->speed_ = Random(0.8, 1.2);
                actor->angle_ = 0;
                break;
            case 1:
                actor->pos_.x += 0.12 * sin(actor->frame() / 20.0);
                break;
            }
        }
    },
    {
        ActorType::Enemy4,
        [](Actor* actor, ActorMgrMap* mgr)
        {
            // UFO
            vector<pair<int, int>> frame_ranges{ { 0, 0 },{ 1, 100 },{ 101, 101 },{102, 150},{151, 151},{152, 9999} };
            const int index = currentActorBehaviorFrameRange(actor->frame(), frame_ranges);

            switch (index) {
            case 0:
                actor->speed_ = 1.6;
                actor->angle_ = 90.0;
                if (actor->pos_.x > SCREEN_SIZE.x / 2)
                    actor->angle_ = -90.0;
                break;
            case 1:
                actor->pos_.y += 0.5 * sin(actor->frame() / 12.0);
                if (actor->frame() == 50) {
                    Enemy* bullet = new Enemy(mgr, ActorType::Bullet1);
                    bullet->pos_.set(actor->pos_);
                    bullet->speed_ = 2.2;
                    bullet->angle_ = aimAngle(actor, mgr->at(L"player")->back());
                    mgr->at(L"enemy")->add(bullet);
                }
                break;
                break;
            case 2:
                actor->speed_ = 0;
                {
                    Enemy* bullet = new Enemy(mgr, ActorType::Bullet1);
                    bullet->pos_.set(actor->pos_);
                    bullet->speed_ = 2.2;
                    bullet->angle_ = aimAngle(actor, mgr->at(L"player")->back());
                    mgr->at(L"enemy")->add(bullet);
                }
                break;
            case 3:
                break;
            case 4:
                actor->speed_ = 1.6;
                break;
            case 5:
                actor->pos_.y += 0.5 * sin(actor->frame() / 12.0);
                break;

            }
        }
    },
    {
        ActorType::Enemy5,
        [](Actor* actor, ActorMgrMap* mgr)
        {
            // UFO(2)
            vector<pair<int, int>> frame_ranges{ { 0, 0 },{ 1, 60 },{ 61, 61 },{ 62, 122 },{ 123, 123 },{ 124, 9999 } };
            const int index = currentActorBehaviorFrameRange(actor->frame(), frame_ranges);

            switch (index) {
            case 0:
                actor->speed_ = 1.4;
                actor->angle_ = 100.0;
                if (actor->pos_.x > SCREEN_SIZE.x / 2)
                    actor->angle_ = -100.0;
                break;
            case 1:
                break;
            case 2:
                actor->speed_ = 0;
                break;
            case 3:
                if (actor->frame() % 30 == 0) {
                    for (int i : step(8)) {
                        Enemy* bullet = new Enemy(mgr, ActorType::Bullet1);
                        bullet->pos_.set(actor->pos_);
                        bullet->speed_ = 2.5;
                        bullet->angle_ = 360.0 * i / 8.0;
                        mgr->at(L"enemy")->add(bullet);
                    }
                }
                break;
            case 4:
                actor->angle_ = aimAngle(actor, mgr->at(L"player")->back());
                break;
            case 5:
                actor->speed_ = Clamp(actor->speed_ + 2.5/60, 0.0, 2.5);
                break;
            }
        }
    },
    {
        ActorType::Enemy6,
        [](Actor* actor, ActorMgrMap* mgr)
        {
            // Boss
            vector<pair<int, int>> frame_ranges{ { 0, 0 },{ 1, 180 },{ 181, 181 },{ 182, 362 },{ 363, 363 },{ 364, 9999 } };
            const int index = currentActorBehaviorFrameRange(actor->frame(), frame_ranges);

            switch (index) {
            case 0:
                actor->speed_ = 0.3;
                actor->angle_ = 0;
                break;
            case 1:
                break;
            case 2:
                actor->speed_ = 0;
                break;
            case 3:
                if (actor->frame() % 60 == 2) {
                    for (int i : step(8)) {
                        Enemy* bullet = new Enemy(mgr, ActorType::Bullet1);
                        bullet->pos_.set(actor->pos_);
                        bullet->speed_ = 2.8;
                        bullet->angle_ = 360.0 * i / 8.0;
                        mgr->at(L"enemy")->add(bullet);
                    }
                }
                break;
            case 4:
                actor->angle_ = 0;
                break;
            case 5:
                actor->speed_ = Clamp(actor->speed_ + 0.7 / 300, 0.0, 0.7);
                actor->pos_.x += 0.9 * sin(actor->frame() / 30.0);
                if (actor->frame() % 80 == 2) {
                    for (int i : step(8)) {
                        Enemy* bullet = new Enemy(mgr, ActorType::Bullet1);
                        bullet->pos_.set(actor->pos_);
                        bullet->speed_ = 2.8;
                        bullet->angle_ = 360.0 * i / 8.0;
                        mgr->at(L"enemy")->add(bullet);
                    }
                }
                break;
            }
        }
    },
    {
        ActorType::FxExplode,
        [](Actor* actor, ActorMgrMap* mgr)
        {
            // 爆発エフェクト
            vector<pair<int, int>> frame_ranges{ { 0, 0 },{ 1, 1 }, { 2, 9999 }, };
            const int index = currentActorBehaviorFrameRange(actor->frame(), frame_ranges);

            switch (index) {
            case 1:
                actor->speed_ = Random(0.1, 1.5);
                actor->angle_ = Random(360.0);
                break;
            case 2:
                if (actor->frame() > 25)
                    actor->dead();
                break;
            }
        }
    },
    {
        ActorType::FxRatio,
        [](Actor* actor, ActorMgrMap* mgr)
        {
            // 倍率エフェクト
            vector<pair<int, int>> frame_ranges{ { 0, 0 },{ 1, 1 },{ 2, 9999 }, };
            const int index = currentActorBehaviorFrameRange(actor->frame(), frame_ranges);

            switch (index) {
            case 1:
                actor->speed_ = 0.15;
                actor->angle_ = 0.0;
                actor->pos_.moveBy(RandomVec2(4.0));
                break;
            case 2:
                if (actor->frame() > 100)
                    actor->dead();
                break;
            }
        }
    },
    {
        ActorType::Bullet1,
        [](Actor* , ActorMgrMap*)
        {
        }
    },
    {
        ActorType::Bullet2,
        [](Actor* , ActorMgrMap*)
        {
        }
    },

};

unordered_map<ActorType, std::function<void(Actor*)>> Actor::funcDraw_ = {
    {
        ActorType::Enemy2,
        [](Actor* actor)
        {
            TextureAsset(L"a")(0+16*((actor->frame() / 15) % 2), 48, 16, 16).drawAt(actor->pos_, actor->color());
        }
    },
    {
        ActorType::Enemy3,
        [](Actor* actor)
        {
            TextureAsset(L"a")(0 + 16 * ((actor->frame() / 15) % 3), 64, 16, 32).drawAt(actor->pos_ + Vec2(0, 8), actor->color());
        }
    },
    {
        ActorType::Enemy4,
        [](Actor* actor)
        {
            TextureAsset(L"a")(48, 32, 32, 32).drawAt(actor->pos_, actor->color());
        }
    },
    {
        ActorType::Enemy5,
        [](Actor* actor)
        {
            TextureAsset(L"a")(48, 64, 32, 32).drawAt(actor->pos_, actor->color());
        }
    },
    {
        ActorType::Enemy6,
        [](Actor* actor)
        {
            TextureAsset(L"a")(48, 64, 32, 32).scale(2.0).drawAt(actor->pos_, actor->color());
        }
    },
    {
        ActorType::Bullet1,
        [](Actor* actor)
        {
            const int f = actor->frame() / 2;
            Circle(actor->pos_, 5).draw(Color(255, 80 + 175 * (f % 2), 80 + 175 * (f % 2)));
            Circle(actor->pos_, 5).drawFrame(1.0, 1.0, Palette::Red);
        }
    },
    {
        ActorType::Bullet2,
        [](Actor* actor)
        {
            const int f = actor->frame() / 2;
            Circle(actor->pos_, 5).draw(Color(80 + 175 * (f % 2), 80 + 175 * (f % 2), 255));
            Circle(actor->pos_, 5).drawFrame(1.0, 1.0, Palette::Blue);
        }
    },
    {
        ActorType::FxExplode,
        [](Actor* actor)
        {
            Circle c(actor->pos_, 2 + 4 * (1.0 - actor->frame() / 35.0) + Random(3.0));
            c.drawFrame(1.0/(1+actor->frame()/10.0), 1.0, Palette::White);
            if (RandomBool(0.2))
                c.draw();
        }
    },
    {
        ActorType::FxRatio,
        [](Actor* actor)
        {
            if ((actor->frame() / 3) % 2 == 0) {
                FontAsset(L"sys").drawCenter(Format(PyFmt, L"X{:.1f}", actor->intparam_ / 1000.0), actor->pos_, Palette::Yellow);
            }
        }
    },
};


Actor::Actor(ActorMgrMap* mgr, const ActorType type)
    :
    collidable_(), pos_(), speed_(), angle_(), hitArea_(), intparam_(),
    mgr_(mgr),
    type_(type), frame_(), dead_(false), damaged_(false), life_(), score_()
{
    Attr& attr = attr_[type];
    life_       = attr.life;
    score_      = attr.score;
    collidable_ = attr.collidable;
}

Actor::~Actor()
{
}

void Actor::postUpdate()
{
    frame_++;
    damaged_ = false;
}

void Actor::damage(const int val)
{
    life_ -= val;
    if (life_ <= 0) { dead(); }

    damaged_ = true;
}

const bool Actor::isCollide(Actor * dst)
{
    return hitArea_.intersects(dst->hitArea_);
}

void Actor::updateHitArea()
{
    hitArea_ = attr_[type()].hitArea;
    hitArea_.moveBy(pos_.asPoint());
    hitArea_.moveBy(-hitArea_.w / 2, -hitArea_.h / 2);
}

Enemy::Enemy(ActorMgrMap* mgr, const ActorType type) : Actor(mgr, type)
{
}

Enemy::~Enemy()
{
}

void Enemy::update()
{
    funcUpdate_[type()](this, mgr_);

    pos_.moveBy(speed_ * sin(Radians(angle_)), -speed_ * cos(Radians(angle_)));

    if (isOffscreen(pos_)) { dead(); }

    updateHitArea();
}

void Enemy::draw()
{
    funcDraw_[type()](this);
}

void Enemy::postUpdate()
{
    Actor::postUpdate();
}

Fx::Fx(ActorMgrMap * mgr, const ActorType type) : Actor(mgr, type)
{
}

Fx::~Fx()
{
}

void Fx::update()
{
    funcUpdate_[type()](this, mgr_);

    pos_.moveBy(speed_ * sin(Radians(angle_)), -speed_ * cos(Radians(angle_)));
}

void Fx::draw()
{
    funcDraw_[type()](this);
}

void Fx::postUpdate()
{
    Actor::postUpdate();
}

Player::Player(ActorMgrMap* mgr) : Actor(mgr, ActorType::Player) , intputFreeTime_(0), damageInvTime_(0), power_(POWER_MAX)
{
    pos_.set((SCREEN_SIZE / 2.0).movedBy(0, -40));
}

Player::~Player()
{
}

void Player::update()
{
    int dir_x = 0, dir_y = 0;
    if (Input::KeyLeft.pressed)  dir_x = -4;
    if (Input::KeyRight.pressed) dir_x = +4;
    if (Input::KeyUp.pressed)    dir_y = -2;
    if (Input::KeyDown.pressed)  dir_y = +2;

    // 斜め入力か？
    double k_diag = 0.25;
    if (dir_x && dir_y) { k_diag /= sqrt(2.0); }

    pos_ += Vec2(dir_x, dir_y) * k_diag;
    pos_.x = Clamp(pos_.x, 5.0, SCREEN_SIZE.x - 5.0);
    pos_.y = Clamp(pos_.y, 5.0, SCREEN_SIZE.y - 5.0);

    // 方向入力があるとイオン力が高まる(?
    if (!Input::KeySpace.pressed && isAnyArrowKeyClicked()) {
        power_ = Clamp(power_ + 200, 0, POWER_MAX);
    }

    // 入力が一定期間なければ無敵に
    if (dir_x || dir_y || Input::KeySpace.pressed) { intputFreeTime_ = 0; }
    collidable_ = (intputFreeTime_++ < 16);

    // ダメージを受けたら無敵に
    collidable_ = collidable_ && (damageInvTime_ == 0);

    // いなずま
    if (Input::KeySpace.pressed) {
        if (power_ >= POWER_USE_LIM) {
            if (mgr_->at(L"weapon")->empty()) {
                mgr_->at(L"weapon")->add(new Lightning(mgr_));
            }
            power_ = Clamp(power_ - POWER_USE, 0, POWER_MAX);
            if (power_ <= POWER_USE_LIM) {
                power_ = 0;
            }
        }
    }

    updateHitArea();
}

void Player::draw()
{
    // 雲
    int gray = 255;
    if (!mgr_->at(L"weapon")->empty()) { gray -= 100 * ((frame() / 2) % 2); }
    int alpha = 255;
    if (!collidable_) { alpha = 128 - 128 * ((frame() / 3) % 2); }
    TextureAsset(L"a")(0 + 48 * ((frame() / 30) % 2), 0, 48, 32).drawAt(pos_, Color(gray, alpha));

    // 稲妻発射エフェクト
    if (!mgr_->at(L"weapon")->empty()) {
        Circle(pos_.movedBy(0.0, 10.0), Random(6.0, 15.0)).drawFrame(2.0, 0.0, Color(255 - 50 * (frame() % 2)));
    }

    // チャージ
    if (!Input::KeySpace.pressed && isAnyArrowKeyClicked()) {
        drawLightning(pos_.movedBy(RandomVec2(10.0)), pos_.movedBy(RandomVec2(10.0)), 6, Palette::Limegreen);
    }
}

void Player::postUpdate()
{
    damageInvTime_ = Clamp(damageInvTime_ - 1, 0, DAMAGE_INV_TIME);
    power_ = Clamp(power_ + POWER_AUTO_RECOVER, 0, POWER_MAX);

    Actor::postUpdate();
}

void Player::damage(const int val)
{
    Actor::damage(val);
    damageInvTime_ = DAMAGE_INV_TIME;
}

void Player::recover(const int val)
{
    life_ = Clamp(life_ + val, 0, LIFE_MAX);
}

Lightning::Lightning(ActorMgrMap* mgr) : Actor(mgr, ActorType::Lightning)
{

}

Lightning::~Lightning()
{
}

void Lightning::update()
{
    Vec2 player_pos = mgr_->at(L"player")->back()->pos_;
    pos_ = player_pos;

    if (Input::KeySpace.released) {
        dead();
    }

    if (((Player*)mgr_->at(L"player")->back())->power() <= 1000) {
        dead();
    }

    // 衝突判定を設定
    // プレイヤー位置から地面まで
    hitArea_.set(pos_.x-12.0/2, pos_.y, 12.0, SCREEN_SIZE.y - pos_.y - 20.0);
}

void Lightning::draw()
{
    drawLightning(pos_, Vec2(pos_.x + Random(-5.0, 5.0), SCREEN_SIZE.y - 20.0 + Random(-5.0, 5.0)), 8, Color(255 - 20 * (System::FrameCount() % 2)));
}

void Lightning::postUpdate()
{
    Actor::postUpdate();
}


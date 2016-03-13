#pragma once

// Player
// パワー最大値
extern const int POWER_MAX;
// ライフ最大値
extern const int LIFE_MAX;
// パワー使用可能最低値（これ以上低いとパワー0になる）
extern const int POWER_USE_LIM;


enum class ActorType
{
    None,
    Player,
    Lightning,
    Enemy2,
    Enemy3,
    Enemy4,
    Enemy5,
    Enemy6,
    Bullet1,
    Bullet2,
    FxExplode,
    FxRatio,

};


// シーンに現れるキャラクターを表す

class ActorMgr;
using ActorMgrMap = unordered_map<String, unique_ptr<ActorMgr>>;

class Actor
{
public:
    Actor(ActorMgrMap* mgr, const ActorType type);
    virtual ~Actor();

    virtual void update() = 0;
    virtual void draw() = 0;
    virtual void postUpdate();
    virtual void damage(const int val);

    const ActorType type()   { return type_; }
    const int       frame()  { return frame_; }
    const bool      isDead() { return dead_; }
    void            dead()   { dead_ = true; }
    const int       life()   { return life_; }
    const int       score()  { return score_; }
    const bool      isCollidable() { return collidable_; }
    const bool      isCollide(Actor* dst);

    // ダメージ時に画像を点滅させる用
    Color color() { return damaged_ ? Color(255, 255 - 255 *( System::FrameCount() % 2), 255 - 255 * (System::FrameCount() % 2)) : Palette::White; }

    // updateFuncとかから直接いじりたい：

    Vec2   pos_;       //
    double speed_;     //
    double angle_;     // 角度[deg]
    int    intparam_;  //

protected:
    void updateHitArea();  // 衝突判定用の図形を現在の位置に合わせる

    ActorMgrMap* mgr_;  // 
    int    life_;       // 
    bool   collidable_; // 衝突判定の対象にするか
    Rect   hitArea_;    // 衝突判定


    // ActorTypeに応じてここからデータを取り出して使う：

    struct Attr {
        int  life;
        int  score;
        bool collidable;
        Rect hitArea;
    };
    static unordered_map<ActorType, Attr> attr_;
    static unordered_map<ActorType, std::function<void(Actor*, ActorMgrMap*)>> funcUpdate_;
    static unordered_map<ActorType, std::function<void(Actor*)>> funcDraw_;

private:
    ActorType type_;
    int  frame_;    //
    bool dead_;     // 死んでる →ActorMgr::eraseDeadAll()で回収される
    bool damaged_;  // このフレームでダメージを受けたか
    int  score_;    //
};


// 敵
// ・プレイヤーへ攻撃するキャラクタ用
// ・画面外に出ると死ぬ

class Enemy : public Actor
{
public:
    Enemy(ActorMgrMap* mgr, const ActorType type);
    virtual ~Enemy();

    virtual void update() override;
    virtual void draw() override;
    virtual void postUpdate() override;
};


// エフェクト
// ・衝突判定を持たないキャラクタ用
// ・一定時間たつと自爆する

class Fx : public Actor
{
public:
    Fx(ActorMgrMap* mgr, const ActorType type);
    virtual ~Fx();

    virtual void update() override;
    virtual void draw() override;
    virtual void postUpdate() override;
};


// プレイヤー
// ・ユーザーの入力で動かすことができる

class Player : public Actor
{
public:
    Player(ActorMgrMap* mgr);
    virtual ~Player();

    virtual void update() override;
    virtual void draw() override;
    virtual void postUpdate() override;
    virtual void damage(const int val) override;

    const int power() { return power_; }
    void recover(const int val);

private:
    int intputFreeTime_;  // 入力がないときに加算され一定フレームを超えると無敵になる
    int damageInvTime_;   // ダメージを受けたときに加算され0を超えると無敵になる、毎フレーム減る
    int power_;           // 稲妻を撃つためのパワー、時間経過で回復する
};


// 稲妻（プレイヤーのウエポン）
// ・プレイヤーの位置に連動
// ・生存期間はプレイヤーの入力に連動

class Lightning : public Actor
{
public:
    Lightning(ActorMgrMap* mgr);
    virtual ~Lightning();

    virtual void update() override;
    virtual void draw() override;
    virtual void postUpdate() override;
};

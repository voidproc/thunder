#pragma once

class Player;


enum class SceneType
{
    None,
    Title,
    Main,
    GameOver,
    Clear,
    Quit,
};

class Scene
{
public:
    Scene(SceneType type);
    virtual ~Scene();

    virtual void update() = 0;
    virtual void draw() = 0;
    virtual void postUpdate() = 0;

    SceneType sceneType() { return type_; }

private:
    SceneType type_;

};


// タイトル画面
// ・ハイスコア等の記録読み込み
// ・ハイスコア、最終到達WAVEを表示
// ・いなずま
// ・キー入力でMainシーンへ

class SceneTitle : public Scene
{
public:
    SceneTitle();
    virtual ~SceneTitle();

    virtual void update() override;
    virtual void draw() override;
    virtual void postUpdate() override;

private:
    int hiscore_;  //ハイスコア
    int wave_;     //最終到達WAVE
};


// メインのゲームシーン
// ・プレイヤーのライフがなくなるとGameOverシーンへ
// ・クリアするとClearシーンへ
// ・シーン移動時ハイスコア等の更新

class Actor;
class ActorMgr;

class SceneMain : public Scene
{
public:
    SceneMain(const int hiscore, const int waveMax);
    virtual ~SceneMain();

    virtual void update() override;
    virtual void draw() override;
    virtual void postUpdate() override;

private:
    void initActorMgr();
    void updateWave();            // 現在のフレームカウンタに応じてWAVEを設定する
    void checkPlayerCollision();  // プレイヤーがなにかに衝突する判定
    void checkEnemyCollision();   // 敵がウエポンに衝突する判定
    void updateHiscore();         // ハイスコア記録ファイルを更新する
    void updateWaveMax();         // 最終到達WAVEを更新する
    Player* player();

    int frame_;
    int hiscore_;  // ハイスコア
    int score_;    // ゲームのスコア
    int ratio_;    // 得点倍率 [RATIO_MIN, RATIO_MAX]
    int waveMax_;  // 最終到達WAVE
    int wave_;     // 現在のWAVE

    // キャラクタをplayer,enemy,effect等のグループごとに管理する
    unordered_map<String, unique_ptr<ActorMgr>> actorMgr_;

};


// ゲームオーバー画面
// ・ハイスコア表示
// ・キー入力でMainシーンへ

class SceneGameOver : public Scene
{
public:
    SceneGameOver(const int score);
    virtual ~SceneGameOver();

    virtual void update() override;
    virtual void draw() override;
    virtual void postUpdate() override;

private:
    int frame_;
    int score_;

};


// クリア画面
// ・ハイスコア表示
// ・キー入力でMainシーンへ

class SceneClear : public Scene
{
public:
    SceneClear(const int score);
    virtual ~SceneClear();

    virtual void update() override;
    virtual void draw() override;
    virtual void postUpdate() override;

private:
    int frame_;
    int score_;

};

#include "scene.h"
#include "game.h"
#include "util.h"
#include "actor.h"
#include "actormgr.h"

// SceneMain
// 倍率の変動範囲
const int RATIO_MIN = 1000;
const int RATIO_MAX = 8000;
// 敵撃破時の倍率増加量
const int RATIO_ADD = 50;
// ライフ回復量
const int LIFE_RECOVER = 1;


Scene::Scene(SceneType type) : type_(type)
{
}

Scene::~Scene()
{
}

SceneTitle::SceneTitle() : Scene(SceneType::Title), hiscore_(), wave_(1)
{
    INIReader ini(L"hiscore.ini");
    if (!ini) { return; }

    hiscore_ = ini.get<int>(L"THUNDER.hiscore");
    wave_ = ini.getOr<int>(L"THUNDER.wave", 1);
}

SceneTitle::~SceneTitle()
{
}

void SceneTitle::update()
{
    if (Input::KeySpace.clicked || Input::KeyEnter.clicked) {
        addScene(new SceneMain(hiscore_, wave_));
        eraseScene(SceneType::Title);
    }
}

void SceneTitle::draw()
{
    Rect(SCREEN_SIZE).draw(Palette::Darkslateblue);

    if (System::FrameCount() % Random(6, 60) == 0) {
        drawLightning(Vec2(Random(SCREEN_SIZE.x), 0), Vec2(Random(SCREEN_SIZE.x), SCREEN_SIZE.y), 12, Palette::White);
    }

    FontAsset(L"title").drawCenter(L"T H U N D E R", (SCREEN_SIZE / 2.0).movedBy(2, -10+2), Color(0, 255*((System::FrameCount()/2)%2)));
    FontAsset(L"title").drawCenter(L"T H U N D E R", (SCREEN_SIZE / 2.0).movedBy(0, -10), Palette::White);
    FontAsset(L"sys").drawCenter(L"@voidproc", (SCREEN_SIZE / 2.0).movedBy(50, 10), Palette::White);
    FontAsset(L"sys").drawCenter(L"PRESS SPC or ENTER", Vec2(SCREEN_SIZE.x / 2, SCREEN_SIZE.y - 15), Color(255 - 30 * ((System::FrameCount() / 3) % 2)));
    FontAsset(L"sys").drawCenter(Format(PyFmt, L"hiscore:{:08d} wave:{}", hiscore_, wave_), Vec2(SCREEN_SIZE.x / 2, 8));
}

void SceneTitle::postUpdate()
{
}

SceneMain::SceneMain(const int hiscore, const int waveMax) : Scene(SceneType::Main), frame_(), hiscore_(hiscore), score_(), ratio_(RATIO_MIN), waveMax_(waveMax), wave_(), actorMgr_()
{
    initActorMgr();
    actorMgr_[L"player"]->add(new Player(&actorMgr_));
}

SceneMain::~SceneMain()
{
}

// キャラ（Actor）管理クラスのインスタンスをグループごとに作成する
void SceneMain::initActorMgr()
{
    actorMgr_.clear();

    // "player" : プレイヤー
    // "enemy"  : 敵や敵弾
    // "effect" : 当たり判定に関係ないもの（爆発エフェクトとか）
    // "weapon" : プレイヤーの攻撃（ショットやボム）
    vector<String> mgr_groups = { L"player", L"enemy", L"effect", L"weapon" };

    for (auto& grp : mgr_groups) {
        actorMgr_[grp] = unique_ptr<ActorMgr>(new ActorMgr());
    }
}

void SceneMain::checkPlayerCollision()
{
    if (!player()->isCollidable()) { return; }

    for (auto i = actorMgr_[L"enemy"]->begin(); i != actorMgr_[L"enemy"]->end(); i++) {
        if ((*i)->isCollide(player())) {
            // プレイヤーが被弾したので、ダメージと、倍率減少
            player()->damage(20);
            ratio_ = Clamp(RATIO_MIN + (ratio_ - RATIO_MIN) / 2, RATIO_MIN, RATIO_MAX);
            break;
        }
    }
}

void SceneMain::checkEnemyCollision()
{
    if (actorMgr_[L"weapon"]->empty()) { return; }

    for (auto i = actorMgr_[L"enemy"]->begin(); i != actorMgr_[L"enemy"]->end(); i++) {
        if (!(*i)->isCollidable()) { continue; }

        if ((*i)->isCollide(actorMgr_[L"weapon"]->back())) {
            (*i)->damage(1);

            // 撃破したか？
            if ((*i)->isDead()) {
                // (1)スコア加算
                int add_score = ((*i)->score() * ratio_ / RATIO_MIN) / 10 * 10;
                score_ += add_score;

                // (2)倍率エフェクト
                {
                    Fx* fx = new Fx(&actorMgr_, ActorType::FxRatio);
                    fx->pos_ = (*i)->pos_;
                    fx->intparam_ = ratio_;
                    actorMgr_[L"effect"]->add(fx);
                }

                // (3)倍率加算
                ratio_ += RATIO_ADD;

                // (4)爆発エフェクト発生
                for (int j : step(Random(4, 7))) {
                    Fx* fx = new Fx(&actorMgr_, ActorType::FxExplode);
                    fx->pos_ = (*i)->pos_;
                    actorMgr_[L"effect"]->add(fx);
                }

                // (5)ライフ回復
                player()->recover(LIFE_RECOVER);
            }
        }
    }
}

void SceneMain::updateHiscore()
{
    hiscore_ = score_;
}

void SceneMain::updateWaveMax()
{
    waveMax_ = wave_;
}

void SceneMain::writeINI()
{
    INIWriter ini(L"hiscore.ini");
    ini.write(L"THUNDER.hiscore", score_);
    ini.write(L"THUNDER.wave", waveMax_);
}

void SceneMain::updateWave()
{
    vector<int> wave_frames{
        0, 60 * 10, 60 * 20, 60 * 28, 60 * 35, 60 * 40, 60 * 55, 60 * 65, 60 * 75, 60 * 85,
        60 * 90, 60 * 100, 60 * 105, 60 * 113, 60 * 121, 60 * 131, 60 * 141, 60 * 160,
        INT_MAX };
    // wave = 1, 2, ...
    wave_ = -1;
    for (int i = 0; i < wave_frames.size() - 1; i++) {
        if (frame_ >= wave_frames[i] && frame_ < wave_frames[i + 1]) {
            wave_ = i + 1;
            break;
        }
    }

    auto make_enemy = [&](const ActorType type, const double x, const double y) {
        Enemy* enemy = new Enemy(&actorMgr_, type);
        enemy->pos_.set(x, y);
        actorMgr_[L"enemy"]->add(enemy);
    };

    auto make_enemy2 = [&](const int frm_itv, const int frm_wait) {
        if (frame_ % frm_itv == frm_wait) {
            make_enemy(ActorType::Enemy2, RandomSelect({ 0.0, 1.0 })*SCREEN_SIZE.x, SCREEN_SIZE.y / 2 + Random(-20, 60));
        }
    };
    auto make_enemy3 = [&](const int frm_itv, const int frm_wait) {
        if (frame_ % frm_itv == frm_wait) {
            make_enemy(ActorType::Enemy3, Random(10, SCREEN_SIZE.x - 10), SCREEN_SIZE.y + 30);
        }
    };
    auto make_enemy4 = [&](const int frm_itv, const int frm_wait) {
        if (frame_ % frm_itv == frm_wait) {
            make_enemy(ActorType::Enemy4, RandomSelect({ 0.0, 1.0 })*SCREEN_SIZE.x, SCREEN_SIZE.y / 2 + Random(-40, 40));
        }
    };
    auto make_enemy5 = [&](const int frm_itv, const int frm_wait) {
        if (frame_ % frm_itv == frm_wait) {
            make_enemy(ActorType::Enemy5, RandomSelect({ 0.0, 1.0 })*SCREEN_SIZE.x, SCREEN_SIZE.y / 2 + Random(0, 40));
        }
    };
    auto make_enemy6 = [&]() {
        make_enemy(ActorType::Enemy6, SCREEN_SIZE.x / 2, SCREEN_SIZE.y + 30);
    };

    switch (wave_) {
    case 1:
        make_enemy2(40, 0);
        break;
    case 2:
        make_enemy3(30, 0);
        break;
    case 3:
        make_enemy2(40, 0);
        make_enemy3(40, 20);
        break;
    case 4:
        make_enemy2(30, 0);
        make_enemy3(30, 15);
        break;
    case 5:
        ;
        break;
    case 6:
        make_enemy2(40, 0);
        make_enemy4(200, 150);
        break;
    case 7:
        make_enemy2(25, 0);
        make_enemy3(40, 20);
        make_enemy4(180, 150);
        break;
    case 8:
        make_enemy2(40, 0);
        make_enemy3(20, 10);
        make_enemy4(180, 150);
        break;
    case 9:
        make_enemy2(20, 0);
        make_enemy5(220, 200);
        break;
    case 10:
        make_enemy2(20, 0);
        make_enemy3(30, 15);
        make_enemy4(220, 50);
        make_enemy5(300, 200);
        break;
    case 11:
        ;
    case 12:
        make_enemy2(25, 0);
        make_enemy3(25, 13);
        make_enemy4(150, 75);
        make_enemy5(220, 120);
        break;
    case 13:
        make_enemy2(10, 0);
        break;
    case 14:
        make_enemy3(10, 0);
        break;
    case 15:
        make_enemy2(20, 0);
        make_enemy4(80, 0);
        break;
    case 16:
        make_enemy2(20, 0);
        make_enemy5(90, 0);
        break;
    case 17:
        make_enemy2(30, 0);
        make_enemy3(23, 10);
        make_enemy4(130, 75);
        make_enemy5(190, 120);
    case 18:
        if (frame_ == 60 * 160) {
            make_enemy6();
        }
        break;
    }
}

Player* SceneMain::player()
{
    return (Player*)actorMgr_[L"player"]->back();
}

void SceneMain::update()
{
    updateWave();

    actorMgr_[L"player"]->update();
    actorMgr_[L"enemy"]->update();
    actorMgr_[L"effect"]->update();
    actorMgr_[L"weapon"]->update();

    // 衝突判定
    checkPlayerCollision();
    checkEnemyCollision();

    // プレイヤーのライフが0になったらゲームオーバー
    if (player()->life() <= 0) {
        if (score_ > hiscore_) {
            updateHiscore();
        }
        if (wave_ > waveMax_) {
            updateWaveMax();
        }
        writeINI();
        addScene(new SceneGameOver(score_));
    }

    // 最終WAVEでオブジェクトがなくなったらクリア
    if (wave_ == 18) {
        if (actorMgr_[L"enemy"]->empty() && actorMgr_[L"effect"]->empty()) {
            if (score_ > hiscore_) {
                updateHiscore();
            }
            if (wave_ > waveMax_) {
                updateWaveMax();
            }
            writeINI();
            addScene(new SceneClear(score_));
        }
    }
}

void SceneMain::draw()
{
    // 背景
    TextureAsset(L"a")(0, 106, 200, 150).draw((frame_ / 3) % 200, 0);
    TextureAsset(L"a")(0, 106, 200, 150).draw((frame_ / 3) % 200 - 200, 0);
    TextureAsset(L"a")(0, 106, 200, 54).draw((frame_ / 10) % 200, 0);
    TextureAsset(L"a")(0, 106, 200, 54).draw((frame_ / 10) % 200 - 200, 0);
    Rect(0, 0, SCREEN_SIZE).draw(Color(0, 0, 0, 20));

    actorMgr_[L"weapon"]->draw();
    actorMgr_[L"player"]->draw();
    actorMgr_[L"enemy"]->draw();
    actorMgr_[L"effect"]->draw();

    // スコア表示
    FontAsset(L"sys").draw(Pad(score_, { 8, L'0' }), 1, 1);
    FontAsset(L"sys").draw(Format(PyFmt, L"X{:.1f}", ratio_ / (double)RATIO_MIN), 1, 10);

    // ライフ表示
    FontAsset(L"sys").draw(L"LIFE:", 55, 1);
    Rect(82, 4, 40, 8).draw(Palette::Black);
    Rect(82, 4, 40 * player()->life() / (double)LIFE_MAX, 8).draw(Palette::Orange);

    // パワー表示
    FontAsset(L"sys").draw(L"POW:", 127, 1);
    Rect(155, 4, 40, 8).draw(Palette::Black);
    Rect(155, 4, 40 * (std::max(player()->power()-POWER_USE_LIM, 0) / (double)(POWER_MAX-POWER_USE_LIM)), 8).draw(player()->power() > POWER_USE_LIM ? Palette::Greenyellow : Palette::Red);

    // WAVE
    FontAsset(L"wave").draw(Format(PyFmt, L"WAVE {}", wave_), 2, 134, Palette::Orangered);
}

void SceneMain::postUpdate()
{
    actorMgr_[L"player"]->postUpdate();
    actorMgr_[L"enemy"]->postUpdate();
    actorMgr_[L"effect"]->postUpdate();
    actorMgr_[L"weapon"]->postUpdate();

    actorMgr_[L"enemy"]->eraseDeadAll();
    actorMgr_[L"effect"]->eraseDeadAll();
    actorMgr_[L"weapon"]->eraseDeadAll();

    frame_++;

    ratio_ = Clamp(ratio_ - 1, RATIO_MIN, RATIO_MAX);
}

SceneGameOver::SceneGameOver(const int score) : Scene(SceneType::GameOver), frame_(), score_(score)
{
}

SceneGameOver::~SceneGameOver()
{
}

void SceneGameOver::update()
{
    if (frame_ <= 100) { return; }

    if (Input::KeySpace.clicked || Input::KeyEnter.clicked) {
        addScene(new SceneTitle());
        eraseScene(SceneType::Main);
        eraseScene(SceneType::GameOver);
    }
}

void SceneGameOver::draw()
{
    Rect(SCREEN_SIZE).draw(Color(0, 0, 0, 225));

    FontAsset(L"over").drawCenter(L"G A M E  O V E R", (SCREEN_SIZE / 2).movedBy(0, -10), Palette::White);
    FontAsset(L"sys").drawCenter(Format(PyFmt, L"SCORE: {}", score_), (SCREEN_SIZE / 2).movedBy(0, 10), Palette::White);

    if (frame_ <= 100) { return; }

    FontAsset(L"sys").drawCenter(L"PRESS SPC or ENTER", Vec2(SCREEN_SIZE.x / 2, SCREEN_SIZE.y - 15), Color(255-30*((frame_/3)%2)));
}

void SceneGameOver::postUpdate()
{
    frame_++;
}

SceneClear::SceneClear(const int score) : Scene(SceneType::Clear), frame_(), score_(score)
{
}

SceneClear::~SceneClear()
{
}

void SceneClear::update()
{
    if (frame_ <= 100) { return; }

    if (Input::KeySpace.clicked || Input::KeyEnter.clicked) {
        addScene(new SceneTitle());
        eraseScene(SceneType::Main);
        eraseScene(SceneType::Clear);
    }
}

void SceneClear::draw()
{
    Rect(SCREEN_SIZE).draw(Color(0, 20, 0, 225));

    FontAsset(L"clear").drawCenter(L"THANK YOU FOR PLAYING!!", (SCREEN_SIZE / 2).movedBy(0, -10), RandomColor());
    FontAsset(L"sys").drawCenter(Format(PyFmt, L"SCORE: {}", score_), (SCREEN_SIZE / 2).movedBy(0, 10), Palette::White);

    if (frame_ <= 100) { return; }

    FontAsset(L"sys").drawCenter(L"PRESS SPC or ENTER", Vec2(SCREEN_SIZE.x / 2, SCREEN_SIZE.y - 15), Color(255 - 30 * ((frame_ / 3) % 2)));
}

void SceneClear::postUpdate()
{
    frame_++;
}

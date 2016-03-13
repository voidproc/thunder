#pragma once

class Scene;
enum class SceneType;


// シーン管理クラス
// 
// シーンをリストに保持し、back()をアクティブなシーンとする
// アクティブなシーンに対してupdate()、draw()、postUpdate()を実行する
// それ以外のシーンに対してはdraw()のみ実行する

class SceneMgr
{
public:
    SceneMgr();
    ~SceneMgr();

    void add(Scene* s);  // シーンを追加しアクティブなシーンとする
    void erase(SceneType scene_erase);  // 指定されたシーンを削除する
    void clear();

    auto size() { return scenes_.size(); }
    const bool empty() { return scenes_.empty(); }
    
    void update();
    void draw();
    void postUpdate();

private:
    list<unique_ptr<Scene>> scenes_;

};

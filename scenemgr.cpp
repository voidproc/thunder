#include "stdafx.h"
#include "scenemgr.h"
#include "scene.h"


SceneMgr::SceneMgr() : scenes_()
{
}

SceneMgr::~SceneMgr()
{
}

void SceneMgr::add(Scene* s)
{
    scenes_.push_back(unique_ptr<Scene>(s));
}

void SceneMgr::erase(SceneType scene_erase)
{
    Erase_if(scenes_, [&](auto& scene) { return scene->sceneType() == scene_erase; });
}

void SceneMgr::clear()
{
    scenes_.clear();
}

void SceneMgr::update()
{
    if (!scenes_.empty())
        scenes_.back()->update();
}

void SceneMgr::draw()
{
    for (auto& s : scenes_) {
        s->draw();
    }
}

void SceneMgr::postUpdate()
{
    if (!scenes_.empty())
        scenes_.back()->postUpdate();
}

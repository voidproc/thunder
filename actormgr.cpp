#include "actormgr.h"
#include "actor.h"


ActorMgr::ActorMgr() : actors_()
{
}

ActorMgr::~ActorMgr()
{
}

void ActorMgr::add(Actor* actor)
{
    actors_.push_back(unique_ptr<Actor>(actor));
}

void ActorMgr::eraseDeadAll()
{
    Erase_if(actors_, [](auto& a) { return a->isDead(); });
}

void ActorMgr::clear()
{
    actors_.clear();
}

void ActorMgr::update()
{
    for (auto& a : actors_) {
        a->update();
    }
}

void ActorMgr::draw()
{
    for (auto& a : actors_) {
        a->draw();
    }
}

void ActorMgr::postUpdate()
{
    for (auto& a : actors_) {
        a->postUpdate();
    }
}

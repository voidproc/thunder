#pragma once


// キャラクターをリストに保持し一括操作する
// ・


class Actor;

class ActorMgr
{
public:
    ActorMgr();
    ~ActorMgr();

    void add(Actor* actor);
    void eraseDeadAll();
    void clear();
    Actor* back() { return actors_.back().get(); }

    auto begin() { return actors_.begin(); }
    auto end() { return actors_.end(); }

    auto size() { return actors_.size(); }
    const bool empty() { return actors_.empty(); }

    void update();
    void draw();
    void postUpdate();

private:
    list<unique_ptr<Actor>> actors_;

};

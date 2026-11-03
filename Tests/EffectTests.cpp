#include "Engine/Event/EventBus.h"
#include "Gameplay/Effects/EffectPool.h"
#include <cmath>
#include <iostream>
#include <limits>

int main()
{
    using namespace MyGame;
    int failures=0;
    const auto check=[&](bool ok,const char* label){if(!ok){std::cerr<<label<<'\n';++failures;}};
    EventBus bus;EffectPool effects;
    check(!bus.Publish({GameEventKind::ToolUsed,"bad",{std::numeric_limits<float>::quiet_NaN(),0},1}),"NaN event accepted");
    for(std::size_t i=0;i<EventBus::MaxPending;++i)
        check(bus.Publish({GameEventKind::ObjectHit,"rock",{10,10},1}),"event capacity too small");
    check(!bus.Publish({GameEventKind::ObjectHit,"rock",{10,10},1}),"unbounded event queue");
    const auto events=bus.Take();check(events.size()==EventBus::MaxPending && bus.Pending()==0,"event drain");
    effects.Consume(events);check(effects.Active().size()==EffectPool::Capacity,"effect cap");
    check(effects.Shake()<=4.0f,"shake cap");
    const auto initial=effects.Active().front().position;
    effects.Update(-1);effects.Update(std::numeric_limits<float>::quiet_NaN());
    check(effects.Active().front().position==initial,"invalid dt moved effect");
    for(int i=0;i<20;++i) effects.Update(0.1f);
    check(effects.Active().empty() && effects.Shake()==0,"effects did not expire");
    check(!effects.Spawn({EffectKind::Particle,{0,0},{0,0},0,-1,1}),"invalid lifetime accepted");
    check(bus.Publish({GameEventKind::ItemPicked,"seed",{5,5},1}),"bus did not recover after drain");
    effects.Consume(bus.Take());check(effects.Active().size()==3,"pickup effect count");
    effects.Clear();check(effects.Active().empty(),"clear did not reset pool");
    return failures?1:0;
}

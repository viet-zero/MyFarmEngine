#include "Gameplay/Graphics/RenderOrder.h"
#include <cmath>
#include <iostream>

int main()
{
    using namespace MyGame;
    int failures=0;
    const auto check=[&](bool value,const char* name){if(!value){std::cerr<<name<<'\n';++failures;}};
    RenderOrder order;
    order.Begin({0,0},{200,200});
    check(order.Submit(RenderKind::Actor,0,{80,20},{40,80},100),"player rejected");
    check(order.Submit(RenderKind::TreeCanopy,0,{50,0},{100,100},140),"tree rejected");
    check(order.Submit(RenderKind::Animal,0,{90,50},{30,40},120),"dog rejected");
    check(order.Submit(RenderKind::Pickup,0,{90,50},{20,20},100),"pickup rejected");
    check(!order.Submit(RenderKind::Actor,1,{300,10},{20,20},10),"offscreen object accepted");
    check(!order.Submit(RenderKind::Actor,2,{10,10},{20,20},NAN),"NaN depth accepted");
    order.Sort();
    const auto& entries=order.Entries();
    check(entries.size()==4,"wrong visible count");
    if(entries.size()==4)
        check(entries[0].kind==RenderKind::Actor && entries[1].kind==RenderKind::Pickup &&
              entries[2].kind==RenderKind::Animal && entries[3].kind==RenderKind::TreeCanopy,
              "Y order or tie stability");
    order.Begin({0,0},{200,200});
    check(order.Entries().empty(),"frame did not clear queue");
    check(!order.Submit(RenderKind::Actor,0,{10,10},{-1,20},10),"invalid bounds accepted");
    return failures?1:0;
}

#include "Gameplay/Systems/CollisionSystem.h"
#include <cmath>
#include <iostream>
#include <limits>

int main(int argc,char** argv)
{
    using namespace MyGame;
    int failures=0;
    const auto check=[&](bool ok,const char* name){if(!ok){std::cerr<<name<<'\n';++failures;}};
    WorldMap world;
    check(argc==3 && world.LoadObjects(argc==3?argv[1]:""),"load objects");
    check(argc==3 && world.LoadStaticColliders(argc==3?argv[2]:""),"load static");
    constexpr ColliderComponent body{{18,12}};
    check(world.CanOccupy(world.Spawn(Zone::Farm),body),"farm spawn");
    check(world.CanOccupy(world.Spawn(Zone::Town),body),"town spawn");
    check(world.CanOccupy(world.Spawn(Zone::LakeForest),body),"lake spawn");
    for(const auto zone:{Zone::Town,Zone::Farm,Zone::LakeForest})
        check(world.CanOccupy(world.Spawn(zone)+glm::vec2{-70,30},{{22,13}}),"dog teleport spawn");
    check(!world.CanOccupy({5,50},body),"body crossed left edge");
    check(!world.CanOccupy({5050,1200},body),"body entered lake");
    check(!world.CanOccupy({500,700},body),"body entered house");
    check(!world.CanOccupy({1024,1800},body),"body entered fountain");
    check(!world.CanOccupy({1980,1510},body),"body entered orchard trunk");
    check(!world.CanOccupy({2700,1210},body),"body entered fence");
    check(!world.HasLineOfSight({1300,700},{200,700}),"interact through building");
    check(world.HasLineOfSight({2200,2200},{2300,2200}),"clear interaction path");
    const auto edge=CollisionSystem::MoveAndSlide(world,{40,3000},{-1,0},300,0.1f,body);
    check(edge.x>=body.halfSize.x && edge.x<40,"edge clamp");
    const auto diagonal=CollisionSystem::MoveAndSlide(world,{2200,2200},{1,1},180,0.1f,body);
    check(std::abs(glm::length(diagonal-glm::vec2{2200,2200})-18.0f)<0.05f,"diagonal normalization");
    const auto slide=CollisionSystem::MoveAndSlide(world,{2570,1300},{1,1},180,0.1f,body);
    check(slide.x<2583 && slide.y>1300,"corner slide along fence");
    const auto blocked=CollisionSystem::MoveAndSlide(world,{2400,1210},{1,0},2000,0.1f,body);
    check(blocked.x<2582,"large step tunneled through fence");
    const auto still=CollisionSystem::MoveAndSlide(world,{2200,2200},{1,0},180,
        std::numeric_limits<float>::quiet_NaN(),body);
    check(still.x==2200 && still.y==2200,"NaN time moved actor");
    const DynamicCollider npc{{2300,2200},body};
    const auto dynamic=CollisionSystem::MoveAndSlide(world,{2250,2200},{1,0},300,0.1f,body,
                                                      std::span{&npc,1});
    check(dynamic.x<=2264,"actor overlap");
    check(!world.CanOccupy({4670,2600},body),"active tree");
    check(world.SetObjectActive("tree.oak.01",false) && world.CanOccupy({4670,2600},body),"removed tree");
    check(!world.CanOccupy({1980,1510},body),"fruit harvest must keep tree collider");
    return failures?1:0;
}

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
    check(!world.CanOccupy({640,1040},body),"body entered house");
    check(world.CanOccupy({1024,1800},body),"invisible fountain still blocks ground");
    check(!world.CanOccupy({1990,1578},body),"body entered orchard trunk");
    check(world.CanOccupy({1990,1740},body) && world.CanOccupy({1920,1770},{{22,13}}),
          "orchard debug spawn blocked");
    check(world.CanOccupy({2140,1450},body) && world.CanOccupy({2240,1480},{{22,13}}),
          "depth debug spawn blocked");
    check(!world.CanOccupy({2700,1256},body),"body entered fence");
    check(!world.HasLineOfSight({1300,1040},{200,1040}),"interact through building");
    check(world.HasLineOfSight({2200,2200},{2300,2200}),"clear interaction path");
    const auto edge=CollisionSystem::MoveAndSlide(world,{40,3000},{-1,0},300,0.1f,body);
    check(edge.x>=body.halfSize.x && edge.x<40,"edge clamp");
    const auto diagonal=CollisionSystem::MoveAndSlide(world,{2200,2200},{1,1},180,0.1f,body);
    check(std::abs(glm::length(diagonal-glm::vec2{2200,2200})-18.0f)<0.05f,"diagonal normalization");
    const auto slide=CollisionSystem::MoveAndSlide(world,{2615,1300},{1,1},180,0.1f,body);
    check(slide.x<=2622 && slide.y>1300,"corner slide along fence");
    const auto blocked=CollisionSystem::MoveAndSlide(world,{2440,1256},{1,0},2000,0.1f,body);
    check(blocked.x<=2622,"large step tunneled through fence");
    const auto still=CollisionSystem::MoveAndSlide(world,{2200,2200},{1,0},180,
        std::numeric_limits<float>::quiet_NaN(),body);
    check(still.x==2200 && still.y==2200,"NaN time moved actor");
    const DynamicCollider npc{{2300,2200},body};
    const auto dynamic=CollisionSystem::MoveAndSlide(world,{2250,2200},{1,0},300,0.1f,body,
                                                      std::span{&npc,1});
    check(dynamic.x<=2264,"actor overlap");
    check(!world.CanOccupy({4670,2655},body),"active tree");
    check(world.SetObjectActive("tree.oak.01",false) && world.CanOccupy({4670,2655},body),"removed tree");
    check(!world.CanOccupy({1990,1578},body),"fruit harvest must keep tree collider");
    check(world.CanOccupy({1980,1510},body),"canopy incorrectly blocks walking behind trunk");
    check(world.CanOccupy({500,700},body),"roof-only area still blocks ground");
    check(world.CanOccupy({2592,1400},body),"west road intersects fence");
    check(world.CanOccupy({3104,1256},body),"north entrance is not wide enough");
    check(!world.CanOccupy({2848,2210},body),"farm wall footprint missing");
    const auto approach=CollisionSystem::MoveAndSlide(world,{2848,2310},{0,-1},1000,0.1f,body);
    check(approach.y>=2272 && approach.y<2310,"player passed through house front wall");
    check(world.LoadPortals(std::filesystem::path(argv[1]).parent_path()/"Portals.csv"),"portal definitions failed");
    check(!world.CanOccupy({1900,2050},body),"closed gate allowed passage");
    check(world.SetPortalOpen("gate.town.01",true),"gate open request failed");
    check(!world.CanOccupy({1900,2050},body),"gate collision opened before animation completed");
    for(int i=0;i<3;++i) world.TickPortals(0.1f);
    check(world.CanOccupy({1900,2050},body),"open gate still blocked");
    check(!world.CanOccupy({1875,2050},body),"open gate hinge/post lost collision");
    const DynamicCollider inGate{{1900,2050},body};
    check(!world.SetPortalOpen("gate.town.01",false,std::span{&inGate,1}) &&
          world.Portal("gate.town.01")->open,"gate closed on an occupant");
    check(world.SetPortalOpen("gate.town.01",false) && !world.CanOccupy({1900,2050},body),"gate failed to close");
    check(world.SetPortalOpen("door.farm.home",true),"door open request failed");
    for(int i=0;i<3;++i) world.TickPortals(0.1f);
    check(world.CanOccupy({2788,2210},body),"door corridor did not open");
    check(!world.CanOccupy({2700,2210},body),"opening door removed adjacent house wall");
    const DynamicCollider inDoor{{2788,2210},body};
    check(!world.SetPortalOpen("door.farm.home",false,std::span{&inDoor,1}),"door closed on an occupant");
    check(world.SetPortalOpen("door.farm.home",false),"empty door did not close");
    return failures?1:0;
}

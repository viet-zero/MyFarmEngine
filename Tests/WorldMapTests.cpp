#include "Gameplay/World/WorldMap.h"
#include "Graphics/Camera2D.h"
#include <iostream>
#include <limits>

int main(int argc,char** argv)
{
    using namespace MyGame;
    int failures=0;
    const auto check=[&](bool ok,const char* label){if(!ok){std::cerr<<label<<'\n';++failures;}};
    WorldMap map;
    check(argc==3 && map.LoadObjects(argc==3?argv[1]:""),"object definitions did not load");
    check(argc==3 && map.LoadStaticColliders(argc==3?argv[2]:""),"static colliders did not load");
    check(map.TryCell(-1,0)==nullptr && map.TryCell(WorldMap::Width,0)==nullptr,"cell bounds");
    check(map.ZoneAt(map.Spawn(Zone::Town))==Zone::Town,"town spawn");
    check(map.ZoneAt(map.Spawn(Zone::Farm))==Zone::Farm,"farm spawn");
    check(map.ZoneAt(map.Spawn(Zone::LakeForest))==Zone::LakeForest,"lake spawn");
    check(map.CanMoveTo(map.Spawn(Zone::Farm)),"farm spawn blocked");
    check(!map.CanMoveTo({-1,50}) && !map.CanMoveTo({std::numeric_limits<float>::quiet_NaN(),50}),"invalid position accepted");
    check(!map.CanMoveTo({5050,1200}),"lake water accepted");
    check(!map.CanMoveTo({4670,2600}),"blocking tree accepted");
    check(map.SetObjectActive("tree.oak.01",false) && map.CanMoveTo({4670,2600}),"destroyed tree still blocked");
    check(!map.SetObjectActive("absent",false),"unknown object state changed");
    check(map.Objects().size()==7,"object definitions missing");
    for(const auto& object:map.Objects()) check(object.gameplayId!=object.visualId,"visual ID reused as gameplay ID");
    MyEngine::Camera2D camera;camera.SetWorldSize(WorldMap::Size());camera.Resize(1280,720);
    camera.SetPosition({-100,-100});
    const auto first=map.VisibleTiles(camera.Position(),camera.ViewSize());
    check(first.left==0 && first.top==0 && first.right<WorldMap::Width,"first viewport culling");
    camera.SetPosition({99999,99999});
    const auto last=map.VisibleTiles(camera.Position(),camera.ViewSize());
    check(last.right==WorldMap::Width && last.bottom==WorldMap::Height && last.left>0,"last viewport culling");
    check((first.right-first.left)*(first.bottom-first.top)<WorldMap::Width*WorldMap::Height/4,"view renders too many tiles");
    const auto chunks=map.VisibleChunks(camera.Position(),camera.ViewSize());
    check(chunks.right<=WorldMap::Width/WorldMap::ChunkTiles && chunks.bottom<=WorldMap::Height/WorldMap::ChunkTiles,
          "visible chunk range escaped world");
    const auto corner=map.ChunkAt(chunks.right-1,chunks.bottom-1);
    check(corner.tiles.right==WorldMap::Width && corner.tiles.bottom==WorldMap::Height,"last chunk bounds");
    return failures?1:0;
}

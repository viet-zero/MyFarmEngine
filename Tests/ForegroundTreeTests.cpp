#include "Gameplay/World/ForegroundTrees.h"
#include <filesystem>
#include <fstream>
#include <iostream>

int main(int argc,char** argv)
{
    using namespace MyGame;
    int failures=0;
    const auto check=[&](bool value,const char* name){if(!value){std::cerr<<name<<'\n';++failures;}};
    WorldMap world;
    check(argc==3 && world.LoadObjects(argc==3?argv[1]:""),"world objects missing");
    ForegroundTrees trees;
    check(argc==3 && trees.Load(argc==3?argv[2]:"",world),"tree overlay data missing");
    check(trees.Entries().size()==2 && trees.Contains("tree.apple.01") &&
          !trees.Contains("rock.field.01"),"tree lookup/count");
    const auto bad=std::filesystem::temp_directory_path()/"myfarm-bad-foreground-trees.csv";
    {std::ofstream file(bad);file<<"gameplay_id,canopy_x,canopy_y,width,height,foot_y,interact_x,interact_y,fruits\n"
        "tree.missing,100,100,200,200,300,100,300,3\n";}
    check(!trees.Load(bad,world) && trees.Entries().size()==2,"invalid tree replaced previous catalog");
    std::error_code error;std::filesystem::remove(bad,error);
    return failures?1:0;
}

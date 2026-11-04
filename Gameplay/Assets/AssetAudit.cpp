#include "AssetManifest.h"
#include "Gameplay/Graphics/VisualCatalog.h"
#include <fstream>
#include <iostream>

int main(int argc,char** argv)
{
    std::filesystem::path root=std::filesystem::path(MYFARM_SOURCE_ASSETS);
    std::filesystem::path manifest=std::filesystem::path(MYFARM_SOURCE_MANIFEST);
    std::filesystem::path report;
    std::filesystem::path visualsPath=std::filesystem::path(MYFARM_SOURCE_VISUALS);
    std::filesystem::path coveragePath=std::filesystem::path(MYFARM_SOURCE_COVERAGE);
    for(int i=1;i<argc;++i)
    {
        const std::string_view arg=argv[i];
        if(i+1>=argc) return 2;
        if(arg=="--root") root=MyEngine::Utf8Path(argv[++i]);
        else if(arg=="--manifest") manifest=MyEngine::Utf8Path(argv[++i]);
        else if(arg=="--visuals") visualsPath=MyEngine::Utf8Path(argv[++i]);
        else if(arg=="--coverage") coveragePath=MyEngine::Utf8Path(argv[++i]);
        else if(arg=="--report") report=MyEngine::Utf8Path(argv[++i]);
        else return 2;
    }
    MyGame::AssetManifest catalog;
    if(!catalog.Load(manifest,root))
    {
        for(const auto& e:catalog.Errors()) std::cerr<<e<<'\n';
        return 1;
    }
    std::ofstream output;
    if(!report.empty())
    {
        output.open(report,std::ios::binary);
        if(!output) return 2;
        output<<"asset_id,status,width,height,alpha_channel,alpha_min,alpha_max,result\n";
    }
    std::size_t ready=0,review=0,disabled=0,errors=catalog.Errors().size();
    for(const auto& error:catalog.Errors()) std::cerr<<error<<'\n';
    for(const auto& record:catalog.Records())
    {
        auto check=catalog.Validate(record);
        const bool held=check.reason=="needs_review" || check.reason=="disabled";
        MyEngine::ImageData image;
        if(check.valid)
        {
            ++ready; image=std::move(check.image);
            std::filesystem::path source;
            std::error_code e;
            if(!MyGame::AssetManifest::ResolveInside(root,record.source,"Original",source) ||
                !std::filesystem::is_regular_file(source,e))
            {++errors;std::cerr<<record.id<<": original_missing\n";}
        }
        else if(held)
        {
            if(record.status=="needs_review") ++review; else ++disabled;
            std::filesystem::path source;
            std::string message;
            if(!MyGame::AssetManifest::ResolveInside(root,record.source,"Original",source) ||
                !MyEngine::DecodePng(source,image,message))
            { ++errors;check.reason="original_decode_failed"; }
        }
        else { ++errors;std::cerr<<record.id<<": "<<check.reason<<'\n'; }
        if(output) output<<record.id<<','<<record.status<<','<<image.width<<','<<image.height<<','
            <<(image.alphaChannel?"true":"false")<<','<<static_cast<int>(image.alphaMin)<<','
            <<static_cast<int>(image.alphaMax)<<','<<(check.valid?"valid":check.reason)<<'\n';
    }
    std::cout<<"Assets: "<<catalog.Records().size()<<"; ready="<<ready<<"; needs_review="<<review
             <<"; disabled="<<disabled<<"; errors="<<errors<<'\n';
    MyGame::VisualCatalog visuals;
    visuals.Load(visualsPath);
    const auto issues=visuals.ValidateAgainst(catalog,coveragePath);
    for(const auto& issue:issues) std::cerr<<"Visual cross-check: "<<issue<<'\n';
    std::cout<<"Visual cross-check: "<<visuals.ClipCount()<<" clips; errors="<<issues.size()<<'\n';
    errors+=issues.size();
    if(output.is_open() && !output.good()) return 2;
    return errors==0?0:1;
}

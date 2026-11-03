#include "Gameplay/Assets/AssetManifest.h"
#include <array>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace
{
    int failures=0,checks=0;
    void Check(bool value,const std::string& message)
    {++checks;if(!value){++failures;std::cerr<<message<<'\n';}}
    void Big(std::vector<unsigned char>& bytes,std::uint32_t value)
    {for(int shift:{24,16,8,0})bytes.push_back(static_cast<unsigned char>(value>>shift));}
    void Chunk(std::vector<unsigned char>& out,const std::array<unsigned char,4>& type,const std::vector<unsigned char>& data)
    {
        Big(out,static_cast<std::uint32_t>(data.size()));
        const auto start=out.size();
        out.insert(out.end(),type.begin(),type.end());out.insert(out.end(),data.begin(),data.end());
        std::uint32_t crc=0xffffffffu;
        for(std::size_t i=start;i<out.size();++i)
        {
            crc^=out[i];
            for(int bit=0;bit<8;++bit)crc=(crc>>1)^((crc&1)?0xedb88320u:0);
        }
        Big(out,~crc);
    }
    // Small, deterministic PNG fixture encoder. This never touches project artwork.
    void Png(const std::filesystem::path& path,int width,int height,bool transparent=true)
    {
        std::vector<unsigned char> png={137,80,78,71,13,10,26,10},header;
        Big(header,static_cast<std::uint32_t>(width));Big(header,static_cast<std::uint32_t>(height));
        header.insert(header.end(),{8,6,0,0,0});Chunk(png,{'I','H','D','R'},header);
        std::vector<unsigned char> raw;
        for(int y=0;y<height;++y)
        {
            raw.push_back(0);
            for(int x=0;x<width;++x)raw.insert(raw.end(),{40,180,90,static_cast<unsigned char>(transparent && x==0?0:255)});
        }
        const auto n=static_cast<std::uint16_t>(raw.size());
        std::vector<unsigned char> zipped={0x78,0x01,0x01,static_cast<unsigned char>(n),static_cast<unsigned char>(n>>8),
            static_cast<unsigned char>(~n),static_cast<unsigned char>((~n)>>8)};
        zipped.insert(zipped.end(),raw.begin(),raw.end());
        std::uint32_t a=1,b=0;
        for(auto value:raw){a=(a+value)%65521;b=(b+a)%65521;}
        Big(zipped,(b<<16)|a);Chunk(png,{'I','D','A','T'},zipped);Chunk(png,{'I','E','N','D'},{});
        std::ofstream file(path,std::ios::binary);file.write(reinterpret_cast<const char*>(png.data()),static_cast<std::streamsize>(png.size()));
    }
    void Text(const std::filesystem::path& path,const std::string& value)
    {std::ofstream file(path,std::ios::binary);file<<value;}
    const std::string Header="asset_id,kind,source,export_path,columns,rows,frame_width,frame_height,anchor_x,anchor_y,has_alpha,status,first_frame,frame_count,margin_x,margin_y,spacing_x,spacing_y\n";
}

int main()
{
    const auto root=std::filesystem::temp_directory_path()/
        ("myfarm_asset_test_"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    if(!std::filesystem::create_directory(root)) throw std::runtime_error("temporary test directory is not unique");
    struct Cleanup {std::filesystem::path path;~Cleanup(){std::error_code e;std::filesystem::remove_all(path,e);}} cleanup{root};
    std::filesystem::create_directories(root/"Original");std::filesystem::create_directories(root/"Export");
    Png(root/"Export/image.png",4,4);
    std::filesystem::copy_file(root/"Export/image.png",root/"Original/image.png");
    const auto csv=root/"assets.csv";
    const std::string base="basic,character,Original/image.png,Export/image.png,1,1,4,4,0.5,1,true,ready,0,1,0,0,0,0";
    MyGame::AssetManifest manifest;
    const auto load=[&](std::string row)
    {
        Text(csv,Header+row+"\n");
        return manifest.Load(csv,root);
    };
    Check(load(base),"valid manifest did not load");
    const auto* found=manifest.Find("basic");
    Check(found!=nullptr,"record lookup failed");
    if(!found)return 1;
    const auto original=*found;
    auto validate=[&](MyGame::AssetRecord record,const char* reason)
    {
        const auto result=manifest.Validate(record);
        Check(!result.valid && result.reason==reason,std::string("expected rejection: ")+reason+"; got "+result.reason);
    };
    auto valid=manifest.Validate(original);
    Check(valid.valid && valid.image.width==4 && valid.image.alphaMin==0,"valid PNG rejected");
    auto atlasRecord=original;atlasRecord.columns=2;atlasRecord.rows=2;
    atlasRecord.frameWidth=2;atlasRecord.frameHeight=2;atlasRecord.frameCount=4;
    const auto uv=MyGame::AssetManifest::FrameUv(atlasRecord,3);
    Check(uv && *uv==std::array<float,4>{0.5f,0.5f,0.5f,0.5f},"atlas UV orientation failed");
    Check(!MyGame::AssetManifest::FrameUv(atlasRecord,4),"UV allowed a frame outside bounds");
    Check(!MyGame::AssetManifest::FrameUv(atlasRecord,-1),"UV allowed a negative frame");
    auto record=original;record.exportPath="Export/missing.png";validate(record,"file_missing_or_unreadable");
    Text(root/"Export/corrupt.png","not a PNG");record=original;record.exportPath="Export/corrupt.png";validate(record,"not_png");
    // Truncation has a genuine PNG signature, but no decodable image.
    {std::ofstream file(root/"Export/truncated.png",std::ios::binary);const unsigned char bytes[]={137,80,78,71,13,10,26,10};file.write(reinterpret_cast<const char*>(bytes),8);}
    record=original;record.exportPath="Export/truncated.png";validate(record,"invalid_png_or_decoded_size_limit");
    Png(root/"Export/odd.png",3,4);record=original;record.exportPath="Export/odd.png";record.columns=2;record.frameWidth=2;validate(record,"grid_dimensions_mismatch");
    record=original;record.columns=0;validate(record,"invalid_grid");
    record=original;record.marginX=-1;validate(record,"invalid_grid");
    record=original;record.marginX=1;validate(record,"grid_dimensions_mismatch");
    record=original;record.firstFrame=1;validate(record,"frame_out_of_bounds");
    record=original;record.frameCount=2;validate(record,"frame_out_of_bounds");
    record=original;record.anchorX=1.1f;validate(record,"invalid_anchor");
    record=original;record.anchorX=std::numeric_limits<float>::quiet_NaN();validate(record,"invalid_anchor");
    record=original;record.anchorY=std::numeric_limits<float>::infinity();validate(record,"invalid_anchor");
    record=original;record.exportPath="Export/../Original/image.png";validate(record,"invalid_path");
    record=original;record.exportPath="C:/image.png";validate(record,"invalid_path");
    record=original;record.source="../../outside.png";validate(record,"invalid_path");
#ifdef _WIN32
    record=original;record.exportPath="Export/"+std::string(1,static_cast<char>(0xff))+".png";
    validate(record,"invalid_path");
#endif
    record=original;record.status="ready_typo";validate(record,"invalid_status");
    record=original;record.status="needs_review";validate(record,"needs_review");
    record=original;record.status="disabled";validate(record,"disabled");
    Png(root/"Export/opaque.png",4,4,false);record=original;record.exportPath="Export/opaque.png";validate(record,"alpha_mismatch");
    record.kind="background";Check(manifest.Validate(record).valid,"opaque RGBA background incorrectly rejected");
    record.hasAlpha=false;validate(record,"alpha_mismatch");
    const auto unicode=std::filesystem::path(u8"nông trại ảnh.png");
    std::filesystem::copy_file(root/"Export/image.png",root/"Export"/unicode);
    record=original;record.exportPath="Export/nông trại ảnh.png";
    Check(manifest.Validate(record).valid,"Unicode image path failed");
    record=original;record.exportPath="Export\\image.png";Check(manifest.Validate(record).valid,"Windows separators failed");
    Check(load(base+"\n"+base) && manifest.Find("basic")==nullptr && !manifest.Errors().empty(),"duplicate IDs were allowed");
    Check(load("bad,character,Original/image.png,Export/image.png,1x,1,4,4,0.5,1,true,ready,0,1,0,0,0,0") &&
        manifest.Records().empty() && !manifest.Errors().empty(),"malformed integer accepted");
    Text(csv,"bad_header\n"+base);Check(!manifest.Load(csv,root),"invalid header accepted");
    Text(csv,Header+"\"unterminated\n");Check(manifest.Load(csv,root) && !manifest.Errors().empty(),"bad CSV quoting accepted");
    std::filesystem::copy_file(root/"Export/image.png",root/"Export/a,b.png");
    Check(load("basic,character,Original/image.png,\"Export/a,b.png\",1,1,4,4,0.5,1,true,ready,0,1,0,0,0,0") &&
        manifest.Find("basic") && manifest.Validate(*manifest.Find("basic")).valid,"quoted comma path failed");
    Text(csv,"\xEF\xBB\xBF"+Header+base+"\n");Check(manifest.Load(csv,root) && manifest.Find("basic"),"UTF-8 BOM failed");
    Check(!manifest.Load(root/"missing.csv",root) && manifest.Records().empty(),"missing manifest retained stale records");
    std::cout<<checks<<" checks, "<<failures<<" failures\n";
    return failures==0?0:1;
}

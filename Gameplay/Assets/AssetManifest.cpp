#include "AssetManifest.h"
#include <algorithm>
#include <array>
#include <charconv>
#include <cmath>
#include <fstream>
#include <limits>
#include <set>
#include <sstream>

namespace
{
    constexpr std::string_view Header="asset_id,kind,source,export_path,columns,rows,frame_width,frame_height,anchor_x,anchor_y,has_alpha,status,first_frame,frame_count,margin_x,margin_y,spacing_x,spacing_y";
    bool CsvRow(std::string_view line,std::vector<std::string>& fields)
    {
        fields.clear();
        std::string field;
        bool quoted=false,closed=false;
        for (std::size_t i=0;i<line.size();++i)
        {
            const char c=line[i];
            if (quoted)
            {
                if (c=='"')
                {
                    if(i+1<line.size() && line[i+1]=='"') { field+='"'; ++i; }
                    else { quoted=false;closed=true; }
                }
                else field+=c;
            }
            else if(c==',') { fields.push_back(field); field.clear();closed=false; }
            else if(c=='"' && field.empty() && !closed) quoted=true;
            else if(c=='"' || closed) return false;
            else field+=c;
        }
        if(quoted) return false;
        fields.push_back(field);
        return true;
    }
    template<class T> bool Number(const std::string& text,T& output)
    {
        const auto [end,error]=std::from_chars(text.data(),text.data()+text.size(),output);
        return error==std::errc{} && end==text.data()+text.size();
    }
    bool Identifier(const std::string& id)
    {
        return !id.empty() && std::all_of(id.begin(),id.end(),[](char c)
            { return (c>='a' && c<='z') || (c>='0' && c<='9') || c=='_'; });
    }
}

namespace MyGame
{
    bool AssetManifest::Load(const std::filesystem::path& csv,const std::filesystem::path& assetsRoot)
    {
        m_Records.clear();m_Errors.clear();m_Root=assetsRoot;
        std::ifstream stream(csv,std::ios::binary);
        if(!stream) { m_Errors.push_back("manifest_missing");return false; }
        std::string line;
        if(!std::getline(stream,line)) { m_Errors.push_back("manifest_empty");return false; }
        if(line.starts_with("\xEF\xBB\xBF")) line.erase(0,3);
        if(!line.empty() && line.back()=='\r') line.pop_back();
        std::vector<std::string> fields,expected;
        CsvRow(Header,expected);
        if(!CsvRow(line,fields) || fields!=expected) {m_Errors.push_back("invalid_header");return false;}
        std::set<std::string> ids,duplicates;
        int lineNumber=1;
        while(std::getline(stream,line))
        {
            ++lineNumber;
            if(!line.empty() && line.back()=='\r') line.pop_back();
            if(line.empty()) continue;
            if(!CsvRow(line,fields) || fields.size()!=expected.size())
            {m_Errors.push_back("csv_fields_at_line_"+std::to_string(lineNumber));continue;}
            AssetRecord r;
            r.id=fields[0];r.kind=fields[1];r.source=fields[2];r.exportPath=fields[3];r.status=fields[11];
            if(!Identifier(r.id)) {m_Errors.push_back("invalid_id_at_line_"+std::to_string(lineNumber));continue;}
            if(!ids.insert(r.id).second)
            {duplicates.insert(r.id);m_Errors.push_back("duplicate_id:"+r.id);continue;}
            const bool numbers=Number(fields[4],r.columns) && Number(fields[5],r.rows) &&
                Number(fields[6],r.frameWidth) && Number(fields[7],r.frameHeight) &&
                Number(fields[8],r.anchorX) && Number(fields[9],r.anchorY) &&
                Number(fields[12],r.firstFrame) && Number(fields[13],r.frameCount) &&
                Number(fields[14],r.marginX) && Number(fields[15],r.marginY) &&
                Number(fields[16],r.spacingX) && Number(fields[17],r.spacingY);
            if(!numbers || (fields[10]!="true" && fields[10]!="false"))
            {m_Errors.push_back("invalid_value:"+r.id);continue;}
            r.hasAlpha=fields[10]=="true";
            m_Records.push_back(std::move(r));
        }
        std::erase_if(m_Records,[&](const auto& r){return duplicates.contains(r.id);});
        if(stream.bad()) {m_Records.clear();m_Errors.push_back("manifest_read_failed");return false;}
        return true;
    }
    const AssetRecord* AssetManifest::Find(std::string_view id) const
    {
        const auto found=std::find_if(m_Records.begin(),m_Records.end(),[&](const auto& r){return r.id==id;});
        return found==m_Records.end()?nullptr:&*found;
    }
    bool AssetManifest::ResolveInside(const std::filesystem::path& root,std::string_view relative,
                                     std::string_view directory,std::filesystem::path& resolved)
    {
        try
        {
        std::string portable(relative);
        std::replace(portable.begin(),portable.end(),'\\','/');
        if(portable.empty() || portable.find(':')!=std::string::npos) return false;
        const auto path=MyEngine::Utf8Path(portable);
        if(path.is_absolute() || path.has_root_path()) return false;
        for(const auto& part:path) if(part=="..") return false;
        if(path.begin()==path.end() || *path.begin()!=std::filesystem::path(directory)) return false;
        std::error_code error;
        const auto base=std::filesystem::weakly_canonical(root/std::filesystem::path(directory),error);
        if(error) return false;
        resolved=std::filesystem::weakly_canonical(root/path,error);
        if(error) return false;
        auto a=base.begin(),b=resolved.begin();
        for(;a!=base.end();++a,++b) if(b==resolved.end() || *a!=*b) return false;
        return b!=resolved.end();
        }
        catch(const std::exception&) { return false; }
    }
    AssetValidation AssetManifest::Validate(const AssetRecord& r) const
    {
        AssetValidation result;
        const auto fail=[&](const char* reason){result.reason=reason;return std::move(result);};
        constexpr std::array<std::string_view,8> kinds={"character","animal","crop","environment","item","item_atlas","background","reference"};
        if(std::find(kinds.begin(),kinds.end(),r.kind)==kinds.end()) return fail("invalid_kind");
        if(r.status!="ready" && r.status!="needs_review" && r.status!="disabled") return fail("invalid_status");
        if(r.columns<=0 || r.rows<=0 || r.frameWidth<=0 || r.frameHeight<=0 ||
            r.columns>8192 || r.rows>8192 || r.frameWidth>8192 || r.frameHeight>8192 ||
            r.marginX<0 || r.marginY<0 || r.spacingX<0 || r.spacingY<0)
            return fail("invalid_grid");
        if(!std::isfinite(r.anchorX) || !std::isfinite(r.anchorY) ||
            r.anchorX<0 || r.anchorX>1 || r.anchorY<0 || r.anchorY>1) return fail("invalid_anchor");
        if(r.firstFrame<0 || r.frameCount<=0 ||
            static_cast<long long>(r.firstFrame)+r.frameCount>static_cast<long long>(r.columns)*r.rows)
            return fail("frame_out_of_bounds");
        std::filesystem::path source;
        if(!ResolveInside(m_Root,r.source,"Original",source) ||
            !ResolveInside(m_Root,r.exportPath,"Export",result.path)) return fail("invalid_path");
        if(result.path.extension()!=".png" || source.extension()!=".png") return fail("not_png_path");
        // Review files stay available in Original, but are never uploaded to the GPU.
        if(r.status!="ready") return fail(r.status=="needs_review"?"needs_review":"disabled");
        if(!MyEngine::DecodePng(result.path,result.image,result.reason)) return result;
        const long long expectedWidth=2LL*r.marginX+static_cast<long long>(r.columns)*r.frameWidth+
            static_cast<long long>(r.columns-1)*r.spacingX;
        const long long expectedHeight=2LL*r.marginY+static_cast<long long>(r.rows)*r.frameHeight+
            static_cast<long long>(r.rows-1)*r.spacingY;
        if(result.image.width!=expectedWidth || result.image.height!=expectedHeight) return fail("grid_dimensions_mismatch");
        if(r.hasAlpha!=result.image.alphaChannel ||
            (r.hasAlpha && (r.kind=="character" || r.kind=="animal" || r.kind=="item" || r.kind=="item_atlas") &&
             (result.image.alphaMin==255 || result.image.alphaMax==0)))
            return fail("alpha_mismatch");
        result.valid=true;
        return result;
    }
    std::optional<std::array<float,4>> AssetManifest::FrameUv(const AssetRecord& r,int relativeFrame)
    {
        if(relativeFrame<0 || relativeFrame>=r.frameCount || r.firstFrame<0 ||
            r.columns<=0 || r.rows<=0 || r.frameWidth<=0 || r.frameHeight<=0 ||
            r.marginX<0 || r.marginY<0 || r.spacingX<0 || r.spacingY<0) return std::nullopt;
        const long long frame=static_cast<long long>(r.firstFrame)+relativeFrame;
        if(frame>=static_cast<long long>(r.columns)*r.rows) return std::nullopt;
        const double w=2.0*r.marginX+static_cast<double>(r.columns)*r.frameWidth+
            static_cast<double>(r.columns-1)*r.spacingX;
        const double h=2.0*r.marginY+static_cast<double>(r.rows)*r.frameHeight+
            static_cast<double>(r.rows-1)*r.spacingY;
        const double x=r.marginX+(frame%r.columns)*(static_cast<double>(r.frameWidth)+r.spacingX);
        const double y=r.marginY+(frame/r.columns)*(static_cast<double>(r.frameHeight)+r.spacingY);
        return std::array<float,4>{static_cast<float>(x/w),static_cast<float>(y/h),
            static_cast<float>(r.frameWidth/w),static_cast<float>(r.frameHeight/h)};
    }
}

#include "RuntimeLog.h"
#include <cstdlib>
#include <chrono>
#include <stdexcept>
#include <string>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif

#ifdef _WIN32
namespace
{
    std::filesystem::path EnvironmentPath(const wchar_t* name)
    {
        std::wstring value(32768,L'\0');
        const auto count=GetEnvironmentVariableW(name,value.data(),static_cast<DWORD>(value.size()));
        if(count==0) return {};
        if(count>=value.size()) throw std::runtime_error("Logging environment path too long");
        value.resize(count);return std::filesystem::path(value);
    }
}
#endif
namespace MyGame
{
    RuntimeLog::RuntimeLog()
    {
        try
        {
            std::filesystem::path directory;
#ifdef _WIN32
            const auto custom=EnvironmentPath(L"MYFARM_LOG_DIR"),local=EnvironmentPath(L"LOCALAPPDATA");
            if(!custom.empty()) directory=custom;
            else if(!local.empty()) directory=local/"MyFarmEngine"/"Logs";
            else directory=std::filesystem::temp_directory_path()/"MyFarmEngine"/"Logs";
            const auto run=std::to_string(GetCurrentProcessId());
#else
            if(const auto* custom=std::getenv("MYFARM_LOG_DIR")) directory=custom;
            else directory=std::filesystem::temp_directory_path()/"MyFarmEngine"/"Logs";
            const auto run=std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
#endif
            std::filesystem::create_directories(directory);
            m_Path=directory/("MyFarmEngine-"+run+".log");
            m_File.open(m_Path,std::ios::app);
        }
        catch(const std::exception& error) { std::cerr<<"Cannot create runtime log: "<<error.what()<<'\n'; }
        if(Ready())
        {
            std::cout.rdbuf(&m_OutTee);std::cerr.rdbuf(&m_ErrorTee);
            std::cout<<"Runtime log: "<<m_Path<<'\n';
#ifdef _WIN32
            std::cout<<"Console attached: "<<(GetConsoleWindow()?"yes":"no")<<'\n';
#endif
            std::cout.flush();
        }
    }
    RuntimeLog::~RuntimeLog()
    {
        if(Ready())
        {
            std::cout.flush();std::cerr.flush();
            std::cout.rdbuf(m_Out);std::cerr.rdbuf(m_Error);
        }
    }
}

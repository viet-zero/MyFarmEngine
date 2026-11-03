#include "AssetPath.h"
#include <system_error>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif

namespace MyEngine
{
    std::filesystem::path FindAsset(const std::filesystem::path& relative)
    {
        std::filesystem::path executableDirectory;
#ifdef _WIN32
        wchar_t filename[32768] = {};
        const auto length = GetModuleFileNameW(nullptr, filename, 32768);
        if (length > 0 && length < 32768)
            executableDirectory = std::filesystem::path(filename).parent_path();
#elif defined(__linux__)
        std::error_code error;
        executableDirectory = std::filesystem::read_symlink("/proc/self/exe", error).parent_path();
#endif
        std::error_code existsError;
        const auto deployed = executableDirectory / "Assets" / relative;
        if (!executableDirectory.empty() && std::filesystem::is_regular_file(deployed, existsError))
            return deployed;
        return std::filesystem::path(MYFARM_SOURCE_ASSETS) / relative;
    }
}

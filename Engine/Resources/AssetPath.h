#pragma once
#include <filesystem>

namespace MyEngine
{
    // Assets are deployed beside the executable; development source is a fallback.
    std::filesystem::path FindAsset(const std::filesystem::path& relative);
}

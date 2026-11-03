#pragma once
#include <filesystem>

namespace MyEngine
{
    // Deployed root is authoritative, even when a file is missing. Images live in
    // Export; shaders/metadata live in Shaders/Data. Source root is development-only.
    std::filesystem::path FindAsset(const std::filesystem::path& relative);
    std::filesystem::path AssetRoot();
}

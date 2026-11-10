#pragma once
#include "Gameplay/Systems/InteractionSystem.h"
#include <optional>
#include <string>
#include <string_view>

namespace MyGame
{
    struct SavedPortal {std::string id;bool open=false;};
    struct FarmSave
    {
        glm::vec2 position{};
        int money=0, selectedSlot=0;
        ItemId seed=ItemId::TurnipSeed;
        Inventory bag;
        HomeStorage home;
        std::vector<InteractionTarget> targets;
        std::vector<SavedPortal> portals;
    };
    std::string EncodeSave(const FarmSave& state);
    std::optional<FarmSave> DecodeSave(std::string_view payload);
    struct SaveResult
    {
        std::string message;
        std::optional<FarmSave> state;
        bool success=false;
    };
    // Synchronous transport. The scene invokes it on a worker with an immutable snapshot.
    class SaveManager
    {
    public:
        static SaveResult Write(const FarmSave& state,bool testSlot=false);
        static SaveResult Read(bool testSlot=false);
    };
}

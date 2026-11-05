#pragma once
#include <array>
#include <cstdint>
#include <span>
#include <string_view>

namespace MyGame
{
    // Explicit IDs are stable data identifiers, never atlas frame indices.
    enum class ItemId : std::uint32_t {None=0,TurnipSeed=1001,Turnip=1002,Apple=1003,Cake=1004,Wood=1005,WateringCan=2001,Axe=2002};
    struct ItemDefinition {ItemId id;std::string_view key;int maxStack;bool tool;};
    const ItemDefinition* FindItem(ItemId id);
    struct ItemStack
    {
        ItemId item=ItemId::None;int quantity=0;
        bool operator==(const ItemStack&) const = default;
    };
    class Inventory
    {
    public:
        static constexpr int MaxSlots=12;
        explicit Inventory(int slots=MaxSlots);
        static Inventory Starter();
        int SlotCount() const {return m_SlotCount;}
        int UsedSlots() const;
        int Count(ItemId item) const;
        int TotalCount() const;
        int ResourceCount() const; // Derived resource units, excluding equipped/tool items.
        std::span<const ItemStack> Slots() const {return {m_Slots.data(),static_cast<std::size_t>(m_SlotCount)};}
        bool CanAdd(ItemId item,int quantity) const;
        bool CanRemove(ItemId item,int quantity) const;
        bool TryAdd(ItemId item,int quantity);
        bool TryRemove(ItemId item,int quantity);
        bool operator==(const Inventory&) const = default;
    private:
        std::array<ItemStack,MaxSlots> m_Slots{};
        int m_SlotCount=MaxSlots;
    };
    inline constexpr std::array<ItemId,4> DefaultHotbar{ItemId::WateringCan,ItemId::Axe,ItemId::TurnipSeed,ItemId::Cake};
}

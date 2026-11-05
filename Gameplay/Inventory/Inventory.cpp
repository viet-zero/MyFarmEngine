#include "Inventory.h"
#include <algorithm>
namespace MyGame
{
    namespace
    {
        constexpr std::array<ItemDefinition,11> Items{{
            {ItemId::TurnipSeed,"seed.turnip",20,false},{ItemId::Turnip,"crop.turnip",20,false},
            {ItemId::Apple,"fruit.apple",20,false},{ItemId::Cake,"food.cake",10,false},
            {ItemId::Wood,"resource.wood",20,false},{ItemId::CarrotSeed,"seed.carrot",20,false},
            {ItemId::Carrot,"crop.carrot",20,false},{ItemId::PotatoSeed,"seed.potato",20,false},{ItemId::Potato,"crop.potato",20,false},{ItemId::WateringCan,"tool.watering_can",1,true},
            {ItemId::Axe,"tool.axe",1,true}}};
    }
    const ItemDefinition* FindItem(ItemId id)
    {for(const auto& item:Items) if(item.id==id) return &item;return nullptr;}
    Inventory::Inventory(int slots):m_SlotCount(std::clamp(slots,0,MaxSlots)){}
    Inventory Inventory::Starter()
    {
        Inventory bag;
        bag.TryAdd(ItemId::WateringCan,1);bag.TryAdd(ItemId::Axe,1);bag.TryAdd(ItemId::TurnipSeed,3);
        return bag;
    }
    int Inventory::UsedSlots() const
    {int count=0;for(const auto& stack:Slots()) if(stack.item!=ItemId::None) ++count;return count;}
    int Inventory::Count(ItemId item) const
    {if(!FindItem(item)) return 0;int count=0;for(const auto& stack:Slots()) if(stack.item==item) count+=stack.quantity;return count;}
    int Inventory::TotalCount() const
    {int count=0;for(const auto& stack:Slots()) count+=stack.quantity;return count;}
    int Inventory::ResourceCount() const
    {int count=0;for(const auto& stack:Slots()) if(const auto* item=FindItem(stack.item);item && !item->tool) count+=stack.quantity;return count;}
    bool Inventory::CanAdd(ItemId item,int quantity) const
    {
        const auto* def=FindItem(item);if(!def || quantity<=0) return false;
        int space=0;
        for(const auto& stack:Slots())
            if(stack.item==item) space+=def->maxStack-stack.quantity;
            else if(stack.item==ItemId::None) space+=def->maxStack;
        return quantity<=space;
    }
    bool Inventory::CanRemove(ItemId item,int quantity) const
    {return FindItem(item) && quantity>0 && Count(item)>=quantity;}
    bool Inventory::TryAdd(ItemId item,int quantity)
    {
        if(!CanAdd(item,quantity)) return false;
        const int maximum=FindItem(item)->maxStack;
        // Top up existing stacks before allocating empty slots; failure never partially adds.
        for(auto& stack:m_Slots)
        {
            if(stack.item!=item) continue;
            const int amount=std::min(quantity,maximum-stack.quantity);stack.quantity+=amount;quantity-=amount;
            if(quantity==0) return true;
        }
        for(int i=0;i<m_SlotCount;++i)
        {
            auto& stack=m_Slots[static_cast<std::size_t>(i)];if(stack.item!=ItemId::None) continue;
            const int amount=std::min(quantity,maximum);stack={item,amount};quantity-=amount;
            if(quantity==0) return true;
        }
        return true;
    }
    bool Inventory::TryRemove(ItemId item,int quantity)
    {
        if(!CanRemove(item,quantity)) return false;
        for(auto& stack:m_Slots)
        {
            if(stack.item!=item) continue;
            const int amount=std::min(quantity,stack.quantity);stack.quantity-=amount;quantity-=amount;
            if(stack.quantity==0) stack={};
            if(quantity==0) return true;
        }
        return true;
    }
}

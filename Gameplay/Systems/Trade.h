#pragma once
#include "InteractionSystem.h"
#include <limits>

namespace MyGame
{
    struct MarketRow
    {
        const char* label;
        ItemId seed, produce;
        int buyPrice, sellPrice;
    };
    inline constexpr std::array<MarketRow,4> Market{{
        {"CU CAI",ItemId::TurnipSeed,ItemId::Turnip,3,6},
        {"CA ROT",ItemId::CarrotSeed,ItemId::Carrot,5,10},
        {"KHOAI TAY",ItemId::PotatoSeed,ItemId::Potato,7,14},
        {"TAO",ItemId::None,ItemId::Apple,0,4}
    }};
    enum class TradeKind {Buy,SellBag,SellHome};
    enum class TradeResult {None,Success,Invalid,Replay,Busy,Outside,NoFunds,Full,NoItems,Overflow};

    inline TradeResult Trade(InteractionActor& actor,TradeKind kind,ItemId item,int quantity,std::uint64_t request)
    {
        if(!request || quantity<=0 || actor.money<0) return TradeResult::Invalid;
        if(request<=actor.lastTradeId) return TradeResult::Replay;
        if(actor.busy || actor.activeActionId || actor.uiCapturesInput) return TradeResult::Busy;
        const bool buy=kind==TradeKind::Buy;
        const bool fromHome=kind==TradeKind::SellHome;
        if(!buy && !fromHome && kind!=TradeKind::SellBag) return TradeResult::Invalid;
        int price=0;
        for(const auto& row:Market)
            if(item==(buy?row.seed:row.produce)) {price=buy?row.buyPrice:row.sellPrice;break;}
        if(price<=0) return TradeResult::Invalid;
        if(fromHome && !actor.home.Receives(actor.ownerId,actor.position,actor.position)) return TradeResult::Outside;
        const auto total=static_cast<std::int64_t>(quantity)*price;
        auto& items=fromHome?actor.home.items:actor.inventory;
        if(buy)
        {
            if(total>actor.money) return TradeResult::NoFunds;
            if(!items.TryAdd(item,quantity)) return TradeResult::Full;
            actor.money-=static_cast<int>(total);
        }
        else
        {
            if(total>std::numeric_limits<int>::max()-actor.money) return TradeResult::Overflow;
            if(!items.TryRemove(item,quantity)) return TradeResult::NoItems;
            actor.money+=static_cast<int>(total);
        }
        actor.lastTradeId=request;
        return TradeResult::Success;
    }
}

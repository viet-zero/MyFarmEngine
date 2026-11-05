#pragma once
#include "Engine/Event/EventBus.h"
#include "Gameplay/Inventory/Inventory.h"
#include "Gameplay/Inventory/HomeStorage.h"
#include "Gameplay/Crops/Crop.h"
#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace MyGame
{
    enum class TargetKind {Object,Crop,Pickup,FruitTree};
    struct InteractionTarget
    {
        std::string id;
        TargetKind kind=TargetKind::Object;
        glm::vec2 position{0,0};
        int health=2;
        bool active=true,planted=false,ready=false;
        double age=0;
        int stock=0,maxStock=0;
        float regrowSeconds=30.0f;
        ItemId item=ItemId::Apple; // Pickup/fruit reward; crops use CropDefinition.
        CropId crop=CropId::None;
        bool tilled=true; // Existing demo plot is prepared soil; tilling tool comes later.
        double moisture=0;
        std::uint64_t revision=0;
        CropState State() const
        {return !tilled?CropState::Untilled:!planted?CropState::Empty:ready?CropState::Ready:age>0?CropState::Growing:CropState::Planted;}
    };
    struct InteractionActor
    {
        glm::vec2 position{0,0},facing{0,1};
        float stamina=20,cooldown=0;
        Inventory inventory=Inventory::Starter();
        HomeStorage home;
        std::uint64_t ownerId=1;
        std::array<ItemId,4> hotbar=DefaultHotbar;
        ItemId SelectedItem(int slot) const
        {return slot>=0 && slot<static_cast<int>(hotbar.size())?hotbar[static_cast<std::size_t>(slot)]:ItemId::None;}
        bool busy=false,uiCapturesInput=false;
        std::uint64_t nextActionId=1,activeActionId=0,lastImpactActionId=0;
    };
    enum class InteractionResult {Success,NoTarget,Inactive,TooFar,WrongFacing,Busy,Cooldown,
                                  NoStamina,InventoryFull,NoSeeds,WrongTool,NotReady,Cancelled,TargetChanged,Blocked,NoAnimation,EventQueueFull,StorageFull,NeedsTilling,AlreadyWatered,InvalidCrop};
    enum class FarmOperation {Hit,Plant,Water,Harvest,Pickup,FruitHarvest};
    enum class ActionPhase {Idle,Start,Active,Impact,Complete,Recovery,Finished,Cancelled};
    // Transient reservation: cancel before replacing/restoring world or inventory state.
    struct InteractionAction
    {
        std::uint64_t id=0;
        std::string targetId;
        TargetKind kind=TargetKind::Object;
        int selectedItem=0;
        ItemId item=ItemId::None,targetItem=ItemId::None;
        FarmOperation operation=FarmOperation::Hit;
        CropId crop=CropId::None;
        std::uint64_t targetRevision=0,storageOwner=0;
        bool toStorage=false;
        bool cropWasPlanted=false,impactConsumed=false;
        ActionPhase phase=ActionPhase::Idle;
        InteractionResult result=InteractionResult::NoTarget;
        float recoveryRemaining=0;
        bool Pending() const
        {return phase==ActionPhase::Start || phase==ActionPhase::Active || phase==ActionPhase::Impact ||
                phase==ActionPhase::Complete || phase==ActionPhase::Recovery;}
    };
    class InteractionSystem
    {
    public:
        static constexpr float Range=100.0f,CooldownSeconds=0.35f,StaminaCost=2.0f;
        static InteractionTarget* Nearest(const InteractionActor& actor,std::vector<InteractionTarget>& targets);
        static InteractionResult Check(const InteractionActor& actor,const InteractionTarget* target,int selectedItem);
        // Low-level immediate operation; live gameplay commits through Impact below.
        static InteractionResult Use(InteractionActor& actor,InteractionTarget* target,int selectedItem,EventBus& events);
        static void Tick(InteractionActor& actor,std::vector<InteractionTarget>& targets,float deltaTime);
        static InteractionResult Start(InteractionActor& actor,const InteractionTarget* target,int selectedItem,InteractionAction& action);
        static void Activate(InteractionActor& actor,InteractionAction& action);
        static InteractionResult Impact(InteractionActor& actor,InteractionAction& action,
            std::vector<InteractionTarget>& targets,EventBus& events,bool lineOfSight=true);
        static void Complete(InteractionActor& actor,InteractionAction& action);
        static void TickAction(InteractionActor& actor,InteractionAction& action,float deltaTime);
        static void Cancel(InteractionActor& actor,InteractionAction& action,InteractionResult reason=InteractionResult::Cancelled);
    private:
        static FarmOperation Operation(const InteractionActor& actor,const InteractionTarget& target,int slot);
        static bool ToStorage(const InteractionActor& actor,const InteractionTarget& target);
        static ItemId Reward(const InteractionTarget& target);
        static InteractionResult CheckOwned(const InteractionActor& actor,const InteractionTarget* target,int selectedItem,bool owned);
        static InteractionResult Apply(InteractionActor& actor,InteractionTarget* target,int selectedItem,EventBus& events,std::uint64_t actionId);
    };
}

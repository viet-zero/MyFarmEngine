#include "InteractionSystem.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace MyGame
{
    FarmOperation InteractionSystem::Operation(const InteractionActor& actor,const InteractionTarget& target,int slot)
    {
        if(target.kind==TargetKind::Crop)
            return !target.planted?FarmOperation::Plant:target.ready?FarmOperation::Harvest:
                   actor.SelectedItem(slot)==ItemId::WateringCan?FarmOperation::Water:FarmOperation::Harvest;
        return target.kind==TargetKind::FruitTree?FarmOperation::FruitHarvest:
               target.kind==TargetKind::Pickup?FarmOperation::Pickup:FarmOperation::Hit;
    }
    ItemId InteractionSystem::Reward(const InteractionTarget& target)
    {
        if(target.kind!=TargetKind::Crop) return target.item;
        const auto* definition=FindCrop(target.crop);return definition?definition->produce:ItemId::None;
    }
    bool InteractionSystem::ToStorage(const InteractionActor& actor,const InteractionTarget& target,bool garden)
    {
        const bool harvest=target.kind==TargetKind::FruitTree || (target.kind==TargetKind::Crop && target.planted && target.ready);
        return harvest && actor.home.Receives(actor.ownerId,garden?target.position:actor.position,target.position);
    }
    InteractionTarget* InteractionSystem::Nearest(const InteractionActor& actor,std::vector<InteractionTarget>& targets)
    {
        if(!std::isfinite(actor.position.x) || !std::isfinite(actor.position.y)) return nullptr;
        const float facingLength=glm::length(actor.facing);
        if(facingLength<=0 || !std::isfinite(facingLength)) return nullptr;
        const auto forward=actor.facing/facingLength;
        InteractionTarget* nearest=nullptr;float best=Range*Range;
        for(auto& target:targets)
        {
            if(!target.active || !std::isfinite(target.position.x) || !std::isfinite(target.position.y)) continue;
            const glm::vec2 delta=target.position-actor.position;
            const float distance2=glm::dot(delta,delta);
            if(distance2>best || !std::isfinite(distance2)) continue;
            if(distance2>1 && glm::dot(delta,forward)<0.5f*std::sqrt(distance2)) continue;
            best=distance2;
            nearest=&target;
        }
        return nearest;
    }
    InteractionResult InteractionSystem::CheckOwned(const InteractionActor& actor,const InteractionTarget* target,
                                                      int selectedItem,bool owned,bool garden)
    {
        if((!owned && (actor.busy || actor.activeActionId!=0)) || actor.uiCapturesInput) return InteractionResult::Busy;
        if(!target) return InteractionResult::NoTarget;
        if(!target->active) return InteractionResult::Inactive;
        if(!std::isfinite(actor.position.x) || !std::isfinite(actor.position.y) ||
           !std::isfinite(actor.facing.x) || !std::isfinite(actor.facing.y) ||
           !std::isfinite(target->position.x) || !std::isfinite(target->position.y)) return InteractionResult::TooFar;
        if(garden && (target->kind!=TargetKind::Crop ||
           !actor.home.Receives(actor.ownerId,target->position,target->position))) return InteractionResult::Blocked;
        const glm::vec2 delta=target->position-actor.position;
        const float distance=glm::length(delta);
        if(!garden && (!std::isfinite(distance) || distance>Range)) return InteractionResult::TooFar;
        const float facingLength=glm::length(actor.facing);
        if(!garden && (facingLength<=0 || !std::isfinite(facingLength) ||
           (distance>1 && glm::dot(delta/distance,actor.facing/facingLength)<0.5f)))
            return InteractionResult::WrongFacing;
        if(!std::isfinite(actor.cooldown) || actor.cooldown>0) return InteractionResult::Cooldown;
        if(!std::isfinite(actor.stamina) || actor.stamina<StaminaCost) return InteractionResult::NoStamina;
        const auto item=actor.SelectedItem(selectedItem);
        if(!FindItem(item)) return InteractionResult::WrongTool;
        if(target->kind==TargetKind::Object &&
           (item!=ItemId::Axe || actor.inventory.Count(ItemId::Axe)==0)) return InteractionResult::WrongTool;
        if(target->kind==TargetKind::Crop)
        {
            if(!target->tilled) return InteractionResult::NeedsTilling;
            if(!target->planted)
            {
                if(!CropFromSeed(item)) return InteractionResult::WrongTool;
                if(!actor.inventory.CanRemove(item,1)) return InteractionResult::NoSeeds;
            }
            else
            {
                const auto* crop=FindCrop(target->crop);
                if(!crop) return InteractionResult::InvalidCrop;
                if(!target->ready)
                {
                    if(item!=ItemId::WateringCan) return InteractionResult::NotReady;
                    if(actor.inventory.Count(item)==0) return InteractionResult::WrongTool;
                    if(target->moisture>=crop->waterSeconds-1e-6) return InteractionResult::AlreadyWatered;
                }
            }
        }
        if(target->kind==TargetKind::FruitTree && target->stock<=0) return InteractionResult::NotReady;
        const bool receivesItem=target->kind==TargetKind::Pickup ||
                                (target->kind==TargetKind::Crop && target->planted && target->ready) ||
                                target->kind==TargetKind::FruitTree;
        if(receivesItem)
        {
            const bool home=ToStorage(actor,*target,garden);
            const auto& destination=home?actor.home.items:actor.inventory;
            if(!destination.CanAdd(Reward(*target),1)) return home?InteractionResult::StorageFull:InteractionResult::InventoryFull;
        }

        return InteractionResult::Success;
    }
    InteractionResult InteractionSystem::Check(const InteractionActor& actor,const InteractionTarget* target,int selectedItem)
    {return CheckOwned(actor,target,selectedItem,false);}
    InteractionResult InteractionSystem::Use(InteractionActor& actor,InteractionTarget* target,int selectedItem,EventBus& events)
    {
        const auto result=Check(actor,target,selectedItem);
        return result==InteractionResult::Success?Apply(actor,target,selectedItem,events,0):result;
    }
    InteractionResult InteractionSystem::UseGarden(InteractionActor& actor,InteractionTarget* target,int selectedItem,EventBus& events)
    {
        const auto result=CheckOwned(actor,target,selectedItem,false,true);
        return result==InteractionResult::Success?Apply(actor,target,selectedItem,events,0,true):result;
    }
    InteractionResult InteractionSystem::Apply(InteractionActor& actor,InteractionTarget* target,
                                              int selectedItem,EventBus& events,std::uint64_t actionId,bool garden)
    {
        const auto operation=Operation(actor,*target,selectedItem);
        const std::size_t needed=target->kind==TargetKind::Pickup?1u:2u;
        if(events.Pending()+needed>EventBus::MaxPending) return InteractionResult::EventQueueFull;
        const bool home=ToStorage(actor,*target,garden);
        if(operation==FarmOperation::Plant)
        {if(!actor.inventory.TryRemove(actor.SelectedItem(selectedItem),1)) return InteractionResult::NoSeeds;}
        else if(operation==FarmOperation::Harvest || operation==FarmOperation::Pickup || operation==FarmOperation::FruitHarvest)
        {
            auto& destination=home?actor.home.items:actor.inventory;
            if(!destination.TryAdd(Reward(*target),1)) return home?InteractionResult::StorageFull:InteractionResult::InventoryFull;
        }
        actor.stamina-=StaminaCost;actor.cooldown=CooldownSeconds;
        if(target->kind!=TargetKind::Pickup)
            events.Publish({GameEventKind::ToolUsed,target->id,target->position,1,actionId});
        switch(target->kind)
        {
        case TargetKind::Object:
            --target->health;if(target->health<=0) target->active=false;
            events.Publish({GameEventKind::ObjectHit,target->id,target->position,1,actionId});break;
        case TargetKind::Crop:
            if(operation==FarmOperation::Plant)
            {
                target->crop=CropFromSeed(actor.SelectedItem(selectedItem))->id;
                target->planted=true;target->age=0;target->ready=false;target->moisture=0;++target->revision;
                events.Publish({GameEventKind::CropPlanted,target->id,target->position,1,actionId});
            }
            else if(operation==FarmOperation::Water)
            {
                target->moisture=FindCrop(target->crop)->waterSeconds;++target->revision;
                events.Publish({GameEventKind::CropWatered,target->id,target->position,1,actionId});
            }
            else
            {
                // Reuse the same plot; do not deactivate it or allow an old harvest token to replay.
                target->planted=false;target->ready=false;target->crop=CropId::None;
                target->age=0;target->moisture=0;++target->revision;
                events.Publish({GameEventKind::CropHarvested,target->id,target->position,1,actionId});
            }
            break;
        case TargetKind::Pickup:
            target->active=false;
            events.Publish({GameEventKind::ItemPicked,target->id,target->position,1,actionId});break;
        case TargetKind::FruitTree:
            --target->stock;target->age=0;
            events.Publish({GameEventKind::FruitHarvested,target->id,target->position,1,actionId});break;
        }
        return InteractionResult::Success;
    }
    InteractionResult InteractionSystem::Start(InteractionActor& actor,const InteractionTarget* target,
                                              int selectedItem,InteractionAction& action)
    {
        if(action.Pending()) return InteractionResult::Busy;
        const auto result=Check(actor,target,selectedItem);
        if(result!=InteractionResult::Success) return result;
        if(target->id.empty() || actor.nextActionId==0 || actor.nextActionId==std::numeric_limits<std::uint64_t>::max())
            return InteractionResult::TargetChanged;
        action={};action.id=actor.nextActionId++;action.targetId=target->id;action.kind=target->kind;
        action.selectedItem=selectedItem;action.item=actor.SelectedItem(selectedItem);action.targetItem=target->item;action.cropWasPlanted=target->planted;
        action.operation=Operation(actor,*target,selectedItem);action.crop=target->crop;action.targetRevision=target->revision;
        action.toStorage=ToStorage(actor,*target);action.storageOwner=action.toStorage?actor.home.ownerId:0;
        action.phase=ActionPhase::Start;action.result=InteractionResult::Success;
        actor.activeActionId=action.id;actor.busy=true;
        return InteractionResult::Success;
    }
    void InteractionSystem::Activate(InteractionActor& actor,InteractionAction& action)
    {
        if(actor.activeActionId==action.id && action.id!=0 && action.phase==ActionPhase::Start) action.phase=ActionPhase::Active;
    }
    InteractionResult InteractionSystem::Impact(InteractionActor& actor,InteractionAction& action,
        std::vector<InteractionTarget>& targets,EventBus& events,bool lineOfSight)
    {
        if(action.id==0 || actor.activeActionId!=action.id || action.id<=actor.lastImpactActionId || action.impactConsumed ||
           (action.phase!=ActionPhase::Start && action.phase!=ActionPhase::Active)) return InteractionResult::Busy;
        actor.lastImpactActionId=action.id;
        action.impactConsumed=true;action.phase=ActionPhase::Impact;
        InteractionTarget* target=nullptr;
        for(auto& entry:targets) if(entry.id==action.targetId)
        {
            if(target){action.result=InteractionResult::TargetChanged;return action.result;}
            target=&entry;
        }
        if(!target){action.result=InteractionResult::NoTarget;return action.result;}
        if(Operation(actor,*target,action.selectedItem)!=action.operation || target->crop!=action.crop || target->revision!=action.targetRevision ||
           ToStorage(actor,*target)!=action.toStorage || (action.toStorage && actor.home.ownerId!=action.storageOwner) ||
           actor.SelectedItem(action.selectedItem)!=action.item || target->item!=action.targetItem || target->kind!=action.kind || (target->kind==TargetKind::Crop && target->planted!=action.cropWasPlanted))
        {action.result=InteractionResult::TargetChanged;return action.result;}
        action.result=lineOfSight?CheckOwned(actor,target,action.selectedItem,true):InteractionResult::Blocked;
        if(action.result==InteractionResult::Success) action.result=Apply(actor,target,action.selectedItem,events,action.id);
        return action.result;
    }
    void InteractionSystem::Complete(InteractionActor& actor,InteractionAction& action)
    {
        if(action.id==0 || actor.activeActionId!=action.id || !action.Pending() ||
           action.phase==ActionPhase::Complete || action.phase==ActionPhase::Recovery) return;
        if(!action.impactConsumed){Cancel(actor,action,InteractionResult::NoAnimation);return;}
        action.phase=ActionPhase::Complete;action.recoveryRemaining=0.15f;
    }
    void InteractionSystem::TickAction(InteractionActor& actor,InteractionAction& action,float deltaTime)
    {
        if(action.id==0 || actor.activeActionId!=action.id || !std::isfinite(deltaTime) || deltaTime<=0) return;
        if(action.phase==ActionPhase::Complete) action.phase=ActionPhase::Recovery;
        if(action.phase!=ActionPhase::Recovery) return;
        action.recoveryRemaining=std::max(0.0f,action.recoveryRemaining-std::min(deltaTime,0.1f));
        if(action.recoveryRemaining==0)
        {action.phase=ActionPhase::Finished;actor.activeActionId=0;actor.busy=false;}
    }
    void InteractionSystem::Cancel(InteractionActor& actor,InteractionAction& action,InteractionResult reason)
    {
        if(!action.Pending()) return;
        if(!action.impactConsumed) action.result=reason;
        action.impactConsumed=true;action.phase=ActionPhase::Cancelled;
        if(action.id!=0 && actor.activeActionId==action.id){actor.activeActionId=0;actor.busy=false;}
    }
    void InteractionSystem::Tick(InteractionActor& actor,std::vector<InteractionTarget>& targets,float deltaTime)
    {
        if(!std::isfinite(deltaTime) || deltaTime<=0) return;
        const float dt=std::min(deltaTime,0.1f);
        actor.cooldown=std::max(0.0f,actor.cooldown-dt);
        actor.stamina=std::min(20.0f,actor.stamina+dt);
        for(auto& target:targets)
            if(target.active && target.kind==TargetKind::Crop && target.planted)
            {
                const auto* crop=FindCrop(target.crop);
                if(!crop || !std::isfinite(target.age) || !std::isfinite(target.moisture)) continue;
                const double wet=std::min(static_cast<double>(dt),std::max(0.0,target.moisture));
                target.moisture=std::max(0.0,target.moisture-dt);
                if(!target.ready)
                {
                    target.age=std::min(crop->growthSeconds,std::max(0.0,target.age)+wet);
                    if(target.age>=crop->growthSeconds-1e-6){target.age=crop->growthSeconds;target.ready=true;}
                }
            }
            else if(target.active && target.kind==TargetKind::FruitTree && target.stock==0 &&
                    std::isfinite(target.regrowSeconds) && target.regrowSeconds>0 && target.maxStock>0)
            {
                target.age=std::min(static_cast<double>(target.regrowSeconds),target.age+dt);
                if(target.age>=target.regrowSeconds-1e-6){target.stock=target.maxStock;target.age=0;}
            }
    }
}

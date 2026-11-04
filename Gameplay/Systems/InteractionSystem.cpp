#include "InteractionSystem.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace MyGame
{
    InteractionTarget* InteractionSystem::Nearest(const InteractionActor& actor,std::vector<InteractionTarget>& targets)
    {
        if(!std::isfinite(actor.position.x) || !std::isfinite(actor.position.y)) return nullptr;
        InteractionTarget* nearest=nullptr;float best=Range*Range;
        for(auto& target:targets)
        {
            if(!target.active || !std::isfinite(target.position.x) || !std::isfinite(target.position.y)) continue;
            const glm::vec2 delta=target.position-actor.position;
            const float distance2=glm::dot(delta,delta);
            const float facingLength=glm::length(actor.facing);
            if(facingLength<=0 || !std::isfinite(facingLength)) return nullptr;
            if(distance2>1 && glm::dot(delta/std::sqrt(distance2),actor.facing/facingLength)<0.5f) continue;
            if(distance2<=best){best=distance2;nearest=&target;}
        }
        return nearest;
    }
    InteractionResult InteractionSystem::CheckOwned(const InteractionActor& actor,const InteractionTarget* target,
                                                      int selectedItem,bool owned)
    {
        if((!owned && (actor.busy || actor.activeActionId!=0)) || actor.uiCapturesInput) return InteractionResult::Busy;
        if(!target) return InteractionResult::NoTarget;
        if(!target->active) return InteractionResult::Inactive;
        if(!std::isfinite(actor.position.x) || !std::isfinite(actor.position.y) ||
           !std::isfinite(actor.facing.x) || !std::isfinite(actor.facing.y) ||
           !std::isfinite(target->position.x) || !std::isfinite(target->position.y)) return InteractionResult::TooFar;
        const glm::vec2 delta=target->position-actor.position;
        const float distance=glm::length(delta);
        if(!std::isfinite(distance) || distance>Range) return InteractionResult::TooFar;
        const float facingLength=glm::length(actor.facing);
        if(facingLength<=0 || !std::isfinite(facingLength) ||
           (distance>1 && glm::dot(delta/distance,actor.facing/facingLength)<0.5f))
            return InteractionResult::WrongFacing;
        if(!std::isfinite(actor.cooldown) || actor.cooldown>0) return InteractionResult::Cooldown;
        if(!std::isfinite(actor.stamina) || actor.stamina<StaminaCost) return InteractionResult::NoStamina;
        if(target->kind==TargetKind::Object && selectedItem!=1) return InteractionResult::WrongTool;
        if(target->kind==TargetKind::Crop)
        {
            if(!target->planted && selectedItem!=2) return InteractionResult::WrongTool;
            if(!target->planted && actor.inventoryCount<=0) return InteractionResult::NoSeeds;
            if(target->planted && !target->ready) return InteractionResult::NotReady;
        }
        if(target->kind==TargetKind::FruitTree && target->stock<=0) return InteractionResult::NotReady;
        const bool receivesItem=target->kind==TargetKind::Pickup ||
                                (target->kind==TargetKind::Crop && target->planted) ||
                                target->kind==TargetKind::FruitTree;
        if(receivesItem && actor.inventoryCount>=actor.inventoryCapacity) return InteractionResult::InventoryFull;

        return InteractionResult::Success;
    }
    InteractionResult InteractionSystem::Check(const InteractionActor& actor,const InteractionTarget* target,int selectedItem)
    {return CheckOwned(actor,target,selectedItem,false);}
    InteractionResult InteractionSystem::Use(InteractionActor& actor,InteractionTarget* target,int selectedItem,EventBus& events)
    {
        const auto result=Check(actor,target,selectedItem);
        return result==InteractionResult::Success?Apply(actor,target,selectedItem,events,0):result;
    }
    InteractionResult InteractionSystem::Apply(InteractionActor& actor,InteractionTarget* target,
                                              int selectedItem,EventBus& events,std::uint64_t actionId)
    {
        (void)selectedItem;
        const std::size_t needed=target->kind==TargetKind::Pickup?1u:2u;
        if(events.Pending()+needed>EventBus::MaxPending) return InteractionResult::EventQueueFull;
        actor.stamina-=StaminaCost;actor.cooldown=CooldownSeconds;
        if(target->kind!=TargetKind::Pickup)
            events.Publish({GameEventKind::ToolUsed,target->id,target->position,1,actionId});
        switch(target->kind)
        {
        case TargetKind::Object:
            --target->health;if(target->health<=0) target->active=false;
            events.Publish({GameEventKind::ObjectHit,target->id,target->position,1,actionId});break;
        case TargetKind::Crop:
            if(!target->planted)
            {
                --actor.inventoryCount;target->planted=true;target->age=0;target->ready=false;
                events.Publish({GameEventKind::CropPlanted,target->id,target->position,1,actionId});
            }
            else
            {
                ++actor.inventoryCount;target->active=false;
                events.Publish({GameEventKind::CropHarvested,target->id,target->position,1,actionId});
            }
            break;
        case TargetKind::Pickup:
            ++actor.inventoryCount;target->active=false;
            events.Publish({GameEventKind::ItemPicked,target->id,target->position,1,actionId});break;
        case TargetKind::FruitTree:
            --target->stock;++actor.inventoryCount;target->age=0;
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
        action.selectedItem=selectedItem;action.cropWasPlanted=target->planted;
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
        if(target->kind!=action.kind || (target->kind==TargetKind::Crop && target->planted!=action.cropWasPlanted))
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
            if(target.active && target.kind==TargetKind::Crop && target.planted && !target.ready)
            {
                target.age=std::min(5.0f,target.age+dt);
                target.ready=target.age>=5.0f;
            }
            else if(target.active && target.kind==TargetKind::FruitTree && target.stock==0 &&
                    std::isfinite(target.regrowSeconds) && target.regrowSeconds>0 && target.maxStock>0)
            {
                target.age=std::min(target.regrowSeconds,target.age+dt);
                if(target.age>=target.regrowSeconds){target.stock=target.maxStock;target.age=0;}
            }
    }
}

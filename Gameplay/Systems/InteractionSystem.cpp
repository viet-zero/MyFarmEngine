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
    InteractionResult InteractionSystem::Use(InteractionActor& actor,InteractionTarget* target,
                                             int selectedItem,EventBus& events)
    {
        if(actor.busy || actor.uiCapturesInput) return InteractionResult::Busy;
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
        if(actor.cooldown>0) return InteractionResult::Cooldown;
        if(actor.stamina<StaminaCost) return InteractionResult::NoStamina;
        if(target->kind==TargetKind::Object && selectedItem!=1) return InteractionResult::WrongTool;
        if(target->kind==TargetKind::Crop)
        {
            if(!target->planted && selectedItem!=2) return InteractionResult::WrongTool;
            if(!target->planted && actor.inventoryCount<=0) return InteractionResult::NoSeeds;
            if(target->planted && !target->ready) return InteractionResult::NotReady;
        }
        const bool receivesItem=target->kind==TargetKind::Pickup ||
                                (target->kind==TargetKind::Crop && target->planted);
        if(receivesItem && actor.inventoryCount>=actor.inventoryCapacity) return InteractionResult::InventoryFull;

        actor.stamina-=StaminaCost;actor.cooldown=CooldownSeconds;
        if(target->kind!=TargetKind::Pickup)
            events.Publish({GameEventKind::ToolUsed,target->id,target->position,1});
        switch(target->kind)
        {
        case TargetKind::Object:
            --target->health;if(target->health<=0) target->active=false;
            events.Publish({GameEventKind::ObjectHit,target->id,target->position,1});break;
        case TargetKind::Crop:
            if(!target->planted)
            {
                --actor.inventoryCount;target->planted=true;target->age=0;target->ready=false;
                events.Publish({GameEventKind::CropPlanted,target->id,target->position,1});
            }
            else
            {
                ++actor.inventoryCount;target->active=false;
                events.Publish({GameEventKind::CropHarvested,target->id,target->position,1});
            }
            break;
        case TargetKind::Pickup:
            ++actor.inventoryCount;target->active=false;
            events.Publish({GameEventKind::ItemPicked,target->id,target->position,1});break;
        }
        return InteractionResult::Success;
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
    }
}

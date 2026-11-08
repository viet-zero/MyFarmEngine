#include "Core/Time.h"
#include "Gameplay/Player/SceneInput.h"
#include <cmath>
#include <iostream>
#include <limits>
namespace {int failures=0;void Check(bool ok,const char* text){if(!ok){std::cerr<<text<<'\n';++failures;}}}
int main()
{
    using MyEngine::FixedStepClock;
    for(const int hz:{30,60,144})
    {
        FixedStepClock clock;int ticks=0;
        for(int frame=0;frame<hz*10;++frame)
        {
            const auto plan=clock.Advance(1.0/hz);ticks+=plan.steps;
            Check(plan.steps<=FixedStepClock::MaxSteps && plan.alpha>=0 && plan.alpha<=1,"invalid frame budget/alpha");
        }
        Check(ticks==600 && clock.TotalSteps()==600 && clock.DroppedSeconds()==0,"render cadence changed simulated time");
    }
    FixedStepClock clock;
    auto plan=clock.Advance(FixedStepClock::StepSeconds/2);
    Check(plan.steps==0 && std::abs(plan.alpha-0.5f)<0.00001f,"partial tick lost");
    plan=clock.Advance(FixedStepClock::StepSeconds/2);
    Check(plan.steps==1 && plan.alpha<0.00001f,"fractional accumulator did not form a tick");
    clock.Advance(FixedStepClock::StepSeconds/2);
    const auto ticks=clock.TotalSteps();plan=clock.Advance(5);
    Check(plan.steps==FixedStepClock::MaxSteps && std::abs(plan.droppedSeconds-4.9)<0.000001 &&
          std::abs(plan.alpha-0.5f)<0.00001f,"stall did not drop whole debt and preserve fraction");
    plan=clock.Advance(0);
    Check(plan.steps==0 && clock.TotalSteps()==ticks+FixedStepClock::MaxSteps,"old catch-up debt leaked to another frame");
    for(double dt:{-1.0,std::numeric_limits<double>::quiet_NaN(),std::numeric_limits<double>::infinity()})
        Check(clock.Advance(dt).steps==0 && std::isfinite(clock.Alpha()),"invalid delta corrupted clock");
    clock.Reset();Check(clock.Alpha()==0 && clock.Advance(0).steps==0,"pause/reset retained accumulator");

    MyGame::SceneInputBuffer input;
    MyGame::SceneInput sample;sample.interact=true;sample.toggleStorage=true;sample.movement={1,0};sample.zoom=1;
    input.Capture(sample);clock.Advance(FixedStepClock::StepSeconds/4);
    sample={};sample.movement={0,1};sample.running=true;sample.selectedItem=2;
    input.Capture(sample);clock.Advance(FixedStepClock::StepSeconds/4);
    const auto first=input.Consume(),second=input.Consume();
    Check(first.interact && first.toggleStorage && first.selectedItem==2 && first.zoom==1 && first.running && first.movement.y==1,"zero-tick frames lost edge or latest held input");
    Check(!second.interact && !second.toggleStorage && second.selectedItem==-1 && second.zoom==0 && second.running && second.movement.y==1,"catch-up replayed an edge or lost held input");
    sample={};sample.interact=true;sample.cancel=true;sample.reset=true;sample.switchCharacter=true;sample.teleport=7;
    input.Capture(sample);const auto edges=input.Consume();const auto consumed=input.Consume();
    Check(edges.interact && edges.cancel && edges.reset && edges.switchCharacter && edges.teleport==7,"command buffer dropped edges");
    Check(!consumed.interact && !consumed.cancel && !consumed.reset && !consumed.switchCharacter && consumed.teleport==0,"command repeated across fixed steps");
    input.Capture(sample);sample={};sample.paused=true;input.Capture(sample);
    const auto paused=input.Consume();Check(paused.paused && !paused.interact && !paused.reset,"pause replayed pending command");
    input.Clear();Check(!input.Consume().paused,"resume retained pause flag");
    sample={};sample.toggleGardenControl=true;sample.pointerWorld=glm::vec2{2208,2016};
    sample.hoverWorld=sample.pointerWorld;input.Capture(sample);
    sample={};sample.hoverWorld=glm::vec2{2300,2080};input.Capture(sample);
    const auto pointer=input.Consume(),noRepeat=input.Consume();
    Check(pointer.toggleGardenControl && pointer.pointerWorld && pointer.pointerWorld->x==2208 &&
          pointer.hoverWorld && pointer.hoverWorld->x==2300,"buffer lost click or latest hover");
    Check(!noRepeat.toggleGardenControl && !noRepeat.pointerWorld && noRepeat.hoverWorld,
          "catch-up replayed mouse click/mode toggle");
    return failures?1:0;
}

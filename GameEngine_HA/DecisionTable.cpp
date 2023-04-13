#include "DecisionTable.h"
#include "Colonist.h" 

DecisionTable::DecisionTable() {
    // isHungry | Command | isIntruderInRange | isInventoryFull | isTargetInRange
    decisionTable = {
        { {true, CommandType::None, false, false, false}, ActionType::Eat },
        { {false, CommandType::Move, false, false, false}, ActionType::Move },
        { {false, CommandType::HarvestTree, false, false, false}, ActionType::Move },
        { {false, CommandType::HarvestTree, false, false, true}, ActionType::HarvestTree },
        { {false, CommandType::HarvestRock, false, false, false}, ActionType::Move },
        { {false, CommandType::HarvestRock, false, false, true}, ActionType::HarvestRock },
        { {false, CommandType::AttackIntruder, false, false, false}, ActionType::Move },
        { {false, CommandType::AttackIntruder, false, false, true}, ActionType::AttackIntruder },
        { {false, CommandType::None, true, false}, ActionType::AttackInRange },
        { {false, CommandType::None, false, true}, ActionType::DropOffLoot },
        { {false, CommandType::None, false, false}, ActionType::Idle },
    };
}

DecisionTable::~DecisionTable()
{
}

ActionType DecisionTable::getNextAction(Colonist& colonist) {
    Condition currentCondition;
    currentCondition.isHungry = colonist.isHungry();
    currentCondition.playerCommand = colonist.mCurrentCommand;
    currentCondition.isIntruderInRange = colonist.getIsIntruderInRange();
    currentCondition.isInventoryFull = colonist.mInventory->isFull();
    if (colonist.mTarget == nullptr)
        currentCondition.isTargetInRange = false;
    else currentCondition.isTargetInRange = (glm::length(*colonist.mTarget->position - colonist.mGOColonist->mesh->position) <= 0.2f);

    for (const Rule& rule : decisionTable) {
        if (rule.condition == currentCondition) {
            return rule.action;
        }
    }

    return ActionType::Idle;
}

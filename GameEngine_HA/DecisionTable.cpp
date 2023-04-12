#include "DecisionTable.h"
#include "Colonist.h" 

DecisionTable::DecisionTable() {
    // isHungry | Command | isIntruderInRange | isInventoryFull
    decisionTable = {
        { {true, CommandType::None, false, false}, ActionType::Eat },
        { {false, CommandType::Move, false, false}, ActionType::Move },
        { {false, CommandType::HarvestTree, false, false}, ActionType::HarvestTree },
        { {false, CommandType::HarvestRock, false, false}, ActionType::HarvestRock },
        { {false, CommandType::AttackIntruder, false, false}, ActionType::AttackIntruder },
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

    for (const Rule& rule : decisionTable) {
        if (rule.condition == currentCondition) {
            return rule.action;
        }
    }

    return ActionType::Idle;
}

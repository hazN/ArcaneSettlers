#pragma once

#include <vector>

class Colonist;

enum class ActionType {
    Eat, Move, HarvestTree, HarvestRock, AttackIntruder, AttackInRange, DropOffLoot, Idle
};
enum class CommandType {
    None, Move, HarvestTree, HarvestRock, AttackIntruder
};

class DecisionTable {
public:
    DecisionTable();
    ~DecisionTable();
    ActionType getNextAction(Colonist& colonist);

private:
    struct Condition {
        bool isHungry;
        CommandType playerCommand;
        bool isIntruderInRange;
        bool isInventoryFull;
        bool isTargetInRange;
        bool operator==(const Condition& other) const {
            return isHungry == other.isHungry &&
                playerCommand == other.playerCommand &&
                isIntruderInRange == other.isIntruderInRange &&
                isInventoryFull == other.isInventoryFull;
        }
    };

    struct Rule {
        Condition condition;
        ActionType action;
    };

    std::vector<Rule> decisionTable;
};

#pragma once

#include <vector>
#include <optional>

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
		// Using optional so that the table can have a rule of either or
		std::optional<bool> isHungry;
		CommandType playerCommand;
		std::optional<bool> isIntruderInRange;
		std::optional<bool> isInventoryFull;
		std::optional<bool> isTargetInRange;
		bool operator==(const Condition& other) const {
			return (isHungry == other.isHungry || !isHungry.has_value() || !other.isHungry.has_value()) &&
				(playerCommand == other.playerCommand) &&
				(isIntruderInRange == other.isIntruderInRange || !isIntruderInRange.has_value() || !other.isIntruderInRange.has_value()) &&
				(isInventoryFull == other.isInventoryFull || !isInventoryFull.has_value() || !other.isInventoryFull.has_value()) &&
				(isTargetInRange == other.isTargetInRange || !isTargetInRange.has_value() || !other.isTargetInRange.has_value());
		}
	};

	struct Rule {
		Condition condition;
		ActionType action;
	};

	std::vector<Rule> decisionTable;
};

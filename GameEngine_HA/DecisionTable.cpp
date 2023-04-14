#include "DecisionTable.h"
#include "Colonist.h"

DecisionTable::DecisionTable() {
	// isHungry | Command | isIntruderInRange | isInventoryFull | isTargetInRange
	decisionTable = {
		{ {true, CommandType::None, false, std::nullopt, false}, ActionType::Move },
		{ {true, CommandType::None, false, std::nullopt, true}, ActionType::Eat },
		{ {false, CommandType::Move, std::nullopt, std::nullopt, std::nullopt}, ActionType::Move },
		{ {false, CommandType::HarvestTree, false, false, false}, ActionType::Move },
		{ {false, CommandType::HarvestTree, false, false, true}, ActionType::HarvestTree },
		{ {false, CommandType::HarvestRock, false, false, false}, ActionType::Move },
		{ {false, CommandType::HarvestRock, false, false, true}, ActionType::HarvestRock },
		{ {false, CommandType::AttackIntruder, false, false, false}, ActionType::Move },
		{ {false, CommandType::AttackIntruder, false, false, true}, ActionType::AttackIntruder },
		{ {false, CommandType::None, true, false}, ActionType::AttackInRange },
		{ {false, CommandType::None, false, true, false}, ActionType::DropOffLoot },
		{ {false, CommandType::None, false, true, true}, ActionType::DropOffLoot },
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
	else
	{
		float distance = glm::length(*colonist.mTarget->position - colonist.mGOColonist->mesh->position);
		float reqDistance;
		switch (colonist.mCurrentCommand)
		{
		case CommandType::HarvestRock:
		{
			if (colonist.mTarget != nullptr)
			{
				if (colonist.mTarget->buildingType == GOLD)
				{
					reqDistance = 5.0f;
				}
				else reqDistance = 3.5f;
			}
		}
			break;
		default:
			reqDistance = 3.5f;
			break;
		}
		currentCondition.isTargetInRange = (distance <= reqDistance);
	}

	for (const Rule& rule : decisionTable)
	{
		if (rule.condition == currentCondition)
		{
			return rule.action;
		}
	}

	return ActionType::Idle;
}
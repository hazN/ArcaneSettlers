#include "DecisionTable.h"
#include "Colonist.h"
#include "cVAOManager/sModelDrawInfo.h"
#include "globalThings.h"
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
		{ {false, CommandType::AttackIntruder, false, false, true}, ActionType::AttackIntruder },
		{ {std::nullopt,  CommandType::None, true, std::nullopt, std::nullopt}, ActionType::AttackIntruder },
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
    {
        currentCondition.isTargetInRange = false;
    }
    else 
    {
        float maxExtent = 0.f;
        if (colonist.mTarget->mesh != nullptr) 
        {
            // Get the target drawinfo 
            sModelDrawInfo drawInfo;
            pVAOManager->FindDrawInfoByModelName(colonist.mTarget->mesh->meshName, drawInfo);
            // Get the max extent of the mesh
            maxExtent = glm::max(drawInfo.extentX, glm::max(drawInfo.extentY, drawInfo.extentZ));
            maxExtent *= colonist.mTarget->mesh->scaleXYZ.x;
            if (colonist.mTarget->buildingType == DEPOT)
                maxExtent += 3.f;
            else if (colonist.mTarget->buildingType == TREE)
                maxExtent -= 2.f;
        }
        // Get the distance between the colonist and the target 
        glm::vec3 colonistPos = colonist.mGOColonist->mesh->position;
        glm::vec3 targetPos = *colonist.mTarget->position;
        float distance = glm::distance(colonistPos, targetPos);

        currentCondition.isTargetInRange = (distance <= (maxExtent));
    }

    for (const Rule& rule : decisionTable) {
        if (rule.condition == currentCondition) {
            return rule.action;
        }
    }

    return ActionType::Idle;
}

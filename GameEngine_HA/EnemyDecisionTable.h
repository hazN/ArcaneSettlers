#pragma once

#include <vector>
#include <optional>

class Enemy;

enum class EnemyActionType {
    Move, AttackBuilding, AttackColonist, Idle
};

class EnemyDecisionTable {
public:
    EnemyDecisionTable();
    ~EnemyDecisionTable();
    EnemyActionType getNextAction(Enemy& enemy);

private:
    struct EnemyCondition {
        std::optional<bool> isColonistInRange;
        std::optional<bool> isBuildingInRange;
        bool operator==(const EnemyCondition& other) const {
            return (isColonistInRange == other.isColonistInRange || !isColonistInRange.has_value() || !other.isColonistInRange.has_value()) &&
                (isBuildingInRange == other.isBuildingInRange || !isBuildingInRange.has_value() || !other.isBuildingInRange.has_value());
        }
    };

    struct EnemyRule {
        EnemyCondition condition;
        EnemyActionType action;
    };

    std::vector<EnemyRule> decisionTable;
};
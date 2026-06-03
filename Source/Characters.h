#ifndef CHARACTERS_H
#define CHARACTERS_H

#include <string>
#include <vector>
#include <algorithm>
#include "Common.h"

struct ActiveCondition {
    ConditionType type;
    int duration;
};

class Player {
public:
    std::string name;
    int hp, maxHp;
    int mp, maxMp;
    std::vector<ActiveCondition> activeConditions;

    Player(std::string n, int h, int m) : name(n), hp(h), maxHp(h), mp(m), maxMp(m) {
        activeConditions.push_back({ ConditionType::Normal, 0 });
    }

    void addCondition(ConditionType type, int duration) {
        if (type == ConditionType::Normal) return;

        if (activeConditions.size() == 1 && activeConditions[0].type == ConditionType::Normal) {
            activeConditions.clear();
        }

        for (auto& ac : activeConditions) {
            if (ac.type == type) {
                ac.duration = duration;
                return;
            }
        }
        activeConditions.push_back({ type, duration });
    }

    bool hasCondition(ConditionType type) const {
        for (const auto& ac : activeConditions) if (ac.type == type) return true;
        return false;
    }

    void removeCondition(ConditionType type) {
        activeConditions.erase(
            std::remove_if(activeConditions.begin(), activeConditions.end(),
                [type](const ActiveCondition& ac) { return ac.type == type; }),
            activeConditions.end()
        );
        if (activeConditions.empty()) activeConditions.push_back({ ConditionType::Normal, 0 });
    }

    void cureAllConditions() {
        if (hp <= 0) return;
        activeConditions.clear();
        activeConditions.push_back({ ConditionType::Normal, 0 });
    }

    void takeDamage(int amount) {
        if (hp <= 0) return;
        hp = (std::max)(0, hp - amount);
        if (hp <= 0) {
            activeConditions.clear();
            activeConditions.push_back({ ConditionType::Dead, 0 });
        }
    }

    void receiveHeal(int amount) {
        if (hp <= 0) return;
        if (hasCondition(ConditionType::Burn)) {
            amount /= 2;
        }
        hp = (std::min)(maxHp, hp + amount);
    }
};

class Enemy {
public:
    std::string name;
    int hp, maxHp;
    Enemy(std::string n, int h) : name(n), hp(h), maxHp(h) {}
};

#endif
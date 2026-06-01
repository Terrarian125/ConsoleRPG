#ifndef CHARACTERS_H
#define CHARACTERS_H

#include <string>
#include <vector>
#include <algorithm>
#include "Common.h"

class Player {
public:
    std::string name;
    int hp, maxHp;
    int mp, maxMp;
    std::vector<ConditionType> conditions;

    Player(std::string n, int h, int m) : name(n), hp(h), maxHp(h), mp(m), maxMp(m) {
        conditions.push_back(ConditionType::Normal);
    }

    void addCondition(ConditionType type) {
        if (type == ConditionType::Normal) return;
        if (conditions.size() == 1 && conditions[0] == ConditionType::Normal) {
            conditions.clear();
        }
        for (auto c : conditions) if (c == type) return;
        conditions.push_back(type);
    }

    bool hasCondition(ConditionType type) const {
        for (auto c : conditions) if (c == type) return true;
        return false;
    }

    void removeCondition(ConditionType type) {
        conditions.erase(std::remove(conditions.begin(), conditions.end(), type), conditions.end());
        if (conditions.empty()) conditions.push_back(ConditionType::Normal);
    }

    void takeDamage(int amount) {
        if (hp <= 0) return;
        hp = (std::max)(0, hp - amount);
        if (hp <= 0) {
            conditions.clear();
            conditions.push_back(ConditionType::Dead);
        }
    }

    void receiveHeal(int amount) {
        if (hp <= 0) return;
        if (hasCondition(ConditionType::Burn)) {
            amount /= 2; // 火傷なら回復半減
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
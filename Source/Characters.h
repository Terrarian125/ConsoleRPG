//#pragma execution_character_set("utf-8")
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

    // ダメージを受ける処理（死亡判定つき）
    void takeDamage(int amount) {
        if (hp <= 0) return;
        hp = (std::max)(0, hp - amount);
        if (hp <= 0) {
            conditions.clear();
            conditions.push_back(ConditionType::Dead);
        }
    }

    // 回復を受ける処理（火傷の効果をここに内包）
    void receiveHeal(int amount) {
        if (hp <= 0) return; // 死亡者は回復しない

        // 火傷状態なら回復量が50%（半分）に低下
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
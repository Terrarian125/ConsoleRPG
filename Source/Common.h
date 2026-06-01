//#pragma execution_character_set("utf-8")
#ifndef COMMON_H
#define COMMON_H

#include <string>

enum class ConditionType { Normal, Bleeding, Burn, Regeneration, Dead };

class Condition {
public:
    static std::string getName(ConditionType type) {
        switch (type) {
        case ConditionType::Bleeding:     return "出血";
        case ConditionType::Burn:         return "火傷";
        case ConditionType::Regeneration: return "再生";
        case ConditionType::Dead:         return "死亡";
        default:                          return "正常";
        }
    }
};

#endif
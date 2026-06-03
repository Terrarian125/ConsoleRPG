#ifndef STAGEMANAGER_H
#define STAGEMANAGER_H

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <windows.h>
#include <algorithm>
#include "Common.h"
#include "Characters.h"
#include "BattleSystem.h"

struct StageLink {
    int up = -1, down = -1, left = -1, right = -1;
};

class StageManager {
private:
    std::map<int, std::vector<std::string>> maps;
    std::map<int, StageLink> links;

public:
    int currentStageId = 0;
    int playerX = 3;
    int playerY = 3;

    StageManager() {
        maps[0] = {
            "##########",
            "#........#",
            "#...D..R.#",
            "#........ ",
            "#....H...#",
            "##########"
        };
        links[0] = { -1, -1, -1, 1 };

        maps[1] = {
            "##########",
            "#........#",
            "#...E....#",
            " ........#",
            "#........#",
            "##########"
        };
        links[1] = { -1, -1, 0, -1 };
    }

    void setLogHandle(HANDLE dummy) {}

    std::vector<std::string>& getCurrentMap() { return maps[currentStageId]; }
    StageLink getCurrentLink() { return links[currentStageId]; }
    int getWidth() { return maps[currentStageId][0].size(); }
    int getHeight() { return maps[currentStageId].size(); }

    void triggerEvent(char cellType, std::vector<Player>& party) {
        if (cellType == 'D') {
            for (auto& p : party) {
                p.takeDamage(10);
                p.addCondition(ConditionType::Bleeding, 5);
                p.addCondition(ConditionType::Burn, 8);
            }
            Beep(200, 150);
        }
        else if (cellType == 'R') {
            for (auto& p : party) {
                p.removeCondition(ConditionType::Bleeding);
                p.removeCondition(ConditionType::Burn);
                p.addCondition(ConditionType::Regeneration, 10);
            }
            Beep(900, 100);
        }
        else if (cellType == 'H') {
            for (auto& p : party) {
                p.receiveHeal(p.maxHp);
                p.removeCondition(ConditionType::Bleeding);
                p.removeCondition(ConditionType::Burn);
            }
            Beep(800, 100);
        }
        else if (cellType == 'E') {
            for (auto& p : party) {
                if (p.hasCondition(ConditionType::Bleeding)) p.addCondition(ConditionType::Bleeding, 3);
                if (p.hasCondition(ConditionType::Burn)) p.addCondition(ConditionType::Burn, 5);
                if (p.hasCondition(ConditionType::Regeneration)) p.addCondition(ConditionType::Regeneration, 4);
            }

            Enemy owl("野生のフクロウ", 150);
            BattleSystem battle(party, owl);
            battle.startBattle();
            system("cls");
        }
    }
};

#endif
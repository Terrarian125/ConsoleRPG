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
    HANDLE logScreenBuffer;

public:
    int currentStageId = 0;
    int playerX = 3;
    int playerY = 3;

    StageManager() : logScreenBuffer(INVALID_HANDLE_VALUE) {
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

    void setLogHandle(HANDLE handle) { logScreenBuffer = handle; }

    std::vector<std::string>& getCurrentMap() { return maps[currentStageId]; }
    StageLink getCurrentLink() { return links[currentStageId]; }
    int getWidth() { return maps[currentStageId][0].size(); }
    int getHeight() { return maps[currentStageId].size(); }

    void triggerEvent(char cellType, std::vector<Player>& party) {
        if (cellType == 'D') {
            for (auto& p : party) {
                p.takeDamage(10);
                p.addCondition(ConditionType::Bleeding);
                p.addCondition(ConditionType::Burn);
            }
            Beep(200, 150);
        }
        else if (cellType == 'R') {
            for (auto& p : party) {
                p.removeCondition(ConditionType::Bleeding);
                p.removeCondition(ConditionType::Burn);
                p.addCondition(ConditionType::Regeneration);
            }
            Beep(900, 100);
        }
        else if (cellType == 'H') { // 泉の処理を修正
            for (auto& p : party) {
                p.receiveHeal(p.maxHp);

                // 出血と火傷を完全に治す
                p.removeCondition(ConditionType::Bleeding);
                p.removeCondition(ConditionType::Burn);

                // 再生(REGEN)を持っている場合は、消さずにそのまま残す
                // 何もない場合はNormal（正常）に戻す
                if (!p.hasCondition(ConditionType::Regeneration)) {
                    p.conditions = { ConditionType::Normal };
                }
            }
            Beep(800, 100);
        }
        else if (cellType == 'E') {
            Enemy owl("野生のフクロウ", 150);
            BattleSystem battle(party, owl, logScreenBuffer); // ログ窓ハンドルを渡す
            battle.startBattle();
            system("cls");
        }
    }
};

#endif
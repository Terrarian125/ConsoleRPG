//#pragma execution_character_set("utf-8")
#ifndef BATTLESYSTEM_H
#define BATTLESYSTEM_H

#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <conio.h>
#include <windows.h>
#include <algorithm>
#include "Common.h"
#include "Characters.h"

class BattleSystem {
private:
    std::vector<Player>& party;
    Enemy enemy;
    int statusAnimIndex;
    std::chrono::steady_clock::time_point lastAnimTime;
    std::string battleLog;

    void gotoxy(int x, int y) {
        COORD coord = { (SHORT)x, (SHORT)y };
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
    }

    void drawEnemyArea() {
        gotoxy(0, 1);
        std::cout << "       ,_,   \n";
        std::cout << "      (o,o)   [ " << enemy.name << " が現れた！ ]\n";
        std::cout << "      {`\"'}   HP: " << enemy.hp << " / " << enemy.maxHp << "        \n";
        std::cout << "      -\"-\"-  \n";
        std::cout << "============================================================\n";
    }

    void drawStatusArea() {
        gotoxy(0, 8);
        std::cout << "------------------------------------------------------------\n";
        for (int i = 0; i < 4; ++i) std::cout << "| " << party[i].name << "\t";
        std::cout << "|\n";
        for (int i = 0; i < 4; ++i) std::cout << "|    HP " << party[i].hp << "/" << party[i].maxHp << "\t";
        std::cout << "|\n";
        for (int i = 0; i < 4; ++i) std::cout << "|    MP " << party[i].mp << "/" << party[i].maxMp << "\t";
        std::cout << "|\n";
        for (int i = 0; i < 4; ++i) {
            if (party[i].hp <= 0) {
                std::cout << "|    状態: " << Condition::getName(ConditionType::Dead) << "\t";
            }
            else {
                int condIdx = statusAnimIndex % party[i].conditions.size();
                std::cout << "|    状態: " << Condition::getName(party[i].conditions[condIdx]) << "\t";
            }
        }
        std::cout << "|\n============================================================\n";
        std::cout << " [ログ]: " << battleLog << "                                       \n";
    }

    char waitKeyWithAnimation() {
        while (true) {
            auto now = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastAnimTime).count() >= 1000) {
                statusAnimIndex++;
                drawStatusArea();
                lastAnimTime = now;
            }
            if (_kbhit()) return _getch();
            Sleep(20);
        }
    }

    // パーティ全員が死亡しているかチェック
    bool isPartyAllDead() {
        for (const auto& p : party) {
            if (p.hp > 0) return false;
        }
        return true;
    }

public:
    BattleSystem(std::vector<Player>& p, Enemy e)
        : party(p), enemy(e), statusAnimIndex(0), battleLog("") {
        lastAnimTime = std::chrono::steady_clock::now();
    }

    bool startBattle() {
        system("cls");
        std::cout << "=== BATTLE START ===========================================\n";
        battleLog = "コマンドを入力してください。";

        while (enemy.hp > 0 && !isPartyAllDead()) {
            for (int i = 0; i < 4; ++i) {
                if (party[i].hp <= 0) continue;

                // --- ターン開始時のスリップダメージ/再生処理 ---
                if (party[i].hasCondition(ConditionType::Bleeding)) {
                    int dot = (std::max)(1, (int)(party[i].hp * 0.05)); // 残りHPの5%
                    party[i].takeDamage(dot);
                    battleLog = party[i].name + " は出血により " + std::to_string(dot) + " ダメージを受けた！";
                    drawStatusArea(); Beep(180, 200); Sleep(800);
                    if (party[i].hp <= 0) continue; // 出血死したら次の人へ
                }
                if (party[i].hasCondition(ConditionType::Regeneration)) {
                    int regen = (std::max)(1, (int)(party[i].maxHp * 0.05)); // 最大HPの5%
                    int oldHp = party[i].hp;
                    party[i].receiveHeal(regen);
                    battleLog = party[i].name + " は再生で " + std::to_string(party[i].hp - oldHp) + " 回復した！";
                    drawStatusArea(); Beep(700, 100); Sleep(800);
                }
                // -----------------------------------------------

                while (true) {
                    drawEnemyArea();
                    drawStatusArea();
                    gotoxy(0, 16);
                    std::cout << "1. たたかう | 2. 防御 | 3. 逃げる                     \n";
                    std::cout << "[" << party[i].name << "] の行動を選択してください ＞ ";

                    char choice = waitKeyWithAnimation();
                    if (choice == '1') {
                        int damage = 25;
                        enemy.hp = (std::max)(0, enemy.hp - damage);
                        battleLog = party[i].name + " の攻撃！ " + enemy.name + " に " + std::to_string(damage) + " のダメージ！";
                        Beep(600, 50);
                        break;
                    }
                    else if (choice == '2') {
                        battleLog = party[i].name + " は身を固めている。";
                        Beep(400, 50);
                        break;
                    }
                    else if (choice == '3') {
                        battleLog = "パーティは逃げ出した！";
                        drawStatusArea();
                        Sleep(1000);
                        return false;
                    }
                }
                if (enemy.hp <= 0) break;
            }

            if (enemy.hp <= 0) {
                battleLog = enemy.name + " を倒した！";
                drawEnemyArea();
                drawStatusArea();
                Beep(1000, 100); Beep(1300, 100); Beep(1600, 300);
                Sleep(1500);
                return true;
            }

            // 敵のターン
            drawEnemyArea();
            drawStatusArea();
            gotoxy(0, 16);
            std::cout << "                                                                \n";

            std::vector<int> aliveTargets;
            for (int i = 0; i < 4; ++i) if (party[i].hp > 0) aliveTargets.push_back(i);
            if (aliveTargets.empty()) break;

            int targetIdx = aliveTargets[rand() % aliveTargets.size()];
            int eDamage = 15;
            party[targetIdx].takeDamage(eDamage);
            battleLog = enemy.name + " の反撃！ " + party[targetIdx].name + " は " + std::to_string(eDamage) + " のダメージを受けた！";
            Beep(200, 200);

            if (isPartyAllDead()) break;
            Sleep(1000);
        }
        return !isPartyAllDead(); // 全滅してたらfalse、勝ってたらtrue
    }
};

#endif
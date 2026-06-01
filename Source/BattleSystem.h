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

    // ★戦闘ログを履歴として保存するベクトル（最大5行保持）
    std::vector<std::string> battleLogs;

    void gotoxy(int x, int y) {
        COORD coord = { (SHORT)x, (SHORT)y };
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
    }

    // ログを追加する関数（5行を超えたら古いものを消す）
    void addLog(std::string text) {
        battleLogs.push_back(text);
        if (battleLogs.size() > 5) {
            battleLogs.erase(battleLogs.begin()); // 一番古いログを削除
        }
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

        // ★ここに最大5行のログ履歴を綺麗に並べて描画する
        std::cout << "【 戦闘ログ履歴 】\n";
        for (int i = 0; i < 5; ++i) {
            if (i < battleLogs.size()) {
                // 行の末尾をスペースで埋めて、古い残像を消す
                std::string line = " " + battleLogs[i];
                if (line.length() < 60) line.append(60 - line.length(), ' ');
                std::cout << line << "\n";
            }
            else {
                // ログがまだ5行ない場合は空行で埋める
                std::cout << "                                                            \n";
            }
        }
        std::cout << "------------------------------------------------------------\n";
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

    bool isPartyAllDead() {
        for (const auto& p : party) {
            if (p.hp > 0) return false;
        }
        return true;
    }

public:
    // main.cppからの第3引数(HANDLE)を受け取れるようにしつつ、内部では使わない形にします
    BattleSystem(std::vector<Player>& p, Enemy e, HANDLE dummy = NULL)
        : party(p), enemy(e), statusAnimIndex(0) {
        lastAnimTime = std::chrono::steady_clock::now();
        addLog("--- 戦闘開始: " + enemy.name + " ---");
    }

    bool startBattle() {
        system("cls");
        std::cout << "=== BATTLE START ===========================================\n";

        while (enemy.hp > 0 && !isPartyAllDead()) {
            bool playerActed = false;

            for (int i = 0; i < 4; ++i) {
                if (party[i].hp <= 0) continue;

                // ターン開始時の状態異常
                if (party[i].hasCondition(ConditionType::Bleeding)) {
                    int dot = (std::max)(1, (int)(party[i].hp * 0.05));
                    party[i].takeDamage(dot);
                    addLog("【出血】" + party[i].name + " は出血で " + std::to_string(dot) + " ダメージを受けた。");
                    drawStatusArea(); Beep(180, 150);
                    if (party[i].hp <= 0) continue;
                }
                if (party[i].hasCondition(ConditionType::Regeneration)) {
                    int regen = (std::max)(1, (int)(party[i].maxHp * 0.05));
                    int oldHp = party[i].hp;
                    party[i].receiveHeal(regen);
                    addLog("【再生】" + party[i].name + " は再生で " + std::to_string(party[i].hp - oldHp) + " 回復した。");
                    drawStatusArea(); Beep(700, 100);
                }

                while (true) {
                    drawEnemyArea();
                    drawStatusArea();
                    gotoxy(0, 21); // ログの下に行動選択を表示
                    std::cout << "1. たたかう | 2. 防御 | 3. 逃げる                     \n";
                    std::cout << "[" << party[i].name << "] の行動を選択してください ＞ ";

                    char choice = waitKeyWithAnimation();
                    if (choice == '1') {
                        int damage = 25;
                        enemy.hp = (std::max)(0, enemy.hp - damage);
                        addLog("⚔️ " + party[i].name + " の攻撃！ " + enemy.name + " に " + std::to_string(damage) + " のダメージ！");
                        Beep(600, 50);
                        playerActed = true;
                        break;
                    }
                    else if (choice == '2') {
                        addLog("🛡️ " + party[i].name + " は身を固めている。");
                        Beep(400, 50);
                        playerActed = true;
                        break;
                    }
                    else if (choice == '3') {
                        addLog("🏃 パーティは逃げ出した！");
                        // 逃げログを一瞬見せる
                        drawEnemyArea(); drawStatusArea();
                        Sleep(1000);
                        return false;
                    }
                }
                if (enemy.hp <= 0) break;
            }

            if (enemy.hp <= 0) {
                addLog("🎉 " + enemy.name + " を倒した！");
                drawEnemyArea();
                drawStatusArea();
                Beep(1000, 100); Beep(1300, 100); Beep(1600, 300);
                Sleep(1500);
                return true;
            }

            // 味方ターン終了後のウェイト
            if (playerActed && !isPartyAllDead()) {
                drawEnemyArea();
                drawStatusArea();
                gotoxy(0, 21);
                std::cout << "―― 味方の行動が終了しました。[Enter] を押して敵のターンへ ――\n";
                std::cout << "                                                                     "; // 下の行をクリア
                while (true) {
                    char nextKey = waitKeyWithAnimation();
                    if (nextKey == 13) break;
                }
            }

            // 敵のターン
            drawEnemyArea();
            drawStatusArea();

            std::vector<int> aliveTargets;
            for (int i = 0; i < 4; ++i) if (party[i].hp > 0) aliveTargets.push_back(i);
            if (aliveTargets.empty()) break;

            int targetIdx = aliveTargets[rand() % aliveTargets.size()];
            int eDamage = 15;
            party[targetIdx].takeDamage(eDamage);
            addLog("💥 " + enemy.name + " の反撃！ " + party[targetIdx].name + " は " + std::to_string(eDamage) + " のダメージ！");
            Beep(200, 200);

            if (isPartyAllDead()) break;

            // 敵ターン終了後のウェイト
            drawEnemyArea();
            drawStatusArea();
            gotoxy(0, 21);
            std::cout << "―― 敵の行動が終了しました。[Enter] を押して次のターンへ  ――\n";
            std::cout << "                                                                     ";
            while (true) {
                char nextKey = waitKeyWithAnimation();
                if (nextKey == 13) break;
            }
        }
        return !isPartyAllDead();
    }
};

#endif
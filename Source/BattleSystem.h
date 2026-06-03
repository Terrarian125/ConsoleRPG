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
    std::vector<std::string> battleLogs;
    int antidoteCount;

    void gotoxy(int x, int y) {
        COORD coord = { (SHORT)x, (SHORT)y };
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
    }

    void addLog(std::string text) {
        battleLogs.push_back(text);
        if (battleLogs.size() > 5) {
            battleLogs.erase(battleLogs.begin());
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
        // 1行目: 名前
        for (int i = 0; i < 4; ++i) std::cout << "| " << party[i].name << "\t";
        std::cout << "|\n";
        // 2行目: HP
        for (int i = 0; i < 4; ++i) std::cout << "|    HP " << party[i].hp << "/" << party[i].maxHp << "\t";
        std::cout << "|\n";
        // 3行目: MP
        for (int i = 0; i < 4; ++i) std::cout << "|    MP " << party[i].mp << "/" << party[i].maxMp << "\t";
        std::cout << "|\n";

        // 4行目: 状態の【名前】だけを表示（文字ズレ防止のためタブをきれいに通す）
        for (int i = 0; i < 4; ++i) {
            if (party[i].hp <= 0) {
                std::cout << "|    状態: 死亡\t";
            }
            else {
                int condIdx = statusAnimIndex % party[i].activeConditions.size();
                auto ac = party[i].activeConditions[condIdx];
                std::cout << "|    状態: " << Condition::getName(ac.type) << "\t";
            }
        }
        std::cout << "|\n";

        // 5行目: ★新規追加：残り時間の行
        for (int i = 0; i < 4; ++i) {
            if (party[i].hp <= 0) {
                std::cout << "|    残り: ---\t";
            }
            else {
                int condIdx = statusAnimIndex % party[i].activeConditions.size();
                auto ac = party[i].activeConditions[condIdx];
                if (ac.type == ConditionType::Normal) {
                    std::cout << "|    残り: ---\t";
                }
                else {
                    // 「残り: 03T」のように2桁に揃えて表示
                    char buf[16];
                    sprintf_s(buf, "%02dT", ac.duration);
                    std::cout << "|    残り: " << buf << "\t";
                }
            }
        }
        std::cout << "|\n============================================================\n";

        std::cout << "【 戦闘ログ履歴 】\n";
        for (int i = 0; i < 5; ++i) {
            if (i < battleLogs.size()) {
                std::string line = " " + battleLogs[i];
                if (line.length() < 60) line.append(60 - line.length(), ' ');
                std::cout << line << "\n";
            }
            else {
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

    int selectTarget() {
        while (true) {
            gotoxy(0, 22); // 行数が増えたので少し下に配置
            std::cout << "誰に使いますか？ (1～4: メンバーを選択 / 0: キャンセル)            \n";
            std::cout << "選択してください ＞ ";
            char ch = waitKeyWithAnimation();
            if (ch == '0') return -1;
            if (ch >= '1' && ch <= '4') {
                int idx = ch - '1';
                if (party[idx].hp > 0) return idx;
            }
        }
    }

public:
    BattleSystem(std::vector<Player>& p, Enemy e, HANDLE dummy = NULL)
        : party(p), enemy(e), statusAnimIndex(0), antidoteCount(3) {
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

                std::vector<ConditionType> toRemove;
                for (auto& ac : party[i].activeConditions) {
                    if (ac.type == ConditionType::Bleeding) {
                        int dot = (std::max)(1, (int)(party[i].hp * 0.05));
                        party[i].takeDamage(dot);
                        addLog("【出血】" + party[i].name + " は出血で " + std::to_string(dot) + " ダメージを受けた。");
                        ac.duration--;
                        if (ac.duration <= 0) toRemove.push_back(ConditionType::Bleeding);
                        drawStatusArea(); Beep(180, 150); Sleep(500);
                        if (party[i].hp <= 0) break;
                    }
                    else if (ac.type == ConditionType::Burn) {
                        ac.duration--;
                        if (ac.duration <= 0) toRemove.push_back(ConditionType::Burn);
                    }
                    else if (ac.type == ConditionType::Regeneration) {
                        int regen = (std::max)(1, (int)(party[i].maxHp * 0.05));
                        int oldHp = party[i].hp;
                        party[i].receiveHeal(regen);
                        addLog("【再生】" + party[i].name + " は再生で " + std::to_string(party[i].hp - oldHp) + " 回復した。");
                        ac.duration--;
                        if (ac.duration <= 0) toRemove.push_back(ConditionType::Regeneration);
                        drawStatusArea(); Beep(700, 100); Sleep(500);
                    }
                }
                for (auto type : toRemove) {
                    party[i].removeCondition(type);
                    addLog("✨ " + party[i].name + " の [" + Condition::getName(type) + "] が治った！");
                }
                if (party[i].hp <= 0) continue;

                while (true) {
                    drawEnemyArea();
                    drawStatusArea();
                    gotoxy(0, 22); // 行数が増えたので位置調整
                    std::cout << "1. たたかう | 2. 防御 | 3. 道具(包帯:" << antidoteCount << "個) | 4. 逃げる       \n";
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
                        if (antidoteCount <= 0) {
                            addLog("❌ 包帯がありません！");
                            continue;
                        }
                        int target = selectTarget();
                        if (target == -1) continue;

                        antidoteCount--;
                        party[target].cureAllConditions();
                        addLog("🧪 " + party[i].name + " は包帯を " + party[target].name + " に使った！");
                        Beep(500, 100); Beep(650, 100);
                        playerActed = true;
                        break;
                    }
                    else if (choice == '4') {
                        addLog("🏃 パーティは逃げ出した！");
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

            if (playerActed && !isPartyAllDead()) {
                drawEnemyArea();
                drawStatusArea();
                gotoxy(0, 22);
                std::cout << "―― 味方の行動が終了しました。[Enter] を押して敵のターンへ ――\n";
                std::cout << "                                                                     ";
                while (true) {
                    char nextKey = waitKeyWithAnimation();
                    if (nextKey == 13) break;
                }
            }

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

            drawEnemyArea();
            drawStatusArea();
            gotoxy(0, 22);
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
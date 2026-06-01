#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <conio.h>
#include <windows.h>
#include <algorithm>

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
};

class Enemy {
public:
    std::string name;
    int hp, maxHp;
    Enemy(std::string n, int h) : name(n), hp(h), maxHp(h) {}
};

// ==========================================
// 戦闘管理クラス (BattleSystem)
// ==========================================
class BattleSystem {
private:
    std::vector<Player>& party;
    Enemy enemy;
    int statusAnimIndex = 0;
    std::chrono::steady_clock::time_point lastAnimTime;
    std::string battleLog = "";

    void gotoxy(int x, int y) {
        COORD coord = { (SHORT)x, (SHORT)y };
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
    }

    // 敵のアスキーアートとHPの描画
    void drawEnemyArea() {
        gotoxy(0, 1);
        std::cout << "       ,_,   \n";
        std::cout << "      (o,o)   [ " << enemy.name << " が現れた！ ]\n";
        std::cout << "      {`\"'}   HP: " << enemy.hp << " / " << enemy.maxHp << "        \n";
        std::cout << "      -\"-\"-  \n";
        std::cout << "============================================================\n";
    }

    // 味方ステータスの描画（1秒ごとのリアルタイム点滅対応）
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

        // バトルログ（行動結果）の表示
        std::cout << " [ログ]: " << battleLog << "                                       \n";
    }

    // 入力待ちの間も裏でタイマーを回す関数（Enter不要の入力）
    char waitKeyWithAnimation() {
        while (true) {
            auto now = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastAnimTime).count() >= 1000) {
                statusAnimIndex++;
                drawStatusArea(); // ステータスだけ書き換え
                lastAnimTime = now;
            }
            if (_kbhit()) {
                return _getch();
            }
            Sleep(20);
        }
    }

public:
    BattleSystem(std::vector<Player>& p, Enemy e) : party(p), enemy(e) {
        lastAnimTime = std::chrono::steady_clock::now();
    }

    bool startBattle() {
        system("cls");
        std::cout << "=== BATTLE START ===========================================\n";

        while (enemy.hp > 0) {
            // 1. プレイヤー側のターン
            for (int i = 0; i < 4; ++i) {
                if (party[i].hp <= 0) continue; // 死亡者はスキップ

                drawEnemyArea();
                drawStatusArea();

                gotoxy(0, 15);
                std::cout << "1. たたかう | 2. 防御 | 3. 逃げる\n";
                std::cout << "[" << party[i].name << "] の行動を選択してください ＞ ";

                char choice = waitKeyWithAnimation();

                if (choice == '1') { // たたかう
                    int damage = 25; // 固定ダメージ（本来は計算式を入れる）
                    enemy.hp = (std::max)(0, enemy.hp - damage);
                    battleLog = party[i].name + " の攻撃！ " + enemy.name + " に " + std::to_string(damage) + " のダメージ！";
                    Beep(600, 50); // 攻撃ビープ音
                }
                else if (choice == '3') { // 逃げる
                    battleLog = "パーティは逃げ出した！";
                    drawStatusArea();
                    Sleep(1000);
                    return false; // 戦闘終了（逃走）
                }
                else {
                    battleLog = party[i].name + " は身を固めている。";
                }

                if (enemy.hp <= 0) break; // 敵を倒したら即終了
            }

            if (enemy.hp <= 0) {
                battleLog = enemy.name + " を倒した！";
                drawEnemyArea();
                drawStatusArea();
                Beep(1000, 100); Beep(1300, 100); Beep(1600, 300); // 勝利ファンファーレ風
                Sleep(1500);
                return true; // 勝利
            }

            // 2. 敵側のターン
            drawEnemyArea();
            drawStatusArea();
            gotoxy(0, 15); std::cout << "                                                                \n"; // 入力プロンプト消去

            // 生きているメンバーからランダムにターゲットを選定
            std::vector<int> aliveTargets;
            for (int i = 0; i < 4; ++i) if (party[i].hp > 0) aliveTargets.push_back(i);

            if (aliveTargets.empty()) break; // 味方全滅

            int targetIdx = aliveTargets[rand() % aliveTargets.size()];
            int eDamage = 15;
            party[targetIdx].hp = (std::max)(0, party[targetIdx].hp - eDamage);

            battleLog = enemy.name + " の反撃！ " + party[targetIdx].name + " は " + std::to_string(eDamage) + " のダメージを受けた！";
            Beep(200, 200); // 敵の攻撃音

            // ターンの終わりに生存チェック
            bool partyAllDead = true;
            for (auto& p : party) if (p.hp > 0) partyAllDead = false;
            if (partyAllDead) {
                battleLog = "パーティは全滅した...";
                drawStatusArea();
                Sleep(2000);
                return false;
            }

            Sleep(1000); // 敵の行動を見せるためのウェイト
        }
        return true;
    }
};

int main() {
    // カーソルを非表示
    std::cout << "\x1b[?25l";

    // パーティデータ
    std::vector<Player> party;
    party.push_back(Player("プレイヤー１", 100, 100));
    party.push_back(Player("プレイヤー２", 80, 50));
    party.push_back(Player("プレイヤー３", 120, 200));
    party.push_back(Player("プレイヤー４", 90, 10));

    // テスト用にプレイヤー2に状態異常をつけておく
    party[1].addCondition(ConditionType::Bleeding);
    party[1].addCondition(ConditionType::Burn);

    // 敵データ（フクロウ型モンスター）
    Enemy owl("野生のフクロウ", 150);

    // バトル開始
    BattleSystem battle(party, owl);
    battle.startBattle();

    std::cout << "\x1b[?25h"; // カーソルを戻す
    return 0;
}
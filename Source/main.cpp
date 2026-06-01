#include <iostream>
#include <vector>
#include <chrono>
#include <conio.h>
#include <windows.h>
#include "Common.h"
#include "Characters.h"
#include "StageManager.h"

class GameMain {
private:
    StageManager stageIdx;
    std::vector<Player> party;
    int statusAnimIndex = 0;
    std::chrono::steady_clock::time_point lastAnimTime;

    // ログウィンドウ専用のハンドル
    HANDLE logOutHandle = INVALID_HANDLE_VALUE;

    void gotoxy(int x, int y) {
        COORD coord = { (SHORT)x, (SHORT)y };
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
    }

    bool isPartyAllDead() {
        for (const auto& p : party) {
            if (p.hp > 0) return false;
        }
        return true;
    }

    void drawGameOver() {
        system("cls");
        Beep(300, 500); Beep(250, 500); Beep(200, 1000);
        std::cout << "========================================================\n";
        std::cout << "                    GAME OVER                           \n";
        std::cout << "========================================================\n";
        std::cout << "          パーティは全滅してしまった...\n\n";
        std::cout << "          何かキーを押すと終了します。\n";
        std::cout << "========================================================\n";
        _getch();
    }

    // ★完全に独立した2つ目のウィンドウ（コンソール）を生成する
    void initLogWindow() {
        // 新しい独立したコンソールを割り当てる
        if (AllocConsole()) {
            // 新しく作られたウィンドウの出力ハンドルを取得
            logOutHandle = CreateFileA("CONOUT$", GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

            if (logOutHandle != INVALID_HANDLE_VALUE) {
                // ログウィンドウ専用のタイトルを設定
                // メイン窓と見分けがつくように、一時的にタイトルを変更
                SetConsoleTitleA("RPG LOG WINDOW");

                // 最初のメッセージを出力
                DWORD written;
                std::string initMsg = "=== RPG LOG WINDOW ===\nここに戦闘の履歴がリアルタイムで流れます。\n\n";
                WriteConsoleA(logOutHandle, initMsg.c_str(), (DWORD)initMsg.length(), &written, NULL);
            }
        }
    }

public:
    GameMain() {
        initLogWindow(); // 先にログ用別ウィンドウを開く

        // メインウィンドウのタイトルを「GAME SCREEN」に設定し直す
        // （これでメイン窓とログ窓のタイトルが別々になります）
        HWND mainWnd = GetConsoleWindow();
        SetWindowTextA(mainWnd, "RPG GAME SCREEN");

        stageIdx.setLogHandle(logOutHandle);

        party.push_back(Player("プレイヤー１", 100, 100));
        party.push_back(Player("プレイヤー２", 80, 50));
        party.push_back(Player("プレイヤー３", 120, 200));
        party.push_back(Player("プレイヤー４", 90, 10));
        lastAnimTime = std::chrono::steady_clock::now();
    }

    ~GameMain() {
        if (logOutHandle != INVALID_HANDLE_VALUE) {
            CloseHandle(logOutHandle);
        }
        FreeConsole(); // 割り当てたログウィンドウを解放
    }

    void drawScreen() {
        if (isPartyAllDead()) return;

        gotoxy(0, 0);
        std::cout << "=== STAGE " << stageIdx.currentStageId << " =====================================\n";
        auto& currentMap = stageIdx.getCurrentMap();
        for (int y = 0; y < stageIdx.getHeight(); ++y) {
            for (int x = 0; x < stageIdx.getWidth(); ++x) {
                if (x == stageIdx.playerX && y == stageIdx.playerY) std::cout << "P";
                else std::cout << currentMap[y][x];
            }
            std::cout << "\n";
        }
        std::cout << "========================================================\n";
        std::cout << "[操作] W,A,S,D:移動 / Q:終了 (D:罠 H:泉 R:再生 E:敵)\n";
        std::cout << "--------------------------------------------------------\n";
        drawStatusArea();
    }

    void drawStatusArea() {
        gotoxy(0, 10);
        for (int i = 0; i < 4; ++i) std::cout << "| " << party[i].name << "\t";
        std::cout << "|\n";
        for (int i = 0; i < 4; ++i) std::cout << "|   HP " << party[i].hp << "/" << party[i].maxHp << "\t";
        std::cout << "|\n";
        for (int i = 0; i < 4; ++i) std::cout << "|   MP " << party[i].mp << "/" << party[i].maxMp << "\t";
        std::cout << "|\n";
        for (int i = 0; i < 4; ++i) {
            if (party[i].hp <= 0) std::cout << "|   状態: " << Condition::getName(ConditionType::Dead) << "\t";
            else {
                int condIdx = statusAnimIndex % party[i].conditions.size();
                std::cout << "|   状態: " << Condition::getName(party[i].conditions[condIdx]) << "\t";
            }
        }
        std::cout << "|\n========================================================\n";
    }

    void movePlayer(int dx, int dy) {
        int nextX = stageIdx.playerX + dx;
        int nextY = stageIdx.playerY + dy;
        auto& currentMap = stageIdx.getCurrentMap();
        StageLink link = stageIdx.getCurrentLink();

        if (nextX < 0 && link.left != -1) {
            stageIdx.currentStageId = link.left;
            stageIdx.playerX = stageIdx.getWidth() - 1;
            system("cls"); drawScreen(); return;
        }
        if (nextX >= stageIdx.getWidth() && link.right != -1) {
            stageIdx.currentStageId = link.right;
            stageIdx.playerX = 0;
            system("cls"); drawScreen(); return;
        }

        if (nextX >= 0 && nextX < stageIdx.getWidth() && nextY >= 0 && nextY < stageIdx.getHeight()) {
            if (currentMap[nextY][nextX] != '#') {
                stageIdx.playerX = nextX;
                stageIdx.playerY = nextY;

                for (auto& p : party) {
                    if (p.hp <= 0) continue;
                    if (p.hasCondition(ConditionType::Bleeding)) {
                        p.takeDamage((std::max)(1, (int)(p.maxHp * 0.05)));
                    }
                    if (p.hasCondition(ConditionType::Regeneration)) {
                        p.receiveHeal((std::max)(1, (int)(p.maxHp * 0.05)));
                    }
                }

                if (currentMap[nextY][nextX] != '.') stageIdx.triggerEvent(currentMap[nextY][nextX], party);
                drawScreen();
            }
        }
    }

    void run() {
        std::cout << "\x1b[?25l";
        system("cls");
        drawScreen();
        while (true) {
            if (isPartyAllDead()) {
                drawGameOver();
                break;
            }

            auto now = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastAnimTime).count() >= 1000) {
                statusAnimIndex++;
                drawStatusArea();
                lastAnimTime = now;
            }

            if (_kbhit()) {
                char ch = _getch();
                if (ch == 'q' || ch == 'Q') break;
                switch (ch) {
                case 'w': case 'W': movePlayer(0, -1); break;
                case 's': case 'S': movePlayer(0, 1); break;
                case 'a': case 'A': movePlayer(-1, 0); break;
                case 'd': case 'D': movePlayer(1, 0); break;
                }
            }
            Sleep(30);
        }
        std::cout << "\x1b[?25h";
    }
};

int main() {
    GameMain game;
    game.run();
    return 0;
}
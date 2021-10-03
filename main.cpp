#include <cstdio>
#include <iostream>
#include <string>
#include <ctime>
#include <cstdlib>
#include <cstring>
#include <conio.h>

// 用户名 人名 对照表
// VVsxmja 蔡一锴
// Mistyreed 张天杰

inline int abs(int x) {
    return x >= 0 ? x : -x;
}

namespace minesweeper {
    const int kMaxMapHeight = 16, kMaxMapWidth = 30; // 最大地图高度 / 宽度
    const int kMaxBomb = 99; // 最大地雷数量
    const char kCharUnopened = '#'; // 未打开的格子的显示字符

    int map_height = 9, map_width = 9;

    int bomb_total = 10, bomb_opened = 0;
    
    // 炸弹地图，0 = 周围无炸弹，1~8 = 周围炸弹数， -1 = 炸弹
    int bomb_map[ kMaxMapHeight ][ kMaxMapWidth ];

    // 标记地图，0 = 未打开未标记，1 = 未打开有标记，-1 = 打开
    int flag_map[ kMaxMapHeight ][ kMaxMapWidth ];

    int foc_x = 0, foc_y = 0; // 光标位置，x为纵坐标，y为横坐标

    bool GetChoice(std::string hint) {
        // 读取y/n，返回1/0
        char choice;
        while (true) {
            std::cout << hint; // 输出提示信息

            std::cin >> choice; 
            // 读入单个字符作为玩家选择
           // if (kbhit()) {
            //    choice=getch();
           // }
            if (choice == 'y') {
                return true;
            } else if (choice == 'n') {
                return false;
            }
        }
    }

    void PrintWelcome() {
        // 打印欢迎语
        std::cout << "This is a Minesweeper game." << std::endl;
        std::cout << "Type 'y' to start a new game, type 'n' to exit." << std::endl;
        return;
    }

    void PlayerLose() {
        // 打印地雷位置
        PrintScreen(1);
        // 打印结束语
        std::cout << "You lose." << std::endl;
        std::cout << "Type 'y' to start another round, type 'n' to exit." << std::endl;
    }

    void PlayerWin() {
        // 打印结束语
        std::cout << "You win." << std::endl;
        std::cout << "Type 'y' to start another round, type 'n' to exit." << std::endl;
        return;
    }

    void ClearScreen() {
        //std::cout << "\033[2J\033[1;1H";
        return;
    }

    void PrintScreen( int opt ) {
        // 输出当前棋盘,opt==0不输出炸弹,opt==1输出炸弹(游戏输了时显示)
        ClearScreen(); // 清屏
        std::cout << "shit" << endl;
        std::cout << opt << endl;
        for (int i = 0; i < map_height; ++i) {
            for (int j = 0; j < map_width; ++j) {
                // 单数位字符，为光标、旗子或空格
                if (i == foc_x && j == foc_y) {
                    // 左光标
                    std::cout << '[';
                } else if (i == foc_x && j == foc_y + 1) {
                    // 右光标
                    std::cout << ']';
                } else if (flag_map[ i ][ j ] == 1) {
                    // 旗子左半边
                    std::cout << '(';
                } else if (j != 0 && flag_map[ i ][ j - 1 ] == 1) {
                    // 旗子右半边
                    std::cout << ')';
                } else {
                    // 空格
                    std::cout << ' ';
                }
                // 偶数位字符，为当前格子信息
                if (opt == 1 && bomb_map[ i ][ j ] == -1 ) {
                    std::cout << '*';
                }else if (flag_map[ i ][ j ] == -1) {
                    // 当前格子已打开
                    if (bomb_map[ i ][ j ] == 0) {
                        // 周围无炸弹，显示空格
                        std::cout << ' ';
                    } else if (bomb_map[ i ][ j ] == -1) {
                        // 当前格子为炸弹
                            std::cout << ' ';
                    } else {
                        // 周围有炸弹，显示炸弹数
                        std::cout << bomb_map[ i ][ j ];
                    }
                } else {
                    // 当前格子未打开
                    std::cout << kCharUnopened;
                }
            }
            // 如果行尾的右光标无法输出，就补全
            if (foc_x == i && foc_y == map_width - 1) {
                std::cout << ']';
            } else if (flag_map[ i ][ map_width - 1 ] == 1) {
                std::cout << ')';
            }
            std::cout << std::endl;
        }
    }

    bool SurroundedByBombs(const int &new_bomb_x, const int &new_bomb_y) {
        // 判断周围八个格子是否都是地雷
        for (int i = -1; i <= 1; ++i) {
            for (int j = -1; j <= 1; ++j) {
                if (new_bomb_x + i >= map_height
                    || new_bomb_x + i < 0
                    || new_bomb_y >= map_width
                    || new_bomb_y < 0
                ) {
                    // 排除越界情况
                    continue;
                }
                if (i == 0 && j == 0) {
                    // 排除当前格子本身
                    continue;
                }
                if (bomb_map[ new_bomb_x + i ][ new_bomb_y + j ] != -1) {
                    // 周围某个格子不是炸弹
                    return false;
                }
            }
        }
        // 周围所有格子都是炸弹
        return true;
    }
    
    void GenerateBomb() {
        // 游戏初始化，开始的格子一定安全
        srand(time(0)); // 置随机种子
        int bomb_generated = 0;
        while (bomb_generated < bomb_total) {
            // 随机生成新坐标
            int new_bomb_x = rand() % map_height;
            int new_bomb_y = rand() % map_width;
            // 判断是否生成在初始格子
            if (new_bomb_x == foc_x && new_bomb_y == foc_y) {
                continue;
            }
            // 判断周围八个格子是否都是地雷
            if (SurroundedByBombs(new_bomb_x, new_bomb_y)) {
                continue;
            }
            // 判断地雷是否重复生成
            if (bomb_map[ new_bomb_x ][ new_bomb_y ]) {
                continue;
            }
            // 摆放地雷
            ++bomb_generated;
            bomb_map[ new_bomb_x ][ new_bomb_y ] = -1;
        }
        return;
    }
    
    void PreCountBomb() {
        // 预处理每个非炸弹格子周围的炸弹数
        for (int i = 0; i < map_height; ++i) {
            for (int j = 0; j < map_width; ++j) {
                // 遍历所有格子
                if (bomb_map[ i ][ j ] == -1) {
                    // 如果当前格子有地雷
                    for (int k = -1; k <= 1; ++k) {
                        for (int z = -1; z <= 1; ++z) {
                            // 遍历周围
                            if (i + k >= map_height
                                || i + k < 0
                                || j + z >= map_width
                                || j + z < 0
                            ) {
                                // 排除越界情况
                                continue;
                            }
                            if (bomb_map[ i + k ][ j + z ] == -1) {
                                // 排除地雷（自身和周围地雷）
                                continue;
                            }
                            ++bomb_map[ i + k ][ j + z ]; // 将周围格子的炸弹计数加一
                        }
                    }
                }
            }
        }
    }

    char GetCommand() {
        // 读取并返回单个字符作为玩家命令
        char input_char;
        std::cin >> input_char;
        return input_char;
    }

    bool CheckAllBlockOpened() {
        // 判断是否所有安全的格子都被揭开
        for (int i = 0; i < map_height; ++i) {
            for (int j = 0; j < map_width; ++j) {
                // 检查所有方块
                if( bomb_map[ i ][ j ] != -1 && flag_map[ i ][ j ] != -1) {
                    // 某个安全的格子没有被打开
                    return false;
                }
            }
        }
        return true;
    }

    bool CheckBombOpened() {
        // 揭开当前格子后，判断当前格子下是否有炸弹
        if (bomb_map[ foc_x ][ foc_y ] == -1) {
            // 当前格子下有炸弹
            return true;
        } else {
            // 当前格子下无炸弹
            return false;
        }
    }

    void GameInit() {
        // 游戏初始化
        bomb_opened = 0; // 重置发现的炸弹
        foc_x = foc_y = 0; // 重置光标位置
        memset(bomb_map, 0, sizeof(bomb_map)); // 重置炸弹地图
        memset(flag_map, 0, sizeof(flag_map)); // 重置标记地图
        return;
    }

    void MoveFocus(int dx, int dy) {
        // 移动光标，遇到边界就忽略
        int new_x = foc_x + dx;
        int new_y = foc_y + dy;
        if (new_x >= 0 && new_x < map_height) {
            foc_x = new_x;
        }
        if (new_y >= 0 && new_y < map_width) {
            foc_y = new_y;
        }
        return;
    }

    void OpenBlock(const int &target_x, const int &target_y) {
        // 揭开指定的的方块
        if (flag_map[ target_x ][ target_y ] != 0) {
            // 当前方块被打开或被标记
            return;
        }
        // 打开当前格子，处理标记地图
        flag_map[ target_x ][ target_y ] = -1;
        // 如果当前格子是空格，打开相邻格子
        if (bomb_map[ target_x ][ target_y ] == 0) {
            for (int i = -1; i <= 1; ++i) {
                for (int j = -1; j <= 1; ++j) {
                    if (i == 0 && j == 0) {
                        // 跳过当前格子
                        continue;
                    }
                    if (target_x + i >= map_height
                        || target_x + i < 0
                        || target_y + j >= map_width
                        || target_y + j < 0
                    ) {
                        // 排除越界情况
                        continue;
                    }
                    // 打开相邻格子
                    OpenBlock(target_x + i, target_y + j);
                }
            }
        }
        return;
    }
    
    void MainLoop() {
        // 主循环
        bool player_first_open_block = true;
        while (true) {
            // 打印棋盘
            PrintScreen(0);
            // 读取输入
            char command = GetCommand();
            // 执行操作
            if (command == 'w') {
                // 光标向上移动
                MoveFocus(-1, 0);
            } else if (command == 'a'){
                // 光标向左移动
                MoveFocus(0, -1);
            } else if (command == 's'){
                // 光标向下移动
                MoveFocus(1, 0);
            } else if (command == 'd'){
                // 光标向右移动
                MoveFocus(0, 1);
            } else if (command == 'j'){
                // 揭开当前格子
                if (player_first_open_block) {
                    // 玩家第一次揭开方块，进行初始化
                    GenerateBomb();
                    PreCountBomb();
                    player_first_open_block = false;
                }
                // 揭开方块
                OpenBlock(foc_x, foc_y);
                if (CheckBombOpened()) {
                    // 玩家揭开炸弹
                    PlayerLose();
                    break;
                }
                if (CheckAllBlockOpened()) {
                    // 玩家胜利
                    PlayerWin();
                    break;
                }
            } else if (command == 'k') {
                // 标记当前格子（取反）
                if (flag_map[ foc_x ][ foc_y ] == 0) {
                    flag_map[ foc_x ][ foc_y ] = 1;
                } else if (flag_map[ foc_x ][ foc_y ] == 1) {
                    flag_map[ foc_x ][ foc_y ] = 0;
                }
            } else {
                // 未读取到合法输入，再次读取
                continue;
            }
        }
        return;
    }

    int GameMain() {
        // 游戏主函数
        PrintWelcome(); // 打印欢迎语
        int if_start_game = GetChoice("Start? (y/n) "); // 玩家决定是否开始游戏
        if (if_start_game) {
            GameInit(); // 游戏初始化
            MainLoop(); // 进入主循环
            // 主循环结束，即一轮游戏结束
            int if_next_round = GetChoice("Start? (y/n) "); // 玩家决定是否再来一局
            while (if_next_round) {
                GameInit(); // 游戏初始化
                MainLoop(); // 再次进入主循环
                if_next_round = GetChoice("Start? (y/n) "); // 玩家决定是否再来一局
            }
        }
        return 0;
    }
}

namespace test {
    void TestBombGenerating() {
        minesweeper::GenerateBomb();
        for (int i = 0; i < minesweeper::map_height; ++i) {
            for (int j = 0; j < minesweeper::map_width; ++j) {
                std::cout << minesweeper::bomb_map[ i ][ j ] << ' ';
            }
            std::cout << std::endl;
        }
        return;
    }
    void TestPreCountBomb() {
        minesweeper::GenerateBomb();
        for (int i = 0; i < minesweeper::map_height; ++i) {
            for (int j = 0; j < minesweeper::map_width; ++j) {
                std::cout << minesweeper::bomb_map[ i ][ j ] << ' ';
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
        minesweeper::PreCountBomb();
        for (int i = 0; i < minesweeper::map_height; ++i) {
            for (int j = 0; j < minesweeper::map_width; ++j) {
                std::cout << minesweeper::bomb_map[ i ][ j ] << ' ';
            }
            std::cout << std::endl;
        }
        return;
    }
    void TestPrintScreen() {
        minesweeper::GenerateBomb(); // 生成地雷
        minesweeper::PreCountBomb(); // 生成地雷地图
        // 随机放置光标
        srand(time(0));
        minesweeper::foc_x = rand() % minesweeper::map_height;
        minesweeper::foc_y = rand() % minesweeper::map_width;
        // 打印正常棋盘
        minesweeper::PrintScreen(); 
        // 打开所有格子
        for (int i = 0; i < minesweeper::map_height; ++i) {
            for (int j = 0; j < minesweeper::map_width; ++j) {
                minesweeper::flag_map[ i ][ j ] = -1;
            }
        }
        // 随机放置光标
        minesweeper::foc_x = rand() % minesweeper::map_height;
        minesweeper::foc_y = rand() % minesweeper::map_width;
        // 打印打开过的棋盘
        minesweeper::PrintScreen();
        return;
    }
}

int main() {
    std::cout << "MINESWEEPER" << std::endl;
    minesweeper::GameMain();
    // test::TestPrintScreen();
    // test::TestPreCountBomb();

    return 0;
}
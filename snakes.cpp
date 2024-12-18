//Start of the program
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <cstdlib>
#include <termios.h>
#include <unistd.h>
#include <ctime>

#define  LOW_SPEED  550
#define  MED_SPEED  400
#define  HIGH_SPEED 250
#define  MIN_MAP_SIZE 9
#define  MAX_MAP_SIZE 16
#define  FILE_RESET_VAL 22140
#define  SAVE_FILE_NAME "snakes_data.dat"

class SnakeGame {
private:
    short horz, vert, score, highscore, points, speed, level;
    short head, index, prevScore, insect, frog, time_;
    short pace, side, area, headShape, bodyShape, pulse;
    char value, wall[3];
    std::vector<short> location, trail;
    std::string saveFileName, map, bline, uline;
    bool bonus;

    std::mutex mtx;
    std::condition_variable cv;

    void hitWall();
    void initialize();
    void readValue();
    void takeInput();
    void gameControl();
    void mapSnake();
    void gameAlgorithm();
    void gameDisplay();
    void gameToggle(bool);
    void speedSelector();
    void handleFileStatus();
    void mazeBuilder();
    void fileManage(std::string, char);
    void mainMenu();
    void clearConsole();

public:
    SnakeGame() 
        : score(0), highscore(0), points(0), speed(MED_SPEED), level(2), pace(2),  
          trail(1, 0), value('d'), wall{':', '|','|'}, side(14), area(0),  
          head(0),insect(0), frog(0), time_(0), headShape(0), bodyShape(0), 
          prevScore(-1), pulse(0), saveFileName(SAVE_FILE_NAME), bonus(false){}

    void run() {
        clearConsole();
        fileManage("0", 'i');
        mainMenu();
    }
};

void SnakeGame::hitWall() {
    if (level == 1) {
        if (head >= area)  head -= area;
        else if (head < 0) head += area;
        else if (head%side == side-1 && value == 'd') head = (head - side)+1;
        else if (head%side == side-1 && value == 'a') head = (head + side)-1;
    }
    if (level == 2) {
        if (head%side == side-1 || head >= area || head < 0) gameToggle(false);
    } 
    if (level == 3) {
        if (head%side == side-1 || head >= area || head < 0) gameToggle(false);
        if (head/side > (side/5) && head%side == (side/2)-1 && 
            head/side < (0.8*side)) gameToggle(false);
        if (head%side > (side/5)-1 && head/side == side/2 && 
            head%side < (0.8*side)-1) gameToggle(false);
    }
}

void SnakeGame::initialize() {
    location.clear();
    area = side * side;
    frog = area + 1;
    uline = std::string(side + 1, '_');
    bline = std::string(side + 1, '"');
    map = std::string(area, ' ');
    srand((unsigned) time(0));
    int loc;
    for (index = 0; index < area/2; index++) {
        if (level == 3) {
            loc = rand() % (area-2) + 1;
            while (loc / side > side / 5 && loc % side == (side/2) - 1 && loc / side < 0.8*side ||
            (loc % side > (side/5)-1 && loc / side == side / 2 && loc % side < (0.8*side) - 1)) {
                loc = rand() % (area - 2) + 1;
            }
            location.push_back(loc);
        } else location.push_back(rand() % (area-2) + 1);
    }
}

void SnakeGame::readValue() {
    static struct termios oldt, newt;
    static char keyPress = ' ';
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    keyPress = getchar();
    if (keyPress == 'w' || keyPress == 's' || keyPress == 'd' || keyPress == 'a') {
        value = keyPress;
    }
    if (keyPress == 't') gameToggle(false);

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    cv.notify_one();
}

void SnakeGame::takeInput() {
    std::thread th(&SnakeGame::readValue, this);
    std::unique_lock<std::mutex> lck(mtx);
    while (cv.wait_for(lck, std::chrono::milliseconds(speed)) == std::cv_status::timeout) {
        gameControl();
    }
    th.join();
    gameControl();
}

void SnakeGame::gameControl() {
    switch (value) {
        case 'd': head ++;      break;
        case 's': head += side; break;
        case 'a': head --;      break;
        case 'w': head -= side; break;
    }
    gameAlgorithm();
}
void SnakeGame::mapSnake() {
    if (headShape == bodyShape + 1) {
        map[head - 1] = '=';
        map[head] = '>';
    } else if (headShape == bodyShape - 1) {
        map[head + 1] = '=';
        map[head] = '<';
    } else if (headShape / side == (bodyShape / side) + 1) {
        map[head - side] = '|';
        map[head] = 'v';
    } else if (headShape / side == (bodyShape / side) - 1) {
        map[head + side] = '|';
        map[head] = '^';
    }
    map[insect] = '+';
}
void SnakeGame::gameAlgorithm() {
    clearConsole();
    hitWall();
    if (prevScore != score) {
        insect = location[score % (area / 2)];
        if ((insect + 1) % side == 0) insect++;
        if (score % 4 == 3 && !bonus) {
            frog = location[(insect/2) % (area/2)];
            if ((frog + 1) % side == 0) frog++;
            bonus = true;
        }
    }
    prevScore = score;
    if (find(trail.begin(), trail.end(), head) != trail.end()) gameToggle(false);
    trail.push_back(head);
    ++ pulse;
    headShape = trail[trail.size() - 1];
    bodyShape = trail[trail.size() - 2];
    mapSnake();
    if (bonus) {
        if (time_ < pulse) {
            map[frog] = '@';
            std::cout << int(1.5 * side) - pulse + time_ << std::endl;
        }
        if (time_ == pulse - int(1.5 * side)) {
            bonus = false;
            map[frog] = ' ';
            frog = area + 1;
        }
    } else {
        time_ = pulse;
        std::cout << std::endl;
    }
    if (head == insect) {
        std::cout << "\a";
        points += pace * level;
        score++;
    }
    if (head == frog) {
        std::cout << "\a";
        points += pace * level * 3;
        bonus = false;
        frog = area + 1;
    }
    if (pulse > 3 - score) {
        map[trail[0]] = ' ';
        if (head != insect) trail.erase(trail.begin());
    }
    gameDisplay();
}

void SnakeGame::gameDisplay() {
    std::cout << points << std::endl << uline << std::endl;
    for (vert = 0; vert < side; vert++) {
        for (horz = 0; horz < side; horz++) {
            if (horz == side - 1 || horz == 0) std::cout << wall[level - 1];
            if (horz == side - 1 && vert == side - 1) std::cout << std::endl << bline;
            switch (map[(vert * side) + horz]) {
                case '<': std::cout << "◀"; break;
                case '>': std::cout << "▶"; break;
                case '^': std::cout << "▲"; break;
                case 'v': std::cout << "▼"; break;
                case '|': std::cout << "∥"; break;
                default : std::cout << map[(vert * side) + horz];
            }
        } std::cout << std::endl;
    }
}

void SnakeGame::gameToggle(bool toggle) {
    if (toggle) takeInput();
    else {
        clearConsole();
        fileManage(std::to_string(pace) + std::to_string(level) + std::to_string(side), 's');
        fileManage(std::to_string(points), 'o');
        std::cout << "Game Over!\nScore:" << points << "\n";
        exit(0);
    }
    gameToggle(true);
}

void SnakeGame::speedSelector() {
    switch (pace) {
        case 1: speed = LOW_SPEED; break;
        case 3: speed = HIGH_SPEED; break;
        default:speed = MED_SPEED; pace = 2; break;
    }
}

void SnakeGame::mainMenu() {
    char choice = 'z';
    mazeBuilder();
    std::cout << "\nPress:\n1 to Play\n2 for Help\n3 for Game Settings\n4 to exit\n";
    std::cin >> choice;
    clearConsole();
    if (choice == '1') gameToggle(true);
    if (choice == '2') {
        std::cout << "CONTROLS\nPRESS\n w TO MOVE UPWARD\n s TO MOVE DOWNWARDS \n ";
        std::cout << "d TO MOVE RIGHT \n a TO MOVE LEFT\n t TO QUIT THE GAME\n";
        sleep(3);
        run();
    }
    if (choice == '3') {
        std::string value_entered;
        std::cout << "Control the Snake speed. PRESS\n1 : Easy\n2 : Medium\n3 : Hard\n";
        std::cin >> value_entered;
        if (all_of(value_entered.begin(),value_entered.end(),::isdigit)) pace = stoi(value_entered);
        std::cout << "Control the Game difficulty level. PRESS\n1 : LEVEL 1\n2 : LEVEL 2\n3 : LEVEL 3\n";
        std::cin >> value_entered;
        if (all_of(value_entered.begin(),value_entered.end(),::isdigit)) level = stoi(value_entered);
        speedSelector();
        std::cout << "Enter the map size of range[10-15]\n";
        std::cin >> value_entered;
        if (all_of(value_entered.begin(),value_entered.end(),::isdigit)) {
            int num = stoi(value_entered);
            if (num > MIN_MAP_SIZE && num < MAX_MAP_SIZE) side = num;
            else std::cout<<"Size entered not within range[10-15]!\nReverting to previous size...\n";
            initialize();
            sleep(2);
        }
        clearConsole();
        mainMenu();
    } 
    else return;
}

void SnakeGame::handleFileStatus() {
    std::cout << "The save file is corrupted! \nKindly restart the game as the save file is reset\n";
    std::ofstream fout(saveFileName, std::ios::app | std::ios::binary);
    fout << std::endl << FILE_RESET_VAL;
    fout.close();
    sleep(1);
    std::cout << "Terminating..\n";
}

void SnakeGame::mazeBuilder()
{
    if (level == 3) {
        for(index = 0; index < map.size(); index++) {
            if (index/side > side/5 && index%side == (side/2)-1 && index/side < (0.8*side)) {
                map[index] = '|';
            }
            if (index%side > (side/5)-1 && index/side == side/2 && index%side < (0.8*side)-1) {
                map[index] = '-';
            }
        }
    }
}
void SnakeGame::fileManage(std::string data, char option) {
    if (option == 'i') {
        std::ifstream fin(saveFileName,std::ios::binary);
        if (!fin) {
            std::cout << "Welcome to the game!";
            initialize();
        }
        else {
            std::string save_data;
            while (fin.good()) getline(fin, save_data);
            if (save_data.size() < 5 || save_data.size() > 8 
                    || !all_of(save_data.begin(), save_data.end(), ::isdigit)) {
                handleFileStatus();
                fin.close();
                exit(0);
            }
            pace = save_data[0] - '0';
            level= save_data[1] - '0';
            side = save_data[2] - '0';
            side*= 10;
            side+= save_data[3] - '0';
            initialize();
            if (highscore < stoi(save_data.substr(4))) highscore = stoi(save_data.substr(4));
            std::cout << "Welcome back to the game!\nThe highscore is " << highscore;
            speedSelector();
        }
        fin.close();
    } else if (option == 's') {
        std::ofstream fout(saveFileName, std::ios::app | std::ios::binary);
        fout << std::endl << data;
        fout.close();
    } else if (option == 'o') {
        std::ofstream fout(saveFileName, std::ios::app | std::ios::binary);
        if (stoi(data) > highscore) {
            highscore = stoi(data);
            std::cout << "HIGHSCORE! " << std::endl;
        }
        fout << highscore;
        fout.close();
    }
}

void SnakeGame::clearConsole() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

int main() {
    SnakeGame game;
    game.run();
    return 0;
}
//End of the program

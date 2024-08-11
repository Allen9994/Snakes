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

using namespace std;

class SnakeGame {
private:
    short i, j, score, highscore, points, speed, level, pace, head, side, area;
    char value;
    vector<int> location;
    vector<int> trail;
    string map;
    string uline;
    string bline;
    char wall[2];
    short tr, insect, frog, time_, p, q, pulse;
    bool flag, bonus;

    mutex mtx;
    condition_variable cv;

    void hitWall();
    void initialize();
    void read_value();
    void takeInput();
    void control(char value);
    void process();
    void display();
    void gameToggle(bool toggle);
    void speedSelector();
    void checkFileStatus();
    void fileManage(string data, char option);
    void mainMenu();
    void clearConsole();

public:
    SnakeGame(int size_map) 
        : score(0), highscore(0), points(0), speed(400), level(2), pace(2), head(0), 
          value('d'), wall{':', '|'}, side(size_map), area(0), trail(1, 0), 
          tr(0), insect(0), frog(0), time_(0), p(0), q(0), pulse(0),
          flag(false), bonus(false){}

    void run() {
        fileManage("0", 'i');
        mainMenu();
    }
};
void SnakeGame::hitWall() {
    if (level == 1) {
        if (head >= area) head -= area;
        else if (head < 0) head += area;
        else if (head%side == side-1 && value == 'd') head = (head - side) + 1;
        else if (head%side == side-1 && value == 'a') head = (head + side) - 1;
    }
    if (level == 2) {
        if (head%side == side-1 || head >= area || head < 0) gameToggle(false);
    } 
}
void SnakeGame::initialize() {
    location.clear();
    area = side * side;
    frog = area + 1;
    uline = string(side + 1, '_');
    bline = string(side + 1, '"');
    map = string(area, ' ');
    srand((unsigned) time(0));
    for (short index = 0; index < area / 2; index++)
        location.push_back((rand() % (area - 2)) + 1);
}

void SnakeGame::read_value() {
    static struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    char c = getchar();
    if (c == 'w' || c == 's' || c == 'd' || c == 'a' || c == 't') value = c;
    if (value == 't') gameToggle(false);

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    cv.notify_one();
}

void SnakeGame::takeInput() {
    thread th(&SnakeGame::read_value, this);
    unique_lock<mutex> lck(mtx);
    while (cv.wait_for(lck, chrono::milliseconds(speed)) == cv_status::timeout)
        control(value);
    th.join();
    control(value);
}

void SnakeGame::control(char value) {
    switch (value) {
        case 'd': head++;       break;
        case 's': head += side; break;
        case 'a': head--;       break;
        case 'w': head -= side; break;
    }
    process();
}

void SnakeGame::process() {
    clearConsole();
    hitWall();
    if (tr != score) flag = false;
    tr = score;
    if (!flag) {
        flag = true;
        insect = location[score % (area / 2)];
        if ((insect + 1) % side == 0) insect++;
        if (score % 4 == 3 && !bonus) {
            frog = location[(insect / 2) % (area / 2)];
            if ((frog + 1) % side == 0) frog++;
            bonus = true;
        }
    }
    

    if (find(trail.begin(), trail.end(), head) != trail.end()) gameToggle(false);
    trail.push_back(head);
    ++pulse;
    p = trail[trail.size() - 1];
    q = trail[trail.size() - 2];

    if (p == q + 1) {
        map[head - 1] = '=';
        map[head] = '>';
    } else if (p == q - 1) {
        map[head + 1] = '=';
        map[head] = '<';
    } else if (p / side == (q / side) + 1) {
        map[head - side] = '|';
        map[head] = 'v';
    } else if (p / side == (q / side) - 1) {
        map[head + side] = '|';
        map[head] = '^';
    }
    map[insect] = '+';
    if (bonus) {
        if (time_ < pulse) {
            map[frog] = '@';
            cout << int(1.5 * side) - pulse + time_ << endl;
        }
        if (time_ == pulse - int(1.5 * side)) {
            bonus = false;
            map[frog] = ' ';
        }
    } else {
        time_ = pulse;
        cout << endl;
    }
    if (head == insect) {
        cout << "\a";
        points += pace * level;
        score++;
    }
    if (head == frog) {
        cout << "\a";
        points += pace * level * 3;
        bonus = false;
        frog = area + 1;
    }
    if (pulse > 3 - score) {
        map[trail[0]] = ' ';
        if (head != insect) trail.erase(trail.begin());
    }
    display();
}

void SnakeGame::display() {
    cout << points << endl << uline << endl;
    for (j = 0; j < side; j++) {
        for (i = 0; i < side; i++) {
            if (i == side - 1 || i == 0) cout << wall[level - 1];
            if (i == side - 1 && j == side - 1) cout << endl << bline;
            switch (map[(j * side) + i]) {
                case '<': cout << "◀"; break;
                case '>': cout << "▶"; break;
                case '^': cout << "▲"; break;
                case 'v': cout << "▼"; break;
                case '|': cout << "∥"; break;
                default : cout << map[(j * side) + i];
            }
        } cout << endl;
    }
}

void SnakeGame::gameToggle(bool toggle) {
    if (toggle) takeInput();
    else {
        clearConsole();
        fileManage(to_string(pace) + to_string(level) + to_string(side), 's');
        fileManage(to_string(points), 'o');
        cout << "Game Over!\nScore:" << points << "\n";
        exit(0);
    }
    gameToggle(true);
}

void SnakeGame::speedSelector() {
    switch (pace) {
        case 1: speed = 550; break;
        case 3: speed = 250; break;
        default:speed = 400; pace = 2; break;
    }
}

void SnakeGame::mainMenu() {
    char choice = 'z';
    cout << "\nCreated by Allen\n";
    cout << "Press:\n1 to Play\n2 for Help\n3 for Game Settings\n4 to exit\n";
    cin >> choice;
    clearConsole();
    if (choice == '1') gameToggle(true);
    if (choice == '2') {
        cout << "CONTROLS\nPRESS\n w TO MOVE UPWARD\n s TO MOVE DOWNWARDS \n d TO MOVE RIGHT \n a TO MOVE LEFT";
        cin >> choice;
        clearConsole();
        mainMenu();
    }
    if (choice == '3') {
        cout << "Control the Snake Speed. PRESS\n1 : Easy\n2 : Medium\n3 : Hard\n";
        cin >> pace;
        cout << "Control the Game Difficulty level. PRESS\n1 : LEVEL 1\n2 : LEVEL 2\n";
        cin >> level;
        level = (level == 1) ? 1 : 2;
        speedSelector();
        cout << "Enter the map size of range[10-20]\n";
        string size_entered;
        cin>>size_entered;
        if(all_of(size_entered.begin(),size_entered.end(),::isdigit)) {
            int num = stoi(size_entered);
            if(num > 9 && num < 21) {
                side = num;
                initialize();
            }
        }
        gameToggle(true);
    } else return;
}

void SnakeGame::checkFileStatus() {
    cout << "The save file is corrupted! \nKindly restart the game as the save file is reset\n";
    ofstream fout("snakes_data.txt", ios::app);
    fout << endl << "220";
    fout.close();
    sleep(1);
    cout << "Terminating..\n";
}

void SnakeGame::fileManage(string data, char option) {
    if (option == 'i') {
        ifstream fin("snakes_data.txt");
        if (!fin) {
            cout << "Welcome to the game!";
            initialize();
        }
        else {
            string save_data;
            while (fin.good()) getline(fin, save_data);
            if (save_data.size() <= 2 || !all_of(save_data.begin(), save_data.end(), ::isdigit)) {
                checkFileStatus();
                fin.close();
                exit(0);
            }
            pace = save_data[0] - '0';
            level = save_data[1] - '0';
            side = save_data[2] - '0';
            side *= 10;
            side += save_data[3] - '0';
            initialize();
            if (highscore < stoi(save_data.substr(4))) highscore = stoi(save_data.substr(4));
            cout << "Welcome back to the game!\nThe highscore is " << highscore;
            speedSelector();
        }
        fin.close();
    } else if (option == 's') {
        ofstream fout("snakes_data.txt", ios::app);
        fout << endl << data;
        fout.close();
    } else if (option == 'o') {
        ofstream fout("snakes_data.txt", ios::app);
        if (stoi(data) > highscore) {
            highscore = stoi(data);
            cout << "HIGHSCORE! " << endl;
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
    SnakeGame game(14);
    game.run();
    return 0;
}
//End of the program

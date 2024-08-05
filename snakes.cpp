//Start of the program
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <stdlib.h>
#include <termios.h> //termios, TCSANOW, ECHO, ICANON
#include <unistd.h> 
#include <time.h>
using namespace std;

short score = 0, highscore = 0, points = 0, speed = 400, level = 2, pace = 2, head = 0;
short i,j;
char value = 'd', wall[] = {':','|'};
const short side = 10;
const short area = side * side;
short p[area-1] = {0};
vector<int> trail(1, 0); 
string map(area,' ');
string uline(side+1,'_');
string bline(side+1,'"');

condition_variable cv;
void control(char);
void display();
void gameToggle(bool);
void process();
void mainMenu();
void fileManage(string,char);
void speedSelector();
void checkFileStatus();

void randomize()
{
    srand((unsigned) time(0));
    for (short index = 0; index < area/2; index++) p[index] = (rand() % area-2) + 1;
}

void read_value() //inputting value from user
{   
    static struct termios oldt, newt;
    tcgetattr( STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON);
    tcsetattr( STDIN_FILENO, TCSANOW, &newt);
 
    char c = getchar();
    if (c == 'w' || c == 's' || c == 'd' || c == 'a' || c == 't') value = c;
    if(value == 't'){
        system("clear");
        gameToggle(false);
    }

    tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
  
    cv.notify_one();
}
void takeInput() //function to accept the value parallelly while game is proceeding.  
{
    thread th(read_value);
    mutex mtx;
    unique_lock<mutex> lck(mtx);
    while (cv.wait_for(lck, chrono::milliseconds(speed)) == cv_status::timeout) control(value);
    th.join();
    control(value);
}
void control(char value) //Converts user input to the direction snake must move and stores all the movements into the array
{   
    switch(value){      
        case 'd': head++;        break;
        case 's': head+=side;    break;
        case 'a': head--;        break;
        case 'w': head-=side;    break;
    }
    process();
}
void process()   //Brain of the program. Entire game operation happens here. 
{
    system("clear");
    static short tr=0, insect=0, frog=101, time_=0;
    static bool flag=false, bonus=false;
    if(tr != score) flag=false;
    tr=score;
    if(!flag){
        insect=p[score%(area/2)];
        if((insect+1)%side == 0) insect++;
        flag = true;
        if(score%4 == 3 && !bonus)
        {
            frog=p[(insect/2)%(area/2)];
            if((frog+1)%side == 0) frog++;
            bonus = true;
        }
    }
    if(level == 1){
        if(head >= area)    head -= area;
        else if(head < 0)   head += area;
    }
    else if(level == 2 && (head >= area || head < 0)) gameToggle(false);
    
    static short t=0,p=0,q=0;
    static int pulse = 0;
    if(find(trail.begin(), trail.end(), head) != trail.end()) gameToggle(false); 
    trail.push_back(head);
    ++pulse;
    p = trail[trail.size()-1];
    q = trail[trail.size()-2];
    
    if(p == q+1){   
        map[head-1]= '=';
        map[head]= '>';
    }else 
    if(p == q-1){
        map[head+1]= '=';
        map[head]= '<';
    }else 
    if(p/side == (q/side)+1){
        map[head-side]= '|';
        map[head]= 'v';
    }else 
    if(p/side == (q/side)-1){
        map[head+side]= '|';
        map[head]= '^';
    }
    map[insect]='+'; //The point which determines the score and increments the length of the snake 
    if(bonus)
    {
        if (time_ < pulse){
            map[frog]='@';
            cout<<int(1.5*side)-pulse+time_<<endl;
        }
        if (time_ == pulse-int(1.5*side)){
            bonus = false; 
            map[frog] = ' ';
            frog=area+1;
        }
    }
    else{
        time_ = pulse;
        cout<<endl;
    }
    if(head == insect){
        cout<<"\a";
        points += pace*level;
        score++;
    }
    if(head == frog){
        cout<<"\a";
        points += pace*level*3;
        bonus = false;
    }
    if(pulse > 3-score){
        map[trail[0]] = ' ';      //For shortening the snake length dynamically
        if (head != insect) trail.erase(trail.begin());
    }   
    display();
}
void display()
{
    cout<<points<<endl<<uline<<endl;
    for (j=0;j<side;j++){  //Designing the 2Dmodel : Borders not made yet   
        for (i=0;i<side;i++){
            if(level == 2){
                if((map[(j*side)+i] == '>' && i == side-1) || (map[(j*side)+i] == '<' && i == side-1)){
                    cout<<endl;
                    system("clear");
                    gameToggle(false);
                }
            }
            else{
                if(map[(j*side)+i] == '<' && i == side-1)        head += side+1;
                else if(map[(j*side)+i] == '>' && i == side-1)   head -= side+1;
            }
            if(i == side-1 || i == 0)       cout<<wall[level-1];
            if(i == side-1 && j == side-1)  cout<<endl<<bline;
            
            switch(map[(j*side)+i]){
                case '<': cout<<"◀"; break;
                case '>': cout<<"▶"; break;
                case '^': cout<<"▲"; break;
                case 'v': cout<<"▼"; break;
                case '|': cout<<"∥"; break;
                default : cout<<map[(j*side)+i];
            }
        }cout<<endl;
    }
}
int main()  
{ 
    fileManage("0",'i');
    randomize();
    mainMenu();
}
void gameToggle(bool toggle) 
{
    if(toggle) takeInput();
    else{
        fileManage(to_string(pace) + to_string(level),'s');
        fileManage(to_string(points),'o');
        cout<<"Game Over!\nScore:"<<points<<"\n";
        abort();
    }
    gameToggle(true);
}
void mainMenu()   //Main Menu
{   
    char choice='z';
    cout<<"\nCreated by Allen\n";
    cout<<"Press:\n1 to Play\n2 for Help\n3 for Game Settings\n4 to exit\n";
    cin>>choice;
    system("clear");
    if(choice == '1') gameToggle(true);
    if(choice == '2')   //Instructions
    {   
        cout<<"CONTROLS\nPRESS\n w TO MOVE UPWARD\n s TO MOVE DOWNWARDS \n d TO MOVE RIGHT \n a TO MOVE LEFT";
        cin>>choice;
        system("clear");
        main();
    }
    if(choice == '3')
    {
        cout<<"Control the Snake Speed. PRESS\n1 : Easy\n2 : Medium\n3 : Hard\n";
        cin>>pace;
        cout<<"Control the Game Difficulty level. PRESS\n1 : LEVEL 1\n2 : LEVEL 2\n";
        cin>>level;
        level = (level == 1) ? 1 : 2;
        speedSelector();
        gameToggle(true);
    }
    else abort();
}
void fileManage(string data, char option)
{
    if(option == 'i')
    {
        ifstream fin("snakes_data.txt"); 
        if(!fin) cout<<"Welcome to the game!"; 
        else
        {
            string save_data;
            while (fin.good()) getline(fin,save_data);
            if(save_data.size() <= 2 || !all_of(save_data.begin(), save_data.end(), ::isdigit)) {
                checkFileStatus();
                fin.close();
                abort();
            }
            pace =  save_data[0] - '0';
            level = save_data[1] - '0';
            if (highscore < stoi(save_data.substr(2))) highscore = stoi(save_data.substr(2)) ;
            cout<<"Welcome back to the game!\nThe highscore is "<<highscore;
            speedSelector();
        }
        fin.close();
    }
    else if(option == 's') {
        ofstream fout("snakes_data.txt",ios::app);
        fout<<endl<<data;
        fout.close();
    }
    else if(option == 'o'){
        ofstream fout("snakes_data.txt", ios::app);
        if(stoi(data) > highscore){
            highscore = stoi(data);
            cout<<"HIGHSCORE! "<<highscore<<endl;
        }
        fout<<highscore;
        fout.close();
    }
}
void speedSelector()
{
    switch(pace){
        case 1: speed = 550;break;
        case 3: speed = 250;break;
        default:speed = 400;pace = 2;break;
    }
}
void checkFileStatus()
{
    cout<<"The save file is corrupted! \nKindly restart the game as the save file is reset\n";
    ofstream fout("snakes_data.txt",ios::app);
    fout<<endl<<"220";
    fout.close();
    sleep(1);
    cout<<"Terminating..\n";
}

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

short score = 0;
short points = 0;
short i,j;
char wall[] = {':','|'};
short side = 10;
short area = side * side;
short speed = 400, level = 2, pace = 2, head = 0;
char value = 'd';
vector<int> trail(1, 0); 
string map(area,' ');
short highscore = 0;
string uline(side+1,'_');
string bline(side+1,'"');

condition_variable cv;
void control(char);
void display();
void gameToggle(short,bool);
void process();
void mainMenu();
void fileManage(string,char);
void speedSelector();

short randomize(short poll)
{   
    srand((unsigned) time(0));
    short p[area-1];
    for (short index = 0; index < area/2; index++) 
    {
        p[index] = (rand() % area-2) + 1;
    }
    return p[poll];
}
void read_value() //inputting value from user
{   
    static struct termios oldt, newt;

     /* tcgetattr gets the parametervas of the current terminal
    STDIN_FILENO will tell tcgetattr that it should write the
    settings of stdin to oldt */
    tcgetattr( STDIN_FILENO, &oldt);
     /*now the settings will be copied*/
    newt = oldt;
    
     /* ICANON normally takes care that one line at a time will be
    processed that means it will return if it sees a "\n" or an EOF
    or an EOL */
    newt.c_lflag &= ~(ICANON);
    
     /* Those new settings will be set to STDIN TCSANOW tells
    tcsetattr to change attributes immediately. */
    tcsetattr( STDIN_FILENO, TCSANOW, &newt);
 
    char c = getchar();
    switch(c)
    {
        case 'w': 
        case 's': 
        case 'd': 
        case 'a': value = c;
    }

 /* restore the old settings */
tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
  
    cv.notify_one();
}
void take_input() //function to accept the value parallelly while game is proceeding.  
{
    thread th(read_value);
    mutex mtx;
    unique_lock<mutex> lck(mtx);
    while (cv.wait_for(lck, chrono::milliseconds(speed)) == cv_status::timeout)
    {
        control(value);
    }
    th.join();
    control(value);
}
void control(char value) //Converts user input to the direction snake must move and stores all the movements into the array
{   
    switch(value)
    {      
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
    short i;
    static short tr=0,insect=0,frog=101,time_=0;
    static bool flag=false,bonus=false;
    if(tr != score) flag=false;
    tr=score;
    if(!flag)
    {
        insect=randomize(score);
        if((insect+1)%side == 0) insect++;
        flag = true;
        if(score%4 == 3 and !bonus)
        {
            frog=randomize(insect/2);
            if((frog+1)%side == 0) frog++;
            bonus = true;
        }
    }
    if(level == 1)
    {
        if(head>=area)     head -= area;
        else if(head<0)    head += area;
    }
    else if(level == 2 && (head >= area || head < 0)) gameToggle(points,false);
    
    static short t=0,p=0,q=0;
    static int pulse = 0;
    if(find(trail.begin(), trail.end(), head) != trail.end()) gameToggle(points,false); 
    trail.push_back(head);
    ++pulse;
    p = trail[trail.size()-1];
    q = trail[trail.size()-2];
    
    if(p == q+1)
    {   
        map[head-1]= '=';
        map[head]= '>';
    }else 
    if(p == q-1)
    {
        map[head+1]= '=';
        map[head]= '<';
    }else 
    if(p/side == (q/side)+1)
    {
        map[head-side]= '|';
        map[head]= 'v';
    }else 
    if(p/side == (q/side)-1)
    {
        map[head+side]= '|';
        map[head]= '^';
    }
    
    map[insect]='+'; //The point which determines the score and increments the length of the snake 
    if(bonus)
    {
        if (time_ < pulse) 
        {
            map[frog]='@';
            cout<<int(1.5*side)-pulse+time_<<endl;
        }
        if (time_ == pulse-int(1.5*side)) 
        {
            bonus = false; 
            map[frog] = ' ';
            frog=area+1;
        }
    }
    else {
        time_ = pulse;
        cout<<endl;
    }
    if(head == insect)    
    {
        cout<<"\a";
        points += pace*level;
        score++;
    }
    if(head == frog)
    {
        cout<<"\a";
        points += pace*level*3;
        bonus = false;
    }
    if(pulse > 3-score) 
    {
        map[trail[0]] = ' ';      //For shortening the snake length dynamically
        if (head != insect) trail.erase(trail.begin());
    }   
    display();
}
void display()
{
    cout<<uline<<endl;
    for (j=0;j<side;j++)  //Designing the 2Dmodel : Borders not made yet
    {   
        for (i=0;i<side;i++)
        {
            if(level == 2)
            {
                if((map[(j*side)+i] == '>' && i == side-1) || (map[(j*side)+i] == '<' && i == side-1)){
                    gameToggle(points,false);
                }
            }
            else
            {
                if(map[(j*side)+i] == '<' && i == side-1)        head += side+1;
                else if(map[(j*side)+i] == '>' && i == side-1)   head -= side+1;
            }
            if(i == side-1 || i == 0)       cout<<wall[level-1];
            if(i == side-1 && j == side-1)  cout<<endl<<bline;
            
            switch(map[(j*side)+i])
            {
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
    mainMenu();
}
void gameToggle(short score, bool toggle) //To exit the game when the snake bites itself
{   
    do{
        if(!toggle)
        {
            system("clear");
            fileManage(to_string(pace) + to_string(level),'s');
            fileManage(to_string(score),'o');
            cout<<"Game Over!\nScore:"<<score;
            exit(0);
        }
        take_input();
    }while(1);
}
void mainMenu()   //Main Menu
{   
    char choice='z';
    cout<<"\nCreated by Allen\n";
    cout<<"Press:\n1 to Play\n2 for Help\n3 for Game Settings\n4 to exit\n";
    cin>>choice;
    if(choice == '1') gameToggle(0,true);
    if(choice == '2')   //Instructions
    {   
        system("clear");
        cout<<"CONTROLS\nPRESS\n w TO MOVE UPWARD\n s TO MOVE DOWNWARDS \n d TO MOVE RIGHT \n a TO MOVE LEFT";
        cin>>choice;
        system("clear");
        main();
    }
    if(choice == '3')
    {
        system("clear");
        cout<<"Control the Snake Speed. PRESS\n1 : Easy\n2 : Medium\n3 : Hard\n";
        cin>>pace;
        cout<<"Control the Game Difficulty level. PRESS\n1 : LEVEL 1\n2 : LEVEL 2\n";
        cin>>level;
        level = (level == 1) ? 1 : 2;
        speedSelector();
        gameToggle(0,true);
    }
    else exit(0);
}
void fileManage(string data, char option)
{
    if(option == 'i')
    {
        ifstream fin("snakes_data.txt"); 
        if(!fin) cout<<"Welcome to the game!"; 
        else
        {
            string s;
            cout<<"Welcome back to the game!\nThe highscore is ";
            while (fin.good()) {
                getline(fin,s);
            }
            pace =  s[0] - '0';
            level = s[1] - '0';
            if (highscore < stoi(s.substr(2))) highscore = stoi(s.substr(2)) ;
            cout<<highscore;
            speedSelector();
        }
        fin.close();
    }
    else if(option == 's') {
        ofstream fout("snakes_data.txt",ios::app);
        fout<<endl<<data;
        fout.close();
    }
    else if(option == 'o')
    {
        ofstream fout("snakes_data.txt", ios::app);
        if(stoi(data) > highscore) 
        {
            highscore = stoi(data);
            cout<<"HIGHSCORE! "<<highscore<<endl;
        }
        fout<<highscore;
        fout.close();
    }
}
void speedSelector()
{
    switch(pace)
    {
        case 1: speed = 550;break;
        case 3: speed = 250;break;
        default:speed = 400;pace = 2;break;
    }
}

//Start of the program
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif
#include <iostream>
#include <fstream>
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
short side = 10;
short area = side * side;
short speed = 400, level = 2, pace = 2, f = 0;
char value = 'd';
string h(area,' ');
short highscore = 0;
string uline(side+1,'_');
string bline(side+1,'"');

condition_variable cv;
void control(char);
void display();
void gameToggle(short,short);
void process(short);
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
        case 'd': f++;        break;
        case 's': f+=side;    break;
        case 'a': f--;        break;
        case 'w': f-=side;    break;
    }
    process(f);
}
void process(short n)   //Brain of the program. Entire game operation happens here. 
{
    system("clear");
    short i;
    static short tr=0,w=0,y=101,tim=0;
    static bool flag=false,bonus=false;
    if(tr != score) flag=false;
    tr=score;
    if(!flag)
    {
        w=randomize(score);
        if((w+1)%side == 0) w++;
        flag = true;
        if(score%4 == 3 and !bonus)
        {
            y=randomize(w/2);
            if((y+1)%side == 0) y++;
            bonus = true;
        }
    }
    if(level == 1)
    {
        if(n>=area)     n -= area;
        else if(n<0)    n += area;
    }
    else if(level == 2)
    {
        if(n>=area||n<0) gameToggle(points,420);
    }
    static short g=0,t=0,p=0,q=0,b[1000];

    b[++g] = n; //Storing the values to another array
    p = b[g];
    q = b[g-1];  //For identifying the previous position and the new position of the snake
    
    if(p == q+1)
    {   
        h[n-1]= '=';
        h[n]= '>';
    }else 
    if(p == q-1)
    {
        h[n+1]= '=';
        h[n]= '<';
    }else 
    if(p/side == (q/side)+1)
    {
        h[n-side]= '|';
        h[n]= 'v';
    }else 
    if(p/side ==(q/side)-1)
    {
        h[n+side]= '|';
        h[n]= '^';
    }
        
    h[w]='+'; //The point which determines the score and increments the length of the snake 
    if(bonus)
    {
        if (tim < g) 
        {
            h[y]='@';
            cout<<int(1.5*side)-g+tim<<endl;
        }
        if (tim == g-int(1.5*side)) 
        {
            bonus = false; 
            h[y] = ' ';
            y=101;
        }
    }
    else {
        tim = g;
        cout<<endl;
    }
    if(n == w)    
    {
        cout<<"\a";
        points += pace*level;
        score++;
    }
    if(n == y)
    {
        cout<<"\a\a";
        points += pace*level*3;
        bonus = false;
    }
    if(g > 4-score) h[b[g-4-score]]= ' ';      //For shortening the snake length dynamically
       
    for(i=g-4-score;i<g;i++)
    {
        if(b[i] == b[g])  gameToggle(points,420);  //When the snake bites itself
    }
    display();
}
void display()
{
    cout<<uline<<endl;
    short i,j;
    for (j=0;j<side;j++)  //Designing the 2Dmodel : Borders not made yet
    {   
        for (i=0;i<side;i++)
        {
            if(level == 2)
            {
                if((h[(j*side)+i] == '>' && i == side-1) || (h[(j*side)+i] == '<' && i == side-1)){
                    gameToggle(points,420);
                }
            }
            else
            {
                if(h[(j*side)+i] == '<' && i == side-1)        f += side+1;
                else if(h[(j*side)+i] == '>' && i == side-1)   f -= side+1;
            }
            if(i == side-1 || i == 0)
            {
                if(level == 1)      cout<<":";
                else if(level == 2) cout<<"|";
            }
            if(i == side-1 && j==side-1) cout<<endl<<bline;
            
            switch(h[(j*side)+i])
            {
                case '<': cout<<"◀"; break;
                case '>': cout<<"▶"; break;
                case '^': cout<<"▲"; break;
                case 'v': cout<<"▼"; break;
                case '|': cout<<"∥"; break;
                default : cout<<h[(j*side)+i];
            }
        }cout<<endl;
    }
}
int main()  
{ 
    fileManage("0",'i');
    mainMenu();
}
void gameToggle(short score,short k) //To exit the game when the snake bites itself
{   
    do{
        if(k == 420)
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
    if(choice == '1') gameToggle(0,0);
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
        gameToggle(0,0);
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

//Start of the program
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif
#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include<stdlib.h>
#include <termios.h> //termios, TCSANOW, ECHO, ICANON
#include <unistd.h> 
#include <time.h>
using namespace std;

short score = 0;
static short points = 0;
short side = 10;
short area = side * side;
short speed = 400, level = 2, value = 3, pace = 2, f = 0;
string h(area,' ');
string uline(side+1,'_');
string bline(side+1,'"');

condition_variable cv;
void control(short);
void gameOver(short,short);
void calc(short);
void mainMenu();

short randomize(short poll)
{   
    srand((unsigned) time(0));
    short p[area-1];
    for (short index = 0; index < 50; index++) 
    {
        p[index] = (rand() % area-1) + 1;
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
        case 'w': value = (int)c - 114;break;
        case 's': value = (int)c - 113;break;
        case 'd': value = (int)c - 97;break;
        case 'a': value = (int)c - 96;break;
        case '1': value = (int)c - 48;break;
        case '2': value = (int)c - 48;break;
        case '3': value = (int)c - 48;break;
        case '5': value = (int)c - 48;break;  
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
void control(short value) //Converts user input to the direction snake must move and stores all the movements into the array
{   
    switch(value)
    {      
        case 3: f++;    break;
        case 2: f+=side;  break;
        case 1: f--;    break;
        case 5: f-=side;  break;
    }
    calc(f);
}
void calc(short n)   //Brain of the program. Entire game operation happens here. 
{
    system("clear");
    short i;
    static short tr=0,w=0;
    static bool flag=false;
    if(tr != score) flag=false;
    tr=score;
    if(!flag)
    {
        w=randomize(score);
        if((w+1)%side == 0) w++;
        flag = true;
    }
    if(level == 1)
    {
        if(n>=area)    n -= area;
        else if(n<0)n += area;
    }
    if(level == 2)
    {
        if(n>=area||n<0) gameOver(points,420);
    }
    static short g=0,t=0,j=0,p=0,q=0,b[1000];

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
    if(n == w)    
    {
        cout<<"\a";
        points += pace*level;
        score++;
    }
    if(g > 4-score) h[b[g-4-score]]= ' ';      //For shortening the snake length dynamically
       
    for(i=g-4-score;i<g;i++)
    {
        if(b[i] == b[g])  gameOver(points,420);  //When the snake bites itself
    }
    cout<<uline<<endl;
    
    for (j=0;j<side;j++)  //Designing the 2Dmodel : Borders not made yet
    {   
        for (i=0;i<side;i++)
        {
            if(level == 2)
            {
                if((h[(j*side)+i] == '>' && i == 9) || (h[(j*side)+i] == '<' && i == 9)){
                    gameOver(points,420);
                }
            }
            else
            {
                if(h[(j*side)+i] == '<' && i == 9)        f += 11;
                else if(h[(j*side)+i] == '>' && i == 9)   f -= 11;
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
    mainMenu();
    gameOver(0,0);
}
void gameOver(short score,short k) //To exit the game when the snake bites itself
{   
    do{
        if(k == 420)
        {
            system("clear");
            cout<<"Game Over!\nScore:"<<score;
            while (cin.get() != '\n');
            system("clear");
            main();
        }
        take_input();
    }while(1);
}
void mainMenu()   //Main Menu
{   
    char choice='z';
    cout<<"Welcome to the game!\nCreated by Allen\n";
    cout<<"Press:\n1 to Play\n2 for Help\n3 for Game Settings\n4 to exit\n";
    cin>>choice;
    if(choice == '2')   //Instructions
    {   
        system("clear");
        cout<<"CONTROLS\nPRESS\n 5 or w TO MOVE UPWARD\n 2 or s TO MOVE DOWNWARDS \n 3 or d TO MOVE RIGHT \n 1 or a TO MOVE LEFT";
        cout<<"\n Press 3 to start the game";
        cout<<"\n Press any key to continue";
        cin>>choice;
    }
    if(choice == '3')
    {
        system("clear");
        cout<<"Control the Snake Speed. PRESS\n1 : Easy\n2 : Medium\n3 : Hard\n";
        cin>>pace;
        cout<<"Control the Game Difficulty level. PRESS\n1 : LEVEL 1\n2 : LEVEL 2\n";
        cin>>level;
        level = (level == 1) ? 1 : 2;
        if(pace == 1)   speed = 500;
        if(pace == 2)   speed = 400;
        else            speed = 300;
    }
    if(choice == '4') exit(0);
}

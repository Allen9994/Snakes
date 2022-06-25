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

int score = 0;
static int points = 0;
int speed = 400;
int level=2;
int a[100] = {0};
using namespace std;
string h = "                                                                                                    ";

condition_variable cv;
void kill(int);
void lame(int,int);
int value = 3;
int f = 0;
void calc(int);
void helpscreen();

int randomz(int poll)
{   
    srand((unsigned) time(0));
    int p[99];
    for (int index = 0; index < 50; index++) {
        p[index] = (rand() % 99) + 1;
    }
    return p[poll];
}

void read_value() //inputting value from user
{   
  int c=0;
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
 
    c = getchar();
    switch(c)
      {
        case 'w': value = (int)c - 114;break;
        case 's': value = (int)c - 113;break;
        case 'd': value = (int)c - 97;break;
        case 'a': value = (int)c - 96;break;
        case '1': value =  (int)c - 48;break;
        case '2': value =  (int)c - 48;break;
        case '3': value =  (int)c - 48;break;
        case '5': value =  (int)c - 48;break;  
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
        kill(value);
    }
    th.join();

    kill(value);
}
void kill(int value) //Converts user input to the direction snake must move and stores all the movements into the array
{   
    if(value==3)
    {   
        f++;
        calc(f);
    }else
    if(value==2)
    {
        f+=10;
        calc(f); 
    }
    else
    if(value==1)
    {
        f--;
        calc(f); 
    }else
    if(value==5)
    {
        f-=10;
        calc(f); 
    }
}
void calc(int n)   //Brain of the program. Entire game operation happens here. 
{
    system("clear");
    int i ;
    static int tr=0,w=0;
    static bool flag=false;
    if(tr!=score)
    {
        flag=false;
    }
    tr=score;
    if(flag==false)
    {
        w=randomz(score);
        if((w+1)%10 == 0)
        {
            w++;
        }
        flag = true;
    }
    if(level == 1)
    {
        if(n>99)
        {
            n = n - 100;
        }else if(n<0)
        {
            n = n + 100;
        }
    }
    if(level == 2)
    {
        if(n>99||n<0)
        {
            lame(points,420);
        }
    }
    static int g=0,t=0,s=0,b[1000];
    int j,z=0,p,q;
    
    g++;
    b[g]=n; //Storing the values to another array
    p=b[g];q=b[g-1];  //For identifying the previous position and the new position of the snake
    
        if(p==q+1)
        {   
            h[n-1]= '=';
            h[n]= '>';
        }else if(p==q-1)
        {
            h[n+1]= '=';
            h[n]= '<';
        }else if(p/10==(q/10)+1)
        {
            h[n-10]= '|';
            h[n]= 'v';
        }else if(p/10==(q/10)-1)
        {
            h[n+10]= '|';
            h[n]= '^';
        }
        
    h[w]='+'; //The point which determines the score and increments the length of the snake 
    if(n==w)    
    {
        switch(speed)
        {
            case 500:points = points + 1*level;break;
            case 400:points = points + 2*level;break;
            case 300:points = points + 3*level;break;
        }score++;
        cout<<"\a";
    }
    if(g>4-score)       //For shortening the snake length dynamically
    {   
       s=b[g-4-score];
       h[s]= ' ';
    }
    for(i=g-4-score;i<g;i++)
    {
        if(b[i]==b[g])  //When the snake bites itself
        {
            lame(points,420); 
        }
    }
    cout<<" _________"<<endl;
    for (j=0;j<10;j++)  //Designing the 2Dmodel : Borders not made yet
    {   
        for (i=0;i<10;i++)
        {
            if(level == 2)
            {
                if((h[(j*10)+i] == '>' && i == 9) || (h[(j*10)+i] == '<' && i == 9))
                {
                    system("clear");
                        lame(points,420);
                }
            }
            else{
                if(h[(j*10)+i] == '<' && i == 9)    
                    f = f + 11;
                else
                if(h[(j*10)+i] == '>' && i == 9)
                {
                    f = f - 11;
                }
            }
            if(i==9||i==0)
            {
                if(level == 1)
                {
                    cout<<":";
                }
                else if(level == 2){
                cout<<"|";
                }
            }
            if(i == 9 && j==9)
            {
                cout<<"\n ͞ ͞ ͞ ͞ ͞ ͞ ͞ ͞ ͞";
            }
            
            switch(h[(j*10)+i])
            {
                case '<': cout<<"◀"; break;
                case '>': cout<<"▶"; break;
                case '^': cout<<"▲"; break;
                case 'v': cout<<"▼"; break;
                case '|': cout<<"∥"; break;
                default : cout<<h[(j*10)+i];
            }
            
        }cout<<endl;
    }
}

int main()  
{
    
    helpscreen();
    lame(0,0);
}
void lame(int sc,int k) //To exit the game when the snake bites itself
{   
    do{
        if(k==420)
        {
            cout<<"LOST";
            cout<<"\nScore:"<<sc;
            exit(0);
        }
        take_input();
        
    }while(1);
}
void helpscreen()   //Main Menu
{   
    int choice=48;
    cout<<"Welcome to the game!\nCreated by Allen\n";
    cout<<"Press:\n1 to Play\n2 for Help\n3 for Game Settings";
    cin>>choice;
    if(choice==2)   //Instructions
    {   
        system("clear");
        cout<<"CONTROLS\nPRESS\n 5 or w TO MOVE UPWARD\n 2 or s TO MOVE DOWNWARDS \n 3 or d TO MOVE RIGHT \n 1 or a TO MOVE LEFT";
        cout<<"\n Press 3 to start the game";
        
        cout<<"\n Press any key to continue";
        cin>>choice;
    }
    if (choice == 3)
    {
        system("clear");
        cout<<"Control the Snake Speed. PRESS\n1 : Easy\n2 : Medium\n3 : Hard";
        cin>>speed;
        cout<<"Control the Game Difficulty level. PRESS\n1 : LEVEL 1\n2 : LEVEL 2";
        cin>>level;
        if(level == 1)
        {
            level = 1;
        }
        else
        {
            level = 2;
        }
        if(speed == 1)
        {
            speed = 500;
        }
        else if (speed == 2)
        {
            speed = 400;
        }else{speed = 300;}
    }
}

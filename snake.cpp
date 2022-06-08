#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif
#include <iostream>
#include <cstdlib>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include<stdlib.h>
#include<stdio.h>
 #include <termios.h> //termios, TCSANOW, ECHO, ICANON
 #include <unistd.h> 
#include <time.h>

static int score=0;
int a[100] = {0};
using namespace std;


condition_variable cv;
void kill(int);
void lame(int,int);
static int value = 3;
void calc(int);
void helpscreen();

int randomz(int poll)
{   
    srand((unsigned) time(0));
  int p[100];
  for (int index = 0; index < 50; index++) {
    p[index] = (rand() % 100) + 1;
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
    value =  (int)c - 48;;

 /* restore the old settings */
 tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
  
    cv.notify_one();
}
void takin() //function to accept the value parallelly while game is proceeding.  
{
    thread th(read_value);

    mutex mtx;
    unique_lock<mutex> lck(mtx);
    while (cv.wait_for(lck, chrono::milliseconds(400)) == cv_status::timeout)
    {
        kill(value);
        
    }

    th.join();

    kill(value);
}
void kill(int value) //Converts user input to the direction snake must move and stores all the movements into the array
{   
    static int i=0;
    if(value==3)
    {   
        i++;
        a[i]=i;
        
        calc(i);
    }else
    if(value==2)
    {
        i+=10;
        a[i]=i;
        
        calc(i); 
    }
    else
    if(value==1)
    {
        i--;
        a[i]=i;
        
        calc(i); 
    }else
    if(value==5)
    {
        i-=10;
        a[i]=i;
        
        calc(i); 
    }
}
void calc(int n)   //Brain of the program. Entire game operation happens here. 
{
    system("clear");
    int i ;
    
    static int tr=0,flag=0,w=0;
    if(tr!=score)
    {
        flag=0;
    }
    tr=score;
    if(flag==0)
    {
        w=randomz(score);
        if((w+1)%10 == 0)
        {
            w++;
        }
        
        
        flag++;
    }
    
    if(n>99||n<0)
    {
        lame(score,420);
    }
    
    static int g=0,t=0,s=0,b[1000];
    int j,z=0,p,q;
    
    g++;
    b[g]=a[n]; //Storing the values to another array
    static int x[10][10] = {0};
    p=b[g];q=b[g-1];  //For identifying the previous position and the new position of the snake
    
    
            if(p==q+1)
            {   
                a[a[n]-1]=1;
                a[a[n]]=6;
            }else if(p==q-1)
            {
                a[a[n]+1]=2;
                a[a[n]]=7;
            }else if(p/10==(q/10)+1)
            {
                a[a[n]-10]=3;
                a[a[n]]=8;
            }else if(p/10==(q/10)-1)
            {
                a[a[n]+10]=4;
                a[a[n]]=9;
            }
        
    
    a[w]=5; //The point which determines the score and increments the length of the snake 
    if(n==w)    
    {
        score++;
        cout<<"\a";
    }
    if(g>4-score)       //For shortening the snake length dynamically
    {   
       s=b[g-4-score];
       a[s]=0;
    }
    for(i=g-4-score;i<g;i++)
    {
        if(b[i]==b[g])  //When the snake bites itself
        {
            lame(score,420);    
        }
    }
    for (i=0;i<10;i++)  //converting 1D array of snake's movements into 2D array 
    {   
        for (j=0;j<10;j++)
        {
            z=i*10+j;
            x[i][j]=a[z];
            if(x[i][0]==2||x[i][9]==6)
            {
                lame(score,420);
            }
        }
    }cout<<"__________"<<endl;
    for (i=0;i<10;i++)  //Designing the 2Dmodel : Borders not made yet
    {   
        for (j=0;j<10;j++)
        {
            if(j==9||j==0)
            {
                cout<<"|";
            }
           if(x[i][j]==0)
            {
                cout<<" ";
            } else 
            
                if(x[i][j]==1||x[i][j]==2)
                {
                    cout<<"=";
                }else
                if(x[i][j]==6)
                {
                    cout<<"▶";
                }else
                if(x[i][j]==7)
                {
                    cout<<"◀";
                }else
                if(x[i][j]==8)
                {
                    cout<<"▼";
                }else
                if(x[i][j]==9)
                {
                    cout<<"▲";
                }else
                if(x[i][j]==5)
                {
                    cout<<"+";
                }else 
                if(x[i][j]==3||x[i][j]==4)
                    cout<<"∥";
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
    int a=0;
    do{
        if(k==420)
    {
        cout<<"LOST";
        cout<<"\nScore:"<<sc;
        exit(0);
    }
        takin();
        
    }while(a<200);
}
void helpscreen()   //Main Menu
{   
    int choice=48;
    cout<<"Welcome to the game!\nCreated by Allen\n";
    cout<<"Press:\n1 to Play\n2 for Help";
    cin>>choice;
    if(choice==2)   //Instructions
    {   system("clear");
        cout<<"CONTROLS\nPRESS\n 5 TO MOVE UPWARD\n 2 TO MOVE DOWNWARDS \n 3 TO MOVE RIGHT \n 1 TO MOVE LEFT";
      cout<<"\n Press 3 to start the game";
        
        cout<<"\n Press any key to continue";
        cin>>choice;
    }
    
}

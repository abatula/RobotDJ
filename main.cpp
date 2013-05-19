/*
 * main.cpp
 * Written by: Mark Koh
 * Modified by: Alyssa Batula
 * 5/18/2013
 *
 * Description: Based off of the action_script demo written by robotis corp.
 * and the beat tracking code by Mark Koh, this program will execute a 
 * choreagraphed DJ motion sequence. 
 * 
 * Written for Philadelphia Music Hack Day 2013
 *
 */

#include <unistd.h>
#include <string.h>
#include <string>
#include <libgen.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>

//#include "Camera.h"
#include "Point.h"
#include "mjpg_streamer.h"
#include "minIni.h"
//#include "LinuxCamera.h"
//#include "ColorFinder.h"

#include "Action.h"
#include "Head.h"
#include "Walking.h"
#include "MX28.h"
#include "MotionManager.h"
#include "LinuxMotionTimer.h"
#include "LinuxCM730.h"
#include "LinuxActionScript.h"

#ifdef MX28_1024
#define MOTION_FILE_PATH    "../../../Data/motion_1024.bin"
#else
#define MOTION_FILE_PATH    "../../../Data/motion_4096.bin"
#endif

#define PORT 9930
#define BUFLEN 70
#define SAMPLE_RATE (44100)


void change_current_dir()
{
    char exepath[1024] = {0};
    if(readlink("/proc/self/exe", exepath, sizeof(exepath)) != -1)
        chdir(dirname(exepath));
}

void sighandler(int sig)
{
    struct termios term;
        Action::GetInstance()->Start(1);    /* Init(stand up) pose */
    tcgetattr( STDIN_FILENO, &term );
    term.c_lflag |= ICANON | ECHO;
    tcsetattr( STDIN_FILENO, TCSANOW, &term );

    exit(0);
}


int main(void)
{   
    signal(SIGABRT, &sighandler);
    signal(SIGTERM, &sighandler);
    signal(SIGQUIT, &sighandler);
    signal(SIGINT, &sighandler);

    /* Decalre server and socket variables */
    struct sockaddr_in sad; //Structure to hold server IP Address
    struct sockaddr_in cad; //Structure to hold client IP Address
    int serverSocket; //Socket descriptor
    int slen=sizeof(cad);
    char buf[BUFLEN];
    //int buf;
    //struct hostent *ptrh; //Pointer to host table entry

    
    printf( "\n=====   DJ Script program   =====\n");
    printf(   "=====     Alyssa Batula     =====\n\n");

    change_current_dir();

    Action::GetInstance()->LoadFile(MOTION_FILE_PATH);

    //////////////////// Framework Initialize ////////////////////////////
    LinuxCM730 linux_cm730("/dev/ttyUSB0");
    CM730 cm730(&linux_cm730);
    if(MotionManager::GetInstance()->Initialize(&cm730) == false)
    {
        printf("Fail to initialize Motion Manager!\n");
            return 0;
    }

    MotionManager::GetInstance()->AddModule((MotionModule*)Action::GetInstance());
    LinuxMotionTimer *motion_timer = new LinuxMotionTimer(MotionManager::GetInstance());
    motion_timer->Start();
    /////////////////////////////////////////////////////////////////////
      
    /////////////////// Run Motion Script //////////////////////////
    
    //Initialize variables
    int numMotions = 5;
    int motionSequence[5] = {60, 61, 62, 61, 62};  // List of motions to perform
    int motionTimings[5] = {2, 2, 2, 2, 2}; // Time in seconds to pause before the next motion
    
    int currentMotion = 0;

    //int base = 0; //Start from base
    int base = 1; //Start from pose

    srand (time(NULL));

    // ---- Go to init pose -----------
    MotionManager::GetInstance()->SetEnable(true); 

    Action::GetInstance()->Start(1);    /* Init(stand up) pose */
    while(Action::GetInstance()->IsRunning()) usleep(8*1000);
    // -----------------------


    printf("Press the ENTER key to begin!\n");
    char mode = getchar();

    // Run through the list of motions
    for(currentMotion=0; currentMotion < numMotions; currentMotion++)
    {
        Action::GetInstance()->Start(motionSequence[currentMotion]); // Call the motion
        while(Action::GetInstance()->IsRunning()) usleep(8*1000); // 1 second = 1,000,000 us
        usleep(motionTimings[currentMotion]*1000000); // Wait for specified delay
    }
   

    return 0;
}


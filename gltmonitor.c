/* monitor program for GLT
 adopted from SMA.

Nimesh Patel
14 June 2012

version 2.2



Only the following pages will be ported for now (for Socorro/Norfolk tests).
1) Main page: "a": combination of array and antenna page
2) Pointing models: "P"
3) Smainit info: "s"
4) Error messages: "e"
5) Commands summary: "l"
6) 2op messages: "m"
7) Satellite Two Line elements display: "s"


 Note added on 17 June 2012:
Only page (1) of above will be implemented for now!


 NAP 1 May 2021. Many changes to document here, but the code is now on github
where more comments will be in commits. 

NAP 22 Mar 2023. Added P page for pointing model display
NAP 19 Dec 2024. Added s page for satellite two line display

*/

#define PRINT_DSM_ERRORS 0 /* set this to 1 to debug DSM problems
			    * with the use of call_dsm_read() */
#define N_LINES 20
#define MIN_LINES_FOR_BOTTOM_2OP 25
#define MIN_LINES_FOR_BOTTOM_OPMSG 26
#define DEBUG 0
#include <stdio.h>
#include <sys/utsname.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <termio.h>
#include <time.h>
#include <ncurses.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <hiredis/hiredis.h>
/*
#include <pthread.h>
*/
#include <termios.h>
/*
#include "smapopt.h"
#include "commonLib.h"
#include "rm.h"
*/
#include "dsm.h"
#include "monitor.h"
#include "antMonitor.h"

enum {
/* to add a page, insert a new name at the top and decrement the initializor */
SAT2LE_DISPLAY,
POINTING_MODEL_DISPLAY,
RECEIVERS_DISPLAY,
HELP_DISPLAY, /* -3 */
METROLOGY_DISPLAY,
MASER_DISPLAY,
HOME_DISPLAY   /* 0 */
};

#define TRUE 1
#define FALSE 0
#define TIMEOUT 3600

#define COLOR_NORMAL   1
#define COLOR_LABEL    2
#define COLOR_EMPHASIS 3
#define COLOR_ENABLED  4
#define COLOR_DISABLED 5
#define COLOR_VARIANT  6

/* redis server (gltobscon) */
#define REDIS_SERVER "192.168.1.141"
#define REDIS_PORT 6379
#define MAX_MESSAGES 100


#include "commandMonitor.h"

int colorFlag = TRUE;
int bottom2op = FALSE;
int bottomOpMsg = FALSE;
int projectLockout;
int defaultTiltFlag[20];
void readIFServerInfo(void);
void handlerForSIGINT(int signum);
void goto80width(void);
void goto80WidthAndExit(int value);
void goto132width(void);
void allow132width(void);

double sunDistance(double az1,double el1,double az2,double el2);

int user, lastUser;
double radian=0.01745329;
struct termio tio, tin;
char *username = NULL;  /* username for 2op messages */
int beepFlag=1;
int antsAvailable[MAX_NUMBER_ANTENNAS+1] = {0,0,0,0,0,0,0,0,0,0,0};
int deadAntennas[MAX_NUMBER_ANTENNAS+1] = {0,0,0,0,0,0,0,0,0,1,1};

/*
int monoFlag = FALSE;
int setSimpleChars = FALSE;
int extCharset = TRUE;
int lowColorFlag = FALSE;
int whiteBG = FALSE;

typedef struct {
  int isColor;
  int isWhiteBG;
  int isReducedColor;
} COLOR_SCHEME;

static COLOR_SCHEME schemes[5] = { { TRUE, FALSE, FALSE }, { TRUE, FALSE, TRUE }, { TRUE, TRUE, FALSE }, { TRUE, TRUE, TRUE}, { FALSE, FALSE, FALSE } };
static int scheme = 0;
*/

char redisData[1024];
redisContext *redisC;
redisReply *redisResp;
struct timeval redisTimeout = {1,500000}; /*1.5 seconds for redis timeout */


int smainitMode = 0; /* Set to 1 when displaying smainit pages */
int antMode = 0;
int commandMode = 0;
int searchMode = 0;
int messageMode = 0;
int errorMode = 0; 
/* remember to add any new entries to the functions SetAllPageModesToZero() 
 * and SetMostPageModesToZero() */

int smainitPage = 0; /* Selects computer within smainit pages */
int antPage = 0;
int statesPage = FALSE;
int pageOffset = 0;
char searchString[100];
int opBeepCount = 3;

extern void help(int count);
extern void timeStamp(void);
/*
extern void messagePage(int count, int kind);
extern int pmodelspage(int icount,int *antlist);
extern void smainitMonitor(int count, int page);
*/
extern void metrologyPage(int count);
extern void receiversPage(int count);
extern void maserPage(int count);
extern void pointingModelPage(int count);
extern void sat2LEpage(int count);

/* on the wide 'a' page, this tells which window to start with in the
 * upper right corner */
int upperRightWindow = UR_MESSAGES;

/* Function to send a 2op message */
void send2opMessage(void) {
    char message[500] = {0};
    char formatted_message[256];
    time_t now;
    struct tm *tm_info;
    char timestamp[20];
    redisReply *reply;

    /* Temporarily restore canonical mode for message input */
    ioctl(0, TCSETA, &tio);

    /* Clear screen and prompt for message */
    clear();
    move(2, 2);
    printw("Enter 2op message (press Enter when done): ");
    refresh();
    echo();

    /* Read the message from user */
    getnstr(message, sizeof(message) - 1);

    noecho();

    /* Check if message is empty */
    if (strlen(message) == 0) {
        /* Restore non-canonical mode and return */
        ioctl(0, TCSETA, &tin);
        clear();
        return;
    }

    /* Get current timestamp */
    now = time(NULL);
    tm_info = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);

    /* Format the complete message with timestamp and username */
    snprintf(formatted_message, sizeof(formatted_message), "%s (%s) %s",
             timestamp, username, message);

    /* Push the message to Redis list */
    reply = redisCommand(redisC, "LPUSH 2opmsg %s", formatted_message);
    if (reply != NULL) {
        freeReplyObject(reply);

        /* Trim the list to keep only the last MAX_MESSAGES entries */
        reply = redisCommand(redisC, "LTRIM 2opmsg 0 %d", MAX_MESSAGES - 1);
        if (reply != NULL) {
            freeReplyObject(reply);
        }
    }

    /* Restore non-canonical mode */
    ioctl(0, TCSETA, &tin);

    /* Clear screen for next display update */
    clear();
}


main(int argc, char *argv[])
{
  int i;
  int ant=HOME_DISPLAY; /* start up on the 'a' page */
  int cycle = FALSE;
  int delay = 1;
  int spinCount = 0;
  char *outputFile = NULL;
  struct utsname unamebuf;
  int rms;
  struct sigaction action, old_action;
  int sigactionInt;
  int icount=0;
  int firstUpdate = TRUE;
  int  rc;
  int startDay;
  int message = 0;
  int ignoreFlag;
  struct stat messageStat, oldMessageStat;
  int opt;

  /* Parse command line options */
  static struct option long_options[] = {
      {"user", required_argument, 0, 'u'},
      {0, 0, 0, 0}
  };

  while ((opt = getopt_long(argc, argv, "u:", long_options, NULL)) != -1) {
      switch (opt) {
          case 'u':
              username = optarg;
              break;
          default:
              fprintf(stderr, "Usage: %s [-u|--user username]\n", argv[0]);
              exit(EXIT_FAILURE);
      }
  }

  /* Set default username if not provided */
  if (username == NULL) {
      username = "anon";
  }

  /* The following variables are for the default Tilt flag read from
     the pointing model files */
   FILE *fpMountModel;
   int itilt;
   char mountModelFile[100],line[BUFSIZ];
   char pointingParameter[20],opticalValue[20],radioValue[20];

  /* signal handler for control C */
  action.sa_flags=0;
  sigemptyset(&action.sa_mask);
  action.sa_handler = handlerForSIGINT;
  sigactionInt = sigaction(SIGINT,&action, &old_action);
  system("resize -s 58 90");
  
  uname(&unamebuf);
  i = 0;

   /* initialize connection to redis */
   redisC = redisConnectWithTimeout(REDIS_SERVER,REDIS_PORT,redisTimeout);
     if (redisC == NULL || redisC->err) {
      if (redisC) {
          printf("Connection error: %s\n", redisC->errstr);
          redisFree(redisC);
        } else {
          printf("Connection error: can't allocate redis context\n");
      }
     }


  rms = dsm_open();
  if(rms != DSM_SUCCESS) {
    dsm_error_message(rms, "dsm_open");
    exit(-1);
  }
#if DEBUG
  fprintf(stderr,"Finished opening DSM\n");
#endif

  /*
   * This is to get the user input as a single unbuffered char and
   * zero-wait
   */
  
  ioctl(0, TCGETA, &tio);
  
  tin = tio;
  tin.c_lflag &= ~ECHO;
  tin.c_lflag &= ~ICANON;
  
  tin.c_cc[VMIN] = 0;
  tin.c_cc[VTIME] = 0;
  
  ioctl(0, TCSETA, &tin);

  stat(MESSAGES_LOG, &oldMessageStat);

#if DEBUG
  fprintf(stderr,"1. Finished stat() on messages file\n");
#endif
  /* starting infinite loop */
  /* begin while loop every 1 second */

	initialize();
	ant=HOME_DISPLAY;

  while (1) {
    time_t lastKeystrokeTime, curTime;

    ioctl(0, TCSETA,&tin);
    
    icount++;

    lastUser = user;
      user = getchar();
      

    switch (user) {
    case 'q':
      move(LINES-1,0);
      printw("Bye.\n");
      refresh();
      ioctl(0, TCSETA, &tio);
      goto80width();
      printf("\n");

      rms = dsm_close();
      if(rms != DSM_SUCCESS) {
      dsm_error_message(rms, "dsm_close");
      exit(-1);
      }

      
      exit(0);
      break;

     case 'm':
     ant = METROLOGY_DISPLAY;
     icount = 1;
     break;

     case 'r':
     ant = RECEIVERS_DISPLAY;
     icount = 1;
     break;

     case 'M':
     ant = MASER_DISPLAY;
     icount = 1;
     break;

     case 'P':
     ant = POINTING_MODEL_DISPLAY;
     icount = 1;
     break;

     case 'a':
     ant = HOME_DISPLAY;
     icount = 0;
     break;

     case 'h':
     ant = HELP_DISPLAY;
     icount = 1;
     break;

     case '?':
     ant = HELP_DISPLAY;
     icount = 1;
     break;

     case 's':
     ant = SAT2LE_DISPLAY;
     icount = 1;
     break;

     case '/':
     send2opMessage();
     icount = 1;
     break;

    }			/* end of switch */

    if (ant == HOME_DISPLAY) {
          antDisplay(ant, icount);
          } else if (ant == HELP_DISPLAY) {
          help(icount);
          } else if (ant == METROLOGY_DISPLAY) {
          metrologyPage(icount);
          } else if (ant == RECEIVERS_DISPLAY) {
          receiversPage(icount);
          } else if (ant == MASER_DISPLAY) {
          maserPage(icount);
          } else if (ant == POINTING_MODEL_DISPLAY) {
          pointingModelPage(icount);
          } else if (ant == SAT2LE_DISPLAY) {
          sat2LEpage(icount);
          }
	sleep(delay);
  }				/* this is the big while loop */
  ioctl(0, TCSETA, &tio);
}				/* end of main Loop */


void handlerForSIGINT(int signum)
{
  user='q'; /* 'q' for quit command */
  fprintf(stderr,"Got the control C signal. Quitting.\n");
  printf("Bye.\n");
  ioctl(0, TCSETA, &tio);
  goto80WidthAndExit(0);
}


int call_dsm_read(char *machine, char *variable, void *ptr, time_t *tstamp) {
  char buf[256];
  int rms;
  rms = dsm_read(machine,variable,ptr,tstamp);
  if (rms != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS) {
      sprintf(buf,"dsm_read - %s",variable);
      dsm_error_message(rms, buf);
    }
  }
  return(rms);
}

int call_dsm_structure_get_element(dsm_structure *ds, char *name, void *ptr) {
  char buf[256];
  int rms;
  rms = dsm_structure_get_element(ds, name, ptr);
  if (rms != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS) {
      sprintf(buf,"dms_structure_get_element - %s",name);
      dsm_error_message(rms, buf);
    }
  }
  return(rms);
}


void allow132width(void) {
  char name[20];
  sprintf(name,"%c?40h",0x9b);
  printf(name);
}

void goto132width(void) {
  char name[20];
  sprintf(name,"%c?3h",0x9b);
  printf(name);
}

void goto80width(void) {
  char name[20];
  sprintf(name,"%c?3l",0x9b);
  printf(name);
}

void initialize() {
  int i;
  int numberAntennas;

  initscr();
  if(colorFlag) {
	start_color();
/*
   init_color(COLOR_WHITE, CSAT, CSAT, CSAT);

      init_color(COLOR_RED, CSAT, 0, 0);
      init_color(COLOR_GREEN, 0, CSAT, 0);
      init_color(COLOR_BLUE, 0, 0, CSAT);

      init_color(COLOR_YELLOW, CSAT, CSAT, CSAT/2);
      init_color(COLOR_CYAN, CSAT/4, (3*CSAT)/4, CSAT);
      init_color(COLOR_MAGENTA, CSAT, CSAT/2, CSAT);

      init_pair(COLOR_NORMAL, COLOR_WHITE, COLOR_BLACK);
 init_pair(COLOR_ENABLED, COLOR_GREEN, bgColor);
    init_pair(COLOR_DISABLED, COLOR_RED, bgColor);

    init_pair(COLOR_EMPHASIS, COLOR_YELLOW, bgColor);
    init_pair(COLOR_LABEL, COLOR_CYAN, bgColor);
    init_pair(COLOR_VARIANT, whiteBG ? COLOR_BLUE : COLOR_MAGENTA, bgColor);


    bkgd(A_NORMAL | COLOR_PAIR(COLOR_NORMAL));
*/
    }

  nonl();
  clear();
   /* LINES and COLS are now defined */
  if (COLS > 132)
    COLS = 132;
  if ((COLS >= MINIMUM_SCREEN_WIDTH_FOR_ESMA) && FALSE){
    numberAntennas = 10;
  } else {
    numberAntennas = 8;
  }
  if (LINES >= MIN_LINES_FOR_BOTTOM_2OP) {
    bottom2op = TRUE;
  }
  if (LINES >= MIN_LINES_FOR_BOTTOM_OPMSG) {
    bottomOpMsg = TRUE;
  }
  if ((COLS >= MINIMUM_SCREEN_WIDTH_FOR_ESMA) && FALSE) {
    numberAntennas = 10;
  } else {
    numberAntennas = 8;
  }
  /*
  box(stdscr, '|','-');
  */

/* draw outline */
/*
  for (i = 1; i < COLS-1; i++) {move(0,i);printw("-");}
  for (i = 1; i < COLS-1; i++) {move(80,i);printw("-");}
  for (i = 1; i < 23; i++) {move(i,0);printw("|");}
  for (i = 1; i < 23; i++) {move(i,COLS-1);printw("|");}
*/
#if 0
  move(0,0);
  printw("COLS=%d",COLS);
#endif
  move(2,2);
}

void goto80WidthAndExit(int value) {
  goto80width();
  exit(value);
}

int present(char *a, char *b) {
  if (strstr(a,b) == NULL) {
    return(0);
  } else {
    return(1);
  }
}

int computeMonthFrom3CharString(char *month) {
  int mon;
  if (!strcmp("Jan", month))
    mon = 0;
  else if (!strcmp("Feb", month))
    mon = 1;
  else if (!strcmp("Feb", month))
    mon = 1;
  else if (!strcmp("Mar", month))
    mon = 2;
  else if (!strcmp("Apr", month))
    mon = 3;
  else if (!strcmp("May", month))
    mon = 4;
  else if (!strcmp("Jun", month))
    mon = 5;
  else if (!strcmp("Jul", month))
    mon = 6;
  else if (!strcmp("Aug", month))
    mon = 7;
  else if (!strcmp("Sep", month))
    mon = 8;
  else if (!strcmp("Oct", month))
    mon = 9;
  else if (!strcmp("Nov", month))
    mon = 10;
  else
    mon = 11;
  return(mon);
}

int oldDate(char *string, int rightnow) {
  int day,year,monthNumber;
  char month[10];
  float yearFraction, presentYearFraction;
  struct tm *now;

  sscanf(string,"%2d%3s%2d",&day,month,&year);
  year += 2000;
#if 0
  printw("%2d%3s%2d= ",day,month,year);
#endif
  monthNumber = computeMonthFrom3CharString(month);
  yearFraction = computeYearFraction(day,monthNumber,year);
#if 0
  printw("%.3f =",yearFraction);
#endif
  /* Now compare it to the present time */
  now = gmtime(&rightnow);
  presentYearFraction = computeYearFraction(now->tm_mday, now->tm_mon, 1900+now->tm_year);  
  if (fabs(presentYearFraction - yearFraction) > 0.1) {
    return(1);  
  } else {
    return(0);  
  }
}

float computeYearFraction(int day, int month, int year) {
  /* day should be 1..31, month should be 0..11, year should be >2000 */
  /* returns a value such as 2005.0534 (for some time in January 2005) */
  float doy, yearFraction; 
  int m;

  doy = day;
  for (m=0; m<month; m++) {
    doy += 30;
    if (m==2) {
      doy--;
      if ((year % 4) != 0) {
	doy--;
      }
    }
    if (m==1 || m==3 || m==5 || m==7 || m==8 || m==10) {
      doy++;
    }
  }  
  if ((year % 4) == 0) {
    yearFraction = year + doy/366;
  } else {
    yearFraction = year + doy/365;
  }
  return(yearFraction);
}

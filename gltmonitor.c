/* monitor program for GLT
 adopted from SMA.

Nimesh Patel
14 June 2012

version 1.0

Only the following pages will be ported for now (for Socorro/Norfolk tests).
1) Main page: "a": combination of array and antenna page
2) Pointing models: "P"
3) Smainit info: "s"
4) Error messages: "e"
5) Commands summary: "l"
6) 2op messages: "m"

*/

/* Note added on 17 June 2012:
Only page (1) of above will be implemented for now!
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
#ifdef LINUX
  #include <sys/stat.h>
  #include <unistd.h>
#endif
#include <sys/file.h>
#include <termio.h>
#include <time.h>
#include <curses.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
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
#include "stateCounts.h"

enum {
/* to add a page, insert a new name at the top and decrement the initializor */
MESSAGE_DISPLAY,PMODEL_DISPLAY,          /* -5 */
SMAINIT_DISPLAY,HELP_DISPLAY, /* -3 */
SMASH_DISPLAY,STDERR_DISPLAY,ARRAY_DISPLAY,   /* 0 */
ANTENNAPAGE_DISPLAY /* 1  (going up 10) */
};

#define TRUE 1
#define FALSE 0
#define TIMEOUT 3600

#include "commandMonitor.h"

int colorFlag = FALSE;
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
/*
void screen(char *source, double *lst_disp,
	    double *utc_disp, double *tjd_disp,
	    double *ra_disp, double *dec_disp,
	    double *ra_cat_disp, double *dec_cat_disp,
	    double *az_disp, double *el_disp, int *icount,
	    double *azoff, double *eloff, 
	    double *az_actual_corrected, double *el_actual_disp, double *tiltx,
	    double *tilty, float *pressure,
	    float *temperature, float *humidity,
	    double *az_error, double *el_error,
	    double *scan_unit, char *messg, 
	    double *focus_counts, double *subx_counts, double *suby_counts,
	    double *subtilt_counts, double *total_power_disp, 
	    double *syncdet_disp,
	    int *integration, float *windspeed,
	    float *winddirection, float *refraction, float *pmdaz, 
	    float *pmdel,
	    double *smoothed_tracking_error, double *tsys, int *ant, 
	    float *plsAzRdg,
	    int *radio_flag, short *padid,double *planetdistance,
	    double *Az_cmd, 
	    double *El_cmd);
*/

double sunDistance(double az1,double el1,double az2,double el2);

int user, lastUser;
double radian=0.01745329;
struct termio tio, tin;
int beepFlag=1;
int antsAvailable[MAX_NUMBER_ANTENNAS+1] = {0,0,0,0,0,0,0,0,0,0,0};
int deadAntennas[MAX_NUMBER_ANTENNAS+1] = {0,0,0,0,0,0,0,0,0,1,1};

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

/* on the wide 'a' page, this tells which window to start with in the
 * upper right corner */
int upperRightWindow = UR_MESSAGES;

void usage(char *a);
void usage(char *a) {
  printf("Usage: %s -H -h --help\n",a);
  /*
    printf("Usage: %s -i -z -Z -H --help\n");
    printf("      -i  ignore messages from 2op\n");
*/
  printf(" -a or --antenna display values for antenna(s) even if 'dead'\n");
  printf("           -H  show the hangar display page only\n");
  printf(" -h or --help  show this message\n");
  exit(0);
}

main(int argc, char *argv[])
{
  int i;
  int ant=ARRAY_DISPLAY; /* start up on the 'a' page */
  int cycle = FALSE;
  int delay = 1;
#ifdef LINUX
  int spinCount = 0;
#endif
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
  
  uname(&unamebuf);
#ifndef LINUX
  if(strcmp(unamebuf.nodename,"hal9000")) {
    printf("This program will run only on hal9000. Bye.\n");
    exit(-1);
  }
#endif
#if DEBUG
  fprintf(stderr,"Finished opening RM\n");
#endif
  i = 0;

#if 0
  while (antlist[i] != RM_ANT_LIST_END) {
    antsAvailable[antlist[i]] = 1;
    deadAntennas[antlist[i]] = 0;
    i++;
  }
#endif 

 
#if 0
  defaultTiltFlag[0]=0;
   for(itilt=1;itilt<=8;itilt++) {
   defaultTiltFlag[itilt]=0;
   sprintf(mountModelFile,"/otherInstances/acc/%d/configFiles/pointingModel",itilt);
   fpMountModel=fopen(mountModelFile,"r");
      if(fpMountModel!=NULL) {
         while(fgets(line,sizeof(line),fpMountModel) != NULL) {
            line[strlen(line)-1]='\0';
                if(line[0]!='#') {
          sscanf(line,"%s %s %s", pointingParameter, opticalValue, radioValue);
                     if(!strcmp(pointingParameter,"TiltFlag")) {
                     defaultTiltFlag[itilt]=(int)atof(radioValue);
                     }
                }
         }
      }
   fclose(fpMountModel);
   }
#endif

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

  while (1) {
    time_t lastKeystrokeTime, curTime;

#ifndef LINUX
    yield();
#endif
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

    }			/* end of switch */

	ant=1;
          antDisplay(ant, icount);
#ifndef LINUX
	sleep(delay);
#else
	usleep(delay*500000);
	spinCount++;
	if ((spinCount % 2) && ((icount % 30) > 1))
	  icount--;
#endif
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
#ifdef LINUX
  if(colorFlag) {
	start_color();
  }
  nonl();
  clear();
#endif
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

  for (i = 1; i < COLS-1; i++) {move(0,i);printw("-");}
  for (i = 1; i < COLS-1; i++) {move(23,i);printw("-");}
  for (i = 1; i < 23; i++) {move(i,0);printw("|");}
  for (i = 1; i < 23; i++) {move(i,COLS-1);printw("|");}
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

#define LYNX_FINGER  0
#define LINUX_FINGER 1

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


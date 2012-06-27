/* gltAntMonitor.c
for GLT.
Only uses DSM, no RM.
Removed all Tilt and Chopper related variables; showing only a small subset
of usual variables.
NAP 17 June 2012.
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <curses.h>
#include <string.h>
#include <ctype.h>
#include "dsm.h"
#include "monitor.h"
#include "antMonitor.h"
#define DSM_HOST "gltacc"
#define DEBUG 0
#if DEBUG
  #define movemacro(x,y) refresh();move(x,y);
#else
  #define movemacro(x,y) move(x,y)
#endif

void screen(char *source,double *lst_disp,double *utc_disp,double *tjd_disp,
	double *ra_disp,double *dec_disp, 
	double *ra_cat_disp, double *dec_cat_disp,
	double *az_disp, double *el_disp,int *icount,double *azoff,double *eloff,
	double *az_actual_corrected,double *el_actual_disp,
	double *az_error, double *el_error, char *messg,
	float *refraction, float *pmdaz, float *pmdel,
	int *radio_flag, double *Az_cmd, double *El_cmd, char *lastcommand);

int antDisplay(int ant, int icount) {
  char dummyByte;
  float plsAzRdg;
  char messg[100];
  char lastcommand[100];
  int rms;
  
  extern double radian;
  
  char source[SOURCE_CHAR_LEN] ;
  double lst_disp, utc_disp, tjd_disp;
  double ra_disp, dec_disp, ra_cat_disp, dec_cat_disp;
  double az_disp, el_disp; 
  double azoff, eloff; 
  double raoff,decoff;
  double az_actual_corrected, el_actual_disp ;
  double az_error, el_error;
  float refraction, pmdaz, pmdel;
  int radio_flag; 
  double Az_cmd, El_cmd; 
  float dummyFloat;
  double dummyDouble;
  short dummyShort;
  double timeError;
  char host[6];
  char azDriveState;
  time_t timestamp;
  short disableDrivesFlag;


          rms=call_dsm_read(DSM_HOST,"DSM_AZOFF_ARCSEC_D",&azoff,&timestamp);	
	  rms=call_dsm_read(DSM_HOST,"DSM_ELOFF_ARCSEC_D",&eloff,&timestamp);	

	  rms=call_dsm_read(DSM_HOST,"DSM_REFRACTION_ARCSEC_D",&dummyDouble,&timestamp);
	  refraction=(float)dummyDouble;
	  rms=call_dsm_read(DSM_HOST,"DSM_REFRACTION_RADIO_FLAG_B",&dummyByte,&timestamp);
	  radio_flag=(int)dummyByte; 
	  
	  rms=call_dsm_read(DSM_HOST,"DSM_COMMANDED_AZ_DEG_F",&dummyFloat,&timestamp);
	  az_disp=(double)dummyFloat;
	  rms=call_dsm_read(DSM_HOST,"DSM_COMMANDED_EL_DEG_F",&dummyFloat,&timestamp);
	  el_disp=(double)dummyFloat;
	  rms=call_dsm_read(DSM_HOST, "DSM_AZ_TRACKING_ERROR_F",&dummyFloat,&timestamp);
printf("az tracking error = %f\n",dummyFloat);
          if(rms != DSM_SUCCESS) {
           dsm_error_message(rms,"dsm_write(DSM_AZ_TRACKING_ERROR_F)");
                }

	  az_error=(double)dummyFloat;
	  rms=call_dsm_read(DSM_HOST, "DSM_EL_TRACKING_ERROR_F",&dummyFloat,&timestamp);
printf("el tracking error = %f\n",dummyFloat);
	  el_error=(double)dummyFloat;

	  rms=call_dsm_read(DSM_HOST,"DSM_RA_APP_HR_D", &ra_disp,&timestamp);
	  rms=call_dsm_read(DSM_HOST,"DSM_DEC_APP_DEG_D",&dec_disp,&timestamp);

	  rms=call_dsm_read(DSM_HOST,"DSM_SOURCE_C34",source,&timestamp);
	  rms=call_dsm_read(DSM_HOST,"DSM_LST_HOURS_F",&dummyFloat,&timestamp);
	  lst_disp=(double)dummyFloat;
	  rms=call_dsm_read(DSM_HOST,"DSM_UTC_HOURS_F",&dummyFloat,&timestamp);
	  utc_disp=(double)dummyFloat;
	  rms=call_dsm_read(DSM_HOST,"DSM_TJD_D",&tjd_disp,&timestamp);
	  rms=call_dsm_read(DSM_HOST,"DSM_RA_CAT_HOURS_F",&dummyFloat,&timestamp);
	  ra_cat_disp=(double)dummyFloat;
	  rms=call_dsm_read(DSM_HOST,"DSM_DEC_CAT_DEG_F",&dummyFloat,&timestamp);
	  dec_cat_disp=(double)dummyFloat;
	  rms=call_dsm_read(DSM_HOST,"DSM_ACTUAL_AZ_DEG_F",&dummyFloat,&timestamp);
	  az_actual_corrected=(double)dummyFloat;
	  rms=call_dsm_read(DSM_HOST,"DSM_ACTUAL_EL_DEG_F",&dummyFloat,&timestamp);
	  el_actual_disp=(double)dummyFloat;
	  rms=call_dsm_read(DSM_HOST,"DSM_PMDAZ_F",&pmdaz,&timestamp);
	  rms=call_dsm_read(DSM_HOST,"DSM_PMDEL_F",&pmdel,&timestamp);
	  rms=call_dsm_read(DSM_HOST,"DSM_TRACK_MESSAGE_C100",messg,&timestamp);
	  rms=call_dsm_read(DSM_HOST,"DSM_TRACK_LAST_COMMAND_C100",lastcommand,&timestamp);

	  rms=call_dsm_read(DSM_HOST,"DSM_COMMANDED_AZ_DEG_F",&dummyFloat,&timestamp);
	  Az_cmd=(double)dummyFloat;
	  rms=call_dsm_read(DSM_HOST,"DSM_COMMANDED_EL_DEG_F",&dummyFloat,&timestamp);
	  El_cmd=(double)dummyFloat;


	  screen(source, &lst_disp, &utc_disp, &tjd_disp, &ra_disp, &dec_disp,
		 &ra_cat_disp, &dec_cat_disp,
		 &az_disp, &el_disp, &icount, &azoff, &eloff, 
		 &az_actual_corrected, &el_actual_disp, 
		 &az_error, &el_error,  messg, 
		 &refraction, 
		 &pmdaz, &pmdel, 
		 &radio_flag,
		 &Az_cmd,&El_cmd,lastcommand);
/*
printf("%s %f %f %f %f %f\n",source,lst_disp, utc_disp,tjd_disp,ra_disp,dec_disp);
printf("%f %f %f %f %d\n",ra_cat_disp,dec_cat_disp,az_disp,el_disp,icount);
printf("%f %f %f %f \n",azoff,eloff,az_actual_corrected,el_actual_disp);
printf("%f %f %f %s \n",az_error,el_error,refraction,messg);
printf("%f %f %d %f %f %s \n",pmdaz,pmdel,radio_flag,Az_cmd,El_cmd,lastcommand);
*/
	return 0;
}

void hms(double *fx, int *fh, int *fm, double *fs,short *dec_sign);
void af(int *i,char s[2]);

void screen(char *source,double *lst_disp,double *utc_disp,double *tjd_disp,
	double *ra_disp,double *dec_disp, 
	double *ra_cat_disp, double *dec_cat_disp,
	double *az_disp,
	double *el_disp,int *icount,double *azoff,double *eloff,
	double *az_actual_corrected,double *el_actual_disp,
	double *az_error, double *el_error, char *messg,
	float *refraction, float *pmdaz, float *pmdel,
	int *radio_flag, double *Az_cmd, 
	double *El_cmd, char *lastcommand)
{
  short az_act_sign,az_sign,el_sign,dec_dum_sign,dec_app_sign,dec_cat_sign;
  char str[3];
  int lsth,lstm,ra_cat_h,ra_cat_m,dec_cat_d,dec_cat_m;
  int dec_app_d,dec_app_m;
  int ra_app_h,ra_app_m;
  int utch,utcm;
  int rms;
  double a1,a2;
  int az_cmd_d,az_cmd_m,el_cmd_d,el_cmd_m;
  int az_act_d,az_act_m,el_act_d,el_act_m;
  double ra_cat_s,dec_cat_s,ra_app_s,dec_app_s,lsts,utcs;
  double az_cmd_s,el_cmd_s,az_act_s,el_act_s;
  int lstsi,utcsi,az_cmd_si,az_act_si,el_cmd_si,el_act_si;
  double ha;
  double timeError;
  char host[6];
  int wackoSource, i;
  char azDriveState;
  char padIDIsFake;
  long  unixTime;
  time_t timestamp;
  extern double radian;
  float dummyFloat;
  float dummyFloat2[2];
  double cmdEpoch;
  float actualEpoch;
  char messgbuf[40];
  short disableDrivesFlag;
  short pair=2;
	
  short foreground,background;

  ha=*lst_disp-*ra_disp;

  if((*icount%30)==0) {
    initialize();
  }
  hms(ra_disp,&ra_app_h,&ra_app_m,&ra_app_s,&dec_dum_sign);
  hms(dec_disp,&dec_app_d,&dec_app_m,&dec_app_s,&dec_app_sign);
  hms(ra_cat_disp,&ra_cat_h,&ra_cat_m,&ra_cat_s,&dec_dum_sign);
  hms(dec_cat_disp,&dec_cat_d,&dec_cat_m,&dec_cat_s,&dec_cat_sign);
  hms(lst_disp,&lsth,&lstm,&lsts,&dec_dum_sign);
  hms(utc_disp,&utch,&utcm,&utcs,&dec_dum_sign);
  hms(az_disp,&az_cmd_d,&az_cmd_m,&az_cmd_s,&az_sign);
  hms(el_disp,&el_cmd_d,&el_cmd_m,&el_cmd_s,&el_sign);
  hms(az_actual_corrected,&az_act_d,&az_act_m,&az_act_s,&az_act_sign);
  hms(el_actual_disp,&el_act_d,&el_act_m,&el_act_s,&dec_dum_sign);
  
  
  
  lstsi=(int)lsts;
  utcsi=(int)utcs;
  az_cmd_si=(int)az_cmd_s;
  el_cmd_si=(int)el_cmd_s;
  az_act_si=(int)az_act_s;
  el_act_si=(int)el_act_s;
  
  /* box(stdscr, '|','-'); */
  
/*
  dsm_read(host,"DSM_ESTOP_BYPASS_L",(char *)&estopBypass, &timestamp);
*/
  movemacro(2,2);
  printw("GLT Antenna tracking ");

 

  movemacro (4,5);
  addstr("LST");
  move (4,15);
  addstr("UTC");
  movemacro(4,25);
  addstr("TJD");
  movemacro(2,32);
  if (strlen(source) > SOURCE_CHAR_LEN) {
    printw("wacko");
  } else { 
    wackoSource = 0;
    for (i=0; i<strlen(source); i++) {
      if (source[i] < 0) {
	printw("wacko");
	wackoSource = 1;
	break;
      }
    }
    if (wackoSource == 0) {
      addstr(source);
    }
  }
  move (5,3);
  af(&lsth,str);
  if (isdigit(str[0]) == 0 || isdigit(str[1]) == 0) {
    printw("wa");
  } else {
    addch(str[0]);
    addch(str[1]);
  }
  movemacro(5,6);
  af(&lstm,str);
  if (isdigit(str[0]) == 0 || isdigit(str[1]) == 0) {
    printw("wa");
  } else {
    addch(str[0]);
    addch(str[1]);
  }
  movemacro(5,9);
  af(&lstsi,str);
  if (isdigit(str[0]) == 0 || isdigit(str[1]) == 0) {
    printw("wa");
  } else {
    addch(str[0]);
    addch(str[1]);
  }
  movemacro(5,13);
  af(&utch,str);
  if (isdigit(str[0]) == 0 || isdigit(str[1]) == 0) {
    printw("wa");
  } else {
    addch(str[0]);
    addch(str[1]);
  }
  movemacro(5,16);
  af(&utcm,str);
  if (isdigit(str[0]) == 0 || isdigit(str[1]) == 0) {
    printw("wa");
  } else {
    addch(str[0]);
    addch(str[1]);
  }
  movemacro(5,19);
  af(&utcsi,str);
  if (isdigit(str[0]) == 0 || isdigit(str[1]) == 0) {
    printw("wa");
  } else {
    addch(str[0]);
    addch(str[1]);
  }

  movemacro(5,23);
  printw("%lf",*tjd_disp);
  movemacro(6,3);
  printw("H.A.: ");
#if DEBUG
  refresh();
#endif  
  if (fabs(ha) < 100) {
    printw("%+.4f",ha);
  } else {
    printw("wacko");
  }
#if DEBUG
  refresh();
#endif  
#if DEBUG
  refresh();
#endif  
  
#if DEBUG
  refresh();
#endif  
  movemacro(7,16);
  addstr("RA");
  movemacro(7,30);
  addstr("DEC");
  movemacro(8,2);
  addstr("CATALOG");
  movemacro(8,12);
  af(&ra_cat_h,str);
  if (isdigit(str[0]) == 0 || isdigit(str[1]) == 0) {
    printw("wa");
  } else {
    addch(str[0]);
    addch(str[1]);
  }
  movemacro(8,15);
  af(&ra_cat_m,str);
  if (isdigit(str[0]) == 0 || isdigit(str[1]) == 0) {
    printw("wa");
  } else {
    addch(str[0]);
    addch(str[1]);
  }
  movemacro(8,18);
  if (fabs(ra_cat_s) >= 100) {
    printw("wacko");
  } else {
    printw("%06.3f",ra_cat_s);
  }
  movemacro(8,27);
  if (dec_cat_sign>0) addch('+');
  if (dec_cat_sign<0) addch('-');
  if (dec_cat_sign==0) addch(' ');
  af(&dec_cat_d,str);
  if (isdigit(str[0]) == 0 || isdigit(str[1]) == 0) {
    printw("wa");
  } else {
    addch(str[0]);
    addch(str[1]);
  }
  movemacro(8,31);
  af(&dec_cat_m,str);
  if (isdigit(str[0]) == 0 || isdigit(str[1]) == 0) {
    printw("wa");
  } else {
    addch(str[0]);
    addch(str[1]);
  }
  movemacro(8,34);
  if (fabs(dec_cat_s) >= 100) {
    printw("wacko");
  } else {
    printw("%05.2f",dec_cat_s);
  }
  
  movemacro(9,2);
  printw("APPARENT");
  movemacro(9,12);
  af(&ra_app_h,str);
  if (isdigit(str[0]) == 0 || isdigit(str[1]) == 0) {
    printw("wa");
  } else {
    addch(str[0]);
    addch(str[1]);
  }
  movemacro(9,15);
  af(&ra_app_m,str);
  if (isdigit(str[0]) == 0 || isdigit(str[1]) == 0) {
    printw("wa");
  } else {
    addch(str[0]);
    addch(str[1]);
  }
  movemacro(9,18);
  if (fabs(ra_app_s) >= 100) {
    standout();
    printw("wacko");
    standend();
  } else {
    printw("%06.3f",ra_app_s);
  }
  movemacro(9,27);
  if(dec_app_sign>0)addch('+');
  if(dec_app_sign<0)addch('-');
  if(dec_app_sign==0)addch(' ');
  af(&dec_app_d,str);
  if (isdigit(str[0]) == 0 || isdigit(str[1]) == 0) {
    printw("wa");
  } else {
    addch(str[0]);
    addch(str[1]);
  }
  movemacro(9,31);
  af(&dec_app_m,str);
  if (isdigit(str[0]) == 0 || isdigit(str[1]) == 0) {
    printw("wa");
  } else {
    addch(str[0]);
    addch(str[1]);
  }
  movemacro(9,34);
  if (fabs(dec_app_s) >= 100) {
    standout();
    printw("wacko");
    standend();
  } else {
    printw("%05.2f",dec_app_s);
  }
  
  movemacro(11,14);
  addstr("AZIM");
  movemacro(11,30);
  addstr("ELEV");
  
  move(12,2);
  addstr("CMD");
  move(12,9);
#if DEBUG
  refresh();
#endif  
  if((az_sign<0)&&(az_cmd_d==0))addch('-');
  move(12,10);
  if (abs(az_cmd_d) > 399) {
    standout();
    printw("wac");
    standend();
  } else {
    printw("%03d",az_cmd_d);
  }
  move(12,15);
  if (az_cmd_m >= 60 || az_cmd_m < 0) {
    standout();
    printw("wa");
    standend();
  } else {
    af(&az_cmd_m,str);
    addch(str[0]);
    addch(str[1]);
  }
  move(12,18);
  if (az_cmd_si >= 60 || az_cmd_si < 0) {
    standout();
    printw("wa");
    standend();
  } else {
    af(&az_cmd_si,str);
    addch(str[0]);
    addch(str[1]);
    move(12,27);
  }
#if DEBUG
  refresh();
#endif  
  if(el_sign>=0)addch(' ');
  if(el_sign<0)addch('-');
  move(12,28);
  if (fabs(el_cmd_d) > 90) {
    standout();
    printw("wa");
    standend();
  } else {
    af(&el_cmd_d,str);
    addch(str[0]);
    addch(str[1]);
  }
#if DEBUG
  refresh();
#endif  
  move(12,31);
  if (el_cmd_m >= 60 || el_cmd_m < 0) {
    standout();
    printw("wa");
    standend();
  } else {
    af(&el_cmd_m,str);
    addch(str[0]);
    addch(str[1]);
  }
#if DEBUG
  refresh();
#endif  
  move(12,34);
  if (el_cmd_si >= 60 || el_cmd_si < 0) {
    standout();
    printw("wa");
    standend();
  } else {
    af(&el_cmd_si,str);
    addch(str[0]);
    addch(str[1]);
  }
  move(13,2);
  addstr("ACTUAL");
  move(13,9);
  if((az_act_sign<0)&&(az_act_d==0))addch('-');
  move(13,10);
  printw("%03d",az_act_d);
  move(13,15);
  af(&az_act_m,str);
  addch(str[0]);
  addch(str[1]);
  move(13,18);
  af(&az_act_si,str);
  addch(str[0]);
  addch(str[1]);
#if DEBUG
  refresh();
#endif  
  if (el_act_d >= 100 || el_act_d <= -10) {
    move(13,27);
    standout();
    printw("wac");
    standend();
  } else {
    move(13,28);
    af(&el_act_d,str);
    addch(str[0]);
    addch(str[1]);
  }
#if DEBUG
  refresh();
#endif  
  move(13,31);
  if (el_cmd_m >= 60 || el_cmd_m < 0) {
    standout();
    printw("wa");
    standend();
  } else {
    af(&el_act_m,str);
    addch(str[0]);
    addch(str[1]);
  }
#if DEBUG
  refresh();
#endif  
  move(13,34);
  if (el_cmd_si >= 60 || el_cmd_si < 0) {
    standout();
    printw("wa");
    standend();
  } else {
    af(&el_act_si,str);
    addch(str[0]);
    addch(str[1]);
  }
#if DEBUG
  refresh();
#endif  
  move(14,2);
  addstr("ERROR");
  move(14,11);
#define LARGE_TRACKING_ERROR 2.0
  if (fabs(*az_error) > LARGE_TRACKING_ERROR) {
    standout();
  }
  printw("%7.1f\"",*az_error);
  standend();
#if DEBUG
  refresh();
#endif  
  move(14,28);
  if (fabs(*el_error) > LARGE_TRACKING_ERROR) {
    standout();
  }
  printw("%7.1f\"",*el_error);
  standend();
#if DEBUG
  refresh();
#endif  
  /*move(18,23);
    printw("(%.1f\")",*smoothed_tracking_error);
  */
  move(15,2);
  addstr("PMODELS(\")");
  move(15,12);
  if (fabs(*pmdaz) >= 1000000) {
    standout();
    printw("(wacko)");
    standend();
  } else {
    printw("(%6.0f)",*pmdaz);
  }
  move(15,32);
  if (fabs(*pmdel) >= 1000000) {
    standout();
    printw("(wacko)");
    standend();
  } else {
    printw("(%6.0f)",*pmdel);
  }
  move(16,2);
  addstr("OFFSETS(\")");
  move(16,12);
#if DEBUG
  refresh();
#endif  
  
  if (fabs(*azoff) > WACKO_OFFSET) {
    standout();
    printw(" wacko");
    standend();
  } else {
    if (fabs(*azoff) >= 20.) {
      standout();
    }
    printw("%6.0f",*azoff);
    standend();
  }
#if DEBUG
  refresh();
#endif  
  move(16,32);
  if (fabs(*eloff) > WACKO_OFFSET) {
    standout();
    printw(" wacko");
    standend();
  } else {
    if(fabs(*eloff) >= 20.) standout();
    printw("%6.0f",*eloff);
    standend();
  }
#if DEBUG
  refresh();
#endif  
  move(18,2);
  addstr("REFRACTION:");
  move(18,13);
#if DEBUG
  refresh();
#endif  
  switch (*radio_flag) {
  case 1:
    printw("%5.1f \" (radio)   ",*refraction);
    break;
  case 0:
    standout();
    printw("%5.1f \" (optical) ",*refraction);
    standend();
    break;
  default:
    printw(" wacko");
    break;
  }
#if DEBUG
  refresh();
#endif
  
#ifdef LINUX
/*
  if(colorFlag) {
  pair_content(pair,&foreground,&background);
  init_pair(1,COLOR_CYAN,background);
  attron(COLOR_PAIR(1));
  }
*/
#endif

  move(20,2);
  addstr("Message:");
  move(20,13);
  printw("%s",messg);

  move(22,2);
  addstr("Last cmd:");
  move(22,13);
  printw("%s",lastcommand);

refresh();
}

void hms(double *fx, int *fh, int *fm, double *fs,short *dec_sign)
{
	double fmt;
	double absfx;

	if(*fx<0.) {
		absfx=-*fx;
		*dec_sign=-1;
		}
	if(*fx>=0.) {
		absfx=*fx;
		*dec_sign=1;
		}
    *fh = (int)absfx;
    fmt = (absfx - *fh) * 60.;
    *fm = (int) fmt;
    *fs = (fmt - *fm) * 60.;
    if (*fx < 0.) {
	*fh = -(*fh);
    }
}

void af(int *i,char s[2])
{
int j,k,l;
if(*i<0)*i=-*i;
j=48+*i;
if(*i<10) {s[0]='0';s[1]=(char)j;}
if(*i>=10){
k=*i%10;
l=(*i-k)/10;
j=48+k;
s[1]=j;
j=48+l;
s[0]=j;
}
}

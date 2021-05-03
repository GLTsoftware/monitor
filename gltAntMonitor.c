/* gltAntMonitor.c
for GLT.
Only uses DSM, no RM.
Removed all Tilt and Chopper related variables; showing only a small subset
of usual variables.
NAP 17 June 2012.

NAP 27 June 2012: added ACU monitor points as written by Servo 
(on the right side of the antenna page).


NAP 30 Apr 2021: rearranged main page display for clarity.

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

#define CSAT 800 

#define REFRESH_INTERVAL 3
#define COLOR_NORMAL   1
#define COLOR_LABEL    2
#define COLOR_EMPHASIS 3
#define COLOR_ENABLED  4
#define COLOR_DISABLED 5
#define COLOR_VARIANT  6


void screen(char *source,double *lst_disp,double *utc_disp,double *tjd_disp,
	double *ra_disp,double *dec_disp, 
	double *ra_cat_disp, double *dec_cat_disp,
	double *az_disp, double *el_disp,int *icount,double *azoff,double *eloff,
	double *az_actual_corrected,double *el_actual_disp,
	double *az_error, double *el_error, char *messg,
	float *refraction, float *pmdaz, float *pmdel,
	int *radio_flag, double *Az_cmd, double *El_cmd, char *lastcommand,
        char *azServoStatus, char *elServoStatus,
        short *acuModeAz,short *acuModeEl,char *acuSystemGS,
        int *acuDay,int *acuHour, char *acuErrorMessage,
        float *temperature,float *pressure,float *humidity,float *windspeed,
        double *windchill,double *dftemp,
        float *winddir,float *tau,float *tsysAmbLeft,float *tsysAmbRight,
        float *tsysAtmLeft, float *tsysAtmRight, float *polar_dut_f,
        short *azm1t,short *azm2t,short *elm1t,short *elm2t,short *elm3t,short *elm4t,
        float *azm1i,float *azm2i,float *elm1i,float *elm2i,float *elm3i,float *elm4i);

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
  char azDriveState;
  time_t timestamp;
  short disableDrivesFlag;

 /* ACU status variables from DSM*/
  short acuModeAz,acuModeEl;
  char azServoStatus[2],elServoStatus[2];
  int acuDay,acuHour;
  char acuErrorMessage[256],acuSystemGS[6];




 /* weather variables from DSM */
  float temperature,pressure,humidity,windspeed,winddir;
  double tamb,tambk,ws,wsp,windchill,dftemp;
  double numSum,denSum,lne_s;

/* radiometer variables (only tau for now) */
  float tau;

/* tsys */
  float tsysAmbLeft,tsysAmbRight,tsysAtmLeft,tsysAtmRight;

  float polar_dut_f;
     short azm1t,azm2t,elm1t,elm2t,elm3t,elm4t;
     float azm1i,azm2i,elm1i,elm2i,elm3i,elm4i;


/*
printf("Time: %d\n",acuStatusResp.timeOfDay);
  hours = acuStatusResp.timeOfDay/3600000.;
  hh = (int)hours;
  minutes = (hours-hh)*60.;
  mm = (int) minutes;
  seconds = (minutes-mm)*60.;
  printf ("ACU Time: (day, hh:mm:ss.sss):  %d %02d:%02d:%02.3f\n",acuStatusResp.dayOfYear,hh,mm,seconds);
*/

          rms=call_dsm_read(DSM_HOST,"DSM_AZOFF_ARCSEC_D",&azoff,&timestamp);	
	  rms=call_dsm_read(DSM_HOST,"DSM_ELOFF_ARCSEC_D",&eloff,&timestamp);	

	  rms=call_dsm_read(DSM_HOST,"DSM_REFRACTION_ARCSEC_D",&dummyDouble,&timestamp);
	  refraction=(float)dummyDouble;
	  rms=call_dsm_read(DSM_HOST,"DSM_REFRACTION_RADIO_FLAG_B",&dummyByte,&timestamp);
	  radio_flag=(int)dummyByte; 
	  
	  rms=call_dsm_read(DSM_HOST,"DSM_CMDDISP_AZ_DEG_D",&dummyDouble,&timestamp);
	  az_disp=(double)dummyDouble;
	  rms=call_dsm_read(DSM_HOST,"DSM_CMDDISP_EL_DEG_D",&dummyDouble,&timestamp);
	  el_disp=(double)dummyDouble;
	  rms=call_dsm_read(DSM_HOST, "DSM_AZ_TRACKING_ERROR_F",&dummyFloat,&timestamp);
	  az_error=(double)dummyFloat;
	  rms=call_dsm_read(DSM_HOST, "DSM_EL_TRACKING_ERROR_F",&dummyFloat,&timestamp);
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
	  rms=call_dsm_read(DSM_HOST,"DSM_ACTUAL_AZ_DEG_D",&az_actual_corrected,&timestamp);
          if (rms!=DSM_SUCCESS) printf("dsm_read error: DSM_ACTUAL_AZ_DEG_D, return code=%d\n",rms);
	  rms=call_dsm_read(DSM_HOST,"DSM_ACTUAL_EL_DEG_D",&el_actual_disp,&timestamp);
          if (rms!=DSM_SUCCESS) printf("dsm_read error: DSM_ACTUAL_EL_DEG_D, return code=%d\n",rms);
	  rms=call_dsm_read(DSM_HOST,"DSM_PMDAZ_F",&pmdaz,&timestamp);
	  rms=call_dsm_read(DSM_HOST,"DSM_PMDEL_F",&pmdel,&timestamp);
	  rms=call_dsm_read(DSM_HOST,"DSM_TRACK_MESSAGE_C100",messg,&timestamp);
	  rms=call_dsm_read(DSM_HOST,"DSM_TRACK_LAST_COMMAND_C100",lastcommand,&timestamp);

/*
	  rms=call_dsm_read(DSM_HOST,"DSM_COMMANDED_AZ_DEG_D",&dummyFloat,&timestamp);
	  Az_cmd=(double)dummyFloat;
	  rms=call_dsm_read(DSM_HOST,"DSM_COMMANDED_EL_DEG_D",&dummyFloat,&timestamp);
	  El_cmd=(double)dummyFloat;
*/

  rms = dsm_read(DSM_HOST,"DSM_ACU_SERVO_STATUS_AZ_C2",azServoStatus,&timestamp);
  rms = dsm_read(DSM_HOST,"DSM_ACU_SERVO_STATUS_EL_C2",elServoStatus,&timestamp);
  rms = dsm_read(DSM_HOST,"DSM_ACU_MODE_STATUS_AZ_S",&acuModeAz,&timestamp);
  rms = dsm_read(DSM_HOST,"DSM_ACU_MODE_STATUS_EL_S",&acuModeEl,&timestamp);
  rms = dsm_read(DSM_HOST,"DSM_ACU_SYSTEMGS_C6",acuSystemGS,&timestamp);
  rms = dsm_read(DSM_HOST,"DSM_ACU_DAYOFYEAR_L",&acuDay,&timestamp);
  rms = dsm_read(DSM_HOST,"DSM_ACU_HOUR_L",&acuHour,&timestamp);
  rms = dsm_read(DSM_HOST,"DSM_WEATHER_TEMP_C_F",&temperature,&timestamp);
  rms = dsm_read(DSM_HOST,"DSM_WEATHER_PRESS_MBAR_F",&pressure,&timestamp);
  rms = dsm_read(DSM_HOST,"DSM_WEATHER_HUMIDITY_F",&humidity,&timestamp);
  rms = dsm_read(DSM_HOST,"DSM_WEATHER_WINDSPEED_MPS_F",&windspeed,&timestamp);
  rms = dsm_read(DSM_HOST,"DSM_WEATHER_WINDDIR_AZDEG_F",&winddir,&timestamp);
  rms = dsm_read(DSM_HOST,"DSM_RADIOMETER_TAU_F",&tau,&timestamp);
  rms = dsm_read(DSM_HOST,"DSM_TSYS_AMB_LEFT_F",&tsysAmbLeft,&timestamp);
  rms = dsm_read(DSM_HOST,"DSM_TSYS_AMB_RIGHT_F",&tsysAmbRight,&timestamp);
  rms = dsm_read(DSM_HOST,"DSM_TSYS_ATM_LEFT_F",&tsysAtmLeft,&timestamp);
  rms = dsm_read(DSM_HOST,"DSM_TSYS_ATM_RIGHT_F",&tsysAtmRight,&timestamp);
  rms = dsm_read(DSM_HOST,"DSM_POLAR_DUT_SECONDS_F", &polar_dut_f,&timestamp);
  rms = dsm_read(DSM_HOST,"DSM_AZ_MOTOR1_TEMP_S",&azm1t,&timestamp);
  rms = dsm_read(DSM_HOST,"DSM_AZ_MOTOR2_TEMP_S",&azm2t,&timestamp);
  rms = dsm_read(DSM_HOST,"DSM_EL_MOTOR1_TEMP_S",&elm1t,&timestamp);
  rms = dsm_read(DSM_HOST,"DSM_EL_MOTOR2_TEMP_S",&elm2t,&timestamp);
  rms = dsm_read(DSM_HOST,"DSM_EL_MOTOR3_TEMP_S",&elm3t,&timestamp);
  rms = dsm_read(DSM_HOST,"DSM_EL_MOTOR4_TEMP_S",&elm4t,&timestamp);
  rms = dsm_read(DSM_HOST,"DSM_AZ_MOTOR1_CURRENT_F",&azm1i,&timestamp);
  rms = dsm_read(DSM_HOST,"DSM_AZ_MOTOR2_CURRENT_F",&azm2i,&timestamp);
  rms = dsm_read(DSM_HOST,"DSM_EL_MOTOR1_CURRENT_F",&elm1i,&timestamp);
  rms = dsm_read(DSM_HOST,"DSM_EL_MOTOR2_CURRENT_F",&elm2i,&timestamp);
  rms = dsm_read(DSM_HOST,"DSM_EL_MOTOR3_CURRENT_F",&elm3i,&timestamp);
  rms = dsm_read(DSM_HOST,"DSM_EL_MOTOR4_CURRENT_F",&elm4i,&timestamp);

/* calculating Wind Chill (p372 of Vaisala AWS310 user guide) */
  tamb = (double)temperature;
  ws = (double)windspeed;
  wsp = pow(ws,0.16);
  windchill = 13.13 + 0.62*tamb
                    - 13.95*wsp
                    + 0.486*tamb*wsp;

/* calculating dew or frost point temperature */
/* formulae and constants are from Bob Hardy, Thunder Scientific Corp.
report: ITS-90 Formulations for vapor pressure, frostpoint temp., dewpoint temp. */
tambk=tamb+273.15;
if (tamb <= 0.) {
lne_s =  0.67063522*log(tambk)-5.866426e3/tambk +22.3870244+1.39387003e-2*tambk-
3.4262402e-5*pow(tambk,2.)+2.7040955e-8*pow(tambk,3.);
numSum=2.1257969e2-10.264612*lne_s+1.4354796e-1*lne_s*lne_s;
denSum=1.0-8.2871619e-2*lne_s+2.3540411e-3*lne_s*lne_s-2.4363951e-5*pow(lne_s,3.0);
} else {
lne_s = 2.7150305 * log(tambk) - 2836.5744* pow(tambk,-2.) -6028.076559* pow(tambk,-1.)+ 19.54263612 - 0.02737830188 * tambk +1.6261698e-5* pow(tambk,2.0)+
7.0229056e-10*pow(tambk,3.0)-1.8680009e-13*pow(tambk,4.0);
numSum=2.0798233e2-2.0156028*lne_s+4.6778925e-1*pow(lne_s,2.)-9.2288067e-6*pow(lne_s,3.0);
denSum=1.0-1.3319669e-1*lne_s+5.6577518e-3*lne_s*lne_s-7.5172865e-5*pow(lne_s,3.);
}
dftemp=(numSum/denSum)-273.15;

	  screen(source, &lst_disp, &utc_disp, &tjd_disp, &ra_disp, &dec_disp,
		 &ra_cat_disp, &dec_cat_disp,
		 &az_disp, &el_disp, &icount, &azoff, &eloff, 
		 &az_actual_corrected, &el_actual_disp, 
		 &az_error, &el_error,  messg, 
		 &refraction, 
		 &pmdaz, &pmdel, 
		 &radio_flag,
		 &Az_cmd,&El_cmd,lastcommand,azServoStatus,elServoStatus,
                 &acuModeAz,&acuModeEl,acuSystemGS,&acuDay,&acuHour,
                 acuErrorMessage,&temperature,&pressure,&humidity,
                 &windspeed,&windchill,&dftemp,&winddir,&tau,
                 &tsysAmbLeft,&tsysAmbRight,
                 &tsysAtmLeft,&tsysAtmRight,&polar_dut_f,
                 &azm1t,&azm2t,&elm1t,&elm2t,&elm3t,&elm4t,
                 &azm1i,&azm2i,&elm1i,&elm2i,&elm3i,&elm4i);
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
	double *az_disp, double *el_disp,int *icount,double *azoff,double *eloff,
	double *az_actual_corrected,double *el_actual_disp,
	double *az_error, double *el_error, char *messg,
	float *refraction, float *pmdaz, float *pmdel,
	int *radio_flag, double *Az_cmd, double *El_cmd, char *lastcommand,
        char *azServoStatus, char *elServoStatus,
        short *acuModeAz,short *acuModeEl,char *acuSystemGS,
        int *acuDay,int *acuHour, char *acuErrorMessage,
        float *temperature, float *pressure, float *humidity,
        float *windspeed, double *windchill, double *dftemp,float *winddir,
        float *tau,
        float *tsysAmbLeft, float *tsysAmbRight, float *tsysAtmLeft,
        float *tsysAtmRight, float *polar_dut_f,
        short *azm1t,short *azm2t,short *elm1t,short *elm2t,short *elm3t,short *elm4t,
        float *azm1i,float *azm2i,float *elm1i,float *elm2i,float *elm3i,float *elm4i)
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

  char acu_error,az_brake,az_status[8],el_brake,el_status[8];
  char stow_pin[2],system_status[3];

  int nextline,nextcol;

  int acuH,acuM;
  double acuS,acuTime;
  short acuTimesign;

 int bgColor = COLOR_BLACK;

  start_color();
  init_pair(1,COLOR_RED,COLOR_BLACK); /*red on black */
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


  ha=*lst_disp-*ra_disp;

  if((*icount%REFRESH_INTERVAL)==0) {
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
  

  acuTime = (double)*acuHour/3600000.;
  hms(&acuTime,&acuH,&acuM,&acuS,&acuTimesign); 
  
  
  lstsi=(int)lsts;
  utcsi=(int)utcs;
  az_cmd_si=(int)az_cmd_s;
  el_cmd_si=(int)el_cmd_s;
  az_act_si=(int)az_act_s;
  el_act_si=(int)el_act_s;
  
  box(stdscr, '|','-'); 
  
/*
  dsm_read(host,"DSM_ESTOP_BYPASS_L",(char *)&estopBypass, &timestamp);
*/
  movemacro(2,2);
  printLabel("GLT Antenna tracking ");

 

  movemacro (4,5);
  printLabel("LST");
  move (4,15);
  printLabel("UTC");
  movemacro(4,25);
  printLabel("TJD");
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
  printLabel("H.A.: ");
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

  movemacro(6,20);
  printLabel("dUT: ");
  printw("%.3f",*polar_dut_f);
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
  printLabel("CATALOG");
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
  printLabel("APPARENT");
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
  printLabel("Azimuth");
  movemacro(11,30);
  printLabel("Elevation");
  
  move(12,2);
  printLabel("CMD");
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
  printLabel("ACTUAL");
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
  printLabel("TrError");
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
  printLabel("PMODELS(\")");
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
  printLabel("OFFSETS(\")");
  move(16,12);
#if DEBUG
  refresh();
#endif  
  
  if (fabs(*azoff) > WACKO_OFFSET) printDisabled(" wacko");
  if (fabs(*azoff) >= 20.) {
      setDisabled();
      printw("%6.0f",*azoff);
      normalText();
    } else { 
      printw("%6.0f",*azoff);
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
  move(17,2);
  printLabel("REFRACTION:");
  move(17,13);
#if DEBUG
  refresh();
#endif  
  switch (*radio_flag) {
  case 1:
    printw("%5.1f \" (radio)   ",*refraction);
    break;
  case 0:
    printDisabled("%5.1f \" (optical) ",*refraction);
    break;
  default:
    printw(" wacko");
    break;
  }
#if DEBUG
  refresh();
#endif
  
  nextline=30;
  nextcol=2;
  move(nextline,nextcol);
   printLabel("Motor currents (A):");
  move(nextline+1,nextcol);
   printLabel("az1: ");
   printw("%.2f ",*azm1i);
   printLabel("az2: ");
   printw("%.2f ",*azm2i);
  move(nextline+2,nextcol);
   printLabel("el1: ");
   printw("%.2f",*elm1i);
   printLabel(" el2: ");
   printw("%.2f",*elm2i);
  move(nextline+3,nextcol);
   printLabel("el3: ");
   printw("%.2f",*elm3i);
   printLabel(" el4: ");
   printw("%.2f",*elm4i);


/* displaying weather */
  nextline=35;
  nextcol = 2;
  move(nextline,nextcol);
  printLabel("Weather:");
  nextline++;
  move(nextline,nextcol);
  printw("%.1f", *temperature);
  printUnit(" C");
  move(nextline+3,nextcol);
  printw("%.1f", *pressure);
  printUnit(" mbar");
  move(nextline+4,nextcol);
  printw("%.1f", *humidity);
  printLabel(" %%");
  move(nextline,nextcol+10);
  printw("%.1f", *windspeed);
  printUnit(" m/s");
  move(nextline,nextcol+20);
  printw("%.1f", *winddir);
  printUnit(" deg");
  move(nextline+4,nextcol+13);
  printLabel("Tau: ");
  printw("%.2f", *tau);

  move(nextline+1,nextcol);
  if(*windchill<=-45.) printDisabled("%.1f C (wind chill)",*windchill);
  else printw("%.1f C (wind chill)",*windchill);

  move(nextline+2,nextcol);
  if(*temperature<=0.) printw("%.1f C (frostpoint)",*dftemp);
  if(*temperature>0.) printw("%.1f C (dewpoint)",*dftemp);

/* tsys */
  move(nextline+6,nextcol);
  printLabel("Tsys (amb) L/R: ");
  move(nextline+6,nextcol+15);
  printw("%.1f %.1f", *tsysAmbLeft,*tsysAmbRight);
  printUnit(" K");
  move(nextline+7,nextcol);
  printLabel("Tsys (atm) L/R: ");
  move(nextline+7,nextcol+15);
  printw("%.1f %.1f", *tsysAtmLeft,*tsysAtmRight);
  printUnit(" K");

  nextline=45;
  nextcol=2;
  move(nextline,nextcol);
  printLabel("gltTrack msg:");
  move(nextline,nextcol+14);
  printw("%s",messg);
  nextline++;
  move(nextline,nextcol);
  printLabel("Last cmd:");
  move(nextline,nextcol+11);
  printw("%s",lastcommand);
  nextline++;
  move(nextline,nextcol);
  printLabel("ACU error msg:");
  move(nextline,nextcol+14);
  printw("%s",acuErrorMessage);



 /* 
  if(az_brake==0x00){
  move(4,60);
  printw("OFF");
  }

  if(az_brake==0x01){
  move(4,60);
  printw("ON");
  }

  if(el_brake==0x00){
  move(4,67);
  printw("OFF");
  }

  if(el_brake==0x01){
  move(4,67);
  printw("ON");
  }
*/

/*
  move(5,45);
  addstr("Stow-pins:");

  if(stow_pin[0]==1){
  move(5,60);
  printw("IN");
  }

  if(stow_pin[0]==2){
  move(5,60);
  printw("OUT");
  }

  if(stow_pin[1]==1){
  move(5,67);
  printw("IN");
  }

  if(stow_pin[1]==2){
  move(5,67);
  printw("OUT");
  }
*/

/* Az status */
  nextline=19;
  nextcol=2;
  move(nextline,nextcol);
  printLabel("Mode: ");
  nextcol=14;
  	move(nextline,nextcol);
        if(*acuModeAz==0x1) addstr("Stop");
        if(*acuModeAz==0x21) addstr("Maintenance");
        if(*acuModeAz==0x22) addstr("Self Test");
        if(*acuModeAz==0x2) addstr("Preset");
        if(*acuModeAz==0x3) addstr("Program Track");
        if(*acuModeAz==0x4) addstr("Rate");
        if(*acuModeAz==0x5) addstr("Sector Scan");
        if(*acuModeAz==0x6) addstr("Survival Stow");
        if(*acuModeAz==0xe) addstr("Maint. Stow");
        if(*acuModeAz==0x4e) addstr("Stow");
        if(*acuModeAz==0x26) addstr("Unstow");
        if(*acuModeAz==0x8) addstr("Az Two line track ");
        if(*acuModeAz==0x9) addstr("Az Star Track ");
        if(*acuModeAz==0x29) addstr("Az Sun Track ");

  move(nextline,nextcol+17);

        if(*acuModeEl==0x1) addstr("Stop");
        if(*acuModeEl==0x21) addstr("Maintenance");
        if(*acuModeEl==0x22) addstr("Self Test");
        if(*acuModeEl==0x2) addstr("Preset");
        if(*acuModeEl==0x3) addstr("Program Track");
        if(*acuModeEl==0x4) addstr("Rate");
        if(*acuModeEl==0x5) addstr("Sector Scan");
        if(*acuModeEl==0x6) addstr("Survival Stow ");
        if(*acuModeEl==0xe) addstr("Maint. Stow");
        if(*acuModeEl==0x4e) addstr("Stow ");
        if(*acuModeEl==0x26) addstr("Unstow");
        if(*acuModeEl==0x8) addstr("Two line track ");
        if(*acuModeEl==0x9) addstr("Star Track ");
        if(*acuModeEl==0x29) addstr("Sun Track ");


  nextline++;
  nextline++;
  nextcol=2;
  move(nextline,nextcol);
  printLabel("Servo: ");
  nextcol = nextcol+8;

        if(azServoStatus[0]&1) {
	move(nextline,nextcol);
        addstr("Emergency limit");
        nextline++;
        }
        if(azServoStatus[0]&2) {
	move(nextline,nextcol);
        addstr("Operating limit ccw");
        nextline++;
        }
        if(azServoStatus[0]&4) {
	move(nextline,nextcol);
        addstr("Operating limit cw");
        nextline++;
        }
        if(azServoStatus[0]&8) {
	move(nextline,nextcol);
        addstr("Prelimit ccw");
        nextline++;
        }
        if(azServoStatus[0]&16) {
	move(nextline,nextcol);
        addstr("Prelimit cw");
        nextline++;
        }
        if(azServoStatus[0]&32) {
	move(nextline,nextcol);
        addstr("stow position");
        nextline++;
        }
        if(azServoStatus[0]&64) {
	move(nextline,nextcol);
        addstr("stow pin inserted");
        nextline++;
        }
        if(azServoStatus[0]&128) {
	move(nextline,nextcol);
        addstr("stow pin retracted");
        nextline++;
        }

        if(azServoStatus[1]&1) {
	move(nextline,nextcol);
        addstr("Servo failure");
        nextline++;
        }
        if(azServoStatus[1]&2) {
	move(nextline,nextcol);
        addstr("Brake failure");
        nextline++;
        }
        if(azServoStatus[1]&4) {
	move(nextline,nextcol);
        addstr("Encoder failure");
        nextline++;
        }
        if(azServoStatus[1]&8) {
	move(nextline,nextcol);
        addstr("Auxiliary mode ");
        nextline++;
        }
        if(azServoStatus[1]&16) {
	move(nextline,nextcol);
        addstr("Motion failure");
        nextline++;
        }
        if(azServoStatus[1]&32) {
	move(nextline,nextcol);
        addstr("CAN bus failure");
        nextline++;
        }
        if(azServoStatus[1]&64) {
	move(nextline,nextcol);
        addstr("Axis disabled");
        nextline++;
        }
        if(azServoStatus[1]&128) {
	move(nextline,nextcol);
        addstr("Local mode");
        nextline++;
        }

  nextline=21;
  nextcol = nextcol+ 18;
  move(nextline,nextcol);

        if(elServoStatus[0]&1) {
	move(nextline,nextcol);
        addstr("Emergency limit");
        nextline++;
        }
        if(elServoStatus[0]&2) {
	move(nextline,nextcol);
        addstr("Operating limit ccw");
        nextline++;
        }
        if(elServoStatus[0]&4) {
	move(nextline,nextcol);
        addstr("Operating limit cw");
        nextline++;
        }
        if(elServoStatus[0]&8) {
	move(nextline,nextcol);
        addstr("Prelimit ccw");
        nextline++;
        }
        if(elServoStatus[0]&16) {
	move(nextline,nextcol);
        addstr("Prelimit cw");
        nextline++;
        }
        if(elServoStatus[0]&32) {
	move(nextline,nextcol);
        addstr("stow position");
        nextline++;
        }
        if(elServoStatus[0]&64) {
	move(nextline,nextcol);
        addstr("stow pin inserted");
        nextline++;
        }
        if(elServoStatus[0]&128) {
	move(nextline,nextcol);
        addstr("stow pin retracted");
        nextline++;
        }

        if(elServoStatus[1]&1) {
	move(nextline,nextcol);
        addstr("Servo failure");
        nextline++;
        }
        if(elServoStatus[1]&2) {
	move(nextline,nextcol);
        addstr("Brake failure");
        nextline++;
        }
        if(elServoStatus[1]&4) {
	move(nextline,nextcol);
        addstr("Encoder failure");
        nextline++;
        }
        if(elServoStatus[1]&8) {
	move(nextline,nextcol);
        addstr("Auxiliary mode ");
        nextline++;
        }
        if(elServoStatus[1]&16) {
	move(nextline,nextcol);
        addstr("Motion failure");
        nextline++;
        }
        if(elServoStatus[1]&32) {
	move(nextline,nextcol);
        addstr("CAN bus failure");
        nextline++;
        }
        if(elServoStatus[1]&64) {
	move(nextline,nextcol);
        addstr("Axis disabled");
        nextline++;
        }
        if(elServoStatus[1]&128) {
	move(nextline,nextcol);
        addstr("Local mode");
        nextline++;
        }

/* System Status */

  nextline=2;
  nextcol=49;
  move(nextline,nextcol);
  printLabel("System status:");
  nextline++;
  move(nextline,nextcol);
  printLabel("--------------");
  nextline++;
  move(nextline,nextcol);
  printLabel("ACU time:");
  move(nextline,nextcol+10);
  printw("%d", *acuDay);
  move(nextline,nextcol+15);
  printw("%02d:", acuH);
  move(nextline,nextcol+18);
  printw("%02d:", acuM);
  move(nextline,nextcol+21);
  printw("%04.2f", acuS);
  nextline++;
  move(nextline,nextcol);

  nextline++;
  if(acuSystemGS[1] & 128) {
	move(nextline,nextcol);
	printEnabled("REMOTE");
	nextline++;
	} else {
	move(nextline,nextcol);
	printDisabled("LOCAL");
	nextline++;
  }

  if(acuSystemGS[0] & 1) {
	move(nextline,nextcol);
        standout();
	printDisabled("Door interlock");
        standend();
	nextline++;
	}
  if(acuSystemGS[0] & 2) {
	move(nextline,nextcol);
        standout();
	printDisabled("SAFE");
        standend();
	nextline++;
	}
  if(acuSystemGS[0] & 64) {
	move(nextline,nextcol);
	printDisabled("Emergency Off");
	nextline++;
	}
  if(acuSystemGS[0] & 128) {
	move(nextline,nextcol);
        standout();
	printDisabled("Not on source");
        standend();
	nextline++;
	}
  if(acuSystemGS[1] & 4) {
	move(nextline,nextcol);
        standout();
	printDisabled("Time error");
        standend();
	nextline++;
	}
  if(acuSystemGS[1] & 8) {
	move(nextline,nextcol);
	printDisabled("Year error");
	nextline++;
	}
  if(acuSystemGS[1] & 32) {
	move(nextline,nextcol);
	printw("Green mode active");
	nextline++;
	}
  if(acuSystemGS[1] & 64) {
	move(nextline,nextcol);
	printw("Speed high");
	nextline++;
	}
  if(acuSystemGS[2] & 1) {
	move(nextline,nextcol);
	printw("Spline green");
	nextline++;
	}
  if(acuSystemGS[2] & 2) {
	move(nextline,nextcol);
	printw("Spline yellow");
	nextline++;
	}
  if(acuSystemGS[2] & 4) {
	move(nextline,nextcol);
	printw("Spline red");
	nextline++;
	}
  if(acuSystemGS[2] & 16) {
	move(nextline,nextcol);
	printw("Gearbox oil level warning");
	nextline++;
	}
  if(acuSystemGS[2] & 32) {
	move(nextline,nextcol);
	printw("PLC interface OK");
	nextline++;
	}
  if(acuSystemGS[3] & 1) {
	move(nextline,nextcol);
	printw("PCU mode");
	nextline++;
	}
  if(acuSystemGS[3] & 4) {
	move(nextline,nextcol);
	printw("Tiltmeter error ");
	nextline++;
	}
  if(acuSystemGS[4] & 1) {
	move(nextline,nextcol);
	printDisabled("Cabinet overtemperature");
	nextline++;
	}
  if(acuSystemGS[4] & 4) {
	move(nextline,nextcol);
	printEnabled("Shutter open");
	nextline++;
	}
  if(acuSystemGS[4] & 8) {
	move(nextline,nextcol);
	printDisabled("Shutter closed");
	nextline++;
	}
  if(acuSystemGS[4] & 16) {
	move(nextline,nextcol);
        attron(COLOR_PAIR(1));
	printw("Shutter failure");
        attroff(COLOR_PAIR(1));
	nextline++;
	}
  if(acuSystemGS[4] & 32) {
	move(nextline,nextcol);
	printDisabled("Shutter fan failure");
	nextline++;
	}
  if(acuSystemGS[4] & 64) {
	move(nextline,nextcol);
	printEnabled("Shutter fan on");
	nextline++;
	}

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

void boldOn() {
  if(!monoFlag) attron(A_BOLD);
}

void boldOff() {
  attroff(A_BOLD);
}


void highlight() {
  attron(A_STANDOUT);
}

void nohighlight() {
  attroff(A_STANDOUT);
}

void setLabel() {
  if(!monoFlag) attron(COLOR_PAIR(COLOR_LABEL));
}

void setBright() {
  if(!monoFlag) attron(COLOR_PAIR(lowColorFlag ? COLOR_NORMAL : COLOR_EMPHASIS));
}

void setFlashing() {
  attron(A_BLINK);
}


void setEnabled() {
  if(!monoFlag) {
    int a, c;
    attr_get(&a, &c, NULL);
    if(a & A_STANDOUT) attron(COLOR_PAIR(COLOR_NORMAL));
    else attron(COLOR_PAIR(lowColorFlag ? COLOR_NORMAL : COLOR_ENABLED));
  }
}

void setDisabled() {
  if(!monoFlag) {
    attron(COLOR_PAIR(lowColorFlag ? COLOR_NORMAL : COLOR_DISABLED));
  }
}

void setAlt() {
  if(!monoFlag) {
    int a, c;
    attr_get(&a, &c, NULL);
    if(a & A_STANDOUT) attron(COLOR_PAIR(COLOR_NORMAL));
    else attron(COLOR_PAIR(lowColorFlag ? COLOR_NORMAL : COLOR_VARIANT));
  }
}



void normalText() {
  int a, c;
  attr_get(&a, &c, NULL);

  if(!monoFlag) {
    attrset(A_NORMAL | (a & A_STANDOUT) | COLOR_PAIR(COLOR_NORMAL));
  }
  else {
    attrset(A_NORMAL | (a & A_STANDOUT));
  }
}


void printLabel(char *text) {
  setLabel();
  printw(text);
  normalText();
}

void printUnit(char *text) {
  int a, c;
  attr_get(&a, &c, NULL);


  setLabel();
  boldOff();

  attroff(A_STANDOUT);

  printw(text);
  normalText();

  if(a & A_STANDOUT) attron(A_STANDOUT);
}

void printHighlighted(char *text) {
  highlight();
  printw(text);
  nohighlight();
}

void printBold(char *text) {
  boldOn();
  printw(text);
  boldOff();
}

void printEnabled(char *text) {
  setEnabled();
  printw(text);
  normalText();
}

void printDisabled(char *text) {
  setDisabled();
  printw(text);
  normalText();
}

void printAlt(char *text) {
  setAlt();
  printw(text);
  normalText();
}


void printBright(char *text) {
  setBright();
  printw(text);
  normalText();
}

void printFlashing(char *text) {
  setBright();
  setFlashing();
  printw(text);
  normalText();
}

void printBooleanState(int value, char *enabledText, char *disabledText) {
  if(value) printEnabled(enabledText);
  else printDisabled(disabledText);
}

void printTriState(int value, char *enabledText, char *disabledText, char *wackoText) {
  if(value == 1) printEnabled(enabledText);
  else if(value == 0) printDisabled(disabledText);
  else printDisabled(wackoText);
}

void printTriValue(int value) {
  if(value == 0) printDisabled("0");
  else if(value == 1) printEnabled("1");
  else {
    setBright(); printw("%d", value); normalText();
  }
}


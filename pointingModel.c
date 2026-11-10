#define DEBUG 1
#include <stdio.h>
#include <termio.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <hiredis/hiredis.h>
#include <curses.h>
#define DSM_HOST "gltacc"
#define DSM_SUCCESS 0
#define MICRODEG_TO_ARCSEC 0.0036

/* The active pointing-model mode is published by gltTrack to redis (hash
   gltTrackComp) on gltacc's redis server, which is not the server gltmonitor
   uses elsewhere, so open a dedicated connection here. Fills type with
   optical/0/86/230/86+230/345, or "unknown" if redis is unreachable or the
   value is stale (> 30 s old). */
#define PM_REDIS_SERVER "192.168.1.11"
#define PM_REDIS_PORT 6379
#define PM_STALE_SEC 30
static redisContext *pmRedis=NULL;
static int pmLastTry=-1000;

static void getPointingModelType(int count, char *type, int len) {
  redisReply *r;
  strcpy(type,"unknown");
  if (pmRedis==NULL) {
    if (count-pmLastTry < 20) return;   /* don't stall the display retrying */
    pmLastTry=count;
    struct timeval tv={0,300000};
    pmRedis=redisConnectWithTimeout(PM_REDIS_SERVER,PM_REDIS_PORT,tv);
    if (pmRedis==NULL || pmRedis->err) {
      if (pmRedis) redisFree(pmRedis);
      pmRedis=NULL;
      return;
    }
    redisSetTimeout(pmRedis,tv);
  }
  r=redisCommand(pmRedis,"HMGET gltTrackComp pointingModelType pointingModelTime");
  if (r==NULL) { redisFree(pmRedis); pmRedis=NULL; return; }
  if (r->type==REDIS_REPLY_ARRAY && r->elements==2 &&
      r->element[0]->type==REDIS_REPLY_STRING &&
      r->element[1]->type==REDIS_REPLY_STRING) {
    long t=atol(r->element[1]->str);
    if (labs((long)time(NULL)-t) <= PM_STALE_SEC) {
      strncpy(type,r->element[0]->str,len-1);
      type[len-1]='\0';
    }
  }
  freeReplyObject(r);
}

void pointingModelPage(int count) {
  time_t timestamp,system_time;
  char timeString[26];
  float seconds;
  struct tm* systemTime;
  int dsm_status;

  double azdc,azcol,eltilt,aztilt_sin,aztilt_cos,aztilt_sin2,aztilt_cos2;
  double azenc_sin,azenc_cos,azenc_sin2,azenc_cos2,azenc_sin3,azenc_cos3;
  double azmodelrms,elmodelrms;
  double eldc,elsag,eaztilt_sin,eaztilt_cos,eaztilt_sin2,eaztilt_cos2;
  char modeldate[10];
  char pmType[10];
  char banner[40];

  if ((count % 20) == 1) {
    /*
      Initialize Curses Display
    */
    initscr();
    clear();
    move(1,1);
    refresh();
  }

  move(0,0);
  system_time=time(NULL);
  getPointingModelType(count,pmType,sizeof(pmType));
  if (!strcmp(pmType,"optical")) strcpy(banner,"OPTICAL (star pointing)");
  else if (!strcmp(pmType,"0")) strcpy(banner,"RADIO 0: ALL ZERO (holography)");
  else if (!strcmp(pmType,"86") || !strcmp(pmType,"230") || !strcmp(pmType,"345"))
    sprintf(banner,"RADIO %s GHz",pmType);
  else if (!strcmp(pmType,"86+230")) strcpy(banner,"RADIO 86+230 GHz (simultaneous)");
  else strcpy(banner,"UNKNOWN");
  move(9,15);
  printw("Pointing model mode: ");
  attron(A_REVERSE|A_BOLD);
  printw(" %-31s",banner);
  attroff(A_REVERSE|A_BOLD);
  move(11,15);
  printw("Current pointing model:");

  dsm_status = dsm_status=dsm_read(DSM_HOST,"DSM_AZDC_ARCSEC_D", &azdc,&timestamp);
  if (dsm_status != DSM_SUCCESS) {
  printf("Warning: DSM read failed! %d\n",dsm_status);
  }


        dsm_status=dsm_read(DSM_HOST,"DSM_AZCOLLIMATION_ARCSEC_D", &azcol,&timestamp);
        dsm_status=dsm_read(DSM_HOST,"DSM_AZELAXISTILT_ARCSEC_D", &eltilt,&timestamp);
        dsm_status=dsm_read(DSM_HOST,"DSM_AZAZTILTSIN_ARCSEC_D", &aztilt_sin,&timestamp);
        dsm_status=dsm_read(DSM_HOST,"DSM_AZAZTILTCOS_ARCSEC_D", &aztilt_cos,&timestamp);
        dsm_status=dsm_read(DSM_HOST,"DSM_AZAZTILTSIN2_ARCSEC_D", &aztilt_sin2,&timestamp);
        dsm_status=dsm_read(DSM_HOST,"DSM_AZAZTILTCOS2_ARCSEC_D", &aztilt_cos2,&timestamp);
        dsm_status=dsm_read(DSM_HOST,"DSM_AZENCSIN_ARCSEC_D", &azenc_sin,&timestamp);
        dsm_status=dsm_read(DSM_HOST,"DSM_AZENCCOS_ARCSEC_D", &azenc_cos,&timestamp);
        dsm_status=dsm_read(DSM_HOST,"DSM_AZENCSIN2_ARCSEC_D", &azenc_sin2,&timestamp);
        dsm_status=dsm_read(DSM_HOST,"DSM_AZENCCOS2_ARCSEC_D", &azenc_cos2,&timestamp);
        dsm_status=dsm_read(DSM_HOST,"DSM_AZENCSIN3_ARCSEC_D", &azenc_sin3,&timestamp);
        dsm_status=dsm_read(DSM_HOST,"DSM_AZENCCOS3_ARCSEC_D", &azenc_cos3,&timestamp);
        dsm_status=dsm_read(DSM_HOST,"DSM_AZRMS_ARCSEC_D", &azmodelrms,&timestamp);
        dsm_status=dsm_read(DSM_HOST,"DSM_ELDC_ARCSEC_D", &eldc,&timestamp);
        dsm_status=dsm_read(DSM_HOST,"DSM_ELSAG_ARCSEC_D", &elsag,&timestamp);
        dsm_status=dsm_read(DSM_HOST,"DSM_ELAZTILTSIN_ARCSEC_D", &eaztilt_sin,&timestamp);
        dsm_status=dsm_read(DSM_HOST,"DSM_ELAZTILTCOS_ARCSEC_D", &eaztilt_cos,&timestamp);
        dsm_status=dsm_read(DSM_HOST,"DSM_ELAZTILTSIN2_ARCSEC_D", &eaztilt_sin2,&timestamp);
        dsm_status=dsm_read(DSM_HOST,"DSM_ELAZTILTCOS2_ARCSEC_D", &eaztilt_cos2,&timestamp);
        dsm_status=dsm_read(DSM_HOST,"DSM_ELRMS_ARCSEC_D", &elmodelrms,&timestamp);
        dsm_status=dsm_read(DSM_HOST,"DSM_MODELDATE_C10",modeldate,&timestamp);



  move(12,15);
  printw("-------------------------");
  move(13,15);
printw("AzDC       %11.3f \"",azdc);
move(14,15);
printw("AzColl     %11.3f \"",azcol);   
move(15,15);
printw("ElTilt     %11.3f \"",eltilt); 
move(16,15);
printw("AAzTltSin  %11.3f \"",aztilt_sin);
move(17,15);
printw("AAzTltCos  %11.3f \"",aztilt_cos);
move(18,15);
printw("AAzTltSin2 %11.3f \"",aztilt_sin2);
move(19,15);
printw("AAzTltCos2 %11.3f \"",aztilt_cos2);
move(20,15);
printw("AzEncSin   %11.3f \"",azenc_sin); 
move(21,15);
printw("AzEncCos   %11.3f \"",azenc_cos); 
move(22,15);
printw("AzEncSin2  %11.3f \"",azenc_sin2);
move(23,15);
printw("AzEncCos2  %11.3f \"",azenc_cos2);
move(24,15);
printw("AzEncSin3  %11.3f \"",azenc_sin3);
move(25,15);
printw("AzEncCos3  %11.3f \"",azenc_cos3);
move(26,15);
printw("AzRms      %11.3f \"",azmodelrms);   
move(27,15);
printw("ElDC       %11.3f \"",eldc);   
move(28,15);
printw("ElSag      %11.3f \"",elsag);  
move(29,15);
printw("EAzTltSin  %11.3f \"",eaztilt_sin);
move(30,15);
printw("EAzTltCos  %11.3f \"",eaztilt_cos);
move(31,15);
printw("EAzTltSin2 %11.3f \"",eaztilt_sin2);
move(32,15);
printw("EAzTltCos2 %11.3f \"",eaztilt_cos2);
move(33,15);
printw("ElRms      %11.3f \"",elmodelrms);     
move(34,15);
printw("Date            %s",modeldate);    
move(35,15);
  printw("-------------------------");


  refresh();
}

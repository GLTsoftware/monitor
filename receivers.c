#define DEBUG 1
#include <stdio.h>
#include <termio.h>
#include <time.h>
#include <math.h>
#include <curses.h>
#define DSM_HOST "gltMandC"
#define DSM_SUCCESS 0
#define MICRODEG_TO_ARCSEC 0.0036

void receiversPage(int count) {
  time_t timestamp,system_time;
  char timeString[26];
  float seconds;
  struct tm* systemTime;
  int dsm_status;

  float power_4_9_caba4_lhc,power_4_9_caba4_rhc;
  float power_4_9_caba5_lhc,power_4_9_caba5_rhc;
  double rfSynthFreq=0.0,rfSynthPower=0.0,rfFinalLO=0.0;
  float rfCaba4VGAgain_lhc,rfCaba4VGAgain_rhc;
  float rfCaba5VGAgain_lhc,rfCaba5VGAgain_rhc;
  float rfCryostatColdhead,rfCryostatPlate4K;
  float rfCryostatPlate15K,rfCryostatPlate100;
  float rfCryostatAmbTemp;
  char c3String[3];
  char c1String[1];
  double volts;
  char pwrCaba4Timestamp[15];
  char pwrCaba5Timestamp[15];
  char rfCaba1Rcvr[3];
  char rfCaba1Toneval[1];
  char rfCaba1Tone[3];
  char rfCaba1Valon[1];
  char rfCaba1ValonLock[3];
  char rfCaba1FloogLock[3];
  char rfCaba11Rcvr[3];
  char rfCaba11Pol[3];
  char rfCaba11Sb[3];
  char rfCaba11Cont[3];
  char rfCaba11_595G[3];
  char rfCaba3AttLevel[3];
  char rfCaba4Rcvr[3];
  char rfCaba5Rcvr[3];
  char rfLoadval[10];
  char rfTimestamp[15];

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
  move(1,5);
  printw("Receivers data at time: %s ",ctime(&system_time));


  dsm_status = dsm_read(DSM_HOST,"DSM_POWER_4_9_CABA4_LHC_F",&power_4_9_caba4_lhc,&timestamp); 
  dsm_status = dsm_read(DSM_HOST,"DSM_POWER_4_9_CABA4_RHC_F",&power_4_9_caba4_rhc,&timestamp); 
  dsm_status = dsm_read(DSM_HOST,"DSM_POWER_4_9_CABA5_LHC_F",&power_4_9_caba5_rhc,&timestamp);       
  dsm_status = dsm_read(DSM_HOST,"DSM_POWER_4_9_CABA5_RHC_F",&power_4_9_caba5_rhc,&timestamp);      
  dsm_status = dsm_read(DSM_HOST,"DSM_PWR_CABA4_TIMESTAMP_C15",pwrCaba4Timestamp,&timestamp);
  dsm_status = dsm_read(DSM_HOST,"DSM_PWR_CABA5_TIMESTAMP_C15",pwrCaba5Timestamp,&timestamp);
  dsm_status = dsm_read(DSM_HOST,"DSM_RF_SYNTH_FREQ_D",&rfSynthFreq,&timestamp);
  dsm_status = dsm_read(DSM_HOST,"DSM_RF_SYNTH_POWER_D",&rfSynthPower,&timestamp);
  dsm_status = dsm_read(DSM_HOST,"DSM_RF_FINAL_LO_D",&rfFinalLO,&timestamp);
  dsm_status = dsm_read(DSM_HOST,"DSM_RF_CABA1_RECEIVER_C3",rfCaba1Rcvr,&timestamp);
  dsm_status = dsm_read(DSM_HOST,"DSM_RF_CABA1_TONEVAL_C1",rfCaba1Toneval,&timestamp);
  dsm_status = dsm_read(DSM_HOST,"DSM_RF_CABA1_TONE_C3",rfCaba1Tone,&timestamp);
  dsm_status = dsm_read(DSM_HOST,"DSM_RF_CABA1_VALON_C1",rfCaba1Valon,&timestamp);
  dsm_status = dsm_read(DSM_HOST,"DSM_RF_CABA1_VALON_LOCK_C3",rfCaba1ValonLock,&timestamp);
  dsm_status = dsm_read(DSM_HOST,"DSM_RF_CABA1_FLOOG_LOCK_C3",rfCaba1FloogLock,&timestamp);

  dsm_status = dsm_read(DSM_HOST,"DSM_RF_CABA11_RECEIVER_C3",rfCaba11Rcvr,&timestamp);
  dsm_status = dsm_read(DSM_HOST,"DSM_RF_CABA11_POL_C3",rfCaba11Pol,&timestamp);
  dsm_status = dsm_read(DSM_HOST,"DSM_RF_CABA11_SB_C3",rfCaba11Sb,&timestamp);
  dsm_status = dsm_read(DSM_HOST,"DSM_RF_CABA11_CONT_C3",rfCaba11Cont,&timestamp);
  dsm_status = dsm_read(DSM_HOST,"DSM_RF_CABA11_595G_C3",rfCaba11_595G,&timestamp);

  dsm_status = dsm_read(DSM_HOST,"DSM_RF_CABA3_ATTLEVEL_C3",rfCaba3AttLevel,&timestamp);

  dsm_status = dsm_read(DSM_HOST,"DSM_RF_CABA4_RECEIVER_C3",rfCaba4Rcvr,&timestamp);
  dsm_status = dsm_read(DSM_HOST,"DSM_RF_CABA4_VGAGAIN_LHC_F",&rfCaba4VGAgain_lhc,&timestamp);
  dsm_status = dsm_read(DSM_HOST,"DSM_RF_CABA4_VGAGAIN_RHC_F",&rfCaba4VGAgain_rhc,&timestamp);
  dsm_status = dsm_read(DSM_HOST,"DSM_RF_CABA5_RECEIVER_C3",rfCaba5Rcvr,&timestamp);
  dsm_status = dsm_read(DSM_HOST,"DSM_RF_CABA5_VGAGAIN_LHC_F",&rfCaba5VGAgain_lhc,&timestamp);
  dsm_status = dsm_read(DSM_HOST,"DSM_RF_CABA5_VGAGAIN_RHC_F",&rfCaba5VGAgain_rhc,&timestamp);
  dsm_status = dsm_read(DSM_HOST,"DSM_RF_CRYOSTAT_COLDHEAD_F",&rfCryostatColdhead,&timestamp);
  dsm_status = dsm_read(DSM_HOST,"DSM_RF_CRYOSTAT_PLATE4K_F",&rfCryostatPlate4K,&timestamp);
  dsm_status = dsm_read(DSM_HOST,"DSM_RF_CRYOSTAT_PLATE15K_F",&rfCryostatPlate15K,&timestamp);
  dsm_status = dsm_read(DSM_HOST,"DSM_RF_CRYOSTAT_PLATE100_F",&rfCryostatPlate100,&timestamp);
  dsm_status = dsm_read(DSM_HOST,"DSM_RF_CRYOSTAT_AMBIENT_F",&rfCryostatAmbTemp,&timestamp);

  dsm_status = dsm_read(DSM_HOST,"DSM_RF_LOADVAL_C10",&rfLoadval,&timestamp);

  dsm_status = dsm_read(DSM_HOST,"DSM_RF_TIMESTAMP_C15",rfTimestamp,&timestamp);

  if (dsm_status != DSM_SUCCESS) {
  printf("Warning: DSM read failed! dsm_status=%d\n",dsm_status);
  }

  move(3,48);
  printw("--------- Cryostat -----------");
  move(4,48);
  printw("| Coldhead temp.:     %4.1f K |",rfCryostatColdhead);
  move(5,48);
  printw("| 4K Plate temp.:     %4.1f K |",rfCryostatPlate4K);
  move(6,48);
  printw("| 15K Plate temp.:    %4.1f K |",rfCryostatPlate15K);
  move(7,48);
  printw("| 100K Plate temp.:  %4.1f K |",rfCryostatPlate100);
  move(8,48);
  printw("| Ambient temp.:     %4.1f K |",rfCryostatAmbTemp);
  move(9,48);
  printw("------------------------------");

  move(3,2);
  printw("--------- Tuning -----------------------");
  move(4,2);
  printw("| Synth. Freq. (Hz):    %12.2f |",rfSynthFreq);
  move(5,2);
  printw("| Synth. Power (dBm):   %12.6f   |",rfSynthPower);
  move(6,2);
  printw("| Final LO Freq. (GHz):  %12.6f  |",rfFinalLO);
  move(7,2);
  printw("----------------------------------------");

  move(10,2);
  printw("---CAB-A1---");
  move(11,2);
  printw("Rcvr: %s",rfCaba1Rcvr);
  move(12,2);
  printw("Tone val: %s",rfCaba1Toneval);
/*
  switch(atoi(rfCaba1Toneval)) {
     case '0':
     printw("Tone: OFF");
     break;
     case '1':
     printw("Tone: ON");
     break;
  }
*/
  move(13,2);
  printw("Tone channel: %s",rfCaba1Tone);
  move(14,2);
  printw("Valon Freq: %s",rfCaba1Valon);
/*
  switch(atoi(rfCaba1Valon)) {
     case '0':
     printw("Valon Freq: 0.5GHz");
     break;
     case '1':
     printw("Valon Freq: 1.5GHz");
     break;
  }
*/
  move(15,2);
  printw("Valon Lock V: %s (V)",rfCaba1ValonLock);
/*
  if(volts < 0.5) {
       standout();
       printw(" UNLOCKED");
       standend();
  }
  if(volts > 3.0) printw(" LOCKED");
*/
  move(16,2);
  printw("(0V: no Lock, 3V: Lock)");
  move(18,2);
  printw("Floog Lock V: %s (V)",rfCaba1FloogLock);
/*
  if(atof(rfCaba1FloogLock) < 0.5) {
        standout();
        printw(" UNLOCKED");
        standend();
  }
  if(atof(rfCaba1FloogLock) > 4.5) printw(" LOCKED");
*/
  move(19,2);
  printw("(0V: no Lock, 5V: Lock)");
  move(20,2);
  printw("----------");
  
  move(10,28);
  printw("---CAB-A3---");
  move(11,28);
  printw("Attenuation (dB): %s",rfCaba3AttLevel);

  move(24,2);
  printw("---CAB-A4---");
  move(25,2);
  printw("Rcvr: %s",rfCaba4Rcvr);
  move(26,2);
  if(abs(power_4_9_caba4_lhc)>1.0e5) {printw("4-9 GHz IF Power LHC (dBm): wacko");}
  else printw("4-9 GHz IF Power LHC (dBm): %.2f",power_4_9_caba4_lhc);
  move(27,2);
  if(abs(power_4_9_caba4_rhc)>1.0e5) {printw("4-9 GHz IF Power RHC (dBm): wacko");}
  else printw("4-9 GHz IF Power RHC (dBm): %.2f",power_4_9_caba4_rhc);
  move(28,2);
  if(abs(rfCaba4VGAgain_lhc)>1.0e5) {printw("VGA gain LHC (dB): wacko");}
  else printw("VGA gain LHC (dB): %f",rfCaba4VGAgain_lhc);
  move(29,2);
  if(abs(rfCaba4VGAgain_rhc)>1.0e5) {printw("VGA gain RHC (dB): wacko");}
  else printw("VGA gain RHC (dB): %f",rfCaba4VGAgain_rhc);

  move(24,40);
  printw("---CAB-A5---");
  move(25,40);
  printw("Rcvr: %s",rfCaba5Rcvr);
  move(26,40);
  if(abs(power_4_9_caba5_lhc)>1.0e5) {printw("4-9 GHz IF Power LHC (dBm): wacko");}
  else printw("4-9 GHz IF Power LHC (dBm): %.2f",power_4_9_caba5_lhc);
  move(27,40);
  if(abs(power_4_9_caba5_rhc)>1.0e5) {printw("4-9 GHz IF Power RHC (dBm): wacko");}
  else printw("4-9 GHz IF Power RHC (dBm): %.2f",power_4_9_caba5_rhc);
  move(28,40);
  if(abs(rfCaba5VGAgain_lhc)>1.0e5) {printw("VGA gain LHC (dB): wacko");}
  else printw("VGA gain LHC (dB): %f",rfCaba5VGAgain_lhc);
  move(29,40);
  if(abs(rfCaba5VGAgain_rhc)>1.0e5) {printw("VGA gain RHC (dB): wacko");}
  else printw("VGA gain RHC (dB): %f",rfCaba5VGAgain_rhc);
 
  move(15,55);
  printw("Position of Load Motor:");
  move(16,60);
  printw("%s",rfLoadval);

  refresh();
}

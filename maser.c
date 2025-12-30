#define DEBUG 1
#include <stdio.h>
#include <termio.h>
#include <time.h>
#include <math.h>
#include <curses.h>
#define DSM_HOST "gltMandC"
#define DSM_SUCCESS 0
#define MICRODEG_TO_ARCSEC 0.0036

void maserPage(int count) {
  time_t timestamp,system_time;
  char timeString[26];
  float seconds;
  struct tm* systemTime;
  int dsm_status;

  float UBattA,IBattA,UBattB,IBattB,setH,measH,Ipur,IDiss,Hlight;
  float ITheater,IBheater,ISheater,UTCheater,ESheater,EBheater,Iheater;
  float Theater,BoxesTemp,IBoxes,AmbTemp,CField,UVaracter;
  float UHText,IHText,UHTint,IHTint,HSTPres,HSTheat;
  float PiraniHeat,UOCXO100MHz,U405KHz,UOCXO,V24dc;
  float V15dc,Minus15Vdc,Minus5Vdc,V5dc,V8dc;
  char Lock100MHz[1],Lock[1];
  double DDS;

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
  printw("Hydrogen Maser data time: %s ",ctime(&system_time));


  dsm_status = dsm_read(DSM_HOST,"DSM_MASER_UBATTA_F",&UBattA,&timestamp); 
  dsm_status = dsm_read(DSM_HOST,"DSM_MASER_IBATTA_F",&IBattA,&timestamp);
  dsm_status = dsm_read(DSM_HOST,"DSM_MASER_UBATTB_F",&UBattB,&timestamp);
  dsm_status = dsm_read(DSM_HOST,"DSM_MASER_IBATTB_F",&IBattB,&timestamp);

  dsm_status = dsm_read(DSM_HOST,"DSM_MASER_SETH_F",&setH,&timestamp);
  dsm_status = dsm_read(DSM_HOST,"DSM_MASER_MEASH_F",&measH,&timestamp);

  dsm_status = dsm_read(DSM_HOST,"DSM_MASER_IPUR_F",&Ipur,&timestamp);
  dsm_status = dsm_read(DSM_HOST,"DSM_MASER_IDISS_F",&IDiss,&timestamp);
  dsm_status = dsm_read(DSM_HOST,"DSM_MASER_HLIGHT_F",&Hlight,&timestamp);

  dsm_status = dsm_read(DSM_HOST,"DSM_MASER_ITHEATER_F",&ITheater,&timestamp);
  dsm_status = dsm_read(DSM_HOST,"DSM_MASER_IBHEATER_F",&IBheater,&timestamp);
  dsm_status = dsm_read(DSM_HOST,"DSM_MASER_ISHEATER_F",&ISheater,&timestamp);
  dsm_status = dsm_read(DSM_HOST,"DSM_MASER_UTCHEATER_F",&UTCheater,&timestamp);
  dsm_status = dsm_read(DSM_HOST,"DSM_MASER_ESHEATER_F",&ESheater,&timestamp);
  dsm_status = dsm_read(DSM_HOST,"DSM_MASER_EBHEATER_F",&EBheater,&timestamp);
  dsm_status = dsm_read(DSM_HOST,"DSM_MASER_IHEATER_F",&Iheater,&timestamp);
  dsm_status = dsm_read(DSM_HOST,"DSM_MASER_THEATER_F",&Theater,&timestamp);


  dsm_status = dsm_read(DSM_HOST,"DSM_MASER_BOXESTEMP_F",&BoxesTemp,&timestamp);
  dsm_status = dsm_read(DSM_HOST,"DSM_MASER_IBOXES_F",&IBoxes,&timestamp);
  dsm_status = dsm_read(DSM_HOST,"DSM_MASER_AMBTEMP_F",&AmbTemp,&timestamp);

  dsm_status = dsm_read(DSM_HOST,"DSM_MASER_CFIELD_F",&CField,&timestamp);

  dsm_status = dsm_read(DSM_HOST,"DSM_MASER_UVARACTOR_F",&UVaracter,&timestamp);

  dsm_status = dsm_read(DSM_HOST,"DSM_MASER_UXTEXT_F",&UHText,&timestamp);
  dsm_status = dsm_read(DSM_HOST,"DSM_MASER_IHTEXT_F",&IHText,&timestamp);
  dsm_status = dsm_read(DSM_HOST,"DSM_MASER_UHTINT_F",&UHTint,&timestamp);
  dsm_status = dsm_read(DSM_HOST,"DSM_MASER_IHTINT_F",&IHTint,&timestamp);

  dsm_status = dsm_read(DSM_HOST,"DSM_MASER_HSTPRES_F",&HSTPres,&timestamp);
  dsm_status = dsm_read(DSM_HOST,"DSM_MASER_HSTHEAT_F",&HSTheat,&timestamp);

  dsm_status = dsm_read(DSM_HOST,"DSM_MASER_PIRANIHEAT_F",&PiraniHeat,&timestamp);

  dsm_status = dsm_read(DSM_HOST,"DSM_MASER_UOCXO100MHZ_F",&UOCXO100MHz,&timestamp);
  dsm_status = dsm_read(DSM_HOST,"DSM_MASER_U405KHZ_F",&U405KHz,&timestamp);
  dsm_status = dsm_read(DSM_HOST,"DSM_MASER_UOCXO_F",&UOCXO,&timestamp);

  dsm_status = dsm_read(DSM_HOST,"DSM_MASER_24VDC_F",&V24dc,&timestamp);
  dsm_status = dsm_read(DSM_HOST,"DSM_MASER_15VDC_F",&V15dc,&timestamp);
  dsm_status = dsm_read(DSM_HOST,"DSM_MASER_MINUS15VDC_F",&Minus15Vdc,&timestamp);
  dsm_status = dsm_read(DSM_HOST,"DSM_MASER_5VDC_F",&V5dc,&timestamp);
  dsm_status = dsm_read(DSM_HOST,"DSM_MASER_MINUS5VDC_F",&Minus5Vdc,&timestamp);
  dsm_status = dsm_read(DSM_HOST,"DSM_MASER_8VDC_F",&V8dc,&timestamp);

  dsm_status = dsm_read(DSM_HOST,"DSM_MASER_LOCK100MHZ_C1",Lock100MHz,&timestamp);
  dsm_status = dsm_read(DSM_HOST,"DSM_MASER_LOCK_C1",Lock,&timestamp);
  dsm_status = dsm_read(DSM_HOST,"DSM_MASER_DDS_D",&DDS,&timestamp);


  if (dsm_status != DSM_SUCCESS) {
  printf("Warning: DSM read failed! dsm_status=%d\n",dsm_status);
  }

  move(3,2);
  printw("---------Battery -----------");
  move(3,45);
  printw("Optimum");
  move(3,55);
  printw("Green");
  move(3,65);
  printw("Red");
  move(4,2);
  printw("| Battery voltage A/B :  %.2f/%.2f V", UBattA,UBattB);
  move(4,45); printw("[26,28]"); 
  move(4,55); printw("[22,30]"); 
  move(4,65); printw("[<18,>30]");
  move(5,2);
  printw("| Battery current A/B :  %.2f/%.2f A",IBattA,IBattB);
  move(5,45); printw("[2.5,3.5]");
  move(5,55); printw("[1.5,4.0]");
  move(5,65); printw("[<1.0,>4.5]");
  move(7,2);
  printw("--------Hydrogen pressure------");
  move(8,2);
  printw("| Pressure setting :      %.2f V", setH);
  move(8,45); printw("[4.5,6.0]");
  move(8,55); printw("[2.0,7.5]");
  move(8,65); printw("[<1.0,>8.0]");
  move(9,2);
  printw("| Pressure measurement :  %.2f V", measH);
  move(9,45); printw("[1.0,2.5]");
  move(9,55); printw("[0.5,4.0]");
  move(9,65); printw("[<0.2,>5.0]");
  move(11,2);
  printw("-------------------------------");
  move(12,2);
  printw("| Purifier current:       %.2f A", Ipur);
  move(12,45); printw("[0.5,0.8]");
  move(12,55); printw("[0.3,0.9]");
  move(12,65); printw("[<0.2,>1.0]");
  move(13,2);
  printw("| Dissociator current:    %.2f A", IDiss);
  move(13,45); printw("[0.1,0.6]");
  move(13,55); printw("[0.5,4.0]");
  move(13,65); printw("[<0.05,>0.7]");
  move(14,2);
  printw("| Dissociator light:      %.2f V", Hlight);
  move(14,45); printw("[1.5,4.5]");
  move(14,55); printw("[1.0,5.0]");
  move(14,65); printw("[<0.5,>5.0]");
  move(16,2);
  printw("--------Heaters----------------");
  move(17,2);
  printw("| Internal top:           %.2f V", ITheater);
  move(17,45); printw("[0.5,2.0]");
  move(17,55); printw("[0.2,4.0]");
  move(17,65); printw("[<0.1,>4.5]");
  move(18,2);
  printw("| Internal bottom:        %.2f V", IBheater);
  move(19,2);
  printw("| Internal side:          %.2f V", ISheater);
  move(20,2);
  printw("| Thermal ctrl unit :     %.2f V", UTCheater);
  move(21,2);
  printw("| External side:          %.2f V", ESheater);
  move(22,2);
  printw("| External bottom:        %.2f V", EBheater);
  move(23,2);
  printw("| Isolator :              %.2f V", Iheater);
  move(24,2);
  printw("| Tube :                  %.2f V", Theater);
  move(26,2);
  printw("-------------------------------");
  move(27,2);
  printw("| Boxes temp. :           %.2f C", BoxesTemp);
  move(27,45); printw("[39,50]");
  move(27,55); printw("[35,60]");
  move(27,65); printw("[<30,>65]");
  move(28,2);
  printw("| Boxes current :         %.2f A", IBoxes);
  move(28,45); printw("[0.1,0.6]");
  move(28,55); printw("[0.05,0.7]");
  move(28,65); printw("[<0.02,>0.8]");
  move(29,2);
  printw("| Ambient temp. :         %.2f C", AmbTemp);
  move(29,45); printw("[21,26]");
  move(29,55); printw("[21,29]");
  move(29,65); printw("[<0,>40]");
  move(31,2);
  printw("-------------------------------");
  move(32,2);
  printw("| C-field voltage :       %.2f V", CField);
  move(32,45); printw("not used");
  move(33,2);
  printw("| Varactor voltage :      %.2f V", UVaracter);
  move(33,45); printw("[1.5,8.0]");
  move(33,55); printw("[0.5,9.5]");
  move(33,65); printw("[<0,>10]");
  move(34,2);
  printw("| External high voltage:  %.2f V", UHText);
  move(34,45); printw("[3.0,4.0]");
  move(34,55); printw("[2.5,4.0]");
  move(34,65); printw("[<1.0,>5.0]");
  move(35,2);
  printw("| External high current:  %.2f A", IHText);
  move(35,45); printw("[1,50]");
  move(35,55); printw("[1,90]");
  move(35,65); printw("[<0,>150]");
  move(36,2);
  printw("| Internal high voltage:  %.2f V", UHTint);
  move(36,45); printw("[3,4]");
  move(36,55); printw("[2.5,4.0]");
  move(36,65); printw("[<1,>5]");
  move(37,2);
  printw("| Internal high current:  %.2f A", IHTint);
  move(37,45); printw("[1,50]");
  move(37,55); printw("[1,90]");
  move(37,65); printw("[<0,>150]");
  move(39,2);
  printw("-------------------------------");
  move(40,2);
  printw("| Hydrogen storage pres.: %.2f bar",HSTPres);
  move(40,45); printw("[0.9,3.0]");
  move(40,55); printw("[0.8,3.0]");
  move(40,65); printw("[<0.75,>3.1]");
  move(41,2);
  printw("| Hydrogen storage heat.: %.2f V",HSTheat);
  move(41,45); printw("[5,15]");
  move(41,55); printw("[1,19]");
  move(41,65); printw("[<0.5,>20]");
  move(42,2);
  printw("| Pirani heater:          %.2f V",PiraniHeat);
  move(42,45); printw("[5,15]");
  move(42,55); printw("[1,19]");
  move(42,65); printw("[<0.5,>20]");
  move(44,2);
  printw("-------------------------------");
  move(45,2);
  printw("| UOCXO 100 MHz :         %.2f V",UOCXO100MHz);
  move(45,45); printw("[1,5]");
  move(45,55); printw("[1,5]");
  move(45,65); printw("[<0.5,>5.5]");
  move(46,2);
  printw("| 405 kHz Amplitude :     %.2f V",U405KHz);
  move(46,45); printw("[6,12]");
  move(46,55); printw("[5,12.5]");
  move(46,65); printw("[<1.0,>13.5]");
  move(47,2);
  printw("| OCXO Varicap volt.:     %.2f V",UOCXO);
  move(47,45); printw("[2,8]");
  move(47,55); printw("[1,9]");
  move(47,65); printw("[<0.1,>9.9]");
  move(49,2);
  printw("------Power Supplies-----------");
  move(50,2);
  printw("| +24V                    %.2f V",V24dc);
  move(50,45); printw("[23,25]");
  move(50,55); printw("[23.5,25.5]");
  move(50,65); printw("[<22,>27]");
  move(51,2);
  printw("| +15                     %.2f V",V15dc);
  move(51,45); printw("[14,16]");
  move(51,55); printw("[13.5,16.5]");
  move(51,65); printw("[<12,>18]");
  move(52,2);
  printw("| -15V                    %.2f V",Minus15Vdc);
  move(52,45); printw("[-16,-14]");
  move(52,55); printw("[-16.5,-13.5]");
  move(52,65); printw("[<-18,>-12]");
  move(53,2);
  printw("| +5V                     %.2f V",V5dc);
  move(53,45); printw("[4.8,5.2]");
  move(53,55); printw("[4.5,5.5]");
  move(53,65); printw("[<3,>7]");
  move(54,2);
  printw("| -5V                     %.2f V",Minus5Vdc);
  move(54,45); printw("N/A");
  move(54,55); printw("N/A");
  move(54,65); printw("N/A");
  move(55,2);
  printw("| +8V                     %.2f V",V8dc);
  move(55,45); printw("[7.5,8.2]");
  move(55,55); printw("[7.4,8.5]");
  move(47,65); printw("[<6,>10]");

  move(18,45);
  printw("-------------------------------");
  move(20,45);
  printw("| LOCK 100 MHz:   %s",Lock100MHz);
  move(21,45);
  printw("| Main PLL LOCK : %s",Lock);
  move(22,45);
  printw("| DDS (Hz) :  %lf",DDS);
  move(24,45);
  printw("-------------------------------");
  dsm_status = dsm_read(DSM_HOST,"DSM_MASER_LOCK100MHZ_C1",Lock100MHz,&timestamp);
  dsm_status = dsm_read(DSM_HOST,"DSM_MASER_LOCK_C1",Lock,&timestamp);
  dsm_status = dsm_read(DSM_HOST,"DSM_MASER_DDS_D",&DDS,&timestamp);



/*
  move(16,2);
  printw("(0V: no Lock, 3V: Lock)");
  move(18,2);
  printw("Floog Lock V: %s (V)",rfCaba1FloogLock);
  if(atof(rfCaba1FloogLock) < 0.5) {
        standout();
        printw(" UNLOCKED");
        standend();
  }
  if(atof(rfCaba1FloogLock) > 4.5) printw(" LOCKED");
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
*/

  refresh();
}

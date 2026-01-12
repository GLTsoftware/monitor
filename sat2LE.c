#define DEBUG 1
#include <stdio.h>
#include <termio.h>
#include <time.h>
#include <math.h>
#include <curses.h>
#include <hiredis.h>
#define DSM_HOST "gltacc"
#define DSM_SUCCESS 0

#define REDIS_SERVER "192.168.1.141"
#define REDIS_PORT 6379

extern char redisData[1024];
extern redisContext *redisC;
extern redisReply *redisResp;

void sat2LEpage(int count) {
  time_t timestamp,system_time;
  char timeString[26];
  float seconds;
  struct tm* systemTime;
  int dsm_status;
  char *stringVar;

  char line0[256] = {0}; // Buffer for Line 0
  char line1[256] = {0}; // Buffer for Line 1
  char line2[256] = {0}; // Buffer for Line 2


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
  move(11,11);
  printw("Currently loaded satellite Two Line data in ACU:");
   redisReadString("gltTrackComp", "sat2LEline0", line0, sizeof(line0));
   move(13,11);
   printw("-----------------------------------------------------------------------");
   move(14,11);
   printw("%s",line0);
   move(15,11);
   redisReadString("gltTrackComp", "sat2LEline1", line1, sizeof(line1));
   printw("%s",line1);
   move(16,11);
   redisReadString("gltTrackComp", "sat2LEline2", line2, sizeof(line2));
   printw("%s",line2);
   move(17,11);
   printw("-----------------------------------------------------------------------");
  refresh();
}

void redisReadString(char *hashName, char *fieldName, char *outputBuffer, size_t bufferSize) {

    // Execute HGET command
    redisResp = redisCommand(redisC, "HGET %s %s", hashName, fieldName);
    if (redisResp == NULL) {
        fprintf(stderr, "Redis command error: %s\n", redisC->errstr);
        snprintf(outputBuffer, bufferSize, "ERROR");
        redisFree(redisC);
        return;
    }

    if (redisResp->type == REDIS_REPLY_STRING) {
        // Copy the string to the output buffer
        strncpy(outputBuffer, redisResp->str, bufferSize - 1);
        outputBuffer[bufferSize - 1] = '\0'; // Ensure null termination
    } else {
        snprintf(outputBuffer, bufferSize, "ERROR: Unexpected response type");
    }

    // Free reply and connection
    freeReplyObject(redisResp);
}

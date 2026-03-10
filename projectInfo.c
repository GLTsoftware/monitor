/* projectInfo.c - Project information page for gltmonitor
 *
 * Displays the current observing project info fetched from Redis.
 * Fields match those written by the project/endProject commands.
 *
 * NAP, March 2026
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <curses.h>
#include <hiredis/hiredis.h>

/* Redis keys (must match project.c / endProject.c) */
#define RKEY_PROJECT_PI          "glt:project:pi"
#define RKEY_PROJECT_OBSERVER    "glt:project:observer"
#define RKEY_PROJECT_LOCATION    "glt:project:location"
#define RKEY_PROJECT_DESCRIPTION "glt:project:description"
#define RKEY_PROJECT_TYPE        "glt:project:type"
#define RKEY_PROJECT_RECEIVER    "glt:project:receiver"
#define RKEY_PROJECT_COMMENT     "glt:project:comment"
#define RKEY_PROJECT_STATUS      "glt:project:status"
#define RKEY_PROJECT_CODE        "glt:project:code"
#define RKEY_PROJECT_TIMESTAMP   "glt:project:timestamp"

/* Shared globals defined in gltmonitor.c */
extern redisContext *redisC;

/* Helper: GET a redis key, copy into buf (empty string if not found) */
static void redis_get_str(redisContext *c, const char *key,
                          char *buf, size_t buflen) {
    redisReply *reply = redisCommand(c, "GET %s", key);
    if (reply != NULL && reply->type == REDIS_REPLY_STRING && reply->str)
        strncpy(buf, reply->str, buflen - 1);
    else
        buf[0] = '\0';
    buf[buflen - 1] = '\0';
    if (reply) freeReplyObject(reply);
}

void projectInfoPage(int count) {
    time_t system_time;
    char tbuf[32];
    int row;

    char pi[64], observer[64], location[128], description[512];
    char type[32], comment[256], code[32], timestamp[32];
    char status_str[16], receiver_str[16];
    int status, receiver;
    const char *status_text;

    /* Periodic full redraw */
    if ((count % 20) == 1) {
        clear();
        refresh();
    }

    system_time = time(NULL);
    strncpy(tbuf, ctime(&system_time), sizeof(tbuf) - 1);
    tbuf[sizeof(tbuf) - 1] = '\0';
    {
        int tlen = (int)strlen(tbuf);
        if (tlen > 0 && tbuf[tlen - 1] == '\n')
            tbuf[tlen - 1] = '\0';
    }

    /* Fetch all project fields from Redis */
    redis_get_str(redisC, RKEY_PROJECT_PI, pi, sizeof(pi));
    redis_get_str(redisC, RKEY_PROJECT_OBSERVER, observer, sizeof(observer));
    redis_get_str(redisC, RKEY_PROJECT_LOCATION, location, sizeof(location));
    redis_get_str(redisC, RKEY_PROJECT_DESCRIPTION, description, sizeof(description));
    redis_get_str(redisC, RKEY_PROJECT_TYPE, type, sizeof(type));
    redis_get_str(redisC, RKEY_PROJECT_RECEIVER, receiver_str, sizeof(receiver_str));
    redis_get_str(redisC, RKEY_PROJECT_COMMENT, comment, sizeof(comment));
    redis_get_str(redisC, RKEY_PROJECT_STATUS, status_str, sizeof(status_str));
    redis_get_str(redisC, RKEY_PROJECT_CODE, code, sizeof(code));
    redis_get_str(redisC, RKEY_PROJECT_TIMESTAMP, timestamp, sizeof(timestamp));

    status = atoi(status_str);
    receiver = atoi(receiver_str);

    if (status == 1)       status_text = "ACTIVE";
    else if (status == -1) status_text = "LOCKOUT";
    else                   status_text = "idle";

    /* ---- Title row ---- */
    move(0, 2);
    clrtoeol();
    printw("GLT Project Information                         %s", tbuf);

    /* ---- Navigation hint ---- */
    move(2, 2);
    clrtoeol();
    printw("[a] antenna page   [2] 2op messages   [h] help");

    /* ---- Top separator ---- */
    move(3, 0);
    clrtoeol();
    printw("  --------------------------------------------------------------------------");

    /* ---- Project fields ---- */
    row = 5;

    move(row, 4);   clrtoeol();
    printw("Project Code : %s", code);
    row++;

    move(row, 4);   clrtoeol();
    printw("Status       : %s", status_text);
    row++;

    move(row, 4);   clrtoeol();
    printw("Timestamp    : %s", timestamp);
    row += 2;

    move(row, 4);   clrtoeol();
    printw("PI           : %s", pi);
    row++;

    move(row, 4);   clrtoeol();
    printw("Observer     : %s", observer);
    row++;

    move(row, 4);   clrtoeol();
    printw("Location     : %s", location);
    row++;

    move(row, 4);   clrtoeol();
    printw("Project Type : %s", type);
    row++;

    move(row, 4);   clrtoeol();
    if (receiver > 0)
        printw("Receiver     : %d GHz", receiver);
    else
        printw("Receiver     :");
    row++;

    move(row, 4);   clrtoeol();
    printw("Comment      : %s", comment);
    row += 2;

    move(row, 4);   clrtoeol();
    printw("Description  : %s", description);
    row++;

    /* ---- Bottom separator ---- */
    row++;
    move(row, 0);
    clrtoeol();
    printw("  --------------------------------------------------------------------------");

    refresh();
}

/* op2msg.c - 2op messages log page for gltmonitor
 *
 * Displays up to N_MSGS_ON_PAGE 2op messages fetched from the Redis
 * list "2opmsg".  Messages are shown in chronological order: oldest
 * at the top, newest at the bottom.
 *
 * Scrolling:
 *   '-'  go back in time  (see older messages)
 *   '+'  go forward in time (see newer messages, back toward present)
 *
 * The global twoOpScrollOffset is owned by gltmonitor.c and adjusted
 * by the main key-dispatch loop.  Offset 0 shows the most recent
 * N_MSGS_ON_PAGE messages; each increment moves one step toward older
 * messages.
 *
 * NAP Feb 2026.
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <curses.h>
#include <hiredis/hiredis.h>

#define N_MSGS_ON_PAGE 12

/* Shared globals defined in gltmonitor.c */
extern redisContext *redisC;
extern int twoOpScrollOffset;

void twoOpPage(int count) {
    time_t system_time;
    int i, row;
    int numFetched = 0;
    long long totalMsgs = 0;
    redisReply *reply;
    char tbuf[32];

    /* Re-initialise the curses screen when the page is first entered
     * (count==1) or periodically (every 20 cycles) to repair any
     * display corruption. */
    if ((count % 20) == 1) {
        initscr();
        clear();
        move(1, 1);
        refresh();
    }

    system_time = time(NULL);

    /* Strip the trailing newline that ctime() appends */
    strncpy(tbuf, ctime(&system_time), sizeof(tbuf) - 1);
    tbuf[sizeof(tbuf) - 1] = '\0';
    {
        int tlen = (int)strlen(tbuf);
        if (tlen > 0 && tbuf[tlen - 1] == '\n')
            tbuf[tlen - 1] = '\0';
    }

    /* ---- Title row ---- */
    move(0, 2);
    clrtoeol();
    printw("2op Messages Log                                %s", tbuf);

    /* ---- Navigation hint ---- */
    move(2, 2);
    clrtoeol();
    printw("[+] newer   [-] older   [/] compose new msg   [a] antenna page");

    /* ---- Top separator ---- */
    move(3, 0);
    clrtoeol();
    printw("  --------------------------------------------------------------------------");

    /* ---- Query total message count ---- */
    reply = redisCommand(redisC, "LLEN 2opmsg");
    if (reply != NULL) {
        if (reply->type == REDIS_REPLY_INTEGER)
            totalMsgs = reply->integer;
        freeReplyObject(reply);
    }

    /* ---- Clamp scroll offset ---- */
    if (twoOpScrollOffset < 0)
        twoOpScrollOffset = 0;
    if (totalMsgs > 0 && twoOpScrollOffset > (int)totalMsgs - 1)
        twoOpScrollOffset = (int)totalMsgs - 1;

    /* ---- Fetch the window of messages ---- */
    /* Redis index 0 = newest.  We fetch indices                         */
    /*   [twoOpScrollOffset .. twoOpScrollOffset + N_MSGS_ON_PAGE - 1]  */
    /* then display in reverse order (index N-1 at top, index 0 at      */
    /* bottom) so the screen reads chronologically top-to-bottom.        */
    int startIdx = twoOpScrollOffset;
    int endIdx   = twoOpScrollOffset + N_MSGS_ON_PAGE - 1;

    reply = redisCommand(redisC, "LRANGE 2opmsg %d %d", startIdx, endIdx);

    row = 4;   /* first display row for message lines */

    if (reply != NULL && reply->type == REDIS_REPLY_ARRAY) {
        numFetched = (int)reply->elements;

        for (i = numFetched - 1; i >= 0; i--) {
            move(row, 2);
            clrtoeol();

            if (reply->element[i]->str != NULL) {
                char *msg = reply->element[i]->str;
                struct tm tm_msg;
                char msguser[50];
                char messageText[512];

                /* Stored format: YYYY-MM-DD HH:MM:SS (username) text */
                if (sscanf(msg,
                           "%d-%d-%d %d:%d:%d (%49[^)]) %511[^\n]",
                           &tm_msg.tm_year, &tm_msg.tm_mon, &tm_msg.tm_mday,
                           &tm_msg.tm_hour, &tm_msg.tm_min, &tm_msg.tm_sec,
                           msguser, messageText) == 8) {

                    /* Truncate message text to keep the line within     */
                    /* ~88 columns:  19 (date+time) + 3 (" ()") +       */
                    /* strlen(user) + 2 (" ") leaves the rest for text. */
                    int prefixLen = 22 + (int)strlen(msguser) + 2;
                    int maxText   = 88 - prefixLen;
                    if (maxText < 10) maxText = 10;
                    if ((int)strlen(messageText) > maxText) {
                        messageText[maxText - 3] = '.';
                        messageText[maxText - 2] = '.';
                        messageText[maxText - 1] = '.';
                        messageText[maxText]     = '\0';
                    }

                    printw("%04d-%02d-%02d %02d:%02d:%02d (%s) %s",
                           tm_msg.tm_year, tm_msg.tm_mon, tm_msg.tm_mday,
                           tm_msg.tm_hour, tm_msg.tm_min, tm_msg.tm_sec,
                           msguser, messageText);

                } else {
                    /* Parsing failed - show raw text, clamped to width */
                    char raw[90];
                    strncpy(raw, msg, 87);
                    raw[87] = '\0';
                    printw("%s", raw);
                }
            }
            row++;
        }
        freeReplyObject(reply);
    } else {
        if (reply != NULL) freeReplyObject(reply);
    }

    /* Clear any unused message rows below the last message */
    while (row < 4 + N_MSGS_ON_PAGE) {
        move(row, 2);
        clrtoeol();
        row++;
    }

    /* ---- Bottom separator ---- */
    move(4 + N_MSGS_ON_PAGE, 0);
    clrtoeol();
    printw("  --------------------------------------------------------------------------");

    /* ---- Status line ---- */
    move(4 + N_MSGS_ON_PAGE + 1, 2);
    clrtoeol();
    if (totalMsgs == 0) {
        printw("No messages stored.");
    } else if (numFetched == 0) {
        printw("No messages at this position.");
    } else if (twoOpScrollOffset == 0) {
        printw("Showing newest %d of %lld messages%s",
               numFetched, totalMsgs,
               (totalMsgs > N_MSGS_ON_PAGE) ? "  [-] to see older" : "");
    } else {
        printw("Showing msgs %d-%d from newest, of %lld total  [+] newer  [-] older",
               twoOpScrollOffset + 1,
               twoOpScrollOffset + numFetched,
               totalMsgs);
    }

    refresh();
}

#include "utceph.h"
#include "protos.h"

static int openlogcnt = 0;

void open_log() {
    openlog(program, LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL0);
    logger_fd = fopen(globals.logfile, "a");
    setRightOwner(globals.logfile);
    if (logger_fd == NULL) {
        syslog(LOG_INFO, "Error: %s", strerror(errno));
    }
}

void close_log() {
    if (NULL != logger_fd) {
        fclose(logger_fd);
    }
    closelog();
}

void vlog(const char *fmt, va_list args) {
    char lbuffer[1024];
    memset(lbuffer, '\0', 1024);
    vsnprintf(lbuffer, 1024, fmt, args);
    if (NULL != logger_fd) {
        fprintf(logger_fd, "%s\n", lbuffer);
        fflush(logger_fd);
    } else {
        if (openlogcnt <= 3) {
            openlogcnt++;
            open_log();
        }
        syslog(LOG_INFO, "%s", lbuffer);
    }
}

void logger(const char *fmt, ...) {
    if (globals.logging) {
        va_list args;
        va_start(args, fmt);
        vlog(fmt, args);
    }
}
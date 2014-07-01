#include "utceph.h"
#include "protos.h"

typedef void Sigfunc(int);

Sigfunc * signal(int signo, Sigfunc *func) {
    struct sigaction act, oact;
    act.sa_handler = func;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    if (signo == SIGALRM) {

        act.sa_flags |= SA_INTERRUPT; /* SunOS 4.x */

    } else {

        act.sa_flags |= SA_RESTART; /* SVR4, 44BSD */

    }
    if (sigaction(signo, &act, &oact) < 0)
        return (SIG_ERR);
    return (oact.sa_handler);
}

/* end signal */

Sigfunc * Signal(int signo, Sigfunc *func) /* for our signal() function */ {
    Sigfunc *sigfunc;

    if ((sigfunc = signal(signo, func)) == SIG_ERR) {
        // TODO
    }
    return (sigfunc);
}

void signal_term(int sig) {
    logger("Caught signal...");
    exitflag = true;
}

void set_thread_signalmask(sigset_t SignalSet) {

    sigemptyset(&SignalSet);
    sigaddset(&SignalSet, SIGHUP);
    sigaddset(&SignalSet, SIGUSR1);
    sigaddset(&SignalSet, SIGUSR2);
    sigaddset(&SignalSet, SIGTRAP);
    sigaddset(&SignalSet, SIGCHLD);
    sigaddset(&SignalSet, SIGIO);    
    sigaddset(&SignalSet, SIGTSTP);
    sigaddset(&SignalSet, SIGTTOU);
    sigaddset(&SignalSet, SIGTTIN);
    sigaddset(&SignalSet, SIGABRT);
    sigaddset(&SignalSet, SIGPIPE);
    sigaddset(&SignalSet, SIGALRM);
    sigaddset(&SignalSet, SIGSEGV);
    sigaddset(&SignalSet, SIGBUS);

    pthread_sigmask(SIG_BLOCK, &SignalSet, NULL);
}

void set_sig_handler() {

    Signal(SIGINT, signal_term);
    Signal(SIGHUP, SIG_IGN);
    Signal(SIGUSR1, SIG_IGN);
    Signal(SIGUSR2, SIG_IGN);
    Signal(SIGTRAP, SIG_IGN);
    Signal(SIGCHLD, SIG_IGN);
    Signal(SIGTSTP, SIG_IGN);
    Signal(SIGTTOU, SIG_IGN);
    Signal(SIGTTIN, SIG_IGN);
    Signal(SIGIO, SIG_IGN);
    Signal(SIGABRT, print_trace);
    Signal(SIGPIPE, SIG_IGN);
    Signal(SIGALRM, SIG_IGN);
    Signal(SIGSEGV, print_trace);
    Signal(SIGBUS, print_trace);
    Signal(SIGWINCH, SIG_IGN);
    Signal(SIGTERM, signal_term);
}

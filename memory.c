#include "utceph.h"
#include "protos.h"
#include <execinfo.h>

void allocate_resources() {
    int e;
    globals.pools = xmalloc(MAXPOOLS * sizeof (char *));
    for (e = 0; e < MAXPOOLS; e++) {
        globals.pools[e] = xmalloc((CFG_PARAM_LEN + 1) * sizeof (char));
    }
}

void free_resources() {
    logger("Freeing resources");
//    int e;
    //printf("1");
//    for (e = 0; e < MAXPOOLS; e++) {
//        if (NULL != globals.pools[e]) {
//            free(globals.pools[e]);
//        }
//    }
    //printf("2");
//    for (e = 0; e < globals.threads_count; e++) {
//        memset(current_data[e].filename, '\0', sizeof (current_data[e].filename));
//        memset(current_data[e].method, '\0', sizeof (current_data[e].method));
//        memset(current_data[e].poolname, '\0', sizeof (current_data[e].poolname));
//        memset(current_data[e].requri, '\0', sizeof (current_data[e].requri));
//        current_data[e].cLength = 0;
//    }
    printf("3");
    if (globals.pools) {
        free(globals.pools);
    }
    printf("4");
    if (globals.configfile) {
        free(globals.configfile);
    }
    printf("5");
    if (globals.ceph_user) {
        free(globals.ceph_user);
    }
    printf("6");
    if (globals.group) {
        free(globals.group);
    }
   printf("7");
    if (globals.pidfile) {
        free(globals.pidfile);
    }
    printf("8");
    if (globals.socket) {
        free(globals.socket);
    }
    printf("9");
}

void * xrealloc(void *ptr, size_t size) {
    void *new_mem = (void *) realloc(ptr, size);

    if (new_mem == NULL) {
        logger("Cannot allocate memory");
        exit(EXIT_FAILURE);
    }
    return new_mem;
}

void * xmalloc(size_t size) {
    void * new_mem = (void *) malloc(size);
    if (new_mem == NULL) {
        logger("Cannot allocate memory");
        exit(EXIT_FAILURE);
    }
    return new_mem;
}

void * xcalloc(size_t nmemb, size_t size) {
    void *new_mem = (void *) calloc(nmemb, size);

    if (new_mem == NULL) {
        fprintf(stderr, "Cannot allocate memory... Dying");
        exit(EXIT_FAILURE);
    }
    return new_mem;
}

void print_trace(int sig) {
    void *array[10];
    size_t size;
    char **strings;
    size_t i;

    size = backtrace(array, 10);
    strings = backtrace_symbols(array, size);

    printf("Obtained %zd stack frames.\n", size);

    for (i = 0; i < size; i++)
        printf("%s\n", strings[i]);

    free(strings);
    exit(EXIT_FAILURE);
}
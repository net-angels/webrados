#include "utceph.h"
#include "protos.h"

static int busyWorkers = 0;
static pthread_mutex_t accept_mutex = PTHREAD_MUTEX_INITIALIZER; //[MAXPOOLS];
static pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER; //[MAXPOOLS];
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void setRightOwner(const char *path) {
    if (NULL == path)return;

    struct passwd *pw = getpwnam(globals.user);
    struct group *gr = getgrnam(globals.group);
    if (NULL == pw || NULL == gr) {
        logger("No such user/group: %s/%s", globals.user, globals.group);
        exit(EXIT_FAILURE);
    }
    if (FileExists((char *) path)) {
        int rc = chown(path, pw->pw_uid, gr->gr_gid);
        if (rc == -1) {
            logger("Cannot chown file/directory '%s',%s", path, strerror(errno));
        }
    }
}

bool FileExists(char *path) {
    FILE *fp = fopen(path, "r");
    if (fp) {
        fclose(fp);
        return true;
    } else {
        return false;
    }
}

bool DirectoryExists(const char* pzPath) {
    if (pzPath == NULL) return false;

    DIR *pDir;
    bool bExists = false;

    pDir = opendir(pzPath);

    if (pDir != NULL) {
        bExists = true;
        (void) closedir(pDir);
    }

    return bExists;
}

int create_and_bind() {
    int sock_fd = 0;
    if (0 != strcmp(globals.socket, "")) {

        int fd = open(globals.socket, O_CREAT, 0644);
        if (fd < 0) {
            int err = errno;
            if (err != ENXIO) {
                logger("BindError: %s", strerror(errno));
                halt();
            }
        } else {
            close(fd);
        }

        sock_fd = FCGX_OpenSocket(globals.socket, SOMAXCONN);
        if (sock_fd < 0) {
            logger("OpenSocketError: %s", strerror(errno));
            halt();
        }
        if (chmod(globals.socket, 0777) < 0) {
            logger("ChmodError: %s", strerror(errno));
        }
    } else if (NULL != strstr(globals.socket, ":")) {
        sock_fd = FCGX_OpenSocket(globals.socket, SOMAXCONN);
        if (sock_fd < 0) {
            logger("OpenSocketError: %s", strerror(errno));
            halt();
        }
    }
    return sock_fd;
}

bool is_valid_opt(char *opts) {
    if (opts == NULL)return false;
    if (opts[0] == '-' || opts[0] == ':' || opts[0] == '?')return false;

    return true;
}

int checkPid() {
    pid_t S_PID;
    if ((pid_fd = fopen(globals.pidfile, "r")) != NULL) {
        (void) fscanf(pid_fd, "%d", &S_PID);
        if (pid_fd)fclose(pid_fd);
        if (kill(S_PID, 18) == 0) {
            logger("%s", "Software already running");
            exit(EXIT_FAILURE);
        }
    }
    return 0;
}

void removePid() {

    if (0 != unlink(globals.pidfile)) {
        logger("Cannot unlink file '%s',%s", globals.pidfile, strerror(errno));
    } else {
        logger("Pid file '%s' removed", globals.pidfile);
    }
}

void savePid() {

    if ((pid_fd = fopen(globals.pidfile, "w")) == NULL) {
        logger("Cannot create PID file '%s'.Exiting nicely! - %s", globals.pidfile, strerror(errno));
        exit(EXIT_FAILURE);
    } else {
        logger("Process ID stored in: '%s',%d", globals.pidfile, errno);
    }

    fprintf(pid_fd, "%d", getpid());

    if (pid_fd)fclose(pid_fd);

    if (0 != chown(globals.pidfile, my_uid, my_gid)) {
        logger("Cannot chown file '%s',%s", globals.pidfile, strerror(errno));
    }
}

int main(int argc, char **argv) {

    set_sig_handler();
    static const char *optString = "c:f:?";
    opt = getopt_long(argc, argv, optString, longOpts, &longIndex);
    while (opt != -1) {
        switch (opt) {
            case 'c':
                if (!is_valid_opt(optarg)) {
                    printf("Usage: %s -c <config file> -f <ceph config file>\n", argv[0]);
                    printf("Usage: %s --config <config file path> --ceph-config <ceph config file path>\n", argv[0]);
                    exit(EXIT_FAILURE);
                }
                globals.configfile = xrealloc(globals.configfile, (sizeof (char) * strlen(optarg) + 1));
                strcpy(globals.configfile, optarg);
                break;
            case 'f':
                if (!is_valid_opt(optarg)) {
                    printf("Usage: %s -c <config file> -f <ceph config file>\n", argv[0]);
                    printf("Usage: %s --config <config file path> --ceph-config <ceph config file path>\n", argv[0]);
                    exit(EXIT_FAILURE);
                }
                globals.ceph_config = xrealloc(globals.ceph_config, (sizeof (char) * strlen(optarg) + 1));
                strcpy(globals.ceph_config, optarg);
                break;
            default:
                ;
                ;
                break;
        }

        opt = getopt_long(argc, argv, optString, longOpts, &longIndex);
    }

    FCGX_Init();
    allocate_resources();
    if (NULL == globals.configfile || NULL == globals.ceph_config) {
        printf("Usage: %s -c <config file> -f <ceph config file>\n", argv[0]);
        printf("Usage: %s --config <config path> --ceph-config <ceph config path>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    if (false == FileExists(globals.configfile)) {
        printf("Specified file '%s' does not exists\n", globals.configfile);
        exit(EXIT_FAILURE);
    } else if (true == DirectoryExists(globals.configfile)) {
        printf("Specified file '%s' is a directory\n", globals.configfile);
        exit(EXIT_FAILURE);
    } else {
        read_config(globals.configfile);
    }
    open_log();
    logger("-=Starting application=-");
    checkPid();

    listenfd = create_and_bind();
    if (!(pwd = getpwnam(globals.user))) {
        logger("No such user: %s. Quitting!", globals.user);
        exit(EXIT_FAILURE);
    } else my_uid = pwd->pw_uid;

    if (!(grp = getgrnam(globals.group))) {
        logger("No such group: %s. Quitting!", globals.group);
        exit(EXIT_FAILURE);
    } else my_gid = grp->gr_gid;
    setRightOwner(globals.pidfile);

    if (!globals.foreground) {
        pid = fork();

        if (pid < 0) {
            logger("%s", "Can not fork! [Invalid PID]");
            removePid();
            exit(EXIT_FAILURE);
        }
        /* If we got a good PID, then
           we can exit the parent process. */
        if (pid > 0) {
            exit(EXIT_SUCCESS);
        }
        //
        logger("%s", "Forked successfully");
        /* Change the file mode mask */
        umask(027);
        /* Create a new SID for the child process */
        sid = setsid();
        if (sid < 0) {
            logger("%s", "Can not create child process");
            /* Log the failure */
            removePid();
            exit(EXIT_FAILURE);
        }
        logger("%s", "Daemonizing");
        /* Change the current working directory */
        if ((chdir("/tmp")) < 0) {
            logger("%s", "Can not change current directory");
            /* Log the failure */
            exit(EXIT_FAILURE);
        }
        logger("%s", "Working directory changed to '/tmp'");

        savePid();

        if ((setgid(my_gid)) < 0) {
            logger("ERROR: setgid(%d) failed: %s", my_gid, strerror(errno));
            exit(EXIT_FAILURE);
        }
        logger("Group ID changed to %d", my_gid);

        if ((setuid(my_uid)) < 0) {
            logger("ERROR: setuid(%d) failed: %s", my_uid, strerror(errno));
            exit(EXIT_FAILURE);
        }
        logger("User ID changed to %d", my_gid);

        /* Close out the standard file descriptors */
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
    }

    fcntl(0, F_SETFL, fcntl(0, F_GETFD, 0) | O_NONBLOCK | O_ASYNC);
    int i;

    for (i = 0; i < globals.threads_count; i++) {
        if (0 > pthread_create(&id[i], NULL, doit, (void*) &i)) {
            logger("Cannot create a thread: %s", strerror(errno));
        }
    }

    while (!exitflag) {
        sleep(1);
    }

    halt();
    return 0;
}

void halt() {
    FCGX_ShutdownPending();
    close(listenfd);
    removePid();
    unlink(globals.socket);
    free_resources();
    logger("-=Closing application=-");
    close_log();
    exit(EXIT_SUCCESS);
}

void *doit(void *ptr) {

    pid_t tid = syscall(__NR_gettid);
    if (NULL == ptr) {
        pthread_exit((void *) 0);
    }
    sigset_t ths;
    int rc, worker_no = *((int *) ptr);
    bool ctr = true;
    rados_ioctx_t io = NULL;
    rados_t cluster;
    FCGX_Request request;

    set_thread_signalmask(ths);
    FCGX_InitRequest(&request, listenfd, 0);

    for (;;) {
        /* counter */
        pthread_mutex_lock(&counter_mutex);
        if (busyWorkers > 0) {
            busyWorkers--;
        }
        pthread_mutex_unlock(&counter_mutex);
        /* accept */
        pthread_mutex_lock(&accept_mutex);
        rc = FCGX_Accept_r(&request);
        pthread_mutex_unlock(&accept_mutex);
        if (rc < 0) {
            logger("[%d] Can not accept: %s", tid, strerror(errno));
            continue;
        }
        //
        if (globals.threads_count == busyWorkers) {
            logger("Workers max count[%d] reached", globals.threads_count);
            continue;
        }
        /* increment busyWorkers */
        pthread_mutex_lock(&counter_mutex);
        busyWorkers++;
        pthread_mutex_unlock(&counter_mutex);
        /* fill data */
        pthread_mutex_lock(&mutex);
        fill_current_data(&request, &io, worker_no);
        pthread_mutex_unlock(&mutex);
        /* show status */
        if (0 == strncmp("status", current_data[worker_no].qstring, 6)) {
            FCGX_FPrintF(request.out, "%s\nTotal workers: %d\nBusy workers: %d\n", ok, globals.threads_count, busyWorkers);
            FCGX_Finish_r(&request);
            continue;
        }
        /* show usage */
        if (0 == strncmp("usage", current_data[worker_no].qstring, 5)) {
            int c;
            FCGX_FPrintF(request.out, "%s%s\n\n", ok, usage_header);

            for (c = 0; c < globals.threads_count; c++) {
                FCGX_FPrintF(request.out, "%d", c);
                FCGX_FPrintF(request.out, "%30ld", current_data[c].usage.ru_maxrss);
                FCGX_FPrintF(request.out, "%30ld", current_data[c].usage.ru_ixrss);
                FCGX_FPrintF(request.out, "%30ld", current_data[c].usage.ru_idrss);
                FCGX_FPrintF(request.out, "%30ld", current_data[c].usage.ru_isrss);
                FCGX_FPrintF(request.out, "%30ld", current_data[c].usage.ru_minflt);
                FCGX_FPrintF(request.out, "%30ld", current_data[c].usage.ru_majflt);
                FCGX_FPrintF(request.out, "%30ld", current_data[c].usage.ru_inblock);
                FCGX_FPrintF(request.out, "%30ld", current_data[c].usage.ru_oublock);
                FCGX_FPrintF(request.out, "\n");

            }
            FCGX_FFlush(request.out);
            FCGX_Finish_r(&request);
            continue;
        }
        //
        if (NULL == current_data[worker_no].poolname || NULL == current_data[worker_no].filename || (0 == strcmp(current_data[worker_no].poolname, "")) || (0 == strcmp(current_data[worker_no].filename, ""))) {
            FCGX_FPrintF(request.out, "%s", usage);
            FCGX_FFlush(request.out);
            FCGX_Finish_r(&request);
            logger("[%d] No parameters given", tid);
            ctr = false;
            continue;
        } else {
            ctr = true;
        }
        //
        if (!is_whitelisted(current_data[worker_no].poolname)) {
            logger("[%d] Pool '%s' is not whitelisted", tid, current_data[worker_no].poolname);
            FCGX_FPrintF(request.out, "%s", forbidden);
            FCGX_FFlush(request.out);
            FCGX_Finish_r(&request);
            continue;
        }
        //
        if (
                (0 != strcmp("GET", current_data[worker_no].method))&&
                (0 != strcmp("PUT", current_data[worker_no].method))&&
                (0 != strcmp("POST", current_data[worker_no].method))&& // Allowing POST method for further processing
                (0 != strcmp("DELETE", current_data[worker_no].method))&&
                (0 != strcmp("OPTIONS", current_data[worker_no].method))
                ) {
            logger("[%d] Method %s not supported", tid, current_data[worker_no].method);
            FCGX_FPrintF(request.out, "%s", not_supported);
            FCGX_FFlush(request.out);
            FCGX_Finish_r(&request);
            continue;
        }
        //
        if (
                current_data[worker_no].cLength <= 0 &&
                (0 != strcmp("GET", current_data[worker_no].method))&&
                (0 != strcmp("POST", current_data[worker_no].method))&&
                (0 != strcmp("DELETE", current_data[worker_no].method))&&
                (0 != strcmp("OPTIONS", current_data[worker_no].method))
                ) {

            logger("[%d] No data from standard input", tid);
            FCGX_FPrintF(request.out, "%s", no_data);
            FCGX_FFlush(request.out);
            FCGX_Finish_r(&request);
            continue;
        } else {
            if (ctr) {
                int err = rados_create(&cluster, globals.ceph_user);
                if (err < 0) {
                    logger("[%d] Cannot create a cluster handle as : %s", tid, globals.ceph_user, strerror(-err));
                    halt();
                }

                err = rados_conf_read_file(cluster, globals.ceph_config);
                if (err < 0) {
                    logger("[%d] Cannot read config file: %s", tid, globals.ceph_config);
                    FCGX_FPrintF(request.out, "%s", ierror);
                    FCGX_FFlush(request.out);
                    goto end;
                }

                err = rados_connect(cluster);
                if (err < 0) {
                    logger("[%d] Cannot connect to cluster: %s", tid, strerror(-err));
                    FCGX_FPrintF(request.out, "%s", ierror);
                    FCGX_FFlush(request.out);
                    goto end;
                }

                err = rados_ioctx_create(cluster, current_data[worker_no].poolname, &io);

                if (err < 0) {
                    logger("[%d] Cannot open rados pool [%s]: %s", tid, current_data[worker_no].poolname, strerror(-err));
                    FCGX_FPrintF(request.out, "Cannot open rados pool [%s]: %s\n", current_data[worker_no].poolname, strerror(-err));
                    FCGX_FFlush(request.out);
                    goto end;
                }

                if (0 == strcmp(current_data[worker_no].method, "PUT")) {
                    logger("[%d] Expecting %d bytes", tid, current_data[worker_no].cLength);
                    logger("[%d] Writing to [%s -> %s]", tid, current_data[worker_no].poolname, current_data[worker_no].filename);

                    uint64_t bytes = PUT(io, &request, current_data[worker_no].filename, current_data[worker_no].cLength, tid);
                    logger("[%d] Upload completed: %s has %d bytes", tid, current_data[worker_no].filename, bytes);
                    if (0 >= bytes) {
                        FCGX_FPrintF(request.out, "Connection: Close\r\n\r\n");
                    } else {
                        FCGX_FPrintF(request.out, "%s", created);
                    }
                    FCGX_FFlush(request.out);
                }/* if PUT */
                else if (0 == strcmp(current_data[worker_no].method, "GET")) {
                    GET(io, &request, current_data[worker_no].filename, tid);
                } else if (0 == strcmp(current_data[worker_no].method, "DELETE")) {
                    if (0 > (err = rados_remove(io, current_data[worker_no].filename))) {
                        logger("[%d] Could not delete object '%s'", tid, current_data[worker_no].filename);
                        FCGX_FPrintF(request.out, "%s", not_found);
                    } else {
                        FCGX_FPrintF(request.out, "%s", deleted);
                    }
                    FCGX_FFlush(request.out);
                } else if (0 == strcmp(current_data[worker_no].method, "OPTIONS")) {
                    uint64_t size;
                    time_t Time;

                    if (0 > (err = rados_stat(io, (const char *) current_data[worker_no].filename, &size, &Time))) {
                        logger("[%d] Could not stat object '%s'", tid, current_data[worker_no].filename);
                        FCGX_FPrintF(request.out, "%s", not_found);
                    } else {
                        FCGX_FPrintF(request.out, "%sSize: %ld\nLast modified: %ld\n", ok, size, Time);
                    }
                    FCGX_FFlush(request.out);
                } else {
                    logger("[%d] Method %s not supported", tid, current_data[worker_no].method);
                    FCGX_FPrintF(request.out, "%s", not_supported);
                    FCGX_FFlush(request.out);
                }
            } /*ctr*/
        }/* else */

end:
        FCGX_Finish_r(&request);
        if (io != NULL) {
            rados_ioctx_destroy(io);
        }
        if (cluster != NULL) {
            rados_shutdown(cluster);
        }

        pthread_mutex_destroy(&accept_mutex);
        pthread_mutex_destroy(&counter_mutex);
    }

    return NULL;
}

void fill_current_data(FCGX_Request *req, rados_ioctx_t *IO, int c) {

    char *duplicate = NULL;
    char *p = NULL;

    memset(current_data[c].poolname, '\0', sizeof (current_data[c].poolname));
    memset(current_data[c].filename, '\0', sizeof (current_data[c].filename));
    memset(current_data[c].method, '\0', sizeof (current_data[c].method));
    memset(current_data[c].qstring, '\0', sizeof (current_data[c].qstring));
    memset(current_data[c].requri, '\0', sizeof (current_data[c].requri));
    current_data[c].cLength = 0;
    getrusage(RUSAGE_THREAD, &current_data[c].usage);

    char *contentLength = FCGX_GetParam("CONTENT_LENGTH", req->envp);
    char *rmethod = FCGX_GetParam("REQUEST_METHOD", req->envp);
    char *requri = FCGX_GetParam("REQUEST_URI", req->envp);
    char *qstring = FCGX_GetParam("QUERY_STRING", req->envp);

    if (contentLength != NULL) {
        current_data[c].cLength = strtol(contentLength, NULL, 10);
    } else {
        current_data[c].cLength = 0;
    }

    if (rmethod != NULL) {
        snprintf(current_data[c].method, sizeof (current_data[c].method), "%s", rmethod);
    }

    if (qstring != NULL) {
        snprintf(current_data[c].qstring, sizeof (current_data[c].qstring), "%s", qstring);
    }

    if (requri != NULL) {
        snprintf(current_data[c].requri, sizeof (current_data[c].method), "%s", requri);
    } else {
        return;
    }

    duplicate = strndup(requri, strlen(requri));

    if (NULL == duplicate) {
        return;
    }
    p = strtok(duplicate, "/");
    if (p) {
        strncpy(current_data[c].poolname, p, sizeof (current_data[c].poolname));
    }

    int z = 0, remains = 0;

    strncat(current_data[c].filename, "/", 1);
    while (NULL != (p = strtok(NULL, "/"))) {
        z = strlen(p);
        remains = 254 - strlen(current_data[c].filename);
        if (z > remains)z = remains;
        strncat(current_data[c].filename, p, z);
        if (remains > 0) {
            strncat(current_data[c].filename, "/", 1);
        }
    }

    int filename_sz = strlen(current_data[c].filename);
    if (current_data[c].filename[filename_sz - 1] == '/') {
        current_data[c].filename[filename_sz - 1] = '\0';
    }
    int i = 0, count = 0;

    for (i = 0; i < filename_sz; i++) {
        if (current_data[c].filename[i] == '/') {
            count++;
        }
    }

    if (count == 1) {
        for (i = 0; i < filename_sz && current_data[c].filename[i] != '\0'; i++) {
            current_data[c].filename[i] = current_data[c].filename[i + 1];
        }
        for (; i < filename_sz; i++)
            current_data[c].filename[i] = '\0';

    }

    if (duplicate)FREE(duplicate);
    if (p)FREE(p);
    contentLength = NULL;
    rmethod = NULL;
    requri = NULL;

}
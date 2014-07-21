#include "utceph.h"
#include "protos.h"

#define debug(fmt, ...) printf(fmt"\n", __VA_ARGS__);

bool is_whitelisted(char *pool) {
    int e;
    if (NULL == pool)return false;
    for (e = 0; e < globals.threads_count; e++) {
        if (0 == strcmp(globals.pools[e], pool))return true;
    }
    return false;
}

void read_config(char *cfg_file) {
    config_t cfg, *cf;
    const config_setting_t *whitelisted_pools = NULL;
    int count, i = 0;
    WBR_INT tc;

    const char *user;
    const char *group;
    const char *pidfile;
    const char *logfile;
    const char *socket;
    const char *cuser;

    if (NULL == cfg_file)return;
    cf = &cfg;
    config_init(cf);
    debug("Reading configuration file '%s'", cfg_file);

    if (CONFIG_FALSE == config_read_file(cf, cfg_file)) {
        debug("Line %d - %s\n", config_error_line(cf), config_error_text(cf));
        config_destroy(cf);
        exit(EXIT_FAILURE);
    }
    if (!config_lookup_int(cf, "threads-count", &tc)) {
        debug("%s", "Configuration entry 'threads-count' not found");
        globals.threads_count = MAXPOOLS;
    } else {

        if (tc > MAXPOOLS) {
            globals.threads_count = MAXPOOLS;
        } else {
            globals.threads_count = tc;
        }

    }
#if (LIBCONFIG_VER_MAJOR == 1 && LIBCONFIG_VER_MINOR >= 4)
    debug("Workers count: %d", globals.threads_count);
#else
	debug("Workers count: %ld", globals.threads_count);
#endif    

    if (!config_lookup_bool(cf, "foreground", &globals.foreground)) {
        debug("%s", "Configuration entry 'foreground' not found");
        debug("%s", "Logging disabled");
        globals.foreground = false;
    }

    if (!config_lookup_bool(cf, "logging", &globals.logging)) {
        debug("%s", "Configuration entry 'logging' not found");
        globals.logging = false;
    }

    cuser = xmalloc(sizeof (char) * CFG_PARAM_LEN);
    if (!config_lookup_string(cf, "ceph-user", &cuser)) {
        debug("%s", "Using default value of 'ceph-user'");
        globals.ceph_user = xrealloc(globals.ceph_user, (sizeof (char) * CFG_PARAM_LEN + 1));
        strncpy(globals.ceph_user, "admin", 5);
        globals.ceph_user[5] = '\0';
        cuser = NULL;
    } else {
        if (0 == strcmp(cuser, "")) {
            debug("%s", "Configuration entry 'ceph-user' is empty");
            exit(EXIT_FAILURE);
        }
        globals.ceph_user = xrealloc(globals.ceph_user, (sizeof (char) * CFG_PARAM_LEN + 1));
        strncpy(globals.ceph_user, cuser, CFG_PARAM_LEN);
        globals.ceph_user[strlen(cuser)] = '\0';
        cuser = NULL;
    }
    debug("Ceph User: %s", globals.ceph_user);

    group = xmalloc(sizeof (char) * CFG_PARAM_LEN);
    if (!config_lookup_string(cf, "group", &group)) {
        debug("%s", "Configuration entry 'group' not found");
        exit(EXIT_FAILURE);
    } else {
        if (0 == strcmp(group, "")) {
            debug("%s", "Configuration entry 'group' is empty");
            exit(EXIT_FAILURE);
        }
        globals.group = xrealloc(globals.group, (sizeof (char) * CFG_PARAM_LEN + 1));
        strncpy(globals.group, group, CFG_PARAM_LEN);
        globals.group[strlen(group)] = '\0';
        group = NULL;
    }
    debug("Group: %s", globals.group);

    user = xmalloc(sizeof (char) * CFG_PARAM_LEN);
    if (!config_lookup_string(cf, "user", &user)) {
        debug("%s", "Configuration entry 'user' not found");
        exit(EXIT_FAILURE);
    } else {
        if (0 == strcmp(user, "")) {
            debug("%s", "Configuration entry 'user' is empty");
            exit(EXIT_FAILURE);
        }
        globals.user = xrealloc(globals.user, (sizeof (char) * CFG_PARAM_LEN + 1));
        strncpy(globals.user, user, CFG_PARAM_LEN);
        globals.user[strlen(user)] = '\0';
        user = NULL;
    }
    debug("User: %s", globals.user);
    pidfile = xmalloc(sizeof (char) * CFG_PARAM_LEN);
    if (!config_lookup_string(cf, "pidfile", &pidfile)) {
        debug("%s", "Configuration entry 'pidfile' not found");
        exit(EXIT_FAILURE);
    } else {
        if (0 == strcmp(pidfile, "")) {
            debug("%s", "Configuration entry 'pidfile' is empty");
            exit(EXIT_FAILURE);
        }
        globals.pidfile = xrealloc(globals.pidfile, (sizeof (char) * CFG_PARAM_LEN + 1));
        strncpy(globals.pidfile, pidfile, CFG_PARAM_LEN);
        globals.user[strlen(pidfile)] = '\0';
        pidfile = NULL;
    }
    debug("Pidfile: %s", globals.pidfile);

    logfile = xmalloc(sizeof (char) * CFG_PARAM_LEN);
    if (!config_lookup_string(cf, "logfile", &logfile)) {
        debug("%s", "Configuration entry 'logfile' not found");
        exit(EXIT_FAILURE);
    } else {
        if (0 == strcmp(logfile, "")) {
            debug("%s", "Configuration entry 'logfile' is empty");
            exit(EXIT_FAILURE);
        }
        globals.logfile = xrealloc(globals.logfile, (sizeof (char) * CFG_PARAM_LEN + 1));
        strncpy(globals.logfile, logfile, CFG_PARAM_LEN);
        globals.logfile[strlen(logfile)] = '\0';
        logfile = NULL;
    }
    debug("Logfile: %s", globals.logfile);
    socket = xmalloc(sizeof (char) * CFG_PARAM_LEN);
    if (!config_lookup_string(cf, "socket", &socket)) {
        debug("%s", "Configuration entry 'socket' not found");
        exit(EXIT_FAILURE);
    } else {
        if (0 == strcmp(socket, "")) {
            debug("%s", "Configuration entry 'socket' is empty");
            exit(EXIT_FAILURE);
        }
        globals.socket = xrealloc(globals.socket, (sizeof (char) * CFG_PARAM_LEN + 1));
        strncpy(globals.socket, socket, CFG_PARAM_LEN);
        globals.socket[strlen(socket)] = '\0';
        socket = NULL;
    }
    debug("Socket: %s\n", globals.socket);


    whitelisted_pools = config_lookup(cf, "rados-mount");
    if (NULL == whitelisted_pools) {
        debug("Undefined section '%s'", "rados-mount");
        exit(EXIT_FAILURE);
    }
    if (NULL != whitelisted_pools && CONFIG_TRUE == config_setting_is_array(whitelisted_pools)) {
        count = config_setting_length(whitelisted_pools);
        for (i = 0; i < count; i++) {
            memset(globals.pools[i], '\0', CFG_PARAM_LEN + 1);
            strncpy(globals.pools[i], config_setting_get_string_elem(whitelisted_pools, i), CFG_PARAM_LEN);
        }
    }


    config_destroy(cf);
}

char *get_mime(char *filename) {
    char *m = xmalloc(32);
    memset(m, '\0', 32);
    if (NULL == filename || 0 == strcmp(filename, "")) {
        strcpy(m, "*");
        return m;
    }

    char *ext = strrchr(filename, '.');

    if (NULL == ext || 0 == strcmp("", ext)) {
        strcpy(m, "application/octet-stream");
        return m;
    }

    if (0 == strncmp(".evy", ext, 4)) {
        strcpy(m, "application/envoy");
        return m;
    }
    if (0 == strncmp(".fif", ext, 4)) {
        strcpy(m, "application/fractals");
        return m;
    }
    if (0 == strncmp(".spl", ext, 4)) {
        strcpy(m, "application/futuresplash");
        return m;
    }
    if (0 == strncmp(".hta", ext, 4)) {
        strcpy(m, "application/hta");
        return m;
    }
    if (0 == strncmp(".acx", ext, 4)) {
        strcpy(m, "application/internet-property-stream");
        return m;
    }
    if (0 == strncmp(".hqx", ext, 4)) {
        strcpy(m, "application/mac-binhex40");
        return m;
    }
    if (0 == strncmp(".doc", ext, 4)) {
        strcpy(m, "application/msword");
        return m;
    }
    if (0 == strncmp(".dot", ext, 4)) {
        strcpy(m, "application/msword");
        return m;
    }
    if (0 == strncmp(".bin", ext, 4)) {
        strcpy(m, "application/octet-stream");
        return m;
    }
    if (0 == strncmp(".class", ext, 6)) {
        strcpy(m, "application/octet-stream");
        return m;
    }
    if (0 == strncmp(".dms", ext, 4)) {
        strcpy(m, "application/octet-stream");
        return m;
    }
    if (0 == strncmp(".exe", ext, 4)) {
        strcpy(m, "application/octet-stream");
        return m;
    }
    if (0 == strncmp(".lha", ext, 4)) {
        strcpy(m, "application/octet-stream");
        return m;
    }
    if (0 == strncmp(".lzh", ext, 4)) {
        strcpy(m, "application/octet-stream");
        return m;
    }
    if (0 == strncmp(".oda", ext, 4)) {
        strcpy(m, "application/oda");
        return m;
    }
    if (0 == strncmp(".axs", ext, 4)) {
        strcpy(m, "application/olescript");
        return m;
    }
    if (0 == strncmp(".pdf", ext, 4)) {
        strcpy(m, "application/pdf");
        return m;
    }
    if (0 == strncmp(".prf", ext, 4)) {
        strcpy(m, "application/pics-rules");
        return m;
    }
    if (0 == strncmp(".p10", ext, 4)) {
        strcpy(m, "application/pkcs10");
        return m;
    }
    if (0 == strncmp(".crl", ext, 4)) {
        strcpy(m, "application/pkix-crl");
        return m;
    }
    if (0 == strncmp(".ai", ext, 4)) {
        strcpy(m, "application/postscript");
        return m;
    }
    if (0 == strncmp(".eps", ext, 4)) {
        strcpy(m, "application/postscript");
        return m;
    }
    if (0 == strncmp(".ps", ext, 4)) {
        strcpy(m, "application/postscript");
        return m;
    }
    if (0 == strncmp(".rtf", ext, 4)) {
        strcpy(m, "application/rtf");
        return m;
    }
    if (0 == strncmp(".setpay", ext, 7)) {
        strcpy(m, "application/set-payment-initiation");
        return m;
    }
    if (0 == strncmp(".setreg", ext, 7)) {
        strcpy(m, "application/set-registration-initiation");
        return m;
    }
    if (0 == strncmp(".xla", ext, 4)) {
        strcpy(m, "application/vnd.ms-excel");
        return m;
    }
    if (0 == strncmp(".xlc", ext, 4)) {
        strcpy(m, "application/vnd.ms-excel");
        return m;
    }
    if (0 == strncmp(".xlm", ext, 4)) {
        strcpy(m, "application/vnd.ms-excel");
        return m;
    }
    if (0 == strncmp(".xls", ext, 4)) {
        strcpy(m, "application/vnd.ms-excel");
        return m;
    }
    if (0 == strncmp(".xlt", ext, 4)) {
        strcpy(m, "application/vnd.ms-excel");
        return m;
    }
    if (0 == strncmp(".xlw", ext, 4)) {
        strcpy(m, "application/vnd.ms-excel");
        return m;
    }
    if (0 == strncmp(".msg", ext, 4)) {
        strcpy(m, "application/vnd.ms-outlook");
        return m;
    }
    if (0 == strncmp(".sst", ext, 4)) {
        strcpy(m, "application/vnd.ms-pkicertstore");
        return m;
    }
    if (0 == strncmp(".cat", ext, 4)) {
        strcpy(m, "application/vnd.ms-pkiseccat");
        return m;
    }
    if (0 == strncmp(".stl", ext, 4)) {
        strcpy(m, "application/vnd.ms-pkistl");
        return m;
    }
    if (0 == strncmp(".pot", ext, 4)) {
        strcpy(m, "application/vnd.ms-powerpoint");
        return m;
    }
    if (0 == strncmp(".pps", ext, 4)) {
        strcpy(m, "application/vnd.ms-powerpoint");
        return m;
    }
    if (0 == strncmp(".ppt", ext, 4)) {
        strcpy(m, "application/vnd.ms-powerpoint");
        return m;
    }
    if (0 == strncmp(".mpp", ext, 4)) {
        strcpy(m, "application/vnd.ms-project");
        return m;
    }
    if (0 == strncmp(".wcm", ext, 4)) {
        strcpy(m, "application/vnd.ms-works");
        return m;
    }
    if (0 == strncmp(".wdb", ext, 4)) {
        strcpy(m, "application/vnd.ms-works");
        return m;
    }
    if (0 == strncmp(".wks", ext, 4)) {
        strcpy(m, "application/vnd.ms-works");
        return m;
    }
    if (0 == strncmp(".wps", ext, 4)) {
        strcpy(m, "application/vnd.ms-works");
        return m;
    }
    if (0 == strncmp(".hlp", ext, 4)) {
        strcpy(m, "application/winhlp");
        return m;
    }
    if (0 == strncmp(".bcpio", ext, 6)) {
        strcpy(m, "application/x-bcpio");
        return m;
    }
    if (0 == strncmp(".cdf", ext, 4)) {
        strcpy(m, "application/x-cdf");
        return m;
    }
    if (0 == strncmp(".z", ext, 2)) {
        strcpy(m, "application/x-compress");
        return m;
    }
    if (0 == strncmp(".tgz", ext, 4)) {
        strcpy(m, "application/x-compressed");
        return m;
    }
    if (0 == strncmp(".cpio", ext, 5)) {
        strcpy(m, "application/x-cpio");
        return m;
    }
    if (0 == strncmp(".csh", ext, 4)) {
        strcpy(m, "application/x-csh");
        return m;
    }
    if (0 == strncmp(".dcr", ext, 4)) {
        strcpy(m, "application/x-director");
        return m;
    }
    if (0 == strncmp(".dir", ext, 4)) {
        strcpy(m, "application/x-director");
        return m;
    }
    if (0 == strncmp(".dxr", ext, 4)) {
        strcpy(m, "application/x-director");
        return m;
    }
    if (0 == strncmp(".dvi", ext, 4)) {
        strcpy(m, "application/x-dvi");
        return m;
    }
    if (0 == strncmp(".gtar", ext, 5)) {
        strcpy(m, "application/x-gtar");
        return m;
    }
    if (0 == strncmp(".gz", ext, 3)) {
        strcpy(m, "application/x-gzip");
        return m;
    }
    if (0 == strncmp(".hdf", ext, 4)) {
        strcpy(m, "application/x-hdf");
        return m;
    }
    if (0 == strncmp(".ins", ext, 4)) {
        strcpy(m, "application/x-internet-signup");
        return m;
    }
    if (0 == strncmp(".isp", ext, 4)) {
        strcpy(m, "application/x-internet-signup");
        return m;
    }
    if (0 == strncmp(".iii", ext, 4)) {
        strcpy(m, "application/x-iphone");
        return m;
    }
    if (0 == strncmp(".js", ext, 3)) {
        strcpy(m, "application/x-javascript");
        return m;
    }
    if (0 == strncmp(".latex", ext, 6)) {
        strcpy(m, "application/x-latex");
        return m;
    }
    if (0 == strncmp(".mdb", ext, 4)) {
        strcpy(m, "application/x-msaccess");
        return m;
    }
    if (0 == strncmp(".crd", ext, 4)) {
        strcpy(m, "application/x-mscardfile");
        return m;
    }
    if (0 == strncmp(".clp", ext, 4)) {
        strcpy(m, "application/x-msclip");
        return m;
    }
    if (0 == strncmp(".dll", ext, 4)) {
        strcpy(m, "application/x-msdownload");
        return m;
    }
    if (0 == strncmp(".m13", ext, 4)) {
        strcpy(m, "application/x-msmediaview");
        return m;
    }
    if (0 == strncmp(".m14", ext, 4)) {
        strcpy(m, "application/x-msmediaview");
        return m;
    }
    if (0 == strncmp(".mvb", ext, 4)) {
        strcpy(m, "application/x-msmediaview");
        return m;
    }
    if (0 == strncmp(".wmf", ext, 4)) {
        strcpy(m, "application/x-msmetafile");
        return m;
    }
    if (0 == strncmp(".mny", ext, 4)) {
        strcpy(m, "application/x-msmoney");
        return m;
    }
    if (0 == strncmp(".pub", ext, 4)) {
        strcpy(m, "application/x-mspublisher");
        return m;
    }
    if (0 == strncmp(".scd", ext, 4)) {
        strcpy(m, "application/x-msschedule");
        return m;
    }
    if (0 == strncmp(".trm", ext, 4)) {
        strcpy(m, "application/x-msterminal");
        return m;
    }
    if (0 == strncmp(".wri", ext, 4)) {
        strcpy(m, "application/x-mswrite");
        return m;
    }
    if (0 == strncmp(".cdf", ext, 4)) {
        strcpy(m, "application/x-netcdf");
        return m;
    }
    if (0 == strncmp(".nc", ext, 4)) {
        strcpy(m, "application/x-netcdf");
        return m;
    }
    if (0 == strncmp(".pma", ext, 4)) {
        strcpy(m, "application/x-perfmon");
        return m;
    }
    if (0 == strncmp(".pmc", ext, 4)) {
        strcpy(m, "application/x-perfmon");
        return m;
    }
    if (0 == strncmp(".pml", ext, 4)) {
        strcpy(m, "application/x-perfmon");
        return m;
    }
    if (0 == strncmp(".pmr", ext, 4)) {
        strcpy(m, "application/x-perfmon");
        return m;
    }
    if (0 == strncmp(".pmw", ext, 4)) {
        strcpy(m, "application/x-perfmon");
        return m;
    }
    if (0 == strncmp(".p12", ext, 4)) {
        strcpy(m, "application/x-pkcs12");
        return m;
    }
    if (0 == strncmp(".pfx", ext, 4)) {
        strcpy(m, "application/x-pkcs12");
        return m;
    }
    if (0 == strncmp(".p7b", ext, 4)) {
        strcpy(m, "application/x-pkcs7-certificates");
        return m;
    }
    if (0 == strncmp(".spc", ext, 4)) {
        strcpy(m, "application/x-pkcs7-certificates");
        return m;
    }
    if (0 == strncmp(".p7r", ext, 4)) {
        strcpy(m, "application/x-pkcs7-certreqresp");
        return m;
    }
    if (0 == strncmp(".p7c", ext, 4)) {
        strcpy(m, "application/x-pkcs7-mime");
        return m;
    }
    if (0 == strncmp(".p7m", ext, 4)) {
        strcpy(m, "application/x-pkcs7-mime");
        return m;
    }
    if (0 == strncmp(".p7s", ext, 4)) {
        strcpy(m, "application/x-pkcs7-signature");
        return m;
    }
    if (0 == strncmp(".sh", ext, 3)) {
        strcpy(m, "application/x-sh");
        return m;
    }
    if (0 == strncmp(".shar", ext, 5)) {
        strcpy(m, "application/x-shar");
        return m;
    }
    if (0 == strncmp(".swf", ext, 4)) {
        strcpy(m, "application/x-shockwave-flash");
        return m;
    }
    if (0 == strncmp(".sit", ext, 4)) {
        strcpy(m, "application/x-stuffit");
        return m;
    }
    if (0 == strncmp(".sv4cpio", ext, 8)) {
        strcpy(m, "application/x-sv4cpio");
        return m;
    }
    if (0 == strncmp(".sv4crc", ext, 7)) {
        strcpy(m, "application/x-sv4crc");
        return m;
    }
    if (0 == strncmp(".tar", ext, 4)) {
        strcpy(m, "application/x-tar");
        return m;
    }
    if (0 == strncmp(".tcl", ext, 4)) {
        strcpy(m, "application/x-tcl");
        return m;
    }
    if (0 == strncmp(".tex", ext, 4)) {
        strcpy(m, "application/x-tex");
        return m;
    }
    if (0 == strncmp(".texi", ext, 5)) {
        strcpy(m, "application/x-texinfo");
        return m;
    }
    if (0 == strncmp(".texinfo", ext, 8)) {
        strcpy(m, "application/x-texinfo");
        return m;
    }
    if (0 == strncmp(".roff", ext, 5)) {
        strcpy(m, "application/x-troff");
        return m;
    }
    if (0 == strncmp(".t", ext, 2)) {
        strcpy(m, "application/x-troff");
        return m;
    }
    if (0 == strncmp(".tr", ext, 3)) {
        strcpy(m, "application/x-troff");
        return m;
    }
    if (0 == strncmp(".man", ext, 4)) {
        strcpy(m, "application/x-troff-man");
        return m;
    }
    if (0 == strncmp(".me", ext, 3)) {
        strcpy(m, "application/x-troff-me");
        return m;
    }
    if (0 == strncmp(".ms", ext, 3)) {
        strcpy(m, "application/x-troff-ms");
        return m;
    }
    if (0 == strncmp(".ustar", ext, 6)) {
        strcpy(m, "application/x-ustar");
        return m;
    }
    if (0 == strncmp(".src", ext, 4)) {
        strcpy(m, "application/x-wais-source");
        return m;
    }
    if (0 == strncmp(".cer", ext, 4)) {
        strcpy(m, "application/x-x509-ca-cert");
        return m;
    }
    if (0 == strncmp(".crt", ext, 4)) {
        strcpy(m, "application/x-x509-ca-cert");
        return m;
    }
    if (0 == strncmp(".der", ext, 4)) {
        strcpy(m, "application/x-x509-ca-cert");
        return m;
    }
    if (0 == strncmp(".pko", ext, 4)) {
        strcpy(m, "application/ynd.ms-pkipko");
        return m;
    }
    if (0 == strncmp(".zip", ext, 4)) {
        strcpy(m, "application/zip");
        return m;
    }
    if (0 == strncmp(".au", ext, 3)) {
        strcpy(m, "audio/basic");
        return m;
    }
    if (0 == strncmp(".snd", ext, 4)) {
        strcpy(m, "audio/basic");
        return m;
    }
    if (0 == strncmp(".mid", ext, 4)) {
        strcpy(m, "audio/mid");
        return m;
    }
    if (0 == strncmp(".rmi", ext, 4)) {
        strcpy(m, "audio/mid");
        return m;
    }
    if (0 == strncmp(".mp3", ext, 4)) {
        strcpy(m, "audio/mpeg");
        return m;
    }
    if (0 == strncmp(".aif", ext, 4)) {
        strcpy(m, "audio/x-aiff");
        return m;
    }
    if (0 == strncmp(".aifc", ext, 5)) {
        strcpy(m, "audio/x-aiff");
        return m;
    }
    if (0 == strncmp(".aiff", ext, 5)) {
        strcpy(m, "audio/x-aiff");
        return m;
    }
    if (0 == strncmp(".m3u", ext, 4)) {
        strcpy(m, "audio/x-mpegurl");
        return m;
    }
    if (0 == strncmp(".ra", ext, 3)) {
        strcpy(m, "audio/x-pn-realaudio");
        return m;
    }
    if (0 == strncmp(".ram", ext, 4)) {
        strcpy(m, "audio/x-pn-realaudio");
        return m;
    }
    if (0 == strncmp(".wav", ext, 4)) {
        strcpy(m, "audio/x-wav");
        return m;
    }
    if (0 == strncmp(".bmp", ext, 4)) {
        strcpy(m, "image/bmp");
        return m;
    }
    if (0 == strncmp(".cod", ext, 4)) {
        strcpy(m, "image/cis-cod");
        return m;
    }
    if (0 == strncmp(".gif", ext, 4)) {
        strcpy(m, "image/gif");
        return m;
    }
    if (0 == strncmp(".ief", ext, 4)) {
        strcpy(m, "image/ief");
        return m;
    }
    if (0 == strncmp(".jpe", ext, 4)) {
        strcpy(m, "image/jpeg");
        return m;
    }
    if (0 == strncmp(".jpeg", ext, 5)) {
        strcpy(m, "image/jpeg");
        return m;
    }
    if (0 == strncmp(".jpg", ext, 4)) {
        strcpy(m, "image/jpeg");
        return m;
    }
    if (0 == strncmp(".jfif", ext, 5)) {
        strcpy(m, "image/pipeg");
        return m;
    }
    if (0 == strncmp(".svg", ext, 4)) {
        strcpy(m, "image/svg+xml");
        return m;
    }
    if (0 == strncmp(".tif", ext, 4)) {
        strcpy(m, "image/tiff");
        return m;
    }
    if (0 == strncmp(".tiff", ext, 5)) {
        strcpy(m, "image/tiff");
        return m;
    }
    if (0 == strncmp(".ras", ext, 4)) {
        strcpy(m, "image/x-cmu-raster");
        return m;
    }
    if (0 == strncmp(".cmx", ext, 4)) {
        strcpy(m, "image/x-cmx");
        return m;
    }
    if (0 == strncmp(".ico", ext, 4)) {
        strcpy(m, "image/x-icon");
        return m;
    }
    if (0 == strncmp(".pnm", ext, 4)) {
        strcpy(m, "image/x-portable-anymap");
        return m;
    }
    if (0 == strncmp(".pbm", ext, 4)) {
        strcpy(m, "image/x-portable-bitmap");
        return m;
    }
    if (0 == strncmp(".pgm", ext, 4)) {
        strcpy(m, "image/x-portable-graymap");
        return m;
    }
    if (0 == strncmp(".ppm", ext, 4)) {
        strcpy(m, "image/x-portable-pixmap");
        return m;
    }
    if (0 == strncmp(".rgb", ext, 4)) {
        strcpy(m, "image/x-rgb");
        return m;
    }
    if (0 == strncmp(".xbm", ext, 4)) {
        strcpy(m, "image/x-xbitmap");
        return m;
    }
    if (0 == strncmp(".xpm", ext, 4)) {
        strcpy(m, "image/x-xpixmap");
        return m;
    }
    if (0 == strncmp(".xwd", ext, 4)) {
        strcpy(m, "image/x-xwindowdump");
        return m;
    }
    if (0 == strncmp(".mht", ext, 4)) {
        strcpy(m, "message/rfc822");
        return m;
    }
    if (0 == strncmp(".mhtml", ext, 6)) {
        strcpy(m, "message/rfc822");
        return m;
    }
    if (0 == strncmp(".nws", ext, 4)) {
        strcpy(m, "message/rfc822");
        return m;
    }
    if (0 == strncmp(".css", ext, 4)) {
        strcpy(m, "text/css");
        return m;
    }
    if (0 == strncmp(".323", ext, 4)) {
        strcpy(m, "text/h323");
        return m;
    }
    if (0 == strncmp(".htm", ext, 4)) {
        strcpy(m, "text/html");
        return m;
    }
    if (0 == strncmp(".html", ext, 5)) {
        strcpy(m, "text/html");
        return m;
    }
    if (0 == strncmp(".stm", ext, 4)) {
        strcpy(m, "text/html");
        return m;
    }
    if (0 == strncmp(".uls", ext, 4)) {
        strcpy(m, "text/iuls");
        return m;
    }
    if (0 == strncmp(".bas", ext, 4)) {
        strcpy(m, "text/plain");
        return m;
    }
    if (0 == strncmp(".c", ext, 2)) {
        strcpy(m, "text/plain");
        return m;
    }
    if (0 == strncmp(".h", ext, 2)) {
        strcpy(m, "text/plain");
        return m;
    }
    if (0 == strncmp(".txt", ext, 4)) {
        strcpy(m, "text/plain");
        return m;
    }
    if (0 == strncmp(".rtx", ext, 4)) {
        strcpy(m, "text/richtext");
        return m;
    }
    if (0 == strncmp(".sct", ext, 4)) {
        strcpy(m, "text/scriptlet");
        return m;
    }
    if (0 == strncmp(".tsv", ext, 4)) {
        strcpy(m, "text/tab-separated-values");
        return m;
    }
    if (0 == strncmp(".htt", ext, 4)) {
        strcpy(m, "text/webviewhtml");
        return m;
    }
    if (0 == strncmp(".htc", ext, 4)) {
        strcpy(m, "text/x-component");
        return m;
    }
    if (0 == strncmp(".etx", ext, 4)) {
        strcpy(m, "text/x-setext");
        return m;
    }
    if (0 == strncmp(".vcf", ext, 4)) {
        strcpy(m, "text/x-vcard");
        return m;
    }
    if (0 == strncmp(".mp2", ext, 4)) {
        strcpy(m, "video/mpeg");
        return m;
    }
    if (0 == strncmp(".mpa", ext, 4)) {
        strcpy(m, "video/mpeg");
        return m;
    }
    if (0 == strncmp(".mpe", ext, 4)) {
        strcpy(m, "video/mpeg");
        return m;
    }
    if (0 == strncmp(".mpeg", ext, 5)) {
        strcpy(m, "video/mpeg");
        return m;
    }
    if (0 == strncmp(".mpg", ext, 4)) {
        strcpy(m, "video/mpeg");
        return m;
    }
    if (0 == strncmp(".mpv2", ext, 4)) {
        strcpy(m, "video/mpeg");
        return m;
    }
    if (0 == strncmp(".mov", ext, 4)) {
        strcpy(m, "video/quicktime");
        return m;
    }
    if (0 == strncmp(".qt", ext, 3)) {
        strcpy(m, "video/quicktime");
        return m;
    }
    if (0 == strncmp(".lsf", ext, 4)) {
        strcpy(m, "video/x-la-asf");
        return m;
    }
    if (0 == strncmp(".lsx", ext, 4)) {
        strcpy(m, "video/x-la-asf");
        return m;
    }
    if (0 == strncmp(".asf", ext, 4)) {
        strcpy(m, "video/x-ms-asf");
        return m;
    }
    if (0 == strncmp(".asr", ext, 4)) {
        strcpy(m, "video/x-ms-asf");
        return m;
    }
    if (0 == strncmp(".asx", ext, 4)) {
        strcpy(m, "video/x-ms-asf");
        return m;
    }
    if (0 == strncmp(".avi", ext, 4)) {
        strcpy(m, "video/x-msvideo");
        return m;
    }
    if (0 == strncmp(".movie", ext, 6)) {
        strcpy(m, "video/x-sgi-movie");
        return m;
    }
    if (0 == strncmp(".flr", ext, 4)) {
        strcpy(m, "x-world/x-vrml");
        return m;
    }
    if (0 == strncmp(".vrml", ext, 5)) {
        strcpy(m, "x-world/x-vrml");
        return m;
    }
    if (0 == strncmp(".wrl", ext, 4)) {
        strcpy(m, "x-world/x-vrml");
        return m;
    }
    if (0 == strncmp(".wrz", ext, 4)) {
        strcpy(m, "x-world/x-vrml");
        return m;
    }
    if (0 == strncmp(".xaf", ext, 4)) {
        strcpy(m, "x-world/x-vrml");
        return m;
    }
    if (0 == strncmp(".xof", ext, 4)) {
        strcpy(m, "x-world/x-vrml");
        return m;
    }
    strcpy(m, "application/octet-stream");
    return m;
}

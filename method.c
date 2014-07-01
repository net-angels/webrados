#include "utceph.h"
#include "protos.h"

uint64_t PUT(rados_ioctx_t io, FCGX_Request *request, char *filename, uint64_t len, int tid) {
    if(NULL == request || NULL == filename || len == 0 || tid == 0)return 0;
    uint64_t i = 0;
    unsigned int 
    written = 0;
    char buffer[BUFFSZ];
    int e;
    while (0 != (written = FCGX_GetStr(buffer, sizeof buffer, request->in))) {
        if (written < 0) {
            logger("[%d] PUT error: %s", tid, strerror(errno));
            return 0;
        }
        e = rados_append(io, filename, (const char *) buffer, written);
        if (e < 0) {
            logger("[%d] Error: %s", tid, strerror(-e));
            rados_ioctx_destroy(io);
            return 0;
        } else {
            i += written;
        }
    }
    return (i > 0) ? i : 0;
}

uint64_t GET(rados_ioctx_t io, FCGX_Request *request, char *filename, int tid) {
    uint64_t osize = 0;
    uint64_t wr = 0;
    uint64_t rd = 0;
    uint64_t off = 0;

    char *mime = xmalloc(32 + 1);
    strncpy(mime, get_mime(filename), 32);

    //logger("[%d] Accessing file: %s", tid, filename);

    int fstat = rados_stat(io, (const char *) filename, &osize, NULL);

    if (0 > fstat) {
        FCGX_FPrintF(request->out, "%s", not_found);
        logger("[%d] Error: could not stat resource [%s]", tid, filename);
        FCGX_FFlush(request->out);
        return 0;
    }
    //logger("[%d] Downloading file size: [%s] %ld", tid,filename,osize);
    FCGX_FPrintF(request->out, "Status: 200 OK\r\nContent-Type: %s\r\nContent-Length: %ld\r\n\r\n", mime, osize);
    if (mime)free(mime);

    while (osize > 0) {
        char buf[8192];
        int rlen = rados_read(io, filename, buf, UMIN(osize, sizeof buf), off);
        if (rlen <= 0) break;
        rd = FCGX_PutStr(buf, rlen, request->out);
        if(rd < 0){
            logger("[%d] GET error: %s", tid, strerror(errno));
            return wr;
        }
        wr += rd;
        FCGX_FFlush(request->out);
        osize -= rlen;
        off += rlen;
    }
    return (wr > 0) ? wr : 0;
}
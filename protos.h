#ifndef PROTOS_H
#define	PROTOS_H

#ifdef	__cplusplus
extern "C" {
#endif
    extern void logger(const char *fmt, ...);
    extern void vlog(const char *fmt, va_list args);
    extern void open_log();
    extern void close_log();

    extern void * xmalloc(size_t size);
    extern void * xrealloc(void *ptr, size_t size);
    extern void * xcalloc(size_t nmemb, size_t size);
    extern void allocate_resources();
    extern void free_resources();
    extern void print_trace(int sig);
    extern bool FileExists(char *path) ;
    extern bool is_whitelisted(char *pool);
    extern void read_config(char *cfg_file);
    extern char *get_mime(char *filename);
    extern void halt();
    extern void *doit(void *ptr);
    extern void set_sig_handler();
    extern void set_thread_signalmask(sigset_t SignalSet);
    extern void fill_current_data(FCGX_Request *,rados_ioctx_t *IO,int);
    extern int create_and_bind();
    extern void setRightOwner(const char *path);
    extern uint64_t PUT(rados_ioctx_t io, FCGX_Request *request, char *filename, uint64_t len,int);
    extern uint64_t GET(rados_ioctx_t io, FCGX_Request *request, char *filename,int);

#ifdef	__cplusplus
}
#endif

#endif	/* PROTOS_H */


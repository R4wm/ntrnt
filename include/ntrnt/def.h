#ifndef _NTRNT_DEF_H_
#define _NTRNT_DEF_H_
//! ----------------------------------------------------------------------------
//! constants
//! ----------------------------------------------------------------------------
#ifndef NTRNT_STATUS_OK
  #define NTRNT_STATUS_OK 0
#endif
#ifndef NTRNT_STATUS_ERROR
  #define NTRNT_STATUS_ERROR -1
#endif
#ifndef NTRNT_STATUS_AGAIN
  #define NTRNT_STATUS_AGAIN -2
#endif
#ifndef NTRNT_STATUS_BUSY
  #define NTRNT_STATUS_BUSY -3
#endif
#ifndef NTRNT_STATUS_DONE
  #define NTRNT_STATUS_DONE -4
#endif
#ifndef NTRNT_ERR_LEN
  #define NTRNT_ERR_LEN 4096
#endif
#ifndef CONFIG_DATE_FORMAT
  #if defined(__APPLE__) || defined(__darwin__)
    #define CONFIG_DATE_FORMAT "%Y-%m-%dT%H:%M:%S"
  #else
    #define CONFIG_DATE_FORMAT "%Y-%m-%dT%H:%M:%S%Z"
  #endif
#endif 
//! ----------------------------------------------------------------------------
//! macros
//! ----------------------------------------------------------------------------
#ifndef NTRNT_PERROR
#define NTRNT_PERROR(...) do { \
    TRC_ERROR(__VA_ARGS__); \
    snprintf(g_ntrnt_err_msg, NTRNT_ERR_LEN, __VA_ARGS__); \
} while(0)
#endif
//! ----------------------------------------------------------------------------
//! global extern
//! ----------------------------------------------------------------------------
extern char g_ntrnt_err_msg[NTRNT_ERR_LEN];
#endif

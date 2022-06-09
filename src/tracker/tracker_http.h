#ifndef _TRACKER_HTTP_H
#define _TRACKER_HTTP_H
//! ----------------------------------------------------------------------------
//! includes
//! ----------------------------------------------------------------------------
#include <stdint.h>
#include <string>
#include "ntrnt/types.h"
#include "tracker/tracker.h"
//! ----------------------------------------------------------------------------
//! constants
//! ----------------------------------------------------------------------------
namespace ns_ntrnt {
//! ----------------------------------------------------------------------------
//! \class: tracker_http
//! ----------------------------------------------------------------------------
class tracker_http: public tracker {
public:
        // -------------------------------------------------
        // public methods
        // -------------------------------------------------
        tracker_http(void);
        ~tracker_http(void);
        int32_t init(const char* a_str, size_t a_str_len);
        virtual int32_t announce(session& a_session, torrent& a_torrent);
private:
        // -------------------------------------------------
        // private methods
        // -------------------------------------------------
        // disallow copy/assign
        tracker_http(const tracker_http&);
        tracker_http& operator=(const tracker_http&);
};
}
#endif

#ifndef _TORRENT_H
#define _TORRENT_H
//! ----------------------------------------------------------------------------
//! includes
//! ----------------------------------------------------------------------------
#include <stdint.h>
#include <string>
#include "ntrnt/types.h"
//! ----------------------------------------------------------------------------
//! constants
//! ----------------------------------------------------------------------------
#ifndef SHA_DIGEST_LENGTH
#define SHA_DIGEST_LENGTH 20
#endif
namespace ns_ntrnt {
//! ----------------------------------------------------------------------------
//! \class: torrent
//! ----------------------------------------------------------------------------
class torrent {
public:
        // -------------------------------------------------
        // public methods
        // -------------------------------------------------
        torrent(void);
        ~torrent(void);
        int32_t init(const char* a_file);
        void display(void);
        const str_list_t& get_accounce_list(void) const { return m_announce_list; }
        const unsigned char* get_info_hash(void) const { return m_info_hash; }
private:
        // -------------------------------------------------
        // private methods
        // -------------------------------------------------
        // disallow copy/assign
        torrent(const torrent&);
        torrent& operator=(const torrent&);
        // -------------------------------------------------
        // private members
        // -------------------------------------------------
        std::string m_announce;
        str_list_t m_announce_list;
        int m_creation_date;
        std::string m_created_by;
        std::string m_encoding;
        std::string m_comment;
        unsigned char m_info_hash[SHA_DIGEST_LENGTH];
        std::string m_info_hash_str;
};
}
#endif

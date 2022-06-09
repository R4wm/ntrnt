//! ----------------------------------------------------------------------------
//! includes
//! ----------------------------------------------------------------------------
// ---------------------------------------------------------
// external includes
// ---------------------------------------------------------
#include "ntrnt/def.h"
// ---------------------------------------------------------
// internal includes
// ---------------------------------------------------------
#include "support/trace.h"
#include "support/ndebug.h"
#include "support/data.h"
#include "support/util.h"
#include "support/net_util.h"
#include "core/torrent.h"
#include "core/session.h"
#include "tracker/tracker_http.h"
#include "tracker/tracker_http_subr.h"
// ---------------------------------------------------------
// ext
// ---------------------------------------------------------
#include "http_parser/http_parser.h"
// ---------------------------------------------------------
// sha1
// ---------------------------------------------------------
#include <openssl/sha.h>
// ---------------------------------------------------------
// std
// ---------------------------------------------------------
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
// ---------------------------------------------------------
// stl
// ---------------------------------------------------------
#include <map>
#include <sstream>
//! ----------------------------------------------------------------------------
//! constants
//! ----------------------------------------------------------------------------
#define _REQUEST_SIZE 16384
namespace ns_ntrnt {
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
static void _http_escape(std::string& ao_out,
                         std::string a_in,
                         bool escape_reserved)
{
        static const std::string ReservedChars = "!*'();:@&=+$,/?%#[]";
        static const std::string UnescapedChars = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-_.~";
        for (auto& ch : a_in)
        {
                if ((UnescapedChars.find(ch) != UnescapedChars.npos) ||
                    (!escape_reserved &&
                    (ReservedChars.find(ch) != ReservedChars.npos)))
                {
                        ao_out += ch;
                }
                else
                {
                        char buf[16];
                        snprintf(buf, sizeof(buf), "%%%02X", (unsigned)(ch & 0xFF));
                        ao_out += buf;
                }
        }
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
static void _encode_digest(char* a_out, const uint8_t* a_digest, size_t a_digest_len)
{
        for (size_t i_c = 0; i_c < a_digest_len; ++i_c)
        {
                // -----------------------------------------
                // see: rfc2396
                //      unreserved  = alphanum | mark
                // -----------------------------------------
                uint8_t l_c = a_digest[i_c];
                if (('0' <= l_c && l_c <= '9') ||
                    ('A' <= l_c && l_c <= 'Z') ||
                    ('a' <= l_c && l_c <= 'z') ||
                    l_c == '.' ||
                    l_c == '-' ||
                    l_c == '_' ||
                    l_c == '~')
                {
                        *a_out++ = (char)l_c;
                }
                else
                {
                        a_out += snprintf(a_out, 4, "%%%02x", (unsigned int)l_c);
                }
        }
        *a_out = '\0';
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
tracker_http::tracker_http(void):
                tracker()
{
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
tracker_http::~tracker_http(void)
{
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t tracker_http::announce(session& a_session, torrent& a_torrent)
{
        // -------------------------------------------------
        // get ipv6
        // -------------------------------------------------
        std::string l_ipv6_str = get_public_address_v6_str();
        std::string l_ipv6_enc;
        _http_escape(l_ipv6_enc, l_ipv6_str, true);
        // -------------------------------------------------
        // create info hash
        // -------------------------------------------------
        char l_info_hash_encoded[64];
        _encode_digest(l_info_hash_encoded, a_torrent.get_info_hash(), SHA_DIGEST_LENGTH);
        // -------------------------------------------------
        // create subrequest
        // -------------------------------------------------
        tracker_http_subr *l_subr = new tracker_http_subr();
        l_subr->m_scheme = m_scheme;
        l_subr->m_port = m_port;
        l_subr->m_host = m_host;
        l_subr->m_path = m_root;
        l_subr->m_verb = "GET";
        // -------------------------------------------------
        // set query string
        // -------------------------------------------------
#if 0
        info_hash=%e1%e5i%d5%d9%bdX%01icP%5b%af%0f%02%16%dc%5d%a0%c9
        peer_id=-TR300Z-mb168nasia07
        port=51413
        uploaded=0
        downloaded=0
        left=1130114013
        numwant=80
        key=8535250
        compact=1
        supportcrypto=1
        event=started
        ipv6=2603:8001:8b01:dd0c:597:6213:dc56:15d9
#endif
        l_subr->set_query("info_hash", l_info_hash_encoded);
        l_subr->set_query("peer_id", get_peer_id().c_str());
        l_subr->set_query("port", "51413");
        l_subr->set_query("uploaded", "0");
        l_subr->set_query("downloaded", "0");
        l_subr->set_query("left", "1130114013");
        l_subr->set_query("numwant", "80");
        l_subr->set_query("key", "8535250");
        l_subr->set_query("compact", "1");
        l_subr->set_query("supportcrypto", "1");
        l_subr->set_query("event", "started");
        l_subr->set_query("ipv6", l_ipv6_enc);
        // -------------------------------------------------
        // enqueue
        // -------------------------------------------------
        int32_t l_s;
        l_s = a_session.enqueue(*l_subr);
        if(l_s != NTRNT_STATUS_OK)
        {
                // TODO --cancel pending...
                return NTRNT_STATUS_ERROR;
        }
        return NTRNT_STATUS_OK;
}
}

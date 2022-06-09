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
#include "core/torrent.h"
#include "tracker/tracker.h"
#include "bencode/bencode.h"
#include "support/ndebug.h"
#include "support/trace.h"
#include "support/util.h"
// ---------------------------------------------------------
// sha1
// ---------------------------------------------------------
#include <openssl/sha.h>
namespace ns_ntrnt {
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
torrent::torrent(void):
        m_announce(),
        m_announce_list(),
        m_creation_date(),
        m_created_by(),
        m_encoding(),
        m_comment(),
        m_info_hash(),
        m_info_hash_str()
{
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
torrent::~torrent(void)
{
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t torrent::init(const char* a_file)
{
        if (!a_file)
        {
                return NTRNT_STATUS_ERROR;
        }
        // -------------------------------------------------
        // bencode decode
        // -------------------------------------------------
        int32_t l_s;
        bencode l_be;
        l_s = l_be.init(a_file);
        if (l_s != NTRNT_STATUS_OK)
        {
                return NTRNT_STATUS_ERROR;
        }
        // -------------------------------------------------
        // find info
        // -------------------------------------------------
        be_dict_t::const_iterator i_obj;
        i_obj = l_be.m_dict.find("info");
        if (i_obj == l_be.m_dict.end())
        {
                NTRNT_PERROR("missing info section in torrent");
                return NTRNT_STATUS_ERROR;
        }
        const be_obj_t& l_info = i_obj->second;
        // -------------------------------------------------
        // get info hash
        // -------------------------------------------------
        SHA1((const unsigned char*)l_info.m_ptr, l_info.m_len, m_info_hash);
        // -------------------------------------------------
        // hex encode
        // -------------------------------------------------
        char* l_buf = nullptr;
        l_s = bin2hex(&l_buf, m_info_hash, SHA_DIGEST_LENGTH);
        if (l_s != NTRNT_STATUS_OK)
        {
                NTRNT_PERROR("performing bin2hex of sha1 hash");
                if (l_buf) { free(l_buf); l_buf = nullptr; }
                return NTRNT_STATUS_ERROR;
        }
        m_info_hash_str.assign(l_buf);
        if (l_buf) { free(l_buf); l_buf = nullptr; }
        // -------------------------------------------------
        // helper
        // -------------------------------------------------
#define _SET_FIELD_STR(_str, _field) do { \
                i_obj = l_be.m_dict.find(_str); \
                if (i_obj != l_be.m_dict.end()) { \
                        const be_obj_t& l_obj = i_obj->second; \
                        if (l_obj.m_type == BE_OBJ_STRING) { \
                                be_string_t* l_be_str = (be_string_t*)(l_obj.m_obj); \
                                _field.assign(l_be_str->m_data, l_be_str->m_len); \
        } } } while(0)
#define _SET_FIELD_INT(_str, _field) do { \
                i_obj = l_be.m_dict.find(_str); \
                if (i_obj != l_be.m_dict.end()) { \
                        const be_obj_t& l_obj = i_obj->second; \
                        if (l_obj.m_type == BE_OBJ_INT) { \
                                be_int_t* l_be_int = (be_int_t*)(l_obj.m_obj); \
                                _field = *l_be_int; \
        } } } while(0)
        // -------------------------------------------------
        // set meta
        // -------------------------------------------------
        _SET_FIELD_STR("announce", m_announce);
        _SET_FIELD_INT("creation date", m_creation_date);
        _SET_FIELD_STR("created by", m_created_by);
        _SET_FIELD_STR("encoding", m_encoding);
        _SET_FIELD_STR("comment", m_comment);
        // -------------------------------------------------
        // announce list
        // -------------------------------------------------
        // ref:
        //   http://bittorrent.org/beps/bep_0012.html
        // TODO this impl is terrible -something like
        // json lib get by dict "[<key>]" would bencode ideal
        // -------------------------------------------------
        i_obj = l_be.m_dict.find("announce-list");
        if (i_obj != l_be.m_dict.end())
        {
                const be_obj_t& l_obj = i_obj->second;
                if (l_obj.m_type == BE_OBJ_LIST)
                {
                        be_list_t* l_bl = (be_list_t*)(l_obj.m_obj);
                        for(auto && i_l : *l_bl)
                        {
                                if (i_l.m_type == BE_OBJ_LIST)
                                {
                                        be_list_t* i_ll = (be_list_t*)(i_l.m_obj);
                                        for(auto && i_t : *i_ll)
                                        {
                                                if (i_t.m_type == BE_OBJ_STRING)
                                                {
                                                        std::string l_t;
                                                        be_string_t* l_str = (be_string_t*)(i_t.m_obj);
                                                        l_t.assign(l_str->m_data, l_str->m_len);
                                                        m_announce_list.push_back(l_t);
                                                }
                                        }
                                }
                        }
                }
        }
        return NTRNT_STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
void torrent::display(void)
{
        NDBG_OUTPUT("+-----------------------------------------------------------\n");
        NDBG_OUTPUT("|             T O R R E N T   M E T A   I N F O\n");
        NDBG_OUTPUT("+-----------------------------------------------------------\n");
        NDBG_OUTPUT("| announce:      %s\n", m_announce.c_str());
        NDBG_OUTPUT("| announce_list: \n");
        for(auto && i_m : m_announce_list)
        {
        NDBG_OUTPUT("|                %s\n", i_m.c_str());
        }
        NDBG_OUTPUT("| creation_date: %d\n", m_creation_date);
        NDBG_OUTPUT("| created_by:    %s\n", m_created_by.c_str());
        NDBG_OUTPUT("| encoding:      %s\n", m_encoding.c_str());
        NDBG_OUTPUT("| comment:       %s\n", m_comment.c_str());
        NDBG_OUTPUT("| info_hash:     %s\n", m_info_hash_str.c_str());
        NDBG_OUTPUT("+-----------------------------------------------------------\n");
}
}

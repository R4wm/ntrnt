//! ----------------------------------------------------------------------------
//! includes
//! ----------------------------------------------------------------------------
#include "bencode/bencode.h"
#include "support/util.h"
#include "support/trace.h"
#include "support/ndebug.h"
// ---------------------------------------------------------
// std libs
// ---------------------------------------------------------
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
//! ----------------------------------------------------------------------------
//! constants
//! ----------------------------------------------------------------------------
#define _DISPLAY_INDENT 2
//! ----------------------------------------------------------------------------
//! macros
//! ----------------------------------------------------------------------------
#define _INCR_PTR() do { \
        ++m_cur_off; \
        ++m_cur_ptr; \
} while(0)
#define _INCR_PTR_BY(_len) do { \
        m_cur_off += _len; \
        m_cur_ptr += _len; \
} while(0)
namespace ns_ntrnt {
//! ----------------------------------------------------------------------------
//! "bencoding types"
//! ref: http://www.bittorrent.org/beps/bep_0003.html
//! ----------------------------------------------------------------------------
//!
//! *************************************************
//! ****** S T R I N G S ****************************
//! *************************************************
//! Strings are length-prefixed base ten followed by a colon and the string.
//! For example 4:spam corresponds to 'spam'.
//!
//! *************************************************
//! ****** I N T E G E R S **************************
//! *************************************************
//! Integers are represented by an 'i' followed by the number in base 10
//! followed by an 'e'.
//! For example i3e corresponds to 3 and i-3e corresponds to -3.
//! Integers have no size limitation. i-0e is invalid.
//! All encodings with a leading zero, such as i03e, are invalid, other than
//! i0e, which of course corresponds to 0.
//!
//! *************************************************
//! ******* L I S T S *******************************
//! *************************************************
//! Lists are encoded as an 'l' followed by their elements (also bencoded)
//! followed by an 'e'.
//! For example l4:spam4:eggse corresponds to
//! ['spam', 'eggs'].
//!
//! *************************************************
//! ******* D I C T I O N A R I E S *****************
//! *************************************************
//! Dictionaries are encoded as a 'd' followed by a list of alternating keys
//! and their corresponding values followed by an 'e'.
//! For example,
//! d3:cow3:moo4:spam4:eggse
//! corresponds to
//! {'cow': 'moo', 'spam': 'eggs'}
//! and
//! d4:spaml1:a1:bee
//! corresponds to
//! {'spam': ['a', 'b']}.
//! Keys must bencode strings and appear in sorted order
//! (sorted as raw strings, not alphanumerics).
//!
//! ----------------------------------------------------------------------------
//! ----------------------------------------------------------------------------
//! static util
//! ----------------------------------------------------------------------------
static void delete_obj(be_obj_t& ao_obj);
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
static void delete_list(be_list_t& ao_list)
{
        for(auto && i_m : ao_list)
        {
                be_obj_t& i_obj = i_m;
                delete_obj(i_obj);
        }
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
static void delete_dict(be_dict_t& ao_dict)
{
        for(auto && i_m : ao_dict)
        {
                be_obj_t& i_obj = i_m.second;
                delete_obj(i_obj);
        }
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
static void delete_obj(be_obj_t& ao_obj)
{
        switch (ao_obj.m_type)
        {
        // -------------------------------------------------
        // INT
        // -------------------------------------------------
        case BE_OBJ_INT:
        {
                be_int_t* l_obj = (be_int_t*)ao_obj.m_obj;
                delete l_obj;
                break;
        }
        // -------------------------------------------------
        // STRING
        // -------------------------------------------------
        case BE_OBJ_STRING:
        {
                be_string_t* l_obj = (be_string_t*)ao_obj.m_obj;
                delete l_obj;
                break;
        }
        // -------------------------------------------------
        // LIST
        // -------------------------------------------------
        case BE_OBJ_LIST:
        {
                be_list_t* l_obj = (be_list_t*)ao_obj.m_obj;
                delete_list(*l_obj);
                delete l_obj;
                break;
        }
        // -------------------------------------------------
        // DICT
        // -------------------------------------------------
        case BE_OBJ_DICT:
        {
                be_dict_t* l_obj = (be_dict_t*)ao_obj.m_obj;
                delete_dict(*l_obj);
                delete l_obj;
                break;
        }
        // -------------------------------------------------
        // default
        // -------------------------------------------------
        default:
        {
                break;
        }
        }
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
bencode::bencode(void):
        m_dict(),
        m_buf(nullptr),
        m_buf_len(0),
        m_cur_off(0),
        m_cur_ptr(nullptr)
{
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
bencode::~bencode(void)
{
        if (m_buf)
        {
                free(m_buf);
                m_buf = nullptr;
                m_buf_len = 0;
        }
        // -------------------------------------------------
        // delete dict
        // -------------------------------------------------
        delete_dict(m_dict);
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t bencode::init(void)
{
        // -------------------------------------------------
        // init
        // -------------------------------------------------
        m_cur_off = 0;
        m_cur_ptr = m_buf;
        // -------------------------------------------------
        // verify dict
        // -------------------------------------------------
        if (*m_cur_ptr != 'd')
        {
                NTRNT_PERROR("metainfo file does not appear to bencode a dict -no preceding 'd'");
                return NTRNT_STATUS_ERROR;
        }
        // -------------------------------------------------
        // parse
        // -------------------------------------------------
        int32_t l_s;
        _INCR_PTR();
        l_s = parse_dict(m_dict);
        if (l_s != NTRNT_STATUS_OK)
        {
                // TODO ERROR
                return NTRNT_STATUS_ERROR;
        }
        return NTRNT_STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t bencode::init(const char* a_buf, size_t a_len)
{
        if (!a_buf ||
            !a_len)
        {
                return NTRNT_STATUS_ERROR;
        }
        // -------------------------------------------------
        // copy in
        // -------------------------------------------------
        m_buf = (char *)malloc(sizeof(char)*a_len);
        memcpy(m_buf, a_buf, a_len);
        m_buf_len = a_len;
        // -------------------------------------------------
        // init
        // -------------------------------------------------
        return init();
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t bencode::init(const char* a_file)
{
        int32_t l_s;
        // -------------------------------------------------
        // read in file
        // -------------------------------------------------
        l_s = read_file(a_file, &m_buf, &m_buf_len);
        if (l_s != NTRNT_STATUS_OK)
        {
                // TODO ERROR
                return NTRNT_STATUS_ERROR;
        }
        // -------------------------------------------------
        // init
        // -------------------------------------------------
        return init();
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t bencode::parse_len(size_t& ao_len)
{
        // -------------------------------------------------
        // expect length
        // -------------------------------------------------
        const char *l_len_begin = m_cur_ptr;
        while (isdigit(*m_cur_ptr))
        {
                _INCR_PTR();
        }
        char *l_len_end = const_cast<char *>(m_cur_ptr);
        // -------------------------------------------------
        // find skip delim
        // -------------------------------------------------
        if (*m_cur_ptr != ':')
        {
                // TODO ERROR
                NDBG_PRINT("error\n");
                return NTRNT_STATUS_ERROR;
        }
        _INCR_PTR();
        // -------------------------------------------------
        // convert len
        // -------------------------------------------------
        errno = 0;
        ao_len = strtoul(l_len_begin, &l_len_end, 10);
        if (ao_len == ULONG_MAX)
        {
                // TODO ERROR
                NDBG_PRINT("error\n");
                return NTRNT_STATUS_ERROR;
        }
        if (errno != 0)
        {
                // TODO ERROR
                NDBG_PRINT("error\n");
                return NTRNT_STATUS_ERROR;
        }
        return NTRNT_STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t bencode::parse_obj(be_obj_t& ao_obj)
{
        int32_t l_s = NTRNT_STATUS_OK;
        // -------------------------------------------------
        // setup buf
        // -------------------------------------------------
        char l_type = *m_cur_ptr;
        // -------------------------------------------------
        // for type
        // -------------------------------------------------
        // -------------------------------------------------
        // string
        // -------------------------------------------------
        if (isdigit(l_type))
        {
                // -----------------------------------------
                // parse
                // -----------------------------------------
                be_string_t* l_str = new be_string_t();
                l_s = parse_string(*l_str);
                if (l_s != NTRNT_STATUS_OK)
                {
                        // TODO ERROR
                        return NTRNT_STATUS_ERROR;
                }
                // -----------------------------------------
                // set type
                // -----------------------------------------
                ao_obj.m_type = BE_OBJ_STRING;
                ao_obj.m_obj = l_str;
        }
        // -------------------------------------------------
        // integer
        // -------------------------------------------------
        else if (l_type == 'i')
        {
                // -----------------------------------------
                // parse
                // -----------------------------------------
                _INCR_PTR();
                be_int_t* l_int = new be_int_t();
                l_s = parse_int(*l_int);
                if (l_s != NTRNT_STATUS_OK)
                {
                        // TODO ERROR
                        return NTRNT_STATUS_ERROR;
                }
                // -----------------------------------------
                // set type
                // -----------------------------------------
                ao_obj.m_type = BE_OBJ_INT;
                ao_obj.m_obj = l_int;
        }
        // -------------------------------------------------
        // list
        // -------------------------------------------------
        else if (l_type == 'l')
        {
                // -----------------------------------------
                // parse
                // -----------------------------------------
                _INCR_PTR();
                be_list_t* l_list = new be_list_t();
                l_s = parse_list(*l_list);
                if (l_s != NTRNT_STATUS_OK)
                {
                        // TODO ERROR
                        return NTRNT_STATUS_ERROR;
                }
                // -----------------------------------------
                // set type
                // -----------------------------------------
                ao_obj.m_type = BE_OBJ_LIST;
                ao_obj.m_obj = l_list;
        }
        // -------------------------------------------------
        // dictionary
        // -------------------------------------------------
        else if (l_type == 'd')
        {
                // -----------------------------------------
                // parse
                // -----------------------------------------
                _INCR_PTR();
                be_dict_t* l_dict = new be_dict_t();
                l_s = parse_dict(*l_dict);
                if (l_s != NTRNT_STATUS_OK)
                {
                        // TODO ERROR
                        return NTRNT_STATUS_ERROR;
                }
                // -----------------------------------------
                // set type
                // -----------------------------------------
                ao_obj.m_type = BE_OBJ_DICT;
                ao_obj.m_obj = l_dict;
        }
        // -------------------------------------------------
        // default
        // -------------------------------------------------
        else
        {
                // TODO ERROR
                return NTRNT_STATUS_ERROR;
        }
        return NTRNT_STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t bencode::parse_int(be_int_t& ao_int)
{
        // -------------------------------------------------
        // expect length
        // -------------------------------------------------
        const char *l_len_begin = m_cur_ptr;
        while ((*m_cur_ptr == '-') ||
               isdigit(*m_cur_ptr))
        {
                _INCR_PTR();
        }
        char *l_len_end = const_cast<char *>(m_cur_ptr);
        // -------------------------------------------------
        // find skip delim
        // -------------------------------------------------
        if (*m_cur_ptr != 'e')
        {
                // TODO ERROR
                return NTRNT_STATUS_ERROR;
        }
        _INCR_PTR();
        // -------------------------------------------------
        // convert len
        // -------------------------------------------------
        errno = 0;
        ao_int = strtol(l_len_begin, &l_len_end, 10);
        if (ao_int == LONG_MAX)
        {
                // TODO ERROR
                return NTRNT_STATUS_ERROR;
        }
        if (errno != 0)
        {
                // TODO ERROR
                return NTRNT_STATUS_ERROR;
        }
        return NTRNT_STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t bencode::parse_string(be_string_t& ao_string)
{
        int32_t l_s = NTRNT_STATUS_OK;
        // -------------------------------------------------
        // expect length
        // -------------------------------------------------
        size_t l_len;
        l_s = parse_len(l_len);
        if (l_s != NTRNT_STATUS_OK)
        {
                return NTRNT_STATUS_ERROR;
        }
        // -------------------------------------------------
        // read in string
        // -------------------------------------------------
        ao_string.m_data = m_cur_ptr;
        ao_string.m_len = l_len;
        _INCR_PTR_BY(l_len);
        return NTRNT_STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t bencode::parse_list(be_list_t& ao_list)
{
        // -------------------------------------------------
        // while not end
        // -------------------------------------------------
        int32_t l_s = NTRNT_STATUS_OK;
        bool l_end = false;
        while (!l_end)
        {
                // -----------------------------------------
                // check for end
                // -----------------------------------------
                if (*m_cur_ptr == 'e')
                {
                        _INCR_PTR();
                        goto done;
                }
                // -----------------------------------------
                // parse obj
                // -----------------------------------------
                be_obj_t l_be_obj;
                l_s = parse_obj(l_be_obj);
                if (l_s != NTRNT_STATUS_OK)
                {
                        // TODO ERROR
                        return NTRNT_STATUS_ERROR;
                }
                // -----------------------------------------
                // append
                // -----------------------------------------
                ao_list.push_back(l_be_obj);
        }
done:
        return NTRNT_STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t bencode::parse_dict(be_dict_t& a_be_dict)
{
        // -------------------------------------------------
        // while not end
        // -------------------------------------------------
        int32_t l_s = NTRNT_STATUS_OK;
        bool l_end = false;
        while (!l_end)
        {
                // -----------------------------------------
                // check for end
                // -----------------------------------------
                if (*m_cur_ptr == 'e')
                {
                        _INCR_PTR();
                        goto done;
                }
                // -----------------------------------------
                // read in key
                // -----------------------------------------
                be_string_t l_key;
                l_s = parse_string(l_key);
                if (l_s != NTRNT_STATUS_OK)
                {
                        // TODO ERROR
                        return NTRNT_STATUS_ERROR;
                }
                std::string l_key_str;
                l_key_str.assign(l_key.m_data, l_key.m_len);
                // -----------------------------------------
                // parse obj
                // -----------------------------------------
                be_obj_t l_be_obj;
                l_be_obj.m_ptr = m_cur_ptr;
                l_s = parse_obj(l_be_obj);
                if (l_s != NTRNT_STATUS_OK)
                {
                        // TODO ERROR
                        return NTRNT_STATUS_ERROR;
                }
                l_be_obj.m_len = m_cur_ptr - l_be_obj.m_ptr;
                // -----------------------------------------
                // append
                // -----------------------------------------
                a_be_dict[l_key_str] = l_be_obj;
        }
done:
        return NTRNT_STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
void bencode::display_obj(const be_obj_t& a_obj, uint16_t a_indent)
{
        switch (a_obj.m_type)
        {
        // -------------------------------------------------
        // INT
        // -------------------------------------------------
        case BE_OBJ_INT:
        {
                const be_int_t& l_obj = *((const be_int_t*)a_obj.m_obj);
                NDBG_OUTPUT("%*c[INT]: %d\n", a_indent, ' ', l_obj);
                break;
        }
        // -------------------------------------------------
        // STRING
        // -------------------------------------------------
        case BE_OBJ_STRING:
        {
                const be_string_t& l_obj = *((const be_string_t*)a_obj.m_obj);
                NDBG_OUTPUT("%*c[STR]: %.*s\n", a_indent, ' ', (int)l_obj.m_len, l_obj.m_data);
                break;
        }
        // -------------------------------------------------
        // LIST
        // -------------------------------------------------
        case BE_OBJ_LIST:
        {
                const be_list_t& l_obj = *((const be_list_t*)a_obj.m_obj);
                NDBG_OUTPUT("%*c[LST]: -------------------> BEGIN\n", a_indent, ' ');
                display_list(l_obj, a_indent+_DISPLAY_INDENT);
                NDBG_OUTPUT("%*c[LST]: -------------------> END\n", a_indent, ' ');
                break;
        }

        // -------------------------------------------------
        // DICT
        // -------------------------------------------------
        case BE_OBJ_DICT:
        {
                const be_dict_t& l_obj = *((const be_dict_t*)a_obj.m_obj);
                NDBG_OUTPUT("%*c[DCT]: -------------------> BEGIN\n", a_indent, ' ');
                display_dict(l_obj, a_indent+_DISPLAY_INDENT);
                NDBG_OUTPUT("%*c[DCT]: -------------------> END\n", a_indent, ' ');
                break;
        }
        // -------------------------------------------------
        // default
        // -------------------------------------------------
        default:
        {
                break;
        }
        }
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
void bencode::display_dict(const be_dict_t& a_dict, uint16_t a_indent)
{
        for(auto && i_m : a_dict)
        {
                const be_obj_t& i_obj = i_m.second;
                NDBG_OUTPUT("%*c[KEY]: %s\n", a_indent+_DISPLAY_INDENT, ' ', i_m.first.c_str());
                display_obj(i_obj, a_indent+_DISPLAY_INDENT+_DISPLAY_INDENT);
        }
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
void bencode::display_list(const be_list_t& a_list, uint16_t a_indent)
{
        for(auto && i_m : a_list)
        {
                const be_obj_t& i_obj = i_m;
                display_obj(i_obj, a_indent+_DISPLAY_INDENT);
        }
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
void bencode::display(void)
{
        display_dict(m_dict, 0);
}
}

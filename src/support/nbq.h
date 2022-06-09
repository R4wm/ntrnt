#ifndef _NBQ_H
#define _NBQ_H
//! ----------------------------------------------------------------------------
//! includes
//! ----------------------------------------------------------------------------
// ---------------------------------------------------------
// internal
// ---------------------------------------------------------
#include "http/http_status.h"
// ---------------------------------------------------------
// std libs
// ---------------------------------------------------------
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
// ---------------------------------------------------------
// stl
// ---------------------------------------------------------
#include <list>
namespace ns_ntrnt {
struct nb_struct;
typedef struct nb_struct nb_t;
typedef std::list <nb_t *> nb_list_t;
//! ----------------------------------------------------------------------------
//! \details: nbq
//! ----------------------------------------------------------------------------
class nbq
{
public:
        // -------------------------------------------------
        // public methods
        // -------------------------------------------------
        nbq(uint32_t a_bsize);
        ~nbq();
        // Getters
        uint64_t get_cur_write_offset(void) { return m_cur_write_offset;}
        // Writing...
        int64_t write(const char *a_buf, uint64_t a_len);
        int64_t write_fd(int a_fd, uint64_t a_len, ssize_t &a_status);
        int64_t write_q(nbq &a_q);
        // Reading
        char peek(void) const;
        int64_t read(char *a_buf, uint64_t a_len);
        uint64_t read_seek(uint64_t a_off);
        uint64_t read_from(uint64_t a_off, char *a_buf, uint64_t a_len);
        uint64_t read_avail(void) const {return m_total_read_avail;}
        // Resetting...
        void reset_read(void);
        void reset_write(void);
        void reset(void);
        // Shrink -free all read blocks
        void shrink(void);
        // Print
        void print(void);
        // Split and create separate nbq with tail at offset
        int32_t split(nbq **ao_nbq_tail, uint64_t a_offset);
        // Join nbq with reference nbq
        int32_t join_ref(const nbq &ao_nbq_tail);
        // Block Writing...
        char * b_write_ptr(void);
        char * b_write_data_ptr(void);
        uint32_t b_write_avail(void);
        int32_t b_write_add_avail();
        void b_write_incr(uint32_t a_len);
        // Block Reading...
        char *b_read_ptr(void) const;
        int32_t b_read_avail(void) const;
        void b_read_incr(uint32_t a_len);
        // Debugging display all
        void b_display_all(void);
        void b_display_written(void);
        // For use with obj pool
        uint64_t get_idx(void) {return m_idx;}
        void set_idx(uint64_t a_idx) {m_idx = a_idx;}
        bool read_avail_is_max_limit(void)
        {
                if((m_max_read_queue > 0) &&
                   (m_total_read_avail >= (uint64_t)m_max_read_queue))
                {
                        return true;
                }
                return false;
        }
        // set max read size
        int64_t get_max_read_queue(void) { return m_max_read_queue;}
        void set_max_read_queue(int64_t a_val) {m_max_read_queue = a_val;}
private:
        // -------------------------------------------------
        // private methods
        // -------------------------------------------------
        // Disallow copy/assign
        nbq& operator=(const nbq &);
        nbq(const nbq &);
        // -------------------------------------------------
        // private members
        // -------------------------------------------------
        // Block list
        nb_list_t m_q;
        // Block size
        uint32_t m_bsize;
        // cur write/read blocks
        nb_list_t::iterator m_cur_write_block;
        nb_list_t::iterator m_cur_read_block;
        // For use with obj pool
        uint64_t m_idx;
        // internal acct'ing
        uint64_t m_cur_write_offset;
        uint64_t m_total_read_avail;
        int64_t m_max_read_queue;
};
//! ----------------------------------------------------------------------------
//! util
//! ----------------------------------------------------------------------------
char *copy_part(nbq &a_nbq, uint64_t a_off, uint64_t a_len);
void print_part(nbq &a_nbq, uint64_t a_off, uint64_t a_len);
//! ----------------------------------------------------------------------------
//! writing utilities
//! ----------------------------------------------------------------------------
int32_t nbq_write_request_line(nbq &ao_q, const char *a_buf, uint32_t a_len);
int32_t nbq_write_status(nbq &ao_q, http_status_t a_status);
int32_t nbq_write_header(nbq &ao_q,
                         const char *a_key_buf, uint32_t a_key_len,
                         const char *a_val_buf, uint32_t a_val_len);
int32_t nbq_write_header(nbq &ao_q, const char *a_key_buf, const char *a_val_buf);
int32_t nbq_write_body(nbq &ao_q, const char *a_buf, uint32_t a_len);
} // ns_ntrnt
#endif

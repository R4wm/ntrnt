#ifndef _UTIL_H
#define _UTIL_H
//! ----------------------------------------------------------------------------
//! includes
//! ----------------------------------------------------------------------------
#include <stddef.h>
#include <stdint.h>
// ---------------------------------------------------------
// stl
// ---------------------------------------------------------
#include <string>
//! ----------------------------------------------------------------------------
//! methods
//! ----------------------------------------------------------------------------
namespace ns_ntrnt
{
int32_t read_file(const char* a_file, char** a_buf, size_t* a_len);
int32_t b64_encode(char** ao_out, const unsigned char* a_in, size_t a_in_len);
int32_t b64_encode(std::string& ao_out, const unsigned char* a_in, size_t a_in_len);
int32_t bin2hex(char** ao_out, const unsigned char *a_bin, size_t a_len);
std::string rand_str(const size_t a_len);
const std::string& get_peer_id(void);
}
#endif

//! ----------------------------------------------------------------------------
//! includes
//! ----------------------------------------------------------------------------
#include <stddef.h>
#include <stdio.h>
#include <time.h>
#include "dht/dht.h"
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int dht_sendto(int sockfd,
               const void *buf,
               int len,
               int flags,
               const struct sockaddr *to,
               int tolen)
{
        // TODO
        return 0;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int dht_blacklisted(const struct sockaddr *sa, int salen)
{
        // TODO
        return 0;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
void dht_hash(void *hash_return,
              int hash_size,
              const void *v1,
              int len1,
              const void *v2,
              int len2,
              const void *v3,
              int len3)
{
        // TODO
        return;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int dht_random_bytes(void *buf, size_t size)
{
        // TODO
        return 0;
}


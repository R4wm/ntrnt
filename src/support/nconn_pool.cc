//! ----------------------------------------------------------------------------
//! includes
//! ----------------------------------------------------------------------------
#include "support/nconn_pool.h"
#include "support/ndebug.h"
#include "conn/nconn_tcp.h"
#include "conn/nconn_tls.h"
#include "ntrnt/def.h"
#include "support/trace.h"
namespace ns_ntrnt {
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
nconn_pool::nconn_pool(uint64_t a_max_active_size,
                       uint64_t a_max_idle_size):
                       m_initd(false),
                       m_active_conn_map(),
                       m_active_conn_map_size(0),
                       m_active_conn_map_max_size(a_max_active_size),
                       m_idle_conn_lru(a_max_idle_size)
{
        //NDBG_PRINT("a_max_active_size: %d\n", a_max_active_size);
        //NDBG_PRINT("a_max_idle_size:   %d\n", a_max_idle_size);
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
nconn_pool::~nconn_pool(void)
{
        // -------------------------------------------------
        // remote all active
        // -------------------------------------------------
        for(active_conn_map_t::const_iterator i_l = m_active_conn_map.begin();
            i_l != m_active_conn_map.end();
            ++i_l)
        {
                for(nconn_set_t::const_iterator i_c = i_l->second.begin();
                    i_c != i_l->second.end();
                    ++i_c)
                {
                        m_reap_list.push_back(*i_c);
                }
        }
        evict_all_idle();
        reap();
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
void nconn_pool::evict_all_idle(void)
{
        while(m_idle_conn_lru.size())
        {
                m_idle_conn_lru.evict();
        }
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
nconn *nconn_pool::get_new_active(const std::string &a_label, scheme_t a_scheme)
{
        if(!m_initd)
        {
                init();
        }
        if(m_active_conn_map_size >= m_active_conn_map_max_size)
        {
                return NULL;
        }
        nconn *l_nconn = s_create_new_conn(a_scheme);
        l_nconn->set_label(a_label);
        int32_t l_s;
        l_s = add_active(l_nconn);
        if(l_s != NTRNT_STATUS_OK)
        {
                TRC_ERROR("Error performing add_active.\n");
                //return NULL;
        }
        return l_nconn;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
uint64_t nconn_pool::get_active_size(void)
{
        return m_active_conn_map_size;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
uint64_t nconn_pool::get_active_available(void)
{
        return m_active_conn_map_max_size - m_active_conn_map_size;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
uint64_t nconn_pool::get_active_label(const std::string &a_label)
{
        active_conn_map_t::const_iterator i_l;
        if((i_l = m_active_conn_map.find(a_label)) != m_active_conn_map.end())
        {
                return i_l->second.size();
        }
        return 0;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
nconn *nconn_pool::get_idle(const std::string &a_label)
{
        if(!m_initd)
        {
                init();
        }
        //NDBG_PRINT("m_idle_conn_lru size: %lu\n", m_idle_conn_lru.size());
        nconn* l_c = m_idle_conn_lru.get(a_label);
        if(l_c)
        {
                add_active(l_c);
                //NDBG_PRINT("%s__GET__%s: nconn: %p\n", ANSI_COLOR_FG_BLUE, ANSI_COLOR_OFF, l_c);
                return l_c;
        }
        return NULL;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
uint64_t nconn_pool::get_idle_size(void)
{
        return m_idle_conn_lru.size();
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t nconn_pool::add_idle(nconn *a_nconn)
{
        if(!m_initd)
        {
                init();
        }
        if(!a_nconn)
        {
                TRC_ERROR("a_nconn == NULL\n");
                return NTRNT_STATUS_ERROR;
        }
        int32_t l_s;
        l_s = remove_active(a_nconn);
        if(l_s != NTRNT_STATUS_OK)
        {
                TRC_ERROR("Error performing remove_active.\n");
                return NTRNT_STATUS_ERROR;
        }
        id_t l_id;
        l_id = m_idle_conn_lru.insert(a_nconn->get_label(), a_nconn);
        a_nconn->set_id(l_id);
        //NDBG_PRINT("%sADD_IDLE%s: nconn: %p size: %lu LABEL: %s ID: %u\n",
        //              ANSI_COLOR_FG_GREEN, ANSI_COLOR_OFF,
        //              a_nconn,
        //              m_idle_conn_lru.size(),
        //              a_nconn->get_label().c_str(),
        //              l_id);
        return NTRNT_STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t nconn_pool::release(nconn *a_nconn)
{
        if(!a_nconn)
        {
                TRC_ERROR("a_nconn == NULL\n");
                return NTRNT_STATUS_ERROR;
        }
        //NDBG_PRINT("%sRELEASE%s:\n", ANSI_COLOR_FG_MAGENTA, ANSI_COLOR_OFF);
        if(!m_initd)
        {
                init();
        }
        int32_t l_s;
        l_s = remove_active(a_nconn);
        if(l_s != NTRNT_STATUS_OK)
        {
                //NDBG_PRINT("Error performing remove_active\n");
                //return NTRNT_STATUS_ERROR;
        }
        l_s = remove_idle(a_nconn);
        if(l_s != NTRNT_STATUS_OK)
        {
                //NDBG_PRINT("Error performing remove_idle\n");
                //return NTRNT_STATUS_ERROR;
        }
        l_s = cleanup(a_nconn);
        if(l_s != NTRNT_STATUS_OK)
        {
                //NDBG_PRINT("Error performing cleanup\n");
                //return NTRNT_STATUS_ERROR;
        }
        return NTRNT_STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
nconn *nconn_pool::s_create_new_conn(scheme_t a_scheme)
{
        nconn *l_nconn = NULL;
        //NDBG_PRINT("CREATING NEW CONNECTION: a_scheme: %d\n", a_scheme);
        if(a_scheme == SCHEME_TCP)
        {
                l_nconn = new nconn_tcp();
        }
        else if(a_scheme == SCHEME_TLS)
        {
                l_nconn = new nconn_tls();
        }
        return l_nconn;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int nconn_pool::s_delete_cb(void* o_1, void *a_2)
{
        nconn_pool *l_nconn_pool = reinterpret_cast<nconn_pool *>(o_1);
        nconn *l_nconn = reinterpret_cast<nconn *>(a_2);
        int32_t l_s;
        l_s = l_nconn_pool->cleanup(l_nconn);
        if(l_s != NTRNT_STATUS_OK)
        {
                //NDBG_PRINT("Error performing cleanup\n");
                //return NTRNT_STATUS_ERROR;
        }
        return NTRNT_STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
void nconn_pool::init(void)
{
        m_idle_conn_lru.set_delete_cb(s_delete_cb, this);
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t nconn_pool::add_active(nconn *a_nconn)
{
        active_conn_map_t::iterator i_cl = m_active_conn_map.find(a_nconn->get_label());
        if(i_cl != m_active_conn_map.end())
        {
                i_cl->second.insert(a_nconn);
        }
        else
        {
                nconn_set_t l_cs;
                l_cs.insert(a_nconn);
                m_active_conn_map[a_nconn->get_label()] = l_cs;
        }
        ++m_active_conn_map_size;
        return NTRNT_STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t nconn_pool::remove_active(nconn *a_nconn)
{
        //if(m_active_conn_map.find(a_nconn->get_label()) != m_active_conn_map.end())
        //{
        //        --m_active_conn_map[a_nconn->get_label()];
        //}
        active_conn_map_t::iterator i_cl = m_active_conn_map.find(a_nconn->get_label());
        if(i_cl == m_active_conn_map.end())
        {
                return NTRNT_STATUS_ERROR;
        }
        nconn_set_t::iterator i_n = i_cl->second.find(a_nconn);
        if(i_n == i_cl->second.end())
        {
                return NTRNT_STATUS_ERROR;
        }
        i_cl->second.erase(i_n);
        if(!i_cl->second.size())
        {
                m_active_conn_map.erase(i_cl);
        }
        --m_active_conn_map_size;
        return NTRNT_STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t nconn_pool::remove_idle(nconn *a_nconn)
{
        if(m_idle_conn_lru.size() &&
           a_nconn)
        {
                uint64_t l_id = a_nconn->get_id();
                m_idle_conn_lru.remove(l_id);
                //NDBG_PRINT("%sDEL_IDLE%s: size: %lu ID: %lu\n", ANSI_COLOR_FG_RED, ANSI_COLOR_OFF,
                //           m_idle_conn_lru.size(),
                //           l_id);
        }
        return NTRNT_STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t nconn_pool::cleanup(nconn *a_nconn)
{
        if(!a_nconn)
        {
                TRC_ERROR("Error a_nconn == NULL\n");
                return NTRNT_STATUS_ERROR;
        }
        if(!m_initd)
        {
                init();
        }
        int32_t l_s;
        l_s = a_nconn->nc_cleanup();
        if(l_s != NTRNT_STATUS_OK)
        {
                //NDBG_PRINT("Error perfrorming a_nconn->nc_cleanup()\n");
                //return NTRNT_STATUS_ERROR;
        }
        m_reap_list.push_back(a_nconn);
        return NTRNT_STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
void nconn_pool::reap(void)
{
        if(m_reap_list.empty())
        {
                return;
        }
        for(nconn_list_t::iterator i_c = m_reap_list.begin();
            i_c != m_reap_list.end();
            ++i_c)
        {
                if(*i_c)
                {
                        delete *i_c;
                        *i_c = NULL;
                }
        }
        m_reap_list.clear();
        return;
}
} //namespace ns_ntrnt {

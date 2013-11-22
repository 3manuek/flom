/*
 * Copyright (c) 2013, Christian Ferrari <tiian@users.sourceforge.net>
 * All rights reserved.
 *
 * This file is part of FLOM.
 *
 * FLOM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 * FLOM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FLOM.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <config.h>



#include "flom_conns.h"
#include "flom_errors.h"
#include "flom_trace.h"



/* set module trace flag */
#ifdef FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE   FLOM_TRACE_MOD_CONNS



void flom_conns_init(flom_conns_t *conns, int domain)
{
    FLOM_TRACE(("flom_conns_init\n"));        
    conns->n = 0;
    conns->poll_array = NULL;
    conns->domain = domain;
    conns->array = g_ptr_array_new();
}



int flom_conns_add(flom_conns_t *conns, int fd,
                   socklen_t addr_len, const struct sockaddr *sa,
                   int main_thread)
{
    enum Exception { G_TRY_MALLOC_ERROR1
                     , INVALID_DOMAIN
                     , G_TRY_MALLOC_ERROR2
                     , G_MARKUP_PARSE_CONTEXT_NEW_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;

    struct flom_conn_data_s *tmp;
    
    FLOM_TRACE(("flom_conns_add\n"));
    TRY {
        if (NULL == (tmp = g_try_malloc0(sizeof(struct flom_conn_data_s))))
            THROW(G_TRY_MALLOC_ERROR1);
        FLOM_TRACE(("flom_conns_add: allocated a new connection:%p\n", tmp));
        switch (conns->domain) {
            case AF_UNIX:
                tmp->saun = *((struct sockaddr_un *)sa);
                break;
            case AF_INET:
                tmp->sain = *((struct sockaddr_in *)sa);
                break;
            default:
                THROW(INVALID_DOMAIN);
        }
        tmp->fd = fd;
        tmp->state = main_thread ?
            FLOM_CONN_STATE_DAEMON : FLOM_CONN_STATE_LOCKER;
        tmp->addr_len = addr_len;
        /* reset the associated message */
        if (NULL == (tmp->msg =
                     g_try_malloc(sizeof(struct flom_msg_s))))
            THROW(G_TRY_MALLOC_ERROR2);
        FLOM_TRACE(("flom_conns_add: allocated a new message:%p\n", tmp->msg));
        flom_msg_init(tmp->msg);
        
        /* initialize the associated parser */
        tmp->gmpc = g_markup_parse_context_new (
            &flom_msg_parser, 0, (gpointer)(tmp->msg), NULL);
        if (NULL == tmp->gmpc)
            THROW(G_MARKUP_PARSE_CONTEXT_NEW_ERROR);
        g_ptr_array_add(conns->array, tmp);
        conns->n++;
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case G_TRY_MALLOC_ERROR1:
                ret_cod = FLOM_RC_G_TRY_MALLOC_ERROR;
                break;
            case INVALID_DOMAIN:
                ret_cod = FLOM_RC_OBJ_CORRUPTED;
                break;
            case G_TRY_MALLOC_ERROR2:
                ret_cod = FLOM_RC_G_TRY_MALLOC_ERROR;
                break;
            case G_MARKUP_PARSE_CONTEXT_NEW_ERROR:
                ret_cod = FLOM_RC_G_MARKUP_PARSE_CONTEXT_NEW_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_conns_add: excp=%d\n", excp));
    if (NONE != excp) {
        if (G_TRY_MALLOC_ERROR2 < excp) /* release message */
            g_free(tmp->msg);
        if (G_TRY_MALLOC_ERROR1 < excp) /* release connection data */
            g_free(tmp);
    }
    FLOM_TRACE(("flom_conns_add/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



void flom_conns_import(flom_conns_t *conns, int fd,
                      struct flom_conn_data_s *cd)
{
    FLOM_TRACE(("flom_conns_import\n"));
    flom_conn_data_trace(cd);
    g_ptr_array_add(conns->array, cd);
    conns->n++;
}
    


struct pollfd *flom_conns_get_fds(flom_conns_t *conns)
{
    struct pollfd *tmp = NULL;
    guint i;
    
    FLOM_TRACE(("flom_conns_get_fds\n"));
    /* resize the previous poll array */
    if (NULL == (tmp = (struct pollfd *)realloc(
                     conns->poll_array,
                     (size_t)(conns->n*sizeof(struct pollfd))))) {
        conns->poll_array = tmp;
        return NULL;
    }    
    /* reset the array */
    memset(tmp, 0, (size_t)(conns->n*sizeof(struct pollfd)));
    /* fill the poll array with file descriptors */
    for (i=0; i<conns->n; ++i) {
        tmp[i].fd =
            ((struct flom_conn_data_s *)g_ptr_array_index(
                conns->array, i))->fd;
    }
    conns->poll_array = tmp;
    return conns->poll_array;
}




int flom_conns_set_events(flom_conns_t *conns, short events)
{
    enum Exception { OBJECT_CORRUPTED
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_conns_set_events\n"));
    TRY {
        guint i;
        for (i=0; i<conns->n; ++i) {
            struct flom_conn_data_s *c =
                (struct flom_conn_data_s *)g_ptr_array_index(conns->array, i);
            if (NULL_FD != c->fd)
                conns->poll_array[i].events = events;
            else {
                FLOM_TRACE(("flom_conns_set_events: i=%u, "
                            "conns->poll_array[i].fd=%d\n", i,
                            conns->poll_array[i].fd));
                THROW(OBJECT_CORRUPTED);
            }
        }
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case OBJECT_CORRUPTED:
                ret_cod = FLOM_RC_OBJ_CORRUPTED;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_conns_set_events/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_conns_close_fd(flom_conns_t *conns, guint id)
{
    enum Exception { OUT_OF_RANGE
                     , NULL_OBJECT
                     , CLOSE_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_conns_close_fd\n"));
    TRY {
        struct flom_conn_data_s *c;
        FLOM_TRACE(("flom_conns_close: closing connection id=%u\n", id));
        if (id >= conns->n)
            THROW(OUT_OF_RANGE);
        if (NULL == (c = (struct flom_conn_data_s *)
                     g_ptr_array_index(conns->array, id)))
            THROW(NULL_OBJECT);
        if (FLOM_CONN_STATE_REMOVE != c->state) {
            c->state = FLOM_CONN_STATE_REMOVE;
            if (NULL_FD == c->fd) {
                FLOM_TRACE(("flom_conns_close: connection id=%u already "
                            "closed, skipping...\n", id));
            } else {
                if (0 != close(c->fd))
                    THROW(CLOSE_ERROR);
                c->fd = NULL_FD;
            }
        } else {
            FLOM_TRACE(("flom_conns_close: connection id=%u already "
                        "in state %d, skipping...\n", id, c->state));
        } /* if (FLOM_CONN_STATE_REMOVE == c->state) */
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case OUT_OF_RANGE:
                ret_cod = FLOM_RC_OUT_OF_RANGE;
                break;
            case NULL_OBJECT:
                ret_cod = FLOM_RC_NULL_OBJECT;
                break;
            case CLOSE_ERROR:
                ret_cod = FLOM_RC_CLOSE_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_conns_close_fd/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_conns_trns_fd(flom_conns_t *conns, guint id)
{
    enum Exception { OUT_OF_RANGE
                     , G_PTR_ARRAY_REMOVE_INDEX_FAST_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_conns_trns_fd\n"));
    TRY {
        struct flom_conn_data_s *c;
        FLOM_TRACE(("flom_conns_trns: marking as transferred connection "
                    "id=%u\n", id));
        if (id >= conns->n)
            THROW(OUT_OF_RANGE);
        /* update connection state */
        c = (struct flom_conn_data_s *)g_ptr_array_index(conns->array, id);
        c->state = FLOM_CONN_STATE_LOCKER;
        /* detach the connection from this connections object (it will be
           attached by a locker connections object */
        if (NULL == g_ptr_array_remove_index_fast(conns->array, id))
            THROW(G_PTR_ARRAY_REMOVE_INDEX_FAST_ERROR);
        conns->n--;

        THROW(NONE);
    } CATCH {
        switch (excp) {
            case OUT_OF_RANGE:
                ret_cod = FLOM_RC_OUT_OF_RANGE;
                break;
            case G_PTR_ARRAY_REMOVE_INDEX_FAST_ERROR:
                ret_cod = FLOM_RC_G_PTR_ARRAY_REMOVE_INDEX_FAST_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_conns_trns_fd/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



void flom_conns_clean(flom_conns_t *conns)
{
    guint i=0;
    FLOM_TRACE(("flom_conns_clean: starting...\n"));
    while (i<conns->n) {
        struct flom_conn_data_s *c =
            (struct flom_conn_data_s *)g_ptr_array_index(conns->array, i);
        FLOM_TRACE(("flom_conns_clean: i=%u, state=%d, fd=%d %s\n",
                    i, c->state, c->fd,
                    FLOM_CONN_STATE_REMOVE == c->state ?
                    "(removing...)" : ""));
        flom_conn_data_trace(c);
        if (FLOM_CONN_STATE_REMOVE == c->state) {
            /* connections with this state are no more valid and must be
               removed and destroyed */
            /* removing message object */
            if (NULL != c->msg) {
                flom_msg_free(c->msg);
                g_free(c->msg);
                c->msg = NULL;
            }
            /* removing parser object */
            if (NULL != c->gmpc) {
                g_markup_parse_context_free(c->gmpc);
                c->gmpc = NULL;
            }
            /* removing from array */
            if (NULL == g_ptr_array_remove_index_fast(conns->array, i))
                FLOM_TRACE(("flom_conns_clean: g_ptr_array_remove_index_fast "
                            "returned NULL\n"));
            else
                conns->n--;
            /* release connection */
            FLOM_TRACE(("flom_conns_clean: releasing connection %p\n", c));
            g_free(c);
        } else i++;
    } /* while (i<conns->n) */
    FLOM_TRACE(("flom_conns_clean: completed\n"));
}



void flom_conns_free(flom_conns_t *conns)
{
    guint i;
    FLOM_TRACE(("flom_conns_free: starting...\n"));
    for (i=0; i<conns->n; ++i)
        flom_conns_close_fd(conns, i);
    flom_conns_clean(conns);
    if (NULL != conns->poll_array) {
        free(conns->poll_array);
        conns->poll_array = NULL;
    }
    if (NULL != conns->array) {
        g_ptr_array_free(conns->array, TRUE);
        conns->array = NULL;
    }
    conns->n = 0;
    FLOM_TRACE(("flom_conns_free: completed\n"));
}



void flom_conn_data_trace(const struct flom_conn_data_s *conn)
{
    FLOM_TRACE(("flom_conn_data_trace: object=%p\n", conn));
    FLOM_TRACE(("flom_conn_data_trace: "
                "fd=%d, state=%d, msg=%p, gmpc=%p, addr_len=%d\n",
                conn->fd, conn->state, conn->msg, conn->gmpc,
                conn->addr_len));
}



void flom_conns_trace(const flom_conns_t *conns)
{
    FLOM_TRACE(("flom_conns_trace: object=%p\n", conns));
    FLOM_TRACE(("flom_conns_trace: domain=%d, n=%u, array=%p, "
                "poll_array=%p\n",
                conns->domain, conns->n, conns->array, conns->poll_array));
}

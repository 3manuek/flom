/*
 * Copyright (c) 2009-2012, Christian Ferrari <tiian@users.sourceforge.net>
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



#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif



#include "flom_connect.h"
#include "flom_daemon.h"
#include "flom_errors.h"
#include "flom_trace.h"



/* set module trace flag */
#ifdef FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE   FLOM_TRACE_MOD_CONNECT



int flom_connect(const flom_config_t *config)
{
    enum Exception { SOCKET_ERROR
                     , DAEMON_ERROR
                     , CONNECT_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_connect\n"));
    TRY {
        int sockfd;
        struct sockaddr_un servaddr;
        
        FLOM_TRACE(("flom_connect: connecting to socket '%s'\n",
                    config->local_socket_path_name));

        if (-1 == (sockfd = socket(AF_LOCAL, SOCK_STREAM, 0)))
            THROW(SOCKET_ERROR);
        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sun_family = AF_LOCAL;
        strcpy(servaddr.sun_path, config->local_socket_path_name);
        if (-1 == connect(sockfd, (struct sockaddr *)&servaddr,
                          sizeof(servaddr))) {
            if (ENOENT == errno) {
                FLOM_TRACE(("flom_connect: ENOENT\n"));
                /* daemon is not active, starting it... @@@ */
                if (FLOM_RC_OK != (ret_cod = flom_daemon(config)))
                    THROW(DAEMON_ERROR);
            } else {
                THROW(CONNECT_ERROR);
            }
        }
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case SOCKET_ERROR:
                ret_cod = FLOM_RC_SOCKET_ERROR;
                break;
            case CONNECT_ERROR:
                ret_cod = FLOM_RC_CONNECT_ERROR;
                break;
            case DAEMON_ERROR:
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_connect/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}
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
#ifndef FLOM_DEFINES_H
# define FLOM_DEFINES_H

#include <config.h>



#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif /* HAVE_STDLIB_H */
#ifdef HAVE_STDINT_H
# include <stdint.h>
#endif /* HAVE_STDINT_H */
#ifdef HAVE_SYS_INTTYPES_H
# include <sys/inttypes.h>
#endif /* HAVE_SYS_INTTYPES_H */
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */
#ifdef HAVE_TIME_H
# include <time.h>
#endif /* HAVE_TIME_H */
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif /* HAVE_UNISTD_H */



/***********************************************************************
 *                                                                     *
 * Boolean macros                                                      *
 *                                                                     *
 ***********************************************************************/
#ifndef TRUE
# define TRUE 1
#endif /* TRUE */

#ifndef FALSE
# define FALSE 0
#endif /* FALSE */



/***********************************************************************
 *                                                                     *
 * Exception macros                                                    *
 *                                                                     *
 ***********************************************************************/
#ifndef TRY
#define TRY
#endif /* TRY */
        
#ifndef CATCH
#define CATCH ExcpHand:
#endif /* CATCH */

#ifndef THROW
#define THROW(e) { excp = e; goto ExcpHand; }
#endif /* THROW */



/**
 * This value is the null file descriptor convention
 */
#define FLOM_NULL_FD    -1



/**
 * Format must be used in *printf family function to print a "uint32_t"
 * value
 */
#if (SIZEOF_UINT32_T == SIZEOF_INT)
# define UINT32_T_FORMAT "%u"
#elif SIZEOF_UINT32_T == SIZEOF_LONG_INT
# define UINT32_T_FORMAT "%lu"
#elif SIZEOF_UINT32_T == SIZEOF_LONG_LONG_INT
# define UINT32_T_FORMAT "%llu"
#else
# error Unable to determine sizeof(uint32_t)
#endif



#ifdef HAVE_PID_T
# if SIZEOF_PID_T == SIZEOF_INT
#  define PID_T_FORMAT "%d"
# elif SIZEOF_PID_T == SIZEOF_LONG_INT
#  define PID_T_FORMAT "%l"
# else
#  error Unable to determine sizeof(pid_t)
# endif
#else
# error pid_t type is not available
#endif



/**
 * Format must be used in *printf family function to print a "size_t"
 * value
 */
#if SIZEOF_SIZE_T == SIZEOF_INT
# define SIZE_T_FORMAT "%u"
#elif SIZEOF_SIZE_T == SIZEOF_LONG_INT
# define SIZE_T_FORMAT "%lu"
#elif SIZEOF_SIZE_T == SIZEOF_LONG_LONG_INT
# define SIZE_T_FORMAT "%llu"
#else
# error Unable to determine sizeof(size_t)
#endif



/**
 * Format must be used in *printf family function to print a "ssize_t"
 * value
 */
#if SIZEOF_SSIZE_T == SIZEOF_INT
# define SSIZE_T_FORMAT "%d"
#elif SSIZEOF_SIZE_T == SIZEOF_LONG_INT
# define SSIZE_T_FORMAT "%ld"
#elif SSIZEOF_SIZE_T == SIZEOF_LONG_LONG_INT
# define SSIZE_T_FORMAT "%lld"
#else
# error Unable to determine sizeof(ssize_t)
#endif



#ifdef HAVE_TIME_H
# if SIZEOF_CLOCK_T == SIZEOF_INT
#  define CLOCK_T_FORMAT "%d"
# elif SIZEOF_CLOCK_T == SIZEOF_LONG_INT
#  define CLOCK_T_FORMAT "%ld"
# elif SIZEOF_CLOCK_T == SIZEOF_LONG_LONG_INT
#  define CLOCK_T_FORMAT "%lld"
# else
#  error Unable to determine sizeof(clock_t)
# endif
#endif /* HAVE_TIME_H */



/**
 * format for printing "off_t" values
 */
#if SIZEOF_OFF_T == SIZEOF_LONG_INT
# define OFF_T_FORMAT "%ld"
#elif SIZEOF_OFF_T == SIZEOF_LONG_LONG_INT
# define OFF_T_FORMAT "%lld"
#else
# error Unable to determine sizeof(off_t)
#endif



/**
 * format for printing uintptr_t
 */
#if SIZEOF_UINTPTR_T == SIZEOF_INT
# define UINTPTR_T_FORMAT "%u"
#elif SIZEOF_UINTPTR_T == SIZEOF_LONG_INT
# define UINTPTR_T_FORMAT "%lu"
#elif SIZEOF_UINTPTR_T == SIZEOF_LONG_LONG_INT
# define UINTPTR_T_FORMAT "%llu"
#else
# error Unable to determine sizeof(uintptr_t)
#endif



#if SIZEOF_IN_PORT_T == SIZEOF_SHORT_INT
# define IN_PORT_T_FORMAT "%hu"
#else
# error Unable to determine format for in_port_t
#endif



#define FLOM_PATH_SEPARATOR '/'



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



#ifndef HAVE_UINT8_T
      /** 8 bit unsigned int */
      typedef unsigned char uint8_t;
#endif /* HAVE_UINT8_T */
#ifndef HAVE_UINT32_T
# if (SIZEOF_INT == 4)
      /** 32 bit unsigned int */
      typedef unsigned int uint32_t;
# else
#  error "Please specify a 32 bit wide unsigned int type"
# endif /* if (SIZEOF_INT == 4) */
#endif /* HAVE_UINT32_T */


      
    /**
     * A byte is the fundamental I/O unit
     */
    typedef uint8_t byte_t;
    
  

#ifdef __cplusplus
}
#endif /* __cplusplus */



#endif /* FLOM_DEFINES_H */


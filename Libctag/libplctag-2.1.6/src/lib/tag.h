/***************************************************************************
 *   Copyright (C) 2020 by Kyle Hayes                                      *
 *   Author Kyle Hayes  kyle.hayes@gmail.com                               *
 *                                                                         *
 * This software is available under either the Mozilla Public License      *
 * version 2.0 or the GNU LGPL version 2 (or later) license, whichever     *
 * you choose.                                                             *
 *                                                                         *
 * MPL 2.0:                                                                *
 *                                                                         *
 *   This Source Code Form is subject to the terms of the Mozilla Public   *
 *   License, v. 2.0. If a copy of the MPL was not distributed with this   *
 *   file, You can obtain one at http://mozilla.org/MPL/2.0/.              *
 *                                                                         *
 *                                                                         *
 * LGPL 2:                                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#ifndef __LIBPLCTAG_TAG_H__
#define __LIBPLCTAG_TAG_H__




#include <lib/libplctag.h>
#include <platform.h>
#include <util/attr.h>

#define PLCTAG_CANARY (0xACA7CAFE)
#define PLCTAG_DATA_LITTLE_ENDIAN   (0)
#define PLCTAG_DATA_BIG_ENDIAN      (1)

//extern mutex_p global_library_mutex;

typedef struct plc_tag_t *plc_tag_p;


/* define tag operation functions */
//typedef int (*tag_abort_func)(plc_tag_p tag);
//typedef int (*tag_destroy_func)(plc_tag_p tag);
//typedef int (*tag_read_func)(plc_tag_p tag);
//typedef int (*tag_status_func)(plc_tag_p tag);
//typedef int (*tag_write_func)(plc_tag_p tag);

typedef int (*tag_vtable_func)(plc_tag_p tag);

/* we'll need to set these per protocol type. */
struct tag_vtable_t {
    tag_vtable_func abort;
    tag_vtable_func read;
    tag_vtable_func status;
    tag_vtable_func tickler;
    tag_vtable_func write;

    /* data accessors. */
//    int (*get_size)(plc_tag_p tag);
    int (*get_int_attrib)(plc_tag_p tag, const char *attrib_name, int default_value);
    int (*set_int_attrib)(plc_tag_p tag, const char *attrib_name, int new_value);

    int (*get_bit)(plc_tag_p tag, int offset_bit);
    int (*set_bit)(plc_tag_p tag, int offset_bit, int val);

    uint64_t (*get_uint64)(plc_tag_p tag, int offset);
    int (*set_uint64)(plc_tag_p tag, int offset, uint64_t val);

    int64_t (*get_int64)(plc_tag_p tag, int offset);
    int (*set_int64)(plc_tag_p tag, int offset, int64_t val);


    uint32_t (*get_uint32)(plc_tag_p tag, int offset);
    int (*set_uint32)(plc_tag_p tag, int offset, uint32_t val);

    int32_t (*get_int32)(plc_tag_p tag, int offset);
    int (*set_int32)(plc_tag_p tag, int offset, int32_t val);


    uint16_t (*get_uint16)(plc_tag_p tag, int offset);
    int (*set_uint16)(plc_tag_p tag, int offset, uint16_t val);

    int16_t (*get_int16)(plc_tag_p tag, int offset);
    int (*set_int16)(plc_tag_p tag, int offset, int16_t val);


    uint8_t (*get_uint8)(plc_tag_p tag, int offset);
    int (*set_uint8)(plc_tag_p tag, int offset, uint8_t val);

    int8_t (*get_int8)(plc_tag_p tag, int offset);
    int (*set_int8)(plc_tag_p tag, int offset, int8_t val);


    double (*get_float64)(plc_tag_p tag, int offset);
    int (*set_float64)(plc_tag_p tag, int offset, double val);

    float (*get_float32)(plc_tag_p tag, int offset);
    int (*set_float32)(plc_tag_p tag, int offset, float val);
};

typedef struct tag_vtable_t *tag_vtable_p;


/*
 * The base definition of the tag structure.  This is used
 * by the protocol-specific implementations.
 *
 * The base type only has a vtable for operations.
 */

#define TAG_BASE_STRUCT tag_vtable_p vtable; \
                        mutex_p ext_mutex; \
                        mutex_p api_mutex; \
                        int status; \
                        int endian; \
                        int tag_id; \
                        int64_t read_cache_expire; \
                        int64_t read_cache_ms; \
                        int read_complete; \
                        int write_complete; \
                        void (*callback)(int32_t tag_id, int event, int status); \
                        int size; \
                        uint8_t *data

struct plc_tag_dummy {
    int tag_id;
};

struct plc_tag_t {
    TAG_BASE_STRUCT;
};

#define PLC_TAG_P_NULL ((plc_tag_p)0)


/* the following may need to be used where the tag is already mapped or is not yet mapped */
extern int lib_init(void);
extern void lib_teardown(void);
extern int plc_tag_abort_mapped(plc_tag_p tag);
extern int plc_tag_destroy_mapped(plc_tag_p tag);
extern int plc_tag_status_mapped(plc_tag_p tag);



#endif

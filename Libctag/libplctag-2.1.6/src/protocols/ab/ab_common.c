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

#include <limits.h>
#include <float.h>
#include <platform.h>
#include <lib/libplctag.h>
#include <lib/tag.h>
#include <ab/ab.h>
#include <ab/ab_common.h>
#include <ab/pccc.h>
#include <ab/cip.h>
#include <ab/defs.h>
#include <ab/eip_cip.h>
#include <ab/eip_lgx_pccc.h>
#include <ab/eip_plc5_pccc.h>
#include <ab/eip_slc_pccc.h>
#include <ab/eip_dhp_pccc.h>
#include <ab/session.h>
#include <ab/tag.h>
#include <util/attr.h>
#include <util/debug.h>
#include <util/vector.h>


/*
 * Externally visible global variables
 */

//volatile ab_session_p sessions = NULL;
//volatile mutex_p global_session_mut = NULL;
//
//volatile vector_p read_group_tags = NULL;


/* request/response handling thread */
volatile thread_p io_handler_thread = NULL;

volatile int ab_protocol_terminating = 0;



/*
 * Generic Rockwell/Allen-Bradley protocol functions.
 *
 * These are the primary entry points into the AB protocol
 * stack.
 */


#define DEFAULT_NUM_RETRIES (5)
#define DEFAULT_RETRY_INTERVAL (300)


/* forward declarations*/
static int get_tag_data_type(ab_tag_p tag, attr attribs);

static void ab_tag_destroy(ab_tag_p tag);
static int default_abort(plc_tag_p tag);
static int default_read(plc_tag_p tag);
static int default_status(plc_tag_p tag);
static int default_tickler(plc_tag_p tag);
static int default_write(plc_tag_p tag);


/* vtables for different kinds of tags */
struct tag_vtable_t default_vtable = {
    default_abort,
    default_read,
    default_status,
    default_tickler,
    default_write,

    /* data accessors */
    ab_get_int_attrib,
    ab_set_int_attrib,

    ab_get_bit,
    ab_set_bit,

    ab_get_uint64,
    ab_set_uint64,

    ab_get_int64,
    ab_set_int64,

    ab_get_uint32,
    ab_set_uint32,

    ab_get_int32,
    ab_set_int32,

    ab_get_uint16,
    ab_set_uint16,

    ab_get_int16,
    ab_set_int16,

    ab_get_uint8,
    ab_set_uint8,

    ab_get_int8,
    ab_set_int8,

    ab_get_float64,
    ab_set_float64,

    ab_get_float32,
    ab_set_float32
};


/*
 * Public functions.
 */


int ab_init(void)
{
    int rc = PLCTAG_STATUS_OK;

    pdebug(DEBUG_INFO,"Initializing AB protocol library.");

    if((rc = session_startup()) != PLCTAG_STATUS_OK) {
        pdebug(DEBUG_ERROR, "Unable to initialize session library!");
        return rc;
    }

    pdebug(DEBUG_INFO,"Finished initializing AB protocol library.");

    return rc;
}

/*
 * called when the whole program is going to terminate.
 */
void ab_teardown(void)
{
    pdebug(DEBUG_INFO,"Releasing global AB protocol resources.");

    pdebug(DEBUG_INFO,"Terminating IO thread.");
    /* kill the IO thread first. */
    ab_protocol_terminating = 1;

    /* wait for the thread to die */
    thread_join(io_handler_thread);
    thread_destroy((thread_p*)&io_handler_thread);

    pdebug(DEBUG_INFO,"Freeing session information.");

    session_teardown();

    ab_protocol_terminating = 0;

    pdebug(DEBUG_INFO,"Done.");
}



plc_tag_p ab_tag_create(attr attribs)
{
    ab_tag_p tag = AB_TAG_NULL;
    const char *path = NULL;
    int rc = PLCTAG_STATUS_OK;

    pdebug(DEBUG_INFO,"Starting.");

    /*
     * allocate memory for the new tag.  Do this first so that
     * we have a vehicle for returning status.
     */

    tag = (ab_tag_p)rc_alloc(sizeof(struct ab_tag_t), (rc_cleanup_func)ab_tag_destroy);

    if(!tag) {
        pdebug(DEBUG_ERROR,"Unable to allocate memory for AB EIP tag!");
        return (plc_tag_p)NULL;
    }

    pdebug(DEBUG_DETAIL, "tag=%p", tag);

    /*
     * we got far enough to allocate memory, set the default vtable up
     * in case we need to abort later.
     */

    tag->vtable = &default_vtable;

    /*
     * check the CPU type.
     *
     * This determines the protocol type.
     */

    if(check_cpu(tag, attribs) != PLCTAG_STATUS_OK) {
        pdebug(DEBUG_WARN,"CPU type not valid or missing.");
        /* tag->status = PLCTAG_ERR_BAD_DEVICE; */
        rc_dec(tag);
        return (plc_tag_p)NULL;
    }

    /* get the connection path.  We need this to make a decision about the PLC. */
    path = attr_get_str(attribs,"path",NULL);

    /* set up PLC-specific information. */
    switch(tag->protocol_type) {
    case AB_PROTOCOL_PLC:
        if(!path) {
            pdebug(DEBUG_DETAIL, "Setting up PLC/5 tag.");
            tag->use_connected_msg = 0;
            tag->vtable = &plc5_vtable;
        } else {
            pdebug(DEBUG_DETAIL, "Setting up PLC/5 via DH+ bridge tag.");
            tag->use_connected_msg = 1;
            tag->vtable = &eip_dhp_pccc_vtable;
        }

        tag->allow_packing = 0;
        break;

    case AB_PROTOCOL_SLC:
    case AB_PROTOCOL_MLGX:
        pdebug(DEBUG_DETAIL, "Setting up SLC, MicroLogix tag.");
        tag->use_connected_msg = 0;
        tag->allow_packing = 0;
        tag->vtable = &slc_vtable;
        break;

    case AB_PROTOCOL_LGX_PCCC:
        pdebug(DEBUG_DETAIL, "Setting up PCCC-mapped Logix tag.");
        tag->use_connected_msg = 0;
        tag->allow_packing = 0;
        tag->vtable = &lgx_pccc_vtable;
        break;

    case AB_PROTOCOL_LGX:
        pdebug(DEBUG_DETAIL, "Setting up Logix tag.");

        /* Logix tags need a path. */
        if(path == NULL && tag->protocol_type == AB_PROTOCOL_LGX) {
            pdebug(DEBUG_WARN,"A path is required for Logix-class PLCs!");
            tag->status = PLCTAG_ERR_BAD_PARAM;
            return (plc_tag_p)tag;
        }

        rc = get_tag_data_type(tag, attribs);
        if(rc != PLCTAG_STATUS_OK) {
            pdebug(DEBUG_WARN, "Error getting tag element data type %s!", plc_tag_decode_error(rc));
            tag->status = rc;
            return (plc_tag_p)tag;
        }

        /* default to requiring a connection. */
        tag->use_connected_msg = attr_get_int(attribs,"use_connected_msg", 1);
        tag->allow_packing = attr_get_int(attribs, "allow_packing", 1);
        tag->vtable = &eip_cip_vtable;

        break;

    case AB_PROTOCOL_MLGX800:
        pdebug(DEBUG_DETAIL, "Setting up Micro8X0 tag.");
        tag->use_connected_msg = 1;
        tag->allow_packing = 0;
        tag->vtable = &eip_cip_vtable;
        break;

    default:
        pdebug(DEBUG_WARN, "Unknown PLC type!");
        tag->status = PLCTAG_ERR_BAD_CONFIG;
        return (plc_tag_p)tag;
        break;
    }

    /* pass the connection requirement since it may be overridden above. */
    attr_set_int(attribs, "use_connected_msg", tag->use_connected_msg);

    /* determine the total tag size if this is not a tag list. */
//    if(!tag->tag_list) {
//        if(!tag->elem_size) {
//            tag->elem_size = attr_get_int(attribs, "elem_size", 0);
//        }
//        tag->elem_count = attr_get_int(attribs,"elem_count", 1);
//    }


    /* get the element count, default to 1 if missing. */
    tag->elem_count = attr_get_int(attribs,"elem_count", 1);

    /* we still need size on non Logix-class PLCs */
    if(tag->protocol_type != AB_PROTOCOL_LGX && tag->protocol_type != AB_PROTOCOL_MLGX800) {
        /* get the element size if it is not already set. */
        if(!tag->elem_size) {
            tag->elem_size = attr_get_int(attribs, "elem_size", 0);
        }

        /* allocate memory for the data */
        tag->size = (tag->elem_count) * (tag->elem_size);
        if(tag->size == 0) {
            /* failure! Need data_size! */
            pdebug(DEBUG_WARN,"Tag size is zero!");
            tag->status = PLCTAG_ERR_BAD_PARAM;
            return (plc_tag_p)tag;
        }

        /* this may be changed in the future if this is a tag list request. */
        tag->data = (uint8_t*)mem_alloc(tag->size);

        if(tag->data == NULL) {
            pdebug(DEBUG_WARN,"Unable to allocate tag data!");
            tag->status = PLCTAG_ERR_NO_MEM;
            return (plc_tag_p)tag;
        }
    } else {
        /* fill this in when we read the tag. */
        tag->elem_size = 0;
        tag->size = 0;
        tag->data = NULL;
    }

    /*
     * Find or create a session.
     *
     * All tags need sessions.  They are the TCP connection to the gateway PLC.
     */
    if(session_find_or_create(&tag->session, attribs) != PLCTAG_STATUS_OK) {
        pdebug(DEBUG_INFO,"Unable to create session!");
        tag->status = PLCTAG_ERR_BAD_GATEWAY;
        return (plc_tag_p)tag;
    }

    pdebug(DEBUG_DETAIL, "using session=%p", tag->session);

    /*
     * check the tag name, this is protocol specific.
     */

    if(!tag->tag_list && check_tag_name(tag, attr_get_str(attribs,"name",NULL)) != PLCTAG_STATUS_OK) {
        pdebug(DEBUG_INFO,"Bad tag name!");
        tag->status = PLCTAG_ERR_BAD_PARAM;
        return (plc_tag_p)tag;
    }

    /* trigger the first read. */
    tag->first_read = 1;

    /* kick off a read to get the tag type and size. */
    if(tag->vtable->read) {
        tag->vtable->read((plc_tag_p)tag);
    }

    pdebug(DEBUG_INFO,"Done.");

    return (plc_tag_p)tag;
}


/*
 * determine the tag's data type and size.  Or at least guess it.
 */

int get_tag_data_type(ab_tag_p tag, attr attribs)
{
    const char *elem_type = NULL;
    const char *tag_name = NULL;

    switch(tag->protocol_type) {
    case AB_PROTOCOL_PLC:
    case AB_PROTOCOL_SLC:
    case AB_PROTOCOL_LGX_PCCC:
    case AB_PROTOCOL_MLGX:
        tag_name = attr_get_str(attribs,"name", NULL);

        /* the first two characters are the important ones. */
        if(tag_name && str_length(tag_name) >= 2) {
            switch(tag_name[0]) {
            case 'b':
            case 'B':
                /*bit*/
                pdebug(DEBUG_DETAIL,"Found tag element type of bit.");
                tag->elem_size=1;
                tag->elem_type = AB_TYPE_BOOL;
                break;

            case 'c':
            case 'C':
                /* counter */
                pdebug(DEBUG_DETAIL,"Found tag element type of counter.");
                tag->elem_size=6;
                tag->elem_type = AB_TYPE_COUNTER;
                break;

            case 'f':
            case 'F':
                /* 32-bit float */
                pdebug(DEBUG_DETAIL,"Found tag element type of float.");
                tag->elem_size=4;
                tag->elem_type = AB_TYPE_FLOAT32;
                break;

            case 'n':
            case 'N':
                /* 16-bit integer */
                pdebug(DEBUG_DETAIL,"Found tag element type of 16-bit integer.");
                tag->elem_size=2;
                tag->elem_type = AB_TYPE_INT16;
                break;

            case 'r':
            case 'R':
                /* control */
                pdebug(DEBUG_DETAIL,"Found tag element type of control.");
                tag->elem_size=6;
                tag->elem_type = AB_TYPE_CONTROL;
                break;

            case 's':
            case 'S':
                /* Status or String */
                if(tag_name[1] == 't' || tag_name[1] == 'T') {
                    /* string */
                    pdebug(DEBUG_DETAIL,"Found tag element type of string.");
                    tag->elem_size = 84;
                    tag->elem_type = AB_TYPE_STRING;
                } else {
                    /* status */
                    pdebug(DEBUG_DETAIL,"Found tag element type of status word.");
                    tag->elem_size = 2;
                    tag->elem_type = AB_TYPE_INT16;
                }
                break;

            case 't':
            case 'T':
                /* timer */
                pdebug(DEBUG_DETAIL,"Found tag element type of timer.");
                tag->elem_size=6;
                tag->elem_type = AB_TYPE_TIMER;
                break;

            default:
                pdebug(DEBUG_DETAIL,"Unknown tag type for tag %s", tag_name);
                break;
            }

        }

        break;

    case AB_PROTOCOL_LGX:
    case AB_PROTOCOL_MLGX800:
        /* look for the elem_type attribute. */
        elem_type = attr_get_str(attribs, "elem_type", NULL);
        if(elem_type) {
            if(str_cmp_i(elem_type,"lint") == 0 || str_cmp_i(elem_type, "ulint") == 0) {
                pdebug(DEBUG_DETAIL,"Found tag element type of 64-bit integer.");
                tag->elem_size = 8;
                tag->elem_type = AB_TYPE_INT64;
            } else if(str_cmp_i(elem_type,"dint") == 0 || str_cmp_i(elem_type,"udint") == 0) {
                pdebug(DEBUG_DETAIL,"Found tag element type of 32-bit integer.");
                tag->elem_size = 4;
                tag->elem_type = AB_TYPE_INT32;
            } else if(str_cmp_i(elem_type,"int") == 0 || str_cmp_i(elem_type,"uint") == 0) {
                pdebug(DEBUG_DETAIL,"Found tag element type of 16-bit integer.");
                tag->elem_size = 2;
                tag->elem_type = AB_TYPE_INT16;
            } else if(str_cmp_i(elem_type,"sint") == 0 || str_cmp_i(elem_type,"usint") == 0) {
                pdebug(DEBUG_DETAIL,"Found tag element type of 8-bit integer.");
                tag->elem_size = 1;
                tag->elem_type = AB_TYPE_INT8;
            } else if(str_cmp_i(elem_type,"bool") == 0) {
                pdebug(DEBUG_DETAIL,"Found tag element type of bit.");
                tag->elem_size = 1;
                tag->elem_type = AB_TYPE_BOOL;
            } else if(str_cmp_i(elem_type,"bool array") == 0) {
                pdebug(DEBUG_DETAIL,"Found tag element type of bit array.");
                tag->elem_size = 4;
                tag->elem_type = AB_TYPE_BOOL_ARRAY;
            } else if(str_cmp_i(elem_type,"real") == 0) {
                pdebug(DEBUG_DETAIL,"Found tag element type of 32-bit float.");
                tag->elem_size = 4;
                tag->elem_type = AB_TYPE_FLOAT32;
            } else if(str_cmp_i(elem_type,"lreal") == 0) {
                pdebug(DEBUG_DETAIL,"Found tag element type of 64-bit float.");
                tag->elem_size = 8;
                tag->elem_type = AB_TYPE_FLOAT64;
            } else if(str_cmp_i(elem_type,"string") == 0) {
                pdebug(DEBUG_DETAIL,"Fount tag element type of string.");
                tag->elem_size = 88;
                tag->elem_type = AB_TYPE_STRING;
            } else if(str_cmp_i(elem_type,"short string") == 0) {
                pdebug(DEBUG_DETAIL,"Found tag element type of short string.");
                tag->elem_size = 256; /* FIXME */
                tag->elem_type = AB_TYPE_SHORT_STRING;
            } else {
                pdebug(DEBUG_DETAIL, "Unknown tag type %s", elem_type);
            }
        } else {
            /* just for Logix, check for tag listing */
            if(tag->protocol_type == AB_PROTOCOL_LGX) {
                const char *tag_name = attr_get_str(attribs, "name", NULL);
                int tag_listing_rc = setup_tag_listing(tag, tag_name);

                if(tag_listing_rc == PLCTAG_ERR_BAD_PARAM) {
                    pdebug(DEBUG_WARN, "Tag listing request is malformed!");
                    return PLCTAG_ERR_BAD_PARAM;
                }
            }
        }

        break;

    default:
        pdebug(DEBUG_WARN, "Unknown PLC type!");
        return PLCTAG_ERR_BAD_CONFIG;
        break;
    }

    pdebug(DEBUG_DETAIL, "Done.");

    return PLCTAG_STATUS_OK;
}



int default_abort(plc_tag_p tag)
{
    (void)tag;

    pdebug(DEBUG_WARN, "This should be overridden by a PLC-specific function!");

    return PLCTAG_STATUS_OK;
}


int default_read(plc_tag_p tag)
{
    (void)tag;

    pdebug(DEBUG_WARN, "This should be overridden by a PLC-specific function!");

    return PLCTAG_STATUS_OK;
}

int default_status(plc_tag_p tag)
{
    (void)tag;

    pdebug(DEBUG_WARN, "This should be overridden by a PLC-specific function!");

    return PLCTAG_STATUS_OK;
}


int default_tickler(plc_tag_p tag)
{
    (void)tag;

    pdebug(DEBUG_WARN, "This should be overridden by a PLC-specific function!");

    return PLCTAG_STATUS_OK;
}



int default_write(plc_tag_p tag)
{
    (void)tag;

    pdebug(DEBUG_WARN, "This should be overridden by a PLC-specific function!");

    return PLCTAG_STATUS_OK;
}



/*
* ab_tag_abort
*
* This does the work of stopping any inflight requests.
* This is not thread-safe.  It must be called from a function
* that locks the tag's mutex or only from a single thread.
*/

int ab_tag_abort(ab_tag_p tag)
{
    pdebug(DEBUG_DETAIL, "Starting.");

    if(tag->req) {
        spin_block(&tag->req->lock) {
            tag->req->abort_request = 1;
        }

        tag->req = rc_dec(tag->req);
    } else {
        pdebug(DEBUG_DETAIL, "Called without a request in flight.");
    }

    tag->read_in_progress = 0;
    tag->write_in_progress = 0;
    tag->offset = 0;

    pdebug(DEBUG_DETAIL, "Done.");

    return PLCTAG_STATUS_OK;
}




/*
 * ab_tag_status
 *
 * Generic status checker.   May be overridden by individual PLC types.
 */
int ab_tag_status(ab_tag_p tag)
{
    int rc = PLCTAG_STATUS_OK;

    if (tag->read_in_progress) {
        return PLCTAG_STATUS_PENDING;
    }

    if (tag->write_in_progress) {
        return PLCTAG_STATUS_PENDING;
    }

    if(tag->session) {
        rc = tag->status;
    } else {
        /* this is not OK.  This is fatal! */
        rc = PLCTAG_ERR_CREATE;
    }

    return rc;
}






/*
 * ab_tag_destroy
 *
 * This blocks on the global library mutex.  This should
 * be fixed to allow for more parallelism.  For now, safety is
 * the primary concern.
 */

void ab_tag_destroy(ab_tag_p tag)
{
    ab_session_p session = NULL;

    pdebug(DEBUG_INFO, "Starting.");

    /* already destroyed? */
    if (!tag) {
        pdebug(DEBUG_WARN,"Tag pointer is null!");

        return;
    }

    session = tag->session;

    /* tags should always have a session.  Release it. */
    pdebug(DEBUG_DETAIL,"Getting ready to release tag session %p",tag->session);
    if(session) {
        pdebug(DEBUG_DETAIL, "Removing tag from session.");
        rc_dec(session);
        tag->session = NULL;
    } else {
        pdebug(DEBUG_WARN,"No session pointer!");
    }

    if(tag->ext_mutex) {
        mutex_destroy(&(tag->ext_mutex));
        tag->ext_mutex = NULL;
    }

    if(tag->api_mutex) {
        mutex_destroy(&(tag->api_mutex));
        tag->api_mutex = NULL;
    }

    if (tag->data) {
        mem_free(tag->data);
        tag->data = NULL;
    }

    pdebug(DEBUG_INFO,"Finished releasing all tag resources.");

    pdebug(DEBUG_INFO, "done");
}


int ab_get_int_attrib(plc_tag_p raw_tag, const char *attrib_name, int default_value)
{
    int res = default_value;
    ab_tag_p tag = (ab_tag_p)raw_tag;

    pdebug(DEBUG_SPEW, "Starting.");

    /* match the attribute. */
    if(str_cmp_i(attrib_name, "elem_size") == 0) {
        res = tag->elem_size;
    } else if(str_cmp_i(attrib_name, "elem_count") == 0) {
        res = tag->elem_count;
    }

    return res;
}


int ab_set_int_attrib(plc_tag_p raw_tag, const char *attrib_name, int new_value)
{
    (void)raw_tag;
    (void)attrib_name;
    (void)new_value;

    return PLCTAG_ERR_UNSUPPORTED;
}



int ab_get_bit(plc_tag_p raw_tag, int offset_bit)
{
    int res = PLCTAG_ERR_OUT_OF_BOUNDS;
    int real_offset = offset_bit;
    ab_tag_p tag = (ab_tag_p)raw_tag;

    pdebug(DEBUG_SPEW, "Starting.");

    /* is there data? */
    if(!tag->data) {
        pdebug(DEBUG_WARN,"Tag has no data!");
        return PLCTAG_ERR_NO_DATA;
    }

    /* if this is a single bit, then make sure the offset is the tag bit. */
    if(tag->is_bit) {
        real_offset = tag->bit;
    } else {
        real_offset = offset_bit;
    }

    /* is there enough data */
    pdebug(DEBUG_SPEW, "real_offset=%d, byte offset=%d, tag size=%d", real_offset, (real_offset/8), tag->size);
    if((real_offset < 0) || ((real_offset / 8) >= tag->size)) {
        pdebug(DEBUG_WARN,"Data offset out of bounds.");
        return PLCTAG_ERR_OUT_OF_BOUNDS;
    }

    pdebug(DEBUG_SPEW, "selecting bit %d with offset %d in byte %d (%x).", real_offset, (real_offset % 8), (real_offset / 8), tag->data[real_offset / 8]);

    res = !!(((1 << (real_offset % 8)) & 0xFF) & (tag->data[real_offset / 8]));

    return res;
}


int ab_set_bit(plc_tag_p raw_tag, int offset_bit, int val)
{
    int res = PLCTAG_STATUS_OK;
    int real_offset = offset_bit;
    ab_tag_p tag = (ab_tag_p)raw_tag;

    pdebug(DEBUG_SPEW, "Starting.");

    /* is there data? */
    if(!tag->data) {
        pdebug(DEBUG_WARN,"Tag has no data!");
        return PLCTAG_ERR_NO_DATA;
    }

    /* if this is a single bit, then make sure the offset is the tag bit. */
    if(tag->is_bit) {
        real_offset = tag->bit;
    } else {
        real_offset = offset_bit;
    }

    /* is there enough data */
    if((real_offset < 0) || ((real_offset / 8) >= tag->size)) {
        pdebug(DEBUG_WARN,"Data offset out of bounds.");
        return PLCTAG_ERR_OUT_OF_BOUNDS;
    }

    if(val) {
        tag->data[real_offset / 8] |= (uint8_t)(1 << (real_offset % 8));
    } else {
        tag->data[real_offset / 8] &= (uint8_t)(~(1 << (real_offset % 8)));
    }

    res = PLCTAG_STATUS_OK;

    return res;
}



uint64_t ab_get_uint64(plc_tag_p raw_tag, int offset)
{
    uint64_t res = UINT64_MAX;
    ab_tag_p tag = (ab_tag_p)raw_tag;

    pdebug(DEBUG_SPEW, "Starting.");

    /* is there data? */
    if(!tag->data) {
        pdebug(DEBUG_WARN,"Tag has no data!");
        return res;
    }

    /* is there enough data? */
    if((offset < 0) || (offset + ((int)sizeof(uint64_t)) > tag->size)) {
        pdebug(DEBUG_WARN,"Data offset out of bounds.");
        return res;
    }

    if(!tag->is_bit) {
        res = ((uint64_t)(tag->data[offset])) +
              ((uint64_t)(tag->data[offset+1]) << 8) +
              ((uint64_t)(tag->data[offset+2]) << 16) +
              ((uint64_t)(tag->data[offset+3]) << 24) +
              ((uint64_t)(tag->data[offset+4]) << 32) +
              ((uint64_t)(tag->data[offset+5]) << 40) +
              ((uint64_t)(tag->data[offset+6]) << 48) +
              ((uint64_t)(tag->data[offset+7]) << 56);
    } else {
        if(ab_get_bit(raw_tag, 0)) {
            res = 1;
        } else {
            res = 0;
        }
    }

    return res;
}



int ab_set_uint64(plc_tag_p raw_tag, int offset, uint64_t val)
{
    int rc = PLCTAG_STATUS_OK;
    ab_tag_p tag = (ab_tag_p)raw_tag;

    pdebug(DEBUG_SPEW, "Starting.");

    /* is there data? */
    if(!tag->data) {
        pdebug(DEBUG_WARN,"Tag has no data!");
        return PLCTAG_ERR_NO_DATA;
    }

    /* is there enough data */
    if((offset < 0) || (offset + ((int)sizeof(uint64_t)) > tag->size)) {
        pdebug(DEBUG_WARN,"Data offset out of bounds.");
        return PLCTAG_ERR_OUT_OF_BOUNDS;
    }

    /* write the data. */
    if(!tag->is_bit) {
        tag->data[offset]   = (uint8_t)(val & 0xFF);
        tag->data[offset+1] = (uint8_t)((val >> 8) & 0xFF);
        tag->data[offset+2] = (uint8_t)((val >> 16) & 0xFF);
        tag->data[offset+3] = (uint8_t)((val >> 24) & 0xFF);
        tag->data[offset+4] = (uint8_t)((val >> 32) & 0xFF);
        tag->data[offset+5] = (uint8_t)((val >> 40) & 0xFF);
        tag->data[offset+6] = (uint8_t)((val >> 48) & 0xFF);
        tag->data[offset+7] = (uint8_t)((val >> 56) & 0xFF);
    } else {
        if(!val) {
            rc = ab_set_bit(raw_tag, 0, 0);
        } else {
            rc = ab_set_bit(raw_tag, 0, 1);
        }
    }

    return rc;
}




int64_t ab_get_int64(plc_tag_p raw_tag, int offset)
{
    int64_t res = INT64_MIN;
    ab_tag_p tag = (ab_tag_p)raw_tag;

    pdebug(DEBUG_SPEW, "Starting.");

    /* is there data? */
    if(!tag->data) {
        pdebug(DEBUG_WARN,"Tag has no data!");
        return res;
    }

    /* is there enough data */
    if((offset < 0) || (offset + ((int)sizeof(int64_t)) > tag->size)) {
        pdebug(DEBUG_WARN,"Data offset out of bounds.");
        return res;
    }

    if(!tag->is_bit) {
        res = (int64_t)(((uint64_t)(tag->data[offset])) +
                        ((uint64_t)(tag->data[offset+1]) << 8) +
                        ((uint64_t)(tag->data[offset+2]) << 16) +
                        ((uint64_t)(tag->data[offset+3]) << 24) +
                        ((uint64_t)(tag->data[offset+4]) << 32) +
                        ((uint64_t)(tag->data[offset+5]) << 40) +
                        ((uint64_t)(tag->data[offset+6]) << 48) +
                        ((uint64_t)(tag->data[offset+7]) << 56));
    } else {
        if(ab_get_bit(raw_tag, 0)) {
            res = 1;
        } else {
            res = 0;
        }
    }

    return res;
}



int ab_set_int64(plc_tag_p raw_tag, int offset, int64_t ival)
{
    uint64_t val = (uint64_t)(ival);
    int rc = PLCTAG_STATUS_OK;
    ab_tag_p tag = (ab_tag_p)raw_tag;

    pdebug(DEBUG_SPEW, "Starting.");

    /* is there data? */
    if(!tag->data) {
        pdebug(DEBUG_WARN,"Tag has no data!");
        return PLCTAG_ERR_NO_DATA;
    }

    /* is there enough data */
    if((offset < 0) || (offset + ((int)sizeof(int64_t)) > tag->size)) {
        pdebug(DEBUG_WARN,"Data offset out of bounds.");
        return PLCTAG_ERR_OUT_OF_BOUNDS;
    }

    if(!tag->is_bit) {
        tag->data[offset]   = (uint8_t)(val & 0xFF);
        tag->data[offset+1] = (uint8_t)((val >> 8) & 0xFF);
        tag->data[offset+2] = (uint8_t)((val >> 16) & 0xFF);
        tag->data[offset+3] = (uint8_t)((val >> 24) & 0xFF);
        tag->data[offset+4] = (uint8_t)((val >> 32) & 0xFF);
        tag->data[offset+5] = (uint8_t)((val >> 40) & 0xFF);
        tag->data[offset+6] = (uint8_t)((val >> 48) & 0xFF);
        tag->data[offset+7] = (uint8_t)((val >> 56) & 0xFF);
    } else {
        if(!val) {
            rc = ab_set_bit(raw_tag, 0, 0);
        } else {
            rc = ab_set_bit(raw_tag, 0, 1);
        }
    }

    return rc;
}









uint32_t ab_get_uint32(plc_tag_p raw_tag, int offset)
{
    uint32_t res = UINT32_MAX;
    ab_tag_p tag = (ab_tag_p)raw_tag;

    pdebug(DEBUG_SPEW, "Starting.");

    /* is there data? */
    if(!tag->data) {
        pdebug(DEBUG_WARN,"Tag has no data!");
        return res;
    }

    /* is there enough data */
    if((offset < 0) || (offset + ((int)sizeof(uint32_t)) > tag->size)) {
        pdebug(DEBUG_WARN,"Data offset out of bounds.");
        return res;
    }

    if(!tag->is_bit) {
        res = ((uint32_t)(tag->data[offset])) +
              ((uint32_t)(tag->data[offset+1]) << 8) +
              ((uint32_t)(tag->data[offset+2]) << 16) +
              ((uint32_t)(tag->data[offset+3]) << 24);
    } else {
        if(ab_get_bit(raw_tag, 0)) {
            res = 1;
        } else {
            res = 0;
        }
    }

    return res;
}



int ab_set_uint32(plc_tag_p raw_tag, int offset, uint32_t val)
{
    int rc = PLCTAG_STATUS_OK;
    ab_tag_p tag = (ab_tag_p)raw_tag;

    pdebug(DEBUG_SPEW, "Starting.");

    /* is there data? */
    if(!tag->data) {
        pdebug(DEBUG_WARN,"Tag has no data!");
        return PLCTAG_ERR_NO_DATA;
    }

    /* is there enough data */
    if((offset < 0) || (offset + ((int)sizeof(uint32_t)) > tag->size)) {
        pdebug(DEBUG_WARN,"Data offset out of bounds.");
        return PLCTAG_ERR_OUT_OF_BOUNDS;
    }

    /* write the data. */
    if(!tag->is_bit) {
        tag->data[offset]   = (uint8_t)(val & 0xFF);
        tag->data[offset+1] = (uint8_t)((val >> 8) & 0xFF);
        tag->data[offset+2] = (uint8_t)((val >> 16) & 0xFF);
        tag->data[offset+3] = (uint8_t)((val >> 24) & 0xFF);
    } else {
        if(!val) {
            rc = ab_set_bit(raw_tag, 0, 0);
        } else {
            rc = ab_set_bit(raw_tag, 0, 1);
        }
    }

    return rc;
}




int32_t ab_get_int32(plc_tag_p raw_tag, int offset)
{
    int32_t res = INT32_MIN;
    ab_tag_p tag = (ab_tag_p)raw_tag;

    pdebug(DEBUG_SPEW, "Starting.");

    /* is there data? */
    if(!tag->data) {
        pdebug(DEBUG_WARN,"Tag has no data!");
        return res;
    }

    /* is there enough data */
    if((offset < 0) || (offset + ((int)sizeof(int32_t)) > tag->size)) {
        pdebug(DEBUG_WARN,"Data offset out of bounds.");
        return res;
    }

    if(!tag->is_bit) {
        res = (int32_t)(((uint32_t)(tag->data[offset])) +
                        ((uint32_t)(tag->data[offset+1]) << 8) +
                        ((uint32_t)(tag->data[offset+2]) << 16) +
                        ((uint32_t)(tag->data[offset+3]) << 24));
    } else {
        if(ab_get_bit(raw_tag, 0)) {
            res = 1;
        } else {
            res = 0;
        }
    }

    return res;
}



int ab_set_int32(plc_tag_p raw_tag, int offset, int32_t ival)
{
    uint32_t val = (uint32_t)(ival);
    int rc = PLCTAG_STATUS_OK;
    ab_tag_p tag = (ab_tag_p)raw_tag;

    pdebug(DEBUG_SPEW, "Starting.");

    /* is there data? */
    if(!tag->data) {
        pdebug(DEBUG_WARN,"Tag has no data!");
        return PLCTAG_ERR_NO_DATA;
    }

    /* is there enough data */
    if((offset < 0) || (offset + ((int)sizeof(int32_t)) > tag->size)) {
        pdebug(DEBUG_WARN,"Data offset out of bounds.");
        return PLCTAG_ERR_OUT_OF_BOUNDS;
    }

    if(!tag->is_bit) {
        tag->data[offset]   = (uint8_t)(val & 0xFF);
        tag->data[offset+1] = (uint8_t)((val >> 8) & 0xFF);
        tag->data[offset+2] = (uint8_t)((val >> 16) & 0xFF);
        tag->data[offset+3] = (uint8_t)((val >> 24) & 0xFF);
    } else {
        if(!val) {
            rc = ab_set_bit(raw_tag, 0, 0);
        } else {
            rc = ab_set_bit(raw_tag, 0, 1);
        }
    }

    return rc;
}









uint16_t ab_get_uint16(plc_tag_p raw_tag, int offset)
{
    uint16_t res = UINT16_MAX;
    ab_tag_p tag = (ab_tag_p)raw_tag;

    pdebug(DEBUG_SPEW, "Starting.");

    /* is there data? */
    if(!tag->data) {
        pdebug(DEBUG_WARN,"Tag has no data!");
        return res;
    }

    /* is there enough data */
    if((offset < 0) || (offset + ((int)sizeof(uint16_t)) > tag->size)) {
        pdebug(DEBUG_WARN,"Data offset out of bounds.");
        return res;
    }

    if(!tag->is_bit) {
        res = (uint16_t)((tag->data[offset]) +
                        ((tag->data[offset+1]) << 8));
    } else {
        if(ab_get_bit(raw_tag, 0)) {
            res = 1;
        } else {
            res = 0;
        }
    }

    return res;
}




int ab_set_uint16(plc_tag_p raw_tag, int offset, uint16_t val)
{
    int rc = PLCTAG_STATUS_OK;
    ab_tag_p tag = (ab_tag_p)raw_tag;

    pdebug(DEBUG_SPEW, "Starting.");

    /* is there data? */
    if(!tag->data) {
        pdebug(DEBUG_WARN,"Tag has no data!");
        return PLCTAG_ERR_NO_DATA;
    }

    /* is there enough data */
    if((offset < 0) || (offset + ((int)sizeof(uint16_t)) > tag->size)) {
        pdebug(DEBUG_WARN,"Data offset out of bounds.");
        return PLCTAG_ERR_OUT_OF_BOUNDS;
    }

    if(!tag->is_bit) {
        tag->data[offset]   = (uint8_t)(val & 0xFF);
        tag->data[offset+1] = (uint8_t)((val >> 8) & 0xFF);
    } else {
        if(!val) {
            rc = ab_set_bit(raw_tag, 0, 0);
        } else {
            rc = ab_set_bit(raw_tag, 0, 1);
        }
    }

    return rc;
}









int16_t  ab_get_int16(plc_tag_p raw_tag, int offset)
{
    int16_t res = INT16_MIN;
    ab_tag_p tag = (ab_tag_p)raw_tag;

    pdebug(DEBUG_SPEW, "Starting.");

    /* is there data? */
    if(!tag->data) {
        pdebug(DEBUG_WARN,"Tag has no data!");
        return res;
    }

    /* is there enough data */
    if((offset < 0) || (offset + ((int)sizeof(int16_t)) > tag->size)) {
        pdebug(DEBUG_WARN,"Data offset out of bounds.");
        return res;
    }

    if(!tag->is_bit) {
        res = (int16_t)(((tag->data[offset])) +
                        ((tag->data[offset+1]) << 8));
    } else {
        if(ab_get_bit(raw_tag, 0)) {
            res = 1;
        } else {
            res = 0;
        }
    }

    return res;
}




int ab_set_int16(plc_tag_p raw_tag, int offset, int16_t ival)
{
    uint16_t val = (uint16_t)(ival);
    int rc = PLCTAG_STATUS_OK;
    ab_tag_p tag = (ab_tag_p)raw_tag;

    pdebug(DEBUG_SPEW, "Starting.");

    /* is there data? */
    if(!tag->data) {
        pdebug(DEBUG_WARN,"Tag has no data!");
        return PLCTAG_ERR_NO_DATA;
    }

    /* is there enough data */
    if((offset < 0) || (offset + ((int)sizeof(int16_t)) > tag->size)) {
        pdebug(DEBUG_WARN,"Data offset out of bounds.");
        return PLCTAG_ERR_OUT_OF_BOUNDS;
    }

    if(!tag->is_bit) {
        tag->data[offset]   = (uint8_t)(val & 0xFF);
        tag->data[offset+1] = (uint8_t)((val >> 8) & 0xFF);
    } else {
        if(!val) {
            rc = ab_set_bit(raw_tag, 0, 0);
        } else {
            rc = ab_set_bit(raw_tag, 0, 1);
        }
    }

    return rc;
}



uint8_t ab_get_uint8(plc_tag_p raw_tag, int offset)
{
    uint8_t res = UINT8_MAX;
    ab_tag_p tag = (ab_tag_p)raw_tag;

    pdebug(DEBUG_SPEW, "Starting.");

    /* is there data? */
    if(!tag->data) {
        pdebug(DEBUG_WARN,"Tag has no data!");
        return res;
    }

    /* is there enough data */
    if((offset < 0) || (offset + ((int)sizeof(uint8_t)) > tag->size)) {
        pdebug(DEBUG_WARN,"Data offset out of bounds.");
        return res;
    }

    if(!tag->is_bit) {
        res = tag->data[offset];
    } else {
        if(ab_get_bit(raw_tag, 0)) {
            res = 1;
        } else {
            res = 0;
        }
    }

    return res;
}




int ab_set_uint8(plc_tag_p raw_tag, int offset, uint8_t val)
{
    int rc = PLCTAG_STATUS_OK;
    ab_tag_p tag = (ab_tag_p)raw_tag;

    pdebug(DEBUG_SPEW, "Starting.");

    /* is there data? */
    if(!tag->data) {
        pdebug(DEBUG_WARN,"Tag has no data!");
        return PLCTAG_ERR_NO_DATA;
    }

    /* is there enough data */
    if((offset < 0) || (offset + ((int)sizeof(uint8_t)) > tag->size)) {
        pdebug(DEBUG_WARN,"Data offset out of bounds.");
        return PLCTAG_ERR_OUT_OF_BOUNDS;
    }

    if(!tag->is_bit) {
        tag->data[offset] = val;
    } else {
        if(!val) {
            rc = ab_set_bit(raw_tag, 0, 0);
        } else {
            rc = ab_set_bit(raw_tag, 0, 1);
        }
    }

    return rc;
}





int8_t ab_get_int8(plc_tag_p raw_tag, int offset)
{
    int8_t res = INT8_MIN;
    ab_tag_p tag = (ab_tag_p)raw_tag;

    pdebug(DEBUG_SPEW, "Starting.");

    /* is there data? */
    if(!tag->data) {
        pdebug(DEBUG_WARN,"Tag has no data!");
        return res;
    }

    /* is there enough data */
    if((offset < 0) || (offset + ((int)sizeof(int8_t)) > tag->size)) {
        pdebug(DEBUG_WARN,"Data offset out of bounds.");
        return res;
    }

    if(!tag->is_bit) {
        res = (int8_t)(tag->data[offset]);
    } else {
        if(ab_get_bit(raw_tag, 0)) {
            res = 1;
        } else {
            res = 0;
        }
    }

    return res;
}




int ab_set_int8(plc_tag_p raw_tag, int offset, int8_t ival)
{
    uint8_t val = (uint8_t)(ival);
    int rc = PLCTAG_STATUS_OK;
    ab_tag_p tag = (ab_tag_p)raw_tag;

    pdebug(DEBUG_SPEW, "Starting.");

    /* is there data? */
    if(!tag->data) {
        pdebug(DEBUG_WARN,"Tag has no data!");
        return PLCTAG_ERR_NO_DATA;
    }

    /* is there enough data */
    if((offset < 0) || (offset + ((int)sizeof(int8_t)) > tag->size)) {
        pdebug(DEBUG_WARN,"Data offset out of bounds.");
        return PLCTAG_ERR_OUT_OF_BOUNDS;
    }

    if(!tag->is_bit) {
        tag->data[offset] = (uint8_t)val;
    } else {
        if(!val) {
            rc = ab_set_bit(raw_tag, 0, 0);
        } else {
            rc = ab_set_bit(raw_tag, 0, 1);
        }
    }

    return rc;
}






double ab_get_float64(plc_tag_p raw_tag, int offset)
{
    uint64_t ures = 0;
    double res = DBL_MAX;
    ab_tag_p tag = (ab_tag_p)raw_tag;

    pdebug(DEBUG_SPEW, "Starting.");

    if(tag->is_bit) {
        pdebug(DEBUG_WARN, "Getting float64 value is unsupported on a bit tag!");
        return res;
    }

    /* is there data? */
    if(!tag->data) {
        pdebug(DEBUG_WARN,"Tag has no data!");
        return res;
    }

    /* is there enough data */
    if((offset < 0) || (offset + ((int)sizeof(ures)) > tag->size)) {
        pdebug(DEBUG_WARN,"Data offset out of bounds.");
        return res;
    }

    ures = ((uint64_t)(tag->data[offset])) +
           ((uint64_t)(tag->data[offset+1]) << 8) +
           ((uint64_t)(tag->data[offset+2]) << 16) +
           ((uint64_t)(tag->data[offset+3]) << 24) +
           ((uint64_t)(tag->data[offset+4]) << 32) +
           ((uint64_t)(tag->data[offset+5]) << 40) +
           ((uint64_t)(tag->data[offset+6]) << 48) +
           ((uint64_t)(tag->data[offset+7]) << 56);

    /* copy the data */
    mem_copy(&res,&ures,sizeof(res));

    return res;
}




int ab_set_float64(plc_tag_p raw_tag, int offset, double fval)
{
    int rc = PLCTAG_STATUS_OK;
    uint64_t val = 0;
    ab_tag_p tag = (ab_tag_p)raw_tag;

    pdebug(DEBUG_SPEW, "Starting.");

    if(tag->is_bit) {
        pdebug(DEBUG_WARN, "Setting float64 value is unsupported on a bit tag!");
        return PLCTAG_ERR_UNSUPPORTED;
    }

    mem_copy(&val, &fval, sizeof(val));

    /* is there data? */
    if(!tag->data) {
        pdebug(DEBUG_WARN,"Tag has no data!");
        return PLCTAG_ERR_NO_DATA;
    }

    /* is there enough data */
    if((offset < 0) || (offset + ((int)sizeof(val)) > tag->size)) {
        pdebug(DEBUG_WARN,"Data offset out of bounds.");
        return PLCTAG_ERR_OUT_OF_BOUNDS;
    }

    tag->data[offset]   = (uint8_t)(val & 0xFF);
    tag->data[offset+1] = (uint8_t)((val >> 8) & 0xFF);
    tag->data[offset+2] = (uint8_t)((val >> 16) & 0xFF);
    tag->data[offset+3] = (uint8_t)((val >> 24) & 0xFF);
    tag->data[offset+4] = (uint8_t)((val >> 32) & 0xFF);
    tag->data[offset+5] = (uint8_t)((val >> 40) & 0xFF);
    tag->data[offset+6] = (uint8_t)((val >> 48) & 0xFF);
    tag->data[offset+7] = (uint8_t)((val >> 56) & 0xFF);

    return rc;
}



float ab_get_float32(plc_tag_p raw_tag, int offset)
{
    uint32_t ures;
    float res = FLT_MAX;
    ab_tag_p tag = (ab_tag_p)raw_tag;

    pdebug(DEBUG_SPEW, "Starting.");

    if(tag->is_bit) {
        pdebug(DEBUG_WARN, "Getting float32 value is unsupported on a bit tag!");
        return res;
    }

    /* is there data? */
    if(!tag->data) {
        pdebug(DEBUG_WARN,"Tag has no data!");
        return res;
    }

    /* is there enough data */
    if((offset < 0) || (offset + ((int)sizeof(ures)) > tag->size)) {
        pdebug(DEBUG_WARN,"Data offset out of bounds.");
        return res;
    }

    ures = ((uint32_t)(tag->data[offset])) +
           ((uint32_t)(tag->data[offset+1]) << 8) +
           ((uint32_t)(tag->data[offset+2]) << 16) +
           ((uint32_t)(tag->data[offset+3]) << 24);

    /* copy the data */
    mem_copy(&res,&ures,sizeof(res));

    return res;
}




int ab_set_float32(plc_tag_p raw_tag, int offset, float fval)
{
    int rc = PLCTAG_STATUS_OK;
    uint32_t val = 0;
    ab_tag_p tag = (ab_tag_p)raw_tag;

    pdebug(DEBUG_SPEW, "Starting.");

    if(tag->is_bit) {
        pdebug(DEBUG_WARN, "Setting float32 value is unsupported on a bit tag!");
        return PLCTAG_ERR_UNSUPPORTED;
    }

    mem_copy(&val, &fval, sizeof(val));

    /* is there data? */
    if(!tag->data) {
        pdebug(DEBUG_WARN,"Tag has no data!");
        return PLCTAG_ERR_NO_DATA;
    }

    /* is there enough data */
    if((offset < 0) || (offset + ((int)sizeof(val)) > tag->size)) {
        pdebug(DEBUG_WARN,"Data offset out of bounds.");
        return PLCTAG_ERR_OUT_OF_BOUNDS;
    }

    tag->data[offset]   = (uint8_t)(val & 0xFF);
    tag->data[offset+1] = (uint8_t)((val >> 8) & 0xFF);
    tag->data[offset+2] = (uint8_t)((val >> 16) & 0xFF);
    tag->data[offset+3] = (uint8_t)((val >> 24) & 0xFF);

    return rc;
}





plc_type_t get_plc_type(attr attribs)
{
    const char *cpu_type = attr_get_str(attribs, "plc", attr_get_str(attribs, "cpu", "NONE"));

    if (!str_cmp_i(cpu_type, "plc") || !str_cmp_i(cpu_type, "plc5")) {
        pdebug(DEBUG_DETAIL,"Found PLC/5 PLC.");
        return AB_PROTOCOL_PLC;
    } else if ( !str_cmp_i(cpu_type, "slc") || !str_cmp_i(cpu_type, "slc500")) {
        pdebug(DEBUG_DETAIL,"Found SLC 500 PLC.");
        return AB_PROTOCOL_SLC;
    } else if (!str_cmp_i(cpu_type, "lgxpccc") || !str_cmp_i(cpu_type, "logixpccc") || !str_cmp_i(cpu_type, "lgxplc5") || !str_cmp_i(cpu_type, "logixplc5") ||
               !str_cmp_i(cpu_type, "lgx-pccc") || !str_cmp_i(cpu_type, "logix-pccc") || !str_cmp_i(cpu_type, "lgx-plc5") || !str_cmp_i(cpu_type, "logix-plc5")) {
        pdebug(DEBUG_DETAIL,"Found Logix-class PLC using PCCC protocol.");
        return AB_PROTOCOL_LGX_PCCC;
    } else if (!str_cmp_i(cpu_type, "micrologix800") || !str_cmp_i(cpu_type, "mlgx800") || !str_cmp_i(cpu_type, "micro800")) {
        pdebug(DEBUG_DETAIL,"Found Micro8xx PLC.");
        return AB_PROTOCOL_MLGX800;
    } else if (!str_cmp_i(cpu_type, "micrologix") || !str_cmp_i(cpu_type, "mlgx")) {
        pdebug(DEBUG_DETAIL,"Found MicroLogix PLC.");
        return AB_PROTOCOL_MLGX;
    } else if (!str_cmp_i(cpu_type, "compactlogix") || !str_cmp_i(cpu_type, "clgx") || !str_cmp_i(cpu_type, "lgx") ||
               !str_cmp_i(cpu_type, "controllogix") || !str_cmp_i(cpu_type, "contrologix") ||
               !str_cmp_i(cpu_type, "logix")) {
        pdebug(DEBUG_DETAIL,"Found ControlLogix/CompactLogix PLC.");
        return AB_PROTOCOL_LGX;
    } else {
        pdebug(DEBUG_WARN, "Unsupported device type: %s", cpu_type);

        return AB_PROTOCOL_NONE;
    }
}



int check_cpu(ab_tag_p tag, attr attribs)
{
    plc_type_t result = get_plc_type(attribs);

    if(result != AB_PROTOCOL_NONE) {
        tag->protocol_type = result;
        return PLCTAG_STATUS_OK;
    } else {
        tag->protocol_type = result;
        return PLCTAG_ERR_BAD_DEVICE;
    }
}

int check_tag_name(ab_tag_p tag, const char* name)
{
    int rc = PLCTAG_STATUS_OK;

    if (!name) {
        pdebug(DEBUG_WARN,"No tag name parameter found!");
        return PLCTAG_ERR_BAD_PARAM;
    }

    /* attempt to parse the tag name */
    switch (tag->protocol_type) {
    case AB_PROTOCOL_PLC:
    case AB_PROTOCOL_LGX_PCCC:
        if ((rc = plc5_encode_tag_name(tag->encoded_name, &(tag->encoded_name_size), &(tag->file_type), name, MAX_TAG_NAME)) != PLCTAG_STATUS_OK) {
            pdebug(DEBUG_WARN, "parse of PLC/5-style tag name %s failed!", name);

            return rc;
        }

        break;

    case AB_PROTOCOL_SLC:
    case AB_PROTOCOL_MLGX:
        if ((rc = slc_encode_tag_name(tag->encoded_name, &(tag->encoded_name_size), &(tag->file_type), name, MAX_TAG_NAME)) != PLCTAG_STATUS_OK) {
            pdebug(DEBUG_WARN, "parse of SLC-style tag name %s failed!", name);

            return rc;
        }

        break;

    case AB_PROTOCOL_MLGX800:
    case AB_PROTOCOL_LGX:
        if ((rc = cip_encode_tag_name(tag, name)) != PLCTAG_STATUS_OK) {
            pdebug(DEBUG_WARN, "parse of CIP-style tag name %s failed!", name);

            return rc;
        }

        break;

    default:
        /* how would we get here? */
        pdebug(DEBUG_WARN, "unsupported protocol %d", tag->protocol_type);

        return PLCTAG_ERR_BAD_PARAM;

        break;
    }

    return PLCTAG_STATUS_OK;
}




///*
// * setup_session_mutex
// *
// * check to see if the global mutex is set up.  If not, do an atomic
// * lock and set it up.
// */
//int setup_session_mutex(void)
//{
//    int rc = PLCTAG_STATUS_OK;
//
//    pdebug(DEBUG_INFO, "Starting.");
//
//    critical_block(global_library_mutex) {
//        /* first see if the mutex is there. */
//        if (!global_session_mut) {
//            rc = mutex_create((mutex_p*)&global_session_mut);
//
//            if (rc != PLCTAG_STATUS_OK) {
//                pdebug(DEBUG_ERROR, "Unable to create global tag mutex!");
//            }
//        }
//    }
//
//    pdebug(DEBUG_INFO, "Done.");
//
//    return rc;
//}

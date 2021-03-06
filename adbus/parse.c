/* vim: ts=4 sw=4 sts=4 et
 *
 * Copyright (c) 2009 James R. McKaskill
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * ----------------------------------------------------------------------------
 */

#define ADBUS_LIBRARY
#include "misc.h"
#include <stddef.h>

/** \struct adbus_Message
 *  
 *  \brief Container for parsed messages
 *
 *  A message structure can be filled out by using adbus_parse() or it may be
 *  filled out manually. When filling out manually all fields except arguments
 *  are required to be set if they are in the message (otherwise they should
 *  be zero intialised). Most places that require arguments to be set should
 *  call adbus_parseargs() before using it.
 *
 *  \warning adbus_parseargs() allocates some space on the heap to store the
 *  arguments vector. Call adbus_freeargs() to free the arguments vector when
 *  finished using them.
 *
 *  For example to parse some incoming data and dispatch using
 *  adbus_conn_dispatch() (this is just a simple version of
 *  adbus_conn_parse()):
 *
 *  \code
 *  // These initialised somewhere else
 *  static adbus_Buffer* buf;
 *  static adbus_Connect* connection;
 *  int ParseData(char* data, size_t sz)
 *  {
 *      // Copy the data into a buffer to ensure its 8 byte aligned
 *      adbus_buf_append(buf, data, sz);
 *      size_t msgsz = 0;
 *      while (1)
 *      {
 *          char* msgdata = adbus_buf_data(buf);
 *          size_t bufdata = adbus_buf_size(buf);
 *          msgsz = adbus_parse_size(msgdata, bufdata);
 *          // Need more data
 *          if (msgsz == 0 || msgsz > bufdata)
 *              return 0;
 *          
 *          adbus_Message m = {};
 *          
 *          // Check for parse errors
 *          if (adbus_parse(&m, msgdata, msgsz))
 *              return -1;
 *
 *          // adbus_conn_dispatch() will return an error on a parse error
 *          // detected further down
 *          int ret = adbus_conn_dispatch(connection, &m);
 *          adbus_freeargs(&m);
 *          if (ret)
 *              return ret;
 *      }
 *  }
 *  \endcode
 */

/** \var adbus_Message::data
 *  Beginning of message data (must be 8 byte aligned).
 */

/** \var adbus_Message::size
 *  Size of message data.
 */

/** \var adbus_Message::argdata
 *  Beginning of argument data.
 */

/** \var adbus_Message::argsize
 *  Size of argument data.
 */

/** \var adbus_Message::type
 *  Type of message.
 */

/** \var adbus_Message::flags
 *  Message flags.
 */

/** \var adbus_Message::serial
 *  Message serial - used to correlate method calls with replies.
 */

/** \var adbus_Message::signature
 *  Argument signature or NULL if not present.
 */

/** \var adbus_Message::signatureSize
 *  Length of adbus_Message::signature field.
 */

/** \var adbus_Message::replySerial
 *  Pointer to reply serial value of NULL if not present.
 */

/** \var adbus_Message::path
 *  Object path header field or NULL if not present.
 */

/** \var adbus_Message::pathSize
 *  Length of path field.
 */

/** \var adbus_Message::interface
 *  Interface header field or NULL if not present.
 */

/** \var adbus_Message::interfaceSize
 *  Length of interface field.
 */


/** \var adbus_Message::member
 *  Member header field or NULL if not present.
 */

/** \var adbus_Message::memberSize
 *  Length of member field.
 */


/** \var adbus_Message::error
 *  Error name header field or NULL if not present.
 */

/** \var adbus_Message::errorSize
 *  Length of error field.
 */

/** \var adbus_Message::destination
 *  Destination header field or NULL if not present.
 */

/** \var adbus_Message::destinationSize
 *  Length of destination field.
 */

/** \var adbus_Message::sender
 *  Sender header field or NULL if not present.
 */

/** \var adbus_Message::senderSize
 *  Length of sender field.
 */

/** \var adbus_Message::arguments
 *  Array of unpacked arguments.
 *
 *  This should only be used for matching against match rules. For proper
 *  unpacking use adbus_Iterator.
 */

/** \var adbus_Message::argumentsSize
 *  Size of arguments field.
 */

/* -------------------------------------------------------------------------- */
// Manually unpack even for native endianness since value not be 4 byte
// aligned
static uint32_t Get32(char endianness, uint32_t* value)
{
    uint8_t* p = (uint8_t*) value;
    if (endianness == 'l') {
        return  ((uint32_t) p[0]) 
            |   ((uint32_t) p[1] << 8)
            |   ((uint32_t) p[2] << 16)
            |   ((uint32_t) p[3] << 24);
    } else {
        return  ((uint32_t) p[3]) 
            |   ((uint32_t) p[2] << 8)
            |   ((uint32_t) p[1] << 16)
            |   ((uint32_t) p[0] << 24);
    }
}

/** Figures out the size of a message.
 *
 *  \relates adbus_Message
 *
 *  The data does not need to be aligned.
 *
 *  \returns The message size or 0 if there is insufficient data to figure out
 *  the message size.
 *
 */
size_t adbus_parse_size(const char* data, size_t size)
{
    if (size < sizeof(adbusI_ExtendedHeader))
        return 0;

    adbusI_ExtendedHeader* h = (adbusI_ExtendedHeader*) data;
    size_t hsize = sizeof(adbusI_Header) + Get32(h->endianness, &h->headerFieldLength);
    return ADBUSI_ALIGN(hsize, 8) + Get32(h->endianness, &h->length);
}

/** Fills out an adbus_Message by parsing the data.
 *
 *  \relates adbus_Message
 *
 *  \param[in] m    The message structure to fill out.
 *  \param[in] data Data to parse. This \e must be 8 byte aligned.
 *  \param[in] size The size of the message. This \e must be the size of the
 *                  message exactly.
 *
 *  Parses a message and fills out the message structure. It will endian flip
 *  the data if it is not native (hence char* instead of const char*). The
 *  pointers in the message structure will point into the passed data.
 *
 *  Due to the 8 byte alignment the size normally needs to be known
 *  beforehand to copy the data into an 8 byte aligned buffer.
 *  adbus_parse_size() can be used to figure out the message size.
 *
 *  \sa adbus_parse_size()
 */ 
int adbus_parse(adbus_Message* m, char* data, size_t size)
{
    assert((char*) ADBUSI_ALIGN(data, 8) == data);
    assert(size > sizeof(adbusI_ExtendedHeader));

    adbusI_ExtendedHeader* h = (adbusI_ExtendedHeader*) data;
    adbus_Bool native = (h->endianness == adbusI_nativeEndianness());

    if (h->type == ADBUS_MSG_INVALID)
        return -1;
    else if (h->type > ADBUS_MSG_SIGNAL)
        return 0;

    if (!native && adbus_flip_data(data, size, "yyyyuua(yv)"))
        return -1;

    h->endianness = adbusI_nativeEndianness();

    memset(m, 0, sizeof(adbus_Message));
    m->data     = data;
    m->size     = ADBUSI_ALIGN(h->headerFieldLength + sizeof(adbusI_ExtendedHeader), 8) + h->length;
    m->argsize  = h->length;
    m->argdata  = data + m->size - m->argsize;
    m->type     = (adbus_MessageType) h->type;
    m->flags    = h->flags;
    m->serial   = h->serial;

    adbus_Iterator i = {
        data + sizeof(adbusI_Header),
        size - sizeof(adbusI_Header),
        "a(yv)"
    };
    adbus_IterArray a;
    if (adbus_iter_beginarray(&i, &a))
        return -1;

    while (adbus_iter_inarray(&i, &a)) {
        const uint8_t* code;
        adbus_IterVariant v;
        if (    adbus_iter_beginstruct(&i) 
            ||  adbus_iter_u8(&i, &code)
            ||  adbus_iter_beginvariant(&i, &v))
        {
            return -1;
        }
        const char** pstr;
        size_t* psize;
        switch (*code) {
            case HEADER_INVALID:
                return -1;

            case HEADER_INTERFACE:
                pstr  = &m->interface;
                psize = &m->interfaceSize;
                goto string;

            case HEADER_MEMBER:
                pstr  = &m->member;
                psize = &m->memberSize;
                goto string;

            case HEADER_ERROR_NAME:
                pstr  = &m->error;
                psize = &m->errorSize;
                goto string;

            case HEADER_DESTINATION:
                pstr  = &m->destination;
                psize = &m->destinationSize;
                goto string;

            case HEADER_SENDER:
                pstr  = &m->sender;
                psize = &m->senderSize;
                goto string;

            string:
                if (    i.sig[0] != 's'
                    ||  i.sig[1] != '\0'
                    ||  adbus_iter_string(&i, pstr, psize))
                {
                    return -1;
                }
                break;

            case HEADER_OBJECT_PATH:
                if (    i.sig[0] != 'o' 
                    ||  i.sig[1] != '\0'
                    ||  adbus_iter_objectpath(&i, &m->path, &m->pathSize))
                {
                    return -1;
                }
                break;

            case HEADER_SIGNATURE:
                if (    i.sig[0] != 'g' 
                    ||  i.sig[1] != '\0'
                    ||  adbus_iter_signature(&i, &m->signature, NULL))
                {
                    return -1;
                }
                break;

            case HEADER_REPLY_SERIAL:
                if (    i.sig[0] != 'u' 
                    ||  i.sig[1] != '\0'
                    ||  adbus_iter_u32(&i, &m->replySerial))
                {
                    return -1;
                }
                break;

            default:
                if (adbus_iter_value(&i)) {
                    return -1;
                }
                break;
        }

        if (adbus_iter_endvariant(&i, &v) || adbus_iter_endstruct(&i))
            return -1;

    }

    if (adbus_iter_endarray(&i, &a))
        return -1;

    /* Check that we have the required fields */
    if (m->type == ADBUS_MSG_METHOD && (!m->path || !m->member)) {
        return -1;
    } else if (m->type == ADBUS_MSG_RETURN && !m->replySerial) {
        return -1;
    } else if (m->type == ADBUS_MSG_ERROR && !m->error) {
        return -1;
    } else if (m->type == ADBUS_MSG_SIGNAL &&  (!m->interface || !m->member)) {
        return -1;
    } else if (m->argsize > 0 && !m->signature) {
        return -1;
    }

    if (!native && m->signature && adbus_flip_data((char*) m->argdata, m->argsize, m->signature))
        return -1;

    return 0;
}

DVECTOR_INIT(Argument, adbus_Argument);
/** Parse the arguments in a message.
 *  
 *  \relates adbus_Message
 *
 *  This should be called after filling out the message struct using
 *  adbus_parse() or manually and will fill out the arguments and
 *  argumentsSize fields. These fields only tell you about string arguments
 *  and should only be used for matching match rules.
 *
 *  \warning The argument vector filled out in the message structure is
 *  allocated on the heap. You must use adbus_freeargs() after using the
 *  message to free this.
 *
 */
int adbus_parseargs(adbus_Message* m)
{
    if (m->arguments)
        return 0;

    d_Vector(Argument) args;
    ZERO(&args);

    adbus_Iterator i = {m->argdata, m->argsize, m->signature};

    assert(m->signature != NULL);

    while (*i.sig) {
        adbus_Argument* arg = dv_push(Argument, &args, 1);
        arg->value = NULL;
        arg->size  = 0;

        if (*i.sig == 's') {
            size_t sz;
            if (adbus_iter_string(&i, &arg->value, &sz))
                goto err;
            arg->size = sz;

        } else {
            if (adbus_iter_value(&i))
                goto err;

        }
    }

    m->argumentsSize = dv_size(&args);
    m->arguments = dv_release(Argument, &args);
    return 0;

err:
    dv_free(Argument, &args);
    return -1;
}

/** Frees the argument vector allocated by adbus_parse_args().
 *  \relates adbus_Message
 */
void adbus_freeargs(adbus_Message* m)
{
    if (m) {
        free(m->arguments);
        m->argumentsSize = 0;
    }
}

/** Clones the message data in \a from into \a to.
 *  \relates adbus_Message
 *
 *  Afterwards the message needs to be freed via adbus_freedata.
 */
void adbus_clonedata(adbus_Message* from, adbus_Message* to)
{
    *to = *from;

    char* data = (char*) malloc(from->size);
    memcpy(data, from->data, from->size);
    to->data = data;

    ptrdiff_t off = from->data - to->data;

    // Update all data pointers to point into the new data section
    to->argdata += off;
    to->signature += off;
    to->replySerial += off;
    to->path += off;
    to->interface += off;
    to->member += off;
    to->error += off;
    to->destination += off;
    to->sender += off;

    if (from->arguments) {
        to->arguments = (adbus_Argument*) malloc(sizeof(adbus_Argument) * from->argumentsSize);

        for (size_t i = 0; i < to->argumentsSize; i++) {
            if (to->arguments[i].value) {
                to->arguments[i].value += off;
            }
        }
    }

}

void adbus_freedata(adbus_Message* m)
{
    adbus_freeargs(m);
    if (m) {
        free((char*) m->data);
    }
}


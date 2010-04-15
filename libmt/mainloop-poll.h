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

#pragma once

#include "internal.h"
#include "message-queue.h"
#include <dmem/vector.h>

#ifndef _WIN32

#include <poll.h>

DVECTOR_INIT(LoopRegistration, MT_LoopRegistration*)
DVECTOR_INIT(pollfd, struct pollfd)

struct MT_LoopRegistration
{
    MT_Handle                   fd;
    MT_Time                     period;

    MT_Callback                 read;
    MT_Callback                 write;
    MT_Callback                 close;
    MT_Callback                 idle;
    void*                       user;
};

struct MT_MainLoop
{
    int                         exit;
    int                         exitcode;

    d_Vector(LoopRegistration)  regs;
    d_Vector(pollfd)            events;
    int                         currentEvent;

    d_Vector(LoopRegistration)  idle;
    int                         currentIdle;

    MTI_MessageQueue            queue;
};


#endif
// vim: ts=2 sw=2 sts=2 et
//
// Copyright (c) 2009 James R. McKaskill
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//
// ----------------------------------------------------------------------------

#pragma once

#include "Common.h"


#ifdef __cplusplus
extern "C" {
#endif


// ----------------------------------------------------------------------------

struct ADBusParser;
struct ADBusMessage;

struct ADBusParser* ADBusCreateParser();
void ADBusFreeParser(struct ADBusParser* parser);

// Returns an error code or 0 on none
int  ADBusParse(struct ADBusParser* parser, uint8_t* data, size_t size);

typedef void (*ADBusParserCallback)(void* /*userData*/, struct ADBusMessage*);

void ADBusSetParserCallback(struct ADBusParser* parser,
                           ADBusParserCallback callback, void* userData);

// ----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif
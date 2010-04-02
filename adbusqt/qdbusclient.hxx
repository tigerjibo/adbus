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

#include "qdbusproxy.hxx"
#include <adbus.h>
#include <adbuscpp.h>
#include <dmem/list.h>
#include <QtCore/qthreadstorage.h>
#include <QtCore/qstring.h>

class QIODevice;
class QAbstractSocket;

class QDBUS_EXPORT QDBusClient : public QObject 
{
    Q_OBJECT
public:
    QDBusClient();

    bool connectToServer(adbus_BusType type, bool connectToBus = true);
    bool connectToServer(const char* envstr, bool connectToBus = true);

    bool waitForConnected();

    adbus_Connection* base() {return m_Connection;}

Q_SIGNALS:
    void connected();
    void disconnected();

private Q_SLOTS:
    void socketReadyRead();
    void socketConnected();
    void disconnect();
    void appQuitting();

private:
    ~QDBusClient();

    virtual bool event(QEvent* e);

    static int                      SendMsg(void* u, adbus_Message* m);
    static int                      Send(void* u, const char* b, size_t sz);
    static int                      Recv(void* u, char* buf, size_t sz);
    static uint8_t                  Rand(void* u);
    static void                     Proxy(void* u, adbus_Callback cb, adbus_Callback release, void* cbuser);
    static adbus_Bool               ShouldProxy(void* u);
    static void                     GetProxy(void* u, adbus_ProxyCallback* cb, adbus_ProxyMsgCallback* msgcb, void** data);
    static int                      Block(void* u, adbus_BlockType type, void** data, int timeoutms);
    static void                     ConnectedToBus(void* u);
    static void                     Free(void* u);

    static const adbus_ConnVTable   s_VTable;

    adbus_Connection*               m_Connection;
    bool                            m_ConnectToBus;
    bool                            m_Connected;
    bool                            m_AppHasQuit;
    adbus_Bool                      m_Authenticated;
    adbus_Auth*                     m_Auth;
    adbus_Buffer*                   m_Buffer;
    QIODevice*                      m_IODevice;
    QString                         m_UniqueName;
};

inline int operator<<(QString& v, adbus::Iterator& i)
{
  i.Check(ADBUS_STRING);

  const char* str;
  size_t sz;
  if (adbus_iter_string(i, &str, &sz))
    return -1;

  v = QString::fromUtf8(str, sz);
  return 0;
}

inline void operator>>(const QString& v, adbus::Buffer& b)
{ 
    QByteArray utf8 = v.toUtf8();
    adbus_buf_string(b.b, utf8.constData(), utf8.size());
}

template<class T>
inline int operator<<(QList<T>& v, adbus::Iterator& i)
{
    adbus_IterArray a;
    i.Check(ADBUS_ARRAY);
    if (adbus_iter_beginarray(i, &a))
        return -1;
    while (adbus_iter_inarray(i, &a)) {
        T t;
        if (t << i)
            return -1;
        v.push_back(t);
    }
    return adbus_iter_endarray(i, &a);
}

template<class T>
inline void operator>>(const QList<T>& v, adbus::Buffer& b)
{ 
    adbus_BufArray a;
    adbus_buf_beginarray(b, &a);

    for (int i = 0; i < v.size(); ++i) {
        adbus_buf_arrayentry(b, &a);
        v[i] >> b;
    }

    adbus_buf_endarray(b, &a);
}

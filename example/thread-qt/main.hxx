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

#include <adbusqt/qdbusclient.hxx>
#include <QtCore/qobject.h>
#include <QtCore/qthread.h>

class Pinger : public QObject, public adbus::State
{
    Q_OBJECT
public:
    Pinger(const adbus::Connection& c);

public Q_SLOTS:
    void start();

Q_SIGNALS:
    void finished();

private:
    void asyncPing();
    void response(const char* str);
    void error(const char* name, const char* msg);
    void sendingMessage();
    void haveReply();

    adbus::Connection   m_Connection;
    adbus::Proxy        m_Proxy;
    int                 m_LeftToSend;
    int                 m_LeftToReceive;
};

class PingThread : public QThread
{
    Q_OBJECT
public:
    PingThread(const adbus::Connection& c)
        : m_Connection(c)
    {}

    void run();

private:
    adbus::Connection m_Connection;
};

class Main : public QObject
{
    Q_OBJECT
public:
    Main(const adbus::Connection& c);
    ~Main();

public Q_SLOTS:
    void threadFinished();

private:
    int                 m_ThreadsLeft;
    QList<PingThread*>  m_Threads;
};


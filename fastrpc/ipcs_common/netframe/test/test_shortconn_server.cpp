// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description: test short conn server

#include "netframe.h"
#include "socket_handler.h"
#include "thread.h"

using namespace netframe;

class TestStreamSocketHandler : public LineStreamSocketHandler
{
public:
    explicit TestStreamSocketHandler(NetFrame& netframe):
        LineStreamSocketHandler(netframe)
    {
    }

    virtual bool OnSent(Packet* packet)
    {
        GetNetFrame().CloseEndPoint(GetEndPoint());
        return true;
    }
    void OnReceived(const Packet& packet)
    {
        // printf("OnPacketReceived, size=%zd\n", packet.Length());
        if (GetNetFrame().SendPacket(GetEndPoint(), packet.Content(), packet.Length()) < 0)
            fprintf(stderr, "SendPacket error\n");
    }
    virtual void OnConnected() {}
    virtual void OnClose(int error_code)
    {
        // printf("OnClosed\n");
    }
};

class TestListenSocketHandler : public ListenSocketHandler
{
public:
    explicit TestListenSocketHandler(NetFrame& netframe):
        ListenSocketHandler(netframe)
    {
    }
    StreamSocketHandler* OnAccepted(SocketId id)
    {
        StreamSocket socket;
        SocketAddressInet4 socket_addr;
        socket.Attach(id.GetFd());
        socket.GetLocalAddress(&socket_addr);
        socket.SetLinger(true, 1);
        socket.Detach();
        return new TestStreamSocketHandler(GetNetFrame());
    }
    void OnConnected() {}
    void OnClose(int error_code)
    {
    }
};

int main()
{
    NetFrame netframe;
    NetFrame::ListenEndPoint listen_endpoint;
    if (netframe.AsyncListen(SocketAddressInet("127.0.0.1:10000"),
                             new TestListenSocketHandler(netframe), 32768) < 0)
        return EXIT_FAILURE;
    sleep(-1);
}


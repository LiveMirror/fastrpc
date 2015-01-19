// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description: netframe test

#include "netframe.h"

#include "socket_handler.h"
#include "this_thread.h"

#include "logging.h"
#include "gtest.h"

const size_t kMaxPacketSize = 1024 * 1024;
const size_t kMaxDatagramPacketSize = 500 * 1024;

std::string message(kMaxPacketSize - 1, 'A');
std::string& message_ref = (message += '\n');
std::string datagram_message(kMaxDatagramPacketSize - 1, 'A');

using namespace netframe;

class TestStreamSocketHandler : public LineStreamSocketHandler
{
public:
    TestStreamSocketHandler(NetFrame& netframe, bool urgent = false) :
        LineStreamSocketHandler(netframe)
    {
        m_urgent = urgent;
    }

    virtual bool OnSent(Packet* packet)
    {
        VLOG(3) << "OnPacketSent, size=" << packet->Length();
        return true;
    }

    virtual void OnReceived(const Packet& packet)
    {
        VLOG(3) << "OnPacketReceived, size=" << packet.Length();
        if (m_urgent) {
            Packet* packet = new Packet();
            packet->SetContent(message.data(), message.length());
            if (GetNetFrame().SendPacket(GetEndPoint(), packet,
                                         m_urgent)) {
                LOG(ERROR) << "SendPacket error";
            }
        } else {
            if (GetNetFrame().SendPacket(GetEndPoint(), message.data(),
                                         message.length()) < 0) {
                LOG(ERROR) << "SendPacket error";
            }
        }
    }

    virtual void OnConnected()
    {
        VLOG(3) << "OnConnected";
        GetNetFrame().SendPacket(GetEndPoint(), message.data(), message.length());
    }

    virtual void OnClose(int error_code)
    {
        VLOG(3) << __PRETTY_FUNCTION__ << ": " << strerror(error_code);
    }

private:
    bool m_urgent;
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
        VLOG(3) << "OnAccepted";
        return new TestStreamSocketHandler(GetNetFrame());
    }
    void OnConnected()
    {
        VLOG(3) << "Server OnConnected";
    }
    void OnClose(int error_code)
    {
        VLOG(3) << __PRETTY_FUNCTION__ << ": " << strerror(error_code);
    }
};

class TestDatagramSocketHandler : public DatagramSocketHandler
{
public:
    explicit TestDatagramSocketHandler(NetFrame& netframe) :
        DatagramSocketHandler(netframe)
    {
    }

    virtual void OnReceived(const Packet& packet)
    {
        VLOG(3) << "OnReceived";
        if (GetNetFrame().SendPacket(GetEndPoint(),
                                     packet.GetRemoteAddress(),
                                     packet.Content(),
                                     packet.Length()) < 0) {
            LOG(ERROR) << "SendPacket Error";
        }
    }

    virtual bool OnSent(Packet* packet)
    {
        VLOG(3) << "OnSent";
        return true;
    }

    virtual void OnClose(int error_code)
    {
        VLOG(3) << "OnClose: " << error_code;
    }

private:
    SocketAddressInet m_remote_address;
};

class DatagramConnectionTest : public testing::Test
{
protected:
    virtual void SetUp()
    {
        m_net_frame = new NetFrame(4);
        SocketAddressInet server_address("127.0.0.1:0");
        int bind_socket_id = m_net_frame->AsyncDatagramBind(
                server_address,
                new TestDatagramSocketHandler(*m_net_frame),
                kMaxDatagramPacketSize);
        ListenerSocket socket;
        socket.Attach(SocketId(bind_socket_id).GetFd());
        socket.GetLocalAddress(&m_server_address);
        socket.Detach();
    }

    virtual void TearDown()
    {
        delete m_net_frame;
    }

protected:
    NetFrame* m_net_frame;
    SocketAddressInet m_server_address;
};

class StreamConnectionTest : public testing::Test
{
protected:
    virtual void SetUp()
    {
        m_net_frame = new NetFrame(4);
        SocketAddressInet server_address("127.0.0.1:0");
        NetFrame::EndPointOptions options;
        options.SendBufferSize(65535);
        options.ReceiveBufferSize(65535);
        options.Priority(5); // Qos priority, should be 0 ~ 6
        int listener_socket_id = m_net_frame->AsyncListen(server_address,
                                                          new TestListenSocketHandler(*m_net_frame),
                                                          kMaxPacketSize,
                                                          options);
        if (listener_socket_id < 0) {
            ASSERT_FALSE(true) << "SetUp AsyncListen error";
        }
        ListenerSocket socket;
        socket.Attach(SocketId(listener_socket_id).GetFd());
        socket.GetLocalAddress(&m_server_address);
        socket.Detach();
    }

    virtual void TearDown()
    {
        delete m_net_frame;
    }

    size_t GetCurrentSendBufferedLength() const
    {
        return m_net_frame->GetCurrentSendBufferedLength();
    }

protected:
    NetFrame* m_net_frame;
    SocketAddressInet m_server_address;
};

TEST_F(StreamConnectionTest, StreamSocket)
{
    NetFrame netframe(1);
    int error = netframe.AsyncConnect(m_server_address,
                                      new TestStreamSocketHandler(netframe),
                                      kMaxPacketSize);
    EXPECT_LT(0, error) << " Async connect error";

    ThisThread::Sleep(1000);
}

TEST_F(StreamConnectionTest, StreamSocketWithSmallerPacketSize)
{
    NetFrame netframe(1);
    int error = netframe.AsyncConnect(m_server_address,
                                      new TestStreamSocketHandler(netframe),
                                      10);
    EXPECT_LT(0, error) << " Async connect error";

    ThisThread::Sleep(1000);
}

TEST_F(StreamConnectionTest, StreamSocketUrgent)
{
    NetFrame netframe(1);
    int error = netframe.AsyncConnect(m_server_address,
                                      new TestStreamSocketHandler(netframe, true),
                                      kMaxPacketSize);
    EXPECT_LT(0, error) << " Async connect error";

    ThisThread::Sleep(1000);
}

TEST_F(StreamConnectionTest, StreamSocketClientCloseFirst)
{
    {
        NetFrame netframe(1);
        int error = netframe.AsyncConnect(m_server_address,
                                          new TestStreamSocketHandler(netframe, true),
                                          kMaxPacketSize);
        EXPECT_LT(0, error) << " Async connect error";

        ThisThread::Sleep(1000);
    }

    ThisThread::Sleep(1000);
    EXPECT_EQ(0U, GetCurrentSendBufferedLength());
}

TEST_F(DatagramConnectionTest, SendReceive)
{
    DatagramSocket socket;
    EXPECT_TRUE(socket.Create(AF_INET));
    std::string sent = "12345\n";
    size_t sent_size, received_size;
    char buffer[6];
    size_t buffer_size = 6;
    EXPECT_TRUE(socket.SendTo(sent.data(), sent.size(),
                              m_server_address, &sent_size));
    EXPECT_EQ(sent.size(), sent_size);
    EXPECT_TRUE(socket.ReceiveFrom(buffer, buffer_size,
                                   &received_size, &m_server_address));
    EXPECT_EQ(sent.size(), received_size);
    EXPECT_STREQ(sent.data(), buffer);
}

TEST(NetFrame, InitAndExit)
{
    for (int i = 0; i < 4; ++i) {
        NetFrame netframe(4);
    }

    // Test pass 0 to netframe constructor
    NetFrame netframe;
}

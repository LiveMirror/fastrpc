// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description: test short conn

#include "socket.h"
#include "posix_time.h"

int main()
{
    using namespace ipcs_common;
    SocketAddressInet4 address("127.0.0.1:10000");
    for (;;)
    {
        long long t0 = GetTimeStampInMs();
        for (int i = 0; i < 1000; ++i)
        {
            StreamSocket connector(AF_INET, IPPROTO_TCP);
            connector.SetLinger(true, 1);
            if (connector.Connect(address))
            {
                static const char message[] = "hello, world!\n";
                connector.SendAll(message, strlen(message));
                char buffer[sizeof(message)];
                size_t received_length;
                connector.Receive(buffer, sizeof(buffer), &received_length);
            }
        }
        t0 = GetTimeStampInMs() - t0;
        printf("time=%lld ms\n", t0);
    }
}


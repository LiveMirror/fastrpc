// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description:

#include "http_request.h"
#include "gtest.h"


TEST(HttpMessage, ParseRequest)
{
    static const char request_text[] =
        "GET / HTTP/1.1\r\n"
        "Accept-Language: zh-cn,zh-hk,zh-tw,en-us\r\n"
        "User-Agent: Sosospider+(+http://help.soso.com/webspider.htm)\r\n"
        "Accept-Encoding: gzip\r\n"
        "Connection: Keep-Alive\r\n"
        "Host: 10.6.222.133\r\n"
        "\r\n";
    HttpRequest request;
    ASSERT_TRUE(request.ParseHeaders(request_text));
    ASSERT_EQ("GET / HTTP/1.1", request.StartLine());
}

TEST(HttpRequest, Swap)
{
    HttpRequest req1;
    req1.set_http_version(HttpRequest::VERSION_1_0);
    req1.SetHeader("A", "1");
    req1.set_body("hello");

    HttpRequest req2;
    req2.set_http_version(HttpRequest::VERSION_1_1);
    req2.SetHeader("B", "1");
    req2.set_body("world");

    std::swap(req1, req2);

    EXPECT_EQ(HttpRequest::VERSION_1_1, req1.http_version());
    EXPECT_FALSE(req1.HasHeader("A"));
    EXPECT_TRUE(req1.HasHeader("B"));
    EXPECT_EQ("world", req1.http_body());

    EXPECT_EQ(HttpRequest::VERSION_1_0, req2.http_version());
    EXPECT_FALSE(req2.HasHeader("B"));
    EXPECT_TRUE(req2.HasHeader("A"));
    EXPECT_EQ("hello", req2.http_body());
}

TEST(HttpRequest, HeadersToString)
{
    HttpRequest request;
    request.set_method(HttpRequest::METHOD_GET);
    request.set_uri("/index.html");
    request.SetHeader("A", "1");
    std::string headers = request.HeadersToString();
    EXPECT_EQ("GET /index.html HTTP/1.1\r\nA: 1\r\n\r\n", headers);
}

// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description:

#ifndef IPCS_COMMON_NET_HTTP_HTTP_SERVER_H
#define IPCS_COMMON_NET_HTTP_HTTP_SERVER_H

#include <string>
#include <vector>
#include "deprecate.h"
#include "http_base.h"
#include "http_handler_manager.h"
#include "http_stats.h"
#include "netframe.h"

class HttpServer : public HttpBase {
    DECLARE_UNCOPYABLE(HttpServer);

public:
    // Pass in a net frame from external. If "own_net_frame" is true, the http
    // server would own the net frame and delete it in destructor.
    explicit HttpServer(netframe::NetFrame* net_frame,
                        bool own_net_frame = false) :
        HttpBase(net_frame, own_net_frame),
        m_listener_manager(net_frame) {
    }
    // The http server create and own a net frame internally.
    // If "server_threads" is 0, it means the number of logic cpus.
    explicit HttpServer(int server_threads = 0) :
        HttpBase(server_threads),
        m_listener_manager(mutable_net_frame()) {
    }
    virtual ~HttpServer();

    void Stop();

    // Register a handler on a specified path.
    // If a handler has been registered successfully, it will be taken over
    // by the handler manager and CANNOT be unregistered.
    void RegisterHandler(const std::string& path, HttpHandler* handler);
    void RegisterSimpleHandler(const std::string& path,
                               HttpClosure* closure);
    void RegisterSimpleHandler(const std::string& path,
                               HttpClosureWithConnection* closure);

    // Register a handler on a specified path prefix.
    void RegisterPrefixHandler(const std::string& path,
                               HttpHandler* handler);
    void RegisterPrefixSimpleHandler(const std::string& path,
                                     HttpClosure* closure);
    void RegisterPrefixSimpleHandler(const std::string& path,
                                     HttpClosureWithConnection* closure);

    // Register a static resource to specified path.
    // Note the content will not be copied so you must guarantee its lifetime.
    // The static resource handher will fill Date, Content-Type and Content-Length
    // headers to the response automatically, but you can specify extra http
    // headers if you need, and they are priority than auto generated headers.
    void RegisterStaticResource(const std::string& path,
                                const StringPiece& content,
                                const HttpHeaders& headers = HttpHeaders());
    // Find a handler registered on a specified path. If no handler is
    // registered, NULL will be returned.
    virtual HttpHandler* FindHandler(const std::string& path);

    // Start server to listen on this address.
    // If the port in server_address is 0, operating system will specify a port
    // to bind to, you can use real_bind_address to get the actual bind address
    bool Start(const std::string& server_address,
               const netframe::NetFrame::EndPointOptions& options =
                     netframe::NetFrame::EndPointOptions(),
               ipcs_common::SocketAddress* real_bind_address = NULL);

    // Overloaded method, call
    // Start(server,address, options, real_bind_address) directly
    bool Start(const std::string& server_address,
               ipcs_common::SocketAddress* real_bind_address);

    // Return exit code of process.
    virtual int Run();

    ConnectionManager* mutable_listener_manager() {
        return &m_listener_manager;
    }

    HttpTrafficStats* mutable_traffic_stats() {
        return &m_traffic_stats;
    }

private:
    static bool IsPrivatePortAddress(const std::string& server_address);

private:
    ConnectionManager  m_listener_manager;
    HttpHandlerManager m_handler_manager;
    HttpTrafficStats m_traffic_stats;
};

class HttpServerConnection : public HttpConnection {
public:
    explicit HttpServerConnection(HttpServer* http_server);
    virtual ~HttpServerConnection() {}

    bool IsViaProxy() const
    {
        return !m_proxy_addresses.empty();
    }

    const std::vector<std::string>& GetProxyAddresses() const
    {
        return m_proxy_addresses;
    }

    const std::string& credential() const
    {
        return m_credential;
    }

    void set_credential(const std::string& credential)
    {
        m_credential = credential;
    }

    const std::string& user() const
    {
        return m_user;
    }

    void set_user(const std::string& user)
    {
        m_user = user;
    }

    const std::string& role() const
    {
        return m_role;
    }

    void set_role(const std::string& role)
    {
        m_role = role;
    }

    const TrafficStatsEntry& stats_entry() const {
        return m_stats_entry;
    }

protected:
    virtual void OnConnected();
    virtual void OnClose(int error_code);
    virtual bool OnSent(netframe::Packet* packet);
    virtual void OnReceived(const netframe::Packet& packet);

private:
    std::string GetClientDescription() const;

private:
    HttpServer* m_http_server;
    std::vector<std::string> m_proxy_addresses;
    std::string m_credential;
    std::string m_user;
    std::string m_role;
    TrafficStatsEntry m_stats_entry;
};

#endif // IPCS_COMMON_NET_HTTP_HTTP_SERVER_H

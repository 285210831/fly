/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                    _______    _                                     *
 *                   (  ____ \  ( \     |\     /|                      * 
 *                   | (    \/  | (     ( \   / )                      *
 *                   | (__      | |      \ (_) /                       *
 *                   |  __)     | |       \   /                        *
 *                   | (        | |        ) (                         *
 *                   | )        | (____/\  | |                         *
 *                   |/         (_______/  \_/                         *
 *                                                                     *
 *                                                                     *
 *     fly is an awesome c++11 network library.                        *
 *                                                                     *
 *   @author: lichuan                                                  *
 *   @qq: 308831759                                                    *
 *   @email: 308831759@qq.com                                          *
 *   @github: https://github.com/lichuan/fly                           *
 *   @date: 2015-06-23 16:50:43                                        *
 *                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>
#include <netinet/in.h>
#include "fly/net/client.hpp"
#include "fly/base/logger.hpp"

namespace fly {
namespace net {

Client::Client(const Addr &addr,
           std::function<void(std::shared_ptr<Connection>)> init_cb,
           std::function<void(std::unique_ptr<Message>)> dispatch_cb,
           std::function<void(std::shared_ptr<Connection>)> close_cb,
           std::function<void(std::shared_ptr<Connection>)> be_closed_cb,
           std::shared_ptr<Poller> poller, std::shared_ptr<Parser> parser)
{
    m_addr = addr;
    m_init_cb = init_cb;
    m_dispatch_cb = dispatch_cb;
    m_close_cb = close_cb;
    m_be_closed_cb = be_closed_cb;
    m_poller = poller;
    m_parser = parser;
}

bool Client::connect(int32 timeout)
{
    int32 fd = socket(AF_INET, SOCK_STREAM, 0);
    
    if(fd < 0)
    {
        LOG_FATAL("socket failed in Client::connect");

        return false;
    }

    int32 flags = fcntl(fd, F_GETFL, 0);

    if(flags == -1)
    {
        LOG_FATAL("set fd to nonblock failed 1 in Client::connect");

        return false;
    }

    flags |= O_NONBLOCK;

    if(fcntl(fd, F_SETFL, flags) == -1)
    {
        LOG_FATAL("set fd to nonblock failed 1 in Client::connect");

        return false;
    }
    
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, m_addr.m_host.c_str(), &server_addr.sin_addr);
    server_addr.sin_port = htons(m_addr.m_port);

    if(::connect(fd, (sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        if(errno != EINPROGRESS)
        {        
            LOG_FATAL("connect failed in Client::connect, server addr is %s:%d %s", m_addr.m_host.c_str(), m_addr.m_port, strerror(errno));

            return false;
        }

        struct pollfd fds;
        fds.fd = fd;
        fds.events = POLLOUT;
        int32 ret = poll(&fds, 1, timeout);

        if(ret <= 0)
        {
            LOG_FATAL("connect failed, poll return <= 0");

            return false;
        }

        socklen_t len = sizeof(ret);

        if(getsockopt(fd, SOL_SOCKET, SO_ERROR, &ret, &len) < 0)
        {
            LOG_FATAL("connect failed, getsockopt return < 0");

            return false;
        }

        if(ret != 0)
        {
            LOG_FATAL("connect failed, getsockopt error: %s", strerror(ret));

            return false;
        }
    }
    
    std::shared_ptr<Connection> connection = std::make_shared<Connection>(fd, m_addr);
    m_id = connection->m_id_allocator.new_id();
    connection->m_id = m_id;
    connection->m_init_cb = m_init_cb;
    connection->m_dispatch_cb = m_dispatch_cb;
    connection->m_close_cb = m_close_cb;
    connection->m_be_closed_cb = m_be_closed_cb;
    m_parser->register_connection(connection);
    m_poller->register_connection(connection);
    
    return true;
}

uint64 Client::id()
{
    return m_id;
}

}
}

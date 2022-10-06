#include <string>
#include <iostream>
#include <thread>
#include "cTCPServerMultiClient.h"

namespace raven
{
    namespace set
    {

        cTCPServerMultiClient::cTCPServerMultiClient() : myfRetryServer(true)
        {
            myConnectSocket.resize(1, INVALID_SOCKET);
        }

        void cTCPServerMultiClient::start(
            const std::string &ServerPort,
            std::function<void(int,const std::string&)> readHandler,
            int maxClient)
        {
            myServerPort = ServerPort;
            myConnectSocket.resize(maxClient, INVALID_SOCKET);
            myReadHandler = readHandler;

            acceptSocket();

            //std::thread t(acceptBlock, this);
            std::thread t(
                [&]
                {
                    acceptBlock();
                });
            t.detach();

        }

        void cTCPServerMultiClient::acceptBlock()
        {
            while (true)
                acceptHandler(acceptClientMultiple());
        }
        void cTCPServerMultiClient::readBlock(
            int client)
        {
            while (true)
                myReadHandler(
                    read(client),
                    readMsg());
        }

        void cTCPServerMultiClient::acceptHandler(int clientIndex)
        {
            std::cout << "Accepted client " << clientIndex << "\n";
             std::thread t(readBlock, this, clientIndex );
            // std::thread t( 
            //     [this](int client)
            //     {
            //         myReadHandler(
            //             client,
            //             readMsg());
            //     });
            t.detach();
        }

        void cTCPServerMultiClient::initWinSock()
        {
            WSADATA wsaData;
            if (WSAStartup(MAKEWORD(2, 2), &wsaData))
            {
                throw std::runtime_error("Winsock init failed");
            }
        }

        void cTCPServerMultiClient::acceptSocket()
        {
            if (myServerPort.empty())
                throw std::runtime_error(
                    "Server not configured");

            initWinSock();

            struct addrinfo *result = NULL,
                            hints;

            ZeroMemory(&hints, sizeof(hints));
            hints.ai_family = AF_INET;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_protocol = IPPROTO_TCP;
            hints.ai_flags = AI_PASSIVE;

            int error = getaddrinfo(
                NULL, myServerPort.c_str(),
                &hints, &result);
            if (error)
            {
                throw std::runtime_error(
                    "getaddrinfo failed " + std::to_string(error));
            }

            myAcceptSocket = ::socket(
                result->ai_family,
                result->ai_socktype,
                result->ai_protocol);
            if (myAcceptSocket == INVALID_SOCKET)
            {
                throw std::runtime_error("socket failed");
            }

            if (::bind(myAcceptSocket,
                       result->ai_addr,
                       (int)result->ai_addrlen) == SOCKET_ERROR)
            {
                closesocket(myAcceptSocket);
                myAcceptSocket = INVALID_SOCKET;
                throw std::runtime_error("bind failed");
            }

            if (::listen(
                    myAcceptSocket,
                    SOMAXCONN) == SOCKET_ERROR)
            {
                closesocket(myAcceptSocket);
                myAcceptSocket = INVALID_SOCKET;
                throw std::runtime_error("listen failed");
            }
        }

        void cTCPServerMultiClient::acceptClient()
        {
            if (countConnectedClients() >= myConnectSocket.size())
                throw std::runtime_error(
                    "connection rejected - too many active clients");
            if (myServerPort.empty())
                throw std::runtime_error(
                    "Server not configured");

            initWinSock();

            struct addrinfo *result = NULL,
                            hints;

            ZeroMemory(&hints, sizeof(hints));
            hints.ai_family = AF_INET;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_protocol = IPPROTO_TCP;
            hints.ai_flags = AI_PASSIVE;

            int error = getaddrinfo(
                NULL, myServerPort.c_str(),
                &hints, &result);
            if (error)
            {
                throw std::runtime_error(
                    "getaddrinfo failed " + std::to_string(error));
            }

            myAcceptSocket = ::socket(
                result->ai_family,
                result->ai_socktype,
                result->ai_protocol);
            if (myAcceptSocket == INVALID_SOCKET)
            {
                throw std::runtime_error("socket failed");
            }

            if (::bind(myAcceptSocket,
                       result->ai_addr,
                       (int)result->ai_addrlen) == SOCKET_ERROR)
            {
                closesocket(myAcceptSocket);
                myAcceptSocket = INVALID_SOCKET;
                throw std::runtime_error("bind failed");
            }

            if (::listen(
                    myAcceptSocket,
                    SOMAXCONN) == SOCKET_ERROR)
            {
                closesocket(myAcceptSocket);
                myAcceptSocket = INVALID_SOCKET;
                throw std::runtime_error("listen failed");
            }

            std::cout << "listening for client on port " << myServerPort << "\n";

            struct sockaddr_in client_info;
            int size = sizeof(client_info);
            SOCKET s = ::accept(
                myAcceptSocket,
                (sockaddr *)&client_info,
                &size);
            if (s == INVALID_SOCKET)
            {
                std::cout << "invalid socket\n";
                return;
            }

            addConnectedSocket(s);
            myRemoteAddress = inet_ntoa(client_info.sin_addr);

            closesocket(myAcceptSocket);

            std::cout << "client " << myRemoteAddress << " accepted\n";
        }

        int cTCPServerMultiClient::acceptClientMultiple()
        {
            std::cout << "cTCP listening for multiple clients on port "
                      << myServerPort << "\n";

            struct sockaddr_in client_info;
            int size = sizeof(client_info);
            SOCKET s = ::accept(
                myAcceptSocket,
                (sockaddr *)&client_info,
                &size);
            if (s == INVALID_SOCKET)
            {
                std::cout << "invalid socket\n";
                return -1;
            }

            int clientIndex = addConnectedSocket(s);
            myRemoteAddress = inet_ntoa(client_info.sin_addr);

            std::cout << "cTCP client from " << myRemoteAddress
                      << " accepted on " << client_info.sin_port << "\n";

            return clientIndex;
        }

        int cTCPServerMultiClient::countConnectedClients()
        {
            int count = 0;
            for (auto s : myConnectSocket)
                if (s != INVALID_SOCKET)
                    count++;
            return count;
        }

        int cTCPServerMultiClient::addConnectedSocket(SOCKET s)
        {
            for (int kc = 0; kc < myConnectSocket.size(); kc++)
                if (myConnectSocket[kc] == INVALID_SOCKET)
                {
                    myConnectSocket[kc] = s;
                    return kc;
                }
            return -1;
        }
        SOCKET cTCPServerMultiClient::clientSocket(int client)
        {
            if (client < 0)
                return INVALID_SOCKET;
            if (client >= myConnectSocket.size())
                return INVALID_SOCKET;
            return myConnectSocket[client];
        }
        int cTCPServerMultiClient::clientPort(int client)
        {
            if (client < 0)
                return 0;
            if (client >= myConnectSocket.size())
                return 0;
            if (myConnectSocket[client] == INVALID_SOCKET)
                return 0;
            SOCKADDR_IN sa;
            int namelen = sizeof(sa);
            getsockname(
                myConnectSocket[client],
                (struct sockaddr *)&sa,
                &namelen);
            int dbg = sa.sin_port;
            return 0;
        }
        int cTCPServerMultiClient::read(int client)
        {
            std::cout << "cTCP::read " << client << "\n";
            if (myConnectSocket[client] == INVALID_SOCKET)
                throw std::runtime_error("cTCP read on invalid socket");

            // clear old message
            ZeroMemory(myReadbuf, 1024);

            // wait to receive message
            int r = ::recv(
                myConnectSocket[client],
                (char *)myReadbuf, 1024, 0);

            // check for message received
            // if no message or error, assume connection closed
            if (r <= 0)
            {
                std::cout << "connection closed\n";
                closesocket(myConnectSocket[0]);
                myConnectSocket[0] = INVALID_SOCKET;
            }
            return client;
        }
        bool cTCPServerMultiClient::serverWait()
        {
            while (1)
            {
                connectToServer();
                if (isConnected())
                {
                    return true;
                }
                if (!myfRetryServer)
                    return false;
            }
        }
        bool cTCPServerMultiClient::connectToServer()
        {
            if (myServerPort.empty())
                throw std::runtime_error("Server not configured");

            initWinSock();

            struct addrinfo *result = NULL,
                            hints;

            ZeroMemory(&hints, sizeof(hints));
            hints.ai_family = AF_UNSPEC;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_protocol = IPPROTO_TCP;

            if (getaddrinfo(
                    myServerIP.c_str(),
                    myServerPort.c_str(),
                    &hints, &result))
            {
                throw std::runtime_error("getaddrinfo failed");
            }

            myConnectSocket[0] = ::socket(
                result->ai_family,
                result->ai_socktype,
                result->ai_protocol);
            if (myConnectSocket[0] == INVALID_SOCKET)
            {
                throw std::runtime_error("socket failed");
            }

            // std::cout << "attempting connect\n";
            int connect_return;
            try
            {
                connect_return = ::connect(
                    myConnectSocket[0],
                    result->ai_addr,
                    (int)result->ai_addrlen);
            }
            catch (std::runtime_error &e)
            {
                std::cout << myServerIP << ":" << myServerPort
                          << " socket connect threw exception";
                connect_return = SOCKET_ERROR;
            }
            if (connect_return == SOCKET_ERROR)
            {
                int err = WSAGetLastError();
                closesocket(myConnectSocket[0]);
                myConnectSocket[0] = INVALID_SOCKET;
                if (err == 10060)
                {
                    std::cout << "connect timed out\n";
                    return false;
                }
                else if (err == 10061)
                {
                    std::cout << myServerIP << ":" << myServerPort
                              << " No connection could be made because the target machine actively refused it. "
                                 " Generally, it happens that something is preventing a connection to the port or hostname. "
                                 " Either there is a firewall blocking the connection "
                                 " or the process that is hosting the service is not listening on that specific port.\n";
                    return false;
                }
                else
                    std::cout << "connect failed error: " << std::to_string(err);
                return false;
            }

            return true;
        }
        void cTCPServerMultiClient::send(const std::string &msg)
        {
            if (myConnectSocket[0] == INVALID_SOCKET)
                throw std::runtime_error("send on invalid socket");
            ::send(
                myConnectSocket[0],
                msg.c_str(),
                (int)msg.length(), 0);
        }
        void cTCPServerMultiClient::send(const std::vector<unsigned char> &msg)
        {
            if (myConnectSocket[0] == INVALID_SOCKET)
                throw std::runtime_error("send on invalid socket");
            ::send(
                myConnectSocket[0],
                (char *)msg.data(),
                msg.size(), 0);
        }

    }
}

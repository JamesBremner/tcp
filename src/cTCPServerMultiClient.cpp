#include <string>
#include <iostream>
#include <thread>
#include "cTCPServerMultiClient.h"

namespace raven
{
    namespace set
    {

        cTCPServerMultiClient::cTCPServerMultiClient()
        {
            myFrameLines = true;
            myConnectSocket.resize(1, INVALID_SOCKET);
        }

        void cTCPServerMultiClient::start(
            const std::string &ServerPort,
            eventHandler_t readHandler,
            processor_t processor,
            int maxClient)
        {
            myServerPort = ServerPort;
            myConnectSocket.resize(maxClient, INVALID_SOCKET);
            myEventHandler = readHandler;
            myProcessor = processor;

            // construct socket where server listens for client connection requests
            acceptSocketCtor();

            /* start thread that listens for client connection requests

            The thread keeps running after this method returns
            Each time a connection is requested the thread runs acceptBlock()

            Meanwhile this method returns to caller
            */

            std::thread t(acceptBlock, this);
            t.detach();

            /* start thread that processes jobs */
            std::thread t2(jobThread, this);
            t2.detach();
        }

        void cTCPServerMultiClient::acceptBlock()
        {
            while (true)
            {
                // wait for client connection request
                int client = acceptClientMultiple();
                if (client < 0)
                {
                    // a problem occurred
                    // sleep, then try again
                    std::this_thread::sleep_for(
                        std::chrono::milliseconds(25));
                    continue;
                }

                // good connection request
                // handle it in its own, new thread
                std::cout << "Accepted client " << client << "\n";
                std::thread t(readBlock, this, client);
                t.detach();

                // loop to listen for next connection request
            }
        }
        void cTCPServerMultiClient::readBlock(
            int client)
        {
            myEventHandler(
                client,
                eEvent::accept,
                "");

            while (true)
            {
                // wait for next message from client
                read(client);

                // check for disconnection
                if (!isConnected(client))
                {
                    // invoke handler with disconnect message
                    myEventHandler(
                        client,
                        eEvent::disconnect,
                        "");

                    // exit ( ends thread )
                    return;
                }

                // get line received if available and option set
                auto line = nextLine(readMsg());
                if (line.empty())
                    continue;

                // invoke handler with message from client
                myEventHandler(
                    client,
                    eEvent::read,
                    line);

                if (mySharedProcessingThread)
                {
                    myJobQ.push(cJob(client, line));
                }
                else
                {
                    send(
                        myProcessor(client, line),
                        client);
                }
            }
        }

        void cTCPServerMultiClient::jobThread()
        {
            while (true)
            {
                // check for waiting job
                if (myJobQ.empty())
                {
                    // let other processes run
                    std::this_thread::sleep_for(
                        std::chrono::milliseconds(100));
                    continue;
                }

                // process job at front of queue
                auto &j = myJobQ.front();
                send(
                    myProcessor(
                        j.client,
                        j.msg),
                    j.client);
                myJobQ.pop();
            }
        }

        std::string cTCPServerMultiClient::nextLine(const std::string &msg)
        {
            static std::string msg_acc = "";

            if (!myFrameLines)
                return msg;

            for (char c : msg)
            {
                if ((int)((unsigned char)c) == 0xFF)
                {
                    for (char c : msg_acc)
                        std::cout << std::hex << (int)((unsigned char)c) << " ";
                    std::cout << std::endl;
                    std::cout << "garbage received, ignoring it\n";
                    return "";
                }
            }

            msg_acc += msg;
            std::cout << "msg_acc " << msg_acc << "\n";
            int p = msg_acc.find_first_of("\n\r");
            if (p == -1)
                return "";

            // complete line received
            std::string ret = msg_acc.substr(0, p) + "\n";
            msg_acc = msg_acc.substr(
                msg_acc.find_last_of("\n\r") + 1);
            return ret;
        }

        void cTCPServerMultiClient::initWinSock()
        {
            WSADATA wsaData;
            if (WSAStartup(MAKEWORD(2, 2), &wsaData))
            {
                throw std::runtime_error("Winsock init failed");
            }
        }

        void cTCPServerMultiClient::acceptSocketCtor()
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

        // void cTCPServerMultiClient::acceptClient()
        // {
        //     if (countConnectedClients() >= myConnectSocket.size())
        //         throw std::runtime_error(
        //             "connection rejected - too many active clients");
        //     if (myServerPort.empty())
        //         throw std::runtime_error(
        //             "Server not configured");

        //     initWinSock();

        //     struct addrinfo *result = NULL,
        //                     hints;

        //     ZeroMemory(&hints, sizeof(hints));
        //     hints.ai_family = AF_INET;
        //     hints.ai_socktype = SOCK_STREAM;
        //     hints.ai_protocol = IPPROTO_TCP;
        //     hints.ai_flags = AI_PASSIVE;

        //     int error = getaddrinfo(
        //         NULL, myServerPort.c_str(),
        //         &hints, &result);
        //     if (error)
        //     {
        //         throw std::runtime_error(
        //             "getaddrinfo failed " + std::to_string(error));
        //     }

        //     myAcceptSocket = ::socket(
        //         result->ai_family,
        //         result->ai_socktype,
        //         result->ai_protocol);
        //     if (myAcceptSocket == INVALID_SOCKET)
        //     {
        //         throw std::runtime_error("socket failed");
        //     }

        //     if (::bind(myAcceptSocket,
        //                result->ai_addr,
        //                (int)result->ai_addrlen) == SOCKET_ERROR)
        //     {
        //         closesocket(myAcceptSocket);
        //         myAcceptSocket = INVALID_SOCKET;
        //         throw std::runtime_error("bind failed");
        //     }

        //     if (::listen(
        //             myAcceptSocket,
        //             SOMAXCONN) == SOCKET_ERROR)
        //     {
        //         closesocket(myAcceptSocket);
        //         myAcceptSocket = INVALID_SOCKET;
        //         throw std::runtime_error("listen failed");
        //     }

        //     std::cout << "listening for client on port " << myServerPort << "\n";

        //     struct sockaddr_in client_info;
        //     int size = sizeof(client_info);
        //     SOCKET s = ::accept(
        //         myAcceptSocket,
        //         (sockaddr *)&client_info,
        //         &size);
        //     if (s == INVALID_SOCKET)
        //     {
        //         std::cout << "invalid socket\n";
        //         return;
        //     }

        //     addConnectedSocket(s);
        //     myRemoteAddress = inet_ntoa(client_info.sin_addr);

        //     closesocket(myAcceptSocket);

        //     std::cout << "client " << myRemoteAddress << " accepted\n";
        // }

        int cTCPServerMultiClient::acceptClientMultiple()
        {
            if (countConnectedClients() >= myConnectSocket.size())
            {
                // maximum clients already connected
                return -2;
            }
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

        bool cTCPServerMultiClient::isConnected(int client) const
        {
            return myConnectSocket[client] != INVALID_SOCKET;
        }

        int cTCPServerMultiClient::maxClient() const
        {
            return myConnectSocket.size();
        }

        std::string cTCPServerMultiClient::readMsg() const
        {
            return std::string(myReadbuf);
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
            // std::cout << "cTCP::read " << client << "\n";
            if (myConnectSocket[client] == INVALID_SOCKET)
            {
                std::cout << "cTCP read on invalid socket\n";
                return -1;
            }

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

        void cTCPServerMultiClient::send(
            const std::string &msg,
            int client)
        {
            if (myConnectSocket[client] == INVALID_SOCKET)
                throw std::runtime_error("send on invalid socket");
            ::send(
                myConnectSocket[client],
                msg.c_str(),
                (int)msg.length(), 0);
        }
        void cTCPServerMultiClient::send(
            const std::vector<unsigned char> &msg,
            int client)
        {
            if (myConnectSocket[client] == INVALID_SOCKET)
                throw std::runtime_error("send on invalid socket");
            ::send(
                myConnectSocket[client],
                (char *)msg.data(),
                msg.size(), 0);
        }

    }
}

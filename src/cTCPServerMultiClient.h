#include <vector>
#include <functional>
#include <winsock2.h>
#include <ws2tcpip.h>

namespace raven
{
    namespace set {

/// A TCP server handling multiple client simultaineously

class cTCPServerMultiClient
{
public:
    cTCPServerMultiClient();

    /// @brief start server
    /// @param[in] ServerPort listens for clients
    /// @param[in] readHandler handler to call when mesage received from client
    /// @param[in] maxClient max number of clients, default 1

    void start(
        const std::string &ServerPort,
        std::function<void(int,const std::string&)> readHandler,
        int maxClient = 1 );
    

    /// Send message to client
    void send(
        const std::string &msg,
        int client = 0);
    void send( 
        const std::vector< unsigned char >& msg,
        int client = 0 );

    /// Get last message from peer
    std::string readMsg() const;

    /// True if peer connected
    bool isConnected( int client = 0) const;

    int countConnectedClients();

    int maxClient() const;

private:
    std::string myServerIP;
    std::string myServerPort;
    SOCKET myAcceptSocket;  //< socket listening for clients
    std::vector<SOCKET> myConnectSocket; //< sockets connected to another tcp
    std::string myRemoteAddress;
    char myReadbuf[1024];
    bool myfRetryServer;
    std::function<void(int,const std::string&)> myReadHandler;

    void initWinSock();

    int addConnectedSocket( SOCKET s );

    void acceptBlock();
    void readBlock( int client );

    void acceptHandler(int clientIndex);

    /// Wait for client connection request
    void acceptClient();

    int acceptClientMultiple();

    /// Wait for message from peer
    int read( int client = 0 );

    /// @brief Construct socker that listens for client connetion requests
    void acceptSocketCtor();

    int clientPort( int client );

    SOCKET clientSocket( int client );

};

    }
}

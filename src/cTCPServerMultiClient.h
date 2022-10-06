#include <vector>
#include <functional>
#include <winsock2.h>
#include <ws2tcpip.h>

namespace raven
{
    namespace set {

/// C++ wrapper for winsock ( TCP communications )

class cTCPServerMultiClient
{
public:
    cTCPServerMultiClient();

    /// @brief configure server
    /// @param[in] ServerPort listens for clients
    /// @param[in] maxClient max number of clients, default 1
    void start(
        const std::string &ServerPort,
        std::function<void(int,const std::string&)> readHandler,
        int maxClient = 1 );
    

    /** Configure serverWait() blocking
     * 
     * true: keep trying until connection made ( default on construction )
     * false: if connection refused return after one attempt
     */
    void RetryConnectServer( bool f )
    {
        myfRetryServer = f;
    }
    /** Connect to server
     * 
     * Throws exeption if there is a configuration problem
     * Otherwise blocks until connection
     */
    bool serverWait();

    /// Wait for client connection request
    void acceptClient();

    /// Send message to peer
    void send(const std::string &msg);
    void send( const std::vector< unsigned char >& msg );

    /// Wait for message from peer
    int read( int client = 0 );

    /// Get last message from peer
    std::string readMsg() const
    {
        return std::string(myReadbuf);
    }

    /// True if peer connected
    bool isConnected( int client = 0) const
    {
        return myConnectSocket[client] != INVALID_SOCKET;
    }

    int countConnectedClients();

    int maxClient() const
    {
        return myConnectSocket.size();
    }

    void acceptSocket();

    int acceptClientMultiple();

    int clientPort( int client );

    SOCKET clientSocket( int client );

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

    /** Connect to server
     * 
     * Throws exception on configuration error
     * 
     * Returns true on success  of connection
     * Returns false if connection timed out
     */
    bool connectToServer();

    int addConnectedSocket( SOCKET s );

    void acceptBlock();
    void readBlock( int client );

    void acceptHandler(int clientIndex);

};

    }
}

#include <vector>
#include <functional>
#include <winsock2.h>
#include <ws2tcpip.h>

namespace raven
{
    namespace set {

/// A TCP server handling multiple clients simultaineously

class cTCPServerMultiClient
{
public:

    enum class eEvent
    {
        accept,
        read,
        disconnect,
    };

    typedef std::function<void(int,eEvent,const std::string&)>
        eventHandler_t;

    cTCPServerMultiClient();

    /** start server

     @param[in] ServerPort listens for clients
     @param[in] eventHandler handler to call when something happens
     @param[in] maxClient max number of clients, default 1

     Starts listening for client connection requests.

     The eventHandler function will be called when
     - a client connects
     - a message is received from client
     - a client diconnects
     and runs in the thread connected to the client
 
     eventHandler signature:  void h( 
        int client,
        eEvent type,
        const std::string& msg )

     client the index among connected clients of client that sent the message
     type of event
     msg the message received from the client

     Threading

    When the server starts, 
        a new threas is started that listens for connection requests.
        This thread runs until the application exits
    When a new client connects,
        a new thread is started that listens to the client.
        This thread runs until the client disconnects
        The eventHandler runs in this thread


     This method returns immediatly, leaving the client connect listening thread running.

     */

    void start(
        const std::string &ServerPort,
        eventHandler_t eventHandler,
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

    std::string myServerPort;
    SOCKET myAcceptSocket;  //< socket listening for clients
    std::vector<SOCKET> myConnectSocket; //< sockets connected to clients
    std::string myRemoteAddress;
    char myReadbuf[1024];
    eventHandler_t myEventHandler;

    void initWinSock();

    int addConnectedSocket( SOCKET s );

    /** Wait for client connection requests
     * 
     * runs in its own thread
     * 
     * Never returns
     */
    void acceptBlock();

    /** Wait for messages from client
     * 
     * runs in its own thread, invoking the read handler when a message is recvd
     * 
     * Does not return until the client disconnects
     */
    void readBlock( int client );

    /// Wait for client connection request
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

#include <string>
#include <queue>
#include <functional>

/// @brief Job processor
class cProcessor
{
public:
    cProcessor();

    /// register handler to call when job completes
    void registerReplyHandler(
        std::function<void(int,const std::string&)> h )
        {
            myReplyHandler = h;
        }

    /** @brief Process job in client thread
     @param client 
     @param msg 

    PRO:
    - long jobs do not block jobs from other clients

    CON:
    - long jobs block server for their client
    - careful data access synchronization required

    */
    void ProcessInClientThread(
        int client,
        const std::string &msg);

    /** @brief process job in thread shared by the jobs from all clients
      
     @param client 
     @param msg 

     PRO:
     - no data access synchronize needed
     - long jobs do not block the server

     CON:
     - long jobs may block execution of other jobs
    */

    void ProcessInSharedThread(
        int client,
        const std::string &msg);

private:
    class cJob
    {
    public:
        int client;
        const std::string &msg;

        cJob(int c,
             const std::string &m)
            : client(c),
              msg(m)
        {
        }
    };

    std::queue<cJob> myJobQ;

    std::function<void(int,const std::string&)> myReplyHandler;

    /// @brief runs in its own thread, processing jobs one by one
    
    void sharedThread();

    /// @brief process a job
    /// @param job the job submited by a client

    void process(const cJob &job);
};

class cGUI : public cStarterGUI
{
public:
    cGUI();

    void status(const std::string &msg);
    void status1(const std::string &msg);
    void status2(const std::string &msg);

    void eventHandler(
        int client,
        raven::set::cTCPServerMultiClient::eEvent type,
        const std::string &msg);

private:
    wex::radiobutton &rbClient;
    wex::radiobutton &rbServer1;
    wex::radiobutton &rbServerM;
    wex::button &bnConnect;
    wex::label &lbStatus;
    wex::label &lbStatus1;
    wex::label &lbStatus2;

    wex::tcp &myTCP;
    raven::set::cTCPServerMultiClient serverM;
    cProcessor myProcessor;

    void registerEventHandlers();

    void clientStart();
    void server1Start();
    void serverMStart();
    void connect();
};

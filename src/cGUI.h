#include <string>
#include <queue>
#include <functional>

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
    //cProcessor myProcessor;

    void registerEventHandlers();

    void clientStart();
    void server1Start();
    void serverMStart();
    void connect();
};

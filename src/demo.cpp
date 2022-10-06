#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <wex.h>
#include "tcp.h"
#include "cTCPServerMultiClient.h"
#include "cStarterGUI.h"

raven::set::cTCPServerMultiClient serverM;

class cGUI : public cStarterGUI
{
public:
    cGUI();

    void status(const std::string &msg);

private:
    wex::radiobutton &rbClient;
    wex::radiobutton &rbServer1;
    wex::radiobutton &rbServerM;
    wex::button &bnConnect;
    wex::label &lbStatus;

    wex::tcp &myTCP;

    void clientStart();
    void server1Start();
    void serverMStart();
    void connect();
};

cGUI * theGUI;

void readHandler(
    int client,
    const std::string &msg)
{
    std::cout << "msg from client " << client
              << ":\n" 
              << msg << "\n";

    theGUI->status( 
        "msg from client " +
        std::to_string( client ) +
              + ":\n" 
              + msg );
}

cGUI::cGUI()
    : cStarterGUI(
          "TCP Demo",
          {50, 50, 1000, 500}),
      rbClient(wex::maker::make<wex::radiobutton>(fm)),
      rbServer1(wex::maker::make<wex::radiobutton>(fm)),
      rbServerM(wex::maker::make<wex::radiobutton>(fm)),
      bnConnect(wex::maker::make<wex::button>(fm)),
      lbStatus(wex::maker::make<wex::label>(fm)),
      myTCP(wex::maker::make<wex::tcp>(fm))
{
    rbClient.move(50, 50, 100, 30);
    rbClient.text("Client");
    rbServer1.move(200, 50, 150, 30);
    rbServer1.text("Server ( 1 client )");
    rbServerM.move(400, 50, 200, 30);
    rbServerM.text("Server ( Many clients )");
    bnConnect.move(50, 100, 100, 30);
    bnConnect.text("Connect");
    lbStatus.move(50, 150, 300, 50);
    lbStatus.text("");

    bnConnect.events()
        .click(
            [this]
            {
                connect();
            });
    fm.events()
        .tcpServerAccept(
            [this]
            {
                status("Client connected");
                myTCP.read();
            });
    fm.events()
        .tcpRead(
            [this]
            {
                     if (!myTCP.isConnected())
                     {
                         if (rbServer1.isChecked())
                         {
                             status("Connection closed, waiting for new client");

                             myTCP.server();
                         }
                         else
                         {
                             status("Disconnected from server");
                         }
                     }
                     else
                     {
                         // display mesage
                         std::stringstream ss;
                         auto msg = myTCP.readMsg();

                         // ascii
                         ss << "Msg read: " + myTCP.readMsg() << "\n\n";

                         ss << "bytes " << msg.length() << ": ";
                         for (int k = 0; k < msg.length(); k++)
                             ss << std::hex << (int)((unsigned char)msg[k]) << " ";

                         status(ss.str());

                         // setup for next message
                         myTCP.read();
                     } });

    show();
}
void cGUI::status(const std::string &msg)
{
    lbStatus.text(msg);
    lbStatus.update();
}
void cGUI::clientStart()
{
    rbServer1.show(false);
    rbServerM.show(false);
    myTCP.client();
}
void cGUI::server1Start()
{
    rbClient.show(false);
    rbServerM.show(false);
    try
    {
        myTCP.server("27678");
        status("Waiting for client to connect on " + myTCP.serverPort());
    }
    catch (std::runtime_error &e)
    {
        status(std::string("Cannot start server ") + e.what());
    }
}
void cGUI::serverMStart()
{
    rbClient.show(false);
    rbServer1.show(false);
    fm.update();
    serverM.start(
        "27678",
        readHandler,
        2);
}

void cGUI::connect()
{
    switch (rbClient.checkedOffset())
    {
    case 0:
        clientStart();
        break;
    case 1:
        server1Start();
        break;
    case 2:
        serverMStart();
        break;
    }
}

main()
{
    theGUI = new cGUI();
    theGUI->run();
    return 0;
}

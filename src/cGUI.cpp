#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <winsock2.h>
#include <wex.h>
#include "tcp.h"
#include "cTCPServerMultiClient.h"
#include "cStarterGUI.h"
#include "cGUI.h"

std::string clientMsgProcessor(
    int client,
    const std::string &msg)
{
    int jobTime = atoi(msg.c_str());
    if (!jobTime)
        return "";

    std::cout << "start job for client " << client
              << " " << msg << "\n";
    std::this_thread::sleep_for(std::chrono::seconds(
        jobTime));
    std::cout << "end job for client " << client
              << " " << msg << "\n";
    std::string reply(
        "end job for client " + std::to_string(client) + " " + msg);
    return reply;
}

void cGUI::eventHandler(
    int client,
    raven::set::cTCPServerMultiClient::eEvent type,
    const std::string &msg)
{
    switch (type)
    {
    case raven::set::cTCPServerMultiClient::eEvent::read:
    {
        std::stringstream ss;
        ss << "msg from client " << client
           << ":\n"
           << msg << "\n";
        std::cout << ss.str();

        switch (client)
        {
        case 0:
            status1(ss.str());
            break;
        case 1:
            status2(ss.str());
            break;
        default:
            status(ss.str());
            break;
        }

    }
    break;

    case raven::set::cTCPServerMultiClient::eEvent::accept:
        switch (client)
        {
        case 0:
            status1(
                "client 0 connected");
            break;
        case 1:
            status2(
                "client 1 connected");
            break;
        }
        break;
        break;

    case raven::set::cTCPServerMultiClient::eEvent::disconnect:
        switch (client)
        {
        case 0:
            status1(
                "client 0 disconnected");
            break;
        case 1:
            status2(
                "client 1 disconnected");
            break;
        }
        break;
    }
}

cGUI::cGUI()
    : cStarterGUI(
          "TCP Demo",
          {50, 50, 1000, 500}),
      rbClient(wex::maker::make<wex::radiobutton>(fm)),
      rbServer1(wex::maker::make<wex::radiobutton>(fm)),
      rbServerM(wex::maker::make<wex::radiobutton>(fm)),
      cbJobThread(wex::maker::make<wex::checkbox>(fm)),
      cbLineAccumulator(wex::maker::make<wex::checkbox>(fm)),
      bnConnect(wex::maker::make<wex::button>(fm)),
      lbStatus(wex::maker::make<wex::label>(fm)),
      lbStatus1(wex::maker::make<wex::label>(fm)),
      lbStatus2(wex::maker::make<wex::label>(fm)),
      myTCP(wex::maker::make<wex::tcp>(fm))
{
    rbClient.move(50, 30, 100, 30);
    rbClient.text("Client");
    rbServer1.move(200, 30, 150, 30);
    rbServer1.text("Server ( 1 client )");
    rbServerM.move(400, 30, 200, 30);
    rbServerM.text("Server ( 2 clients )");
    cbLineAccumulator.move(400,60,200,30);
    cbLineAccumulator.text("Complete Lines");
    cbLineAccumulator.check();
    cbJobThread.move(400,90,200,30);
    cbJobThread.text("Job Thread");
    cbJobThread.check();
    bnConnect.move(50, 100, 100, 30);
    bnConnect.text("Connect");
    lbStatus.move(50, 150, 300, 50);
    lbStatus.text("");
    lbStatus1.move(50, 250, 300, 200);
    lbStatus1.text("client 0 not connected");
    lbStatus2.move(400, 250, 300, 200);
    lbStatus2.text("client 2 not connected");

    registerEventHandlers();

    show();
    run();
}
void cGUI::registerEventHandlers()
{
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

}
void cGUI::status(const std::string &msg)
{
    lbStatus.text(msg);
    lbStatus.update();
}
void cGUI::status1(const std::string &msg)
{
    lbStatus1.text(msg);
    lbStatus1.update();
}
void cGUI::status2(const std::string &msg)
{
    lbStatus2.text(msg);
    lbStatus2.update();
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
    // hide unused options
    rbClient.show(false);
    rbServer1.show(false);

    // server configuration
    std::string port("27678");
    int maxClient = 2;
    serverM.lineAccumulator(
        cbLineAccumulator.isChecked()    );
    serverM.sharedProcessingThread(
        cbJobThread.isChecked()    );

    // construct function object from cGUI::eventHandler
    // this can be passed to server so it can run the class method
    auto ef = std::bind(
        &cGUI::eventHandler, this,
        std::placeholders::_1,
        std::placeholders::_2,
        std::placeholders::_3);

    auto pf = std::bind(
        &clientMsgProcessor,
        std::placeholders::_1,
        std::placeholders::_2);

    // start server listening for clients in its own thread
    // call eventHandler in the client threads when something happens
    serverM.start(
        port, // port to listen for client connection requests
        ef,   // event handler function object
        pf,
        maxClient); // maximum number of simultaineous clients

    status("Listening for clients on port " + port);
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

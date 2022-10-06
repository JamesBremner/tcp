#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <wex.h>
#include "cStarterGUI.h"

class cGUI : public cStarterGUI
{
public:
    cGUI();

private:
    wex::radiobutton &rbClient;
    wex::radiobutton &rbServer1;
    wex::radiobutton &rbServerM;
};

cGUI::cGUI()
    : cStarterGUI(
          "TCP Demo",
          {50, 50, 1000, 500}),
      rbClient(wex::maker::make<wex::radiobutton>(fm)),
      rbServer1(wex::maker::make<wex::radiobutton>(fm)),
      rbServerM(wex::maker::make<wex::radiobutton>(fm))
{
    rbClient.move(50, 50, 100, 30);
    rbClient.text("Client");
    rbServer1.move(200, 50, 150, 30);
    rbServer1.text("Server ( 1 client )");
    rbServerM.move(400, 50, 200, 30);
    rbServerM.text("Server ( Many clients )");

    show();
    run();
}

main()
{
    cGUI theGUI;
    return 0;
}

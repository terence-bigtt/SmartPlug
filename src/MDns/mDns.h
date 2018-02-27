#ifndef mDns_h
#define mDns_h

#include <ESP8266mDNS.h>        // Include the mDNS library
#include <ESP8266WebServer.h>
#include <string>

using namespace std;

class MdnsResponder{
public:
  MdnsResponder(){};
  MdnsResponder(string networkName, int serverPort);
  ~MdnsResponder(){};

  void loop();
  bool setup();
  bool isUp();
  void setPort(int port){_port = port;};
  void setName(string networkName){_name = networkName;};
  unique_ptr<ESP8266WebServer> server;

private:
  string _name ;
  int _port = 80;
  bool _mdnsUp = false;
};




#endif

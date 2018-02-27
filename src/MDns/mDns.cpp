#include "mDns.h"

MdnsResponder::MdnsResponder(string networkName, int port){
setPort(port);
setName(networkName);
}

bool MdnsResponder::setup(){
  _mdnsUp = MDNS.begin(_name.c_str());
  if(_mdnsUp){
    server.reset(new ESP8266WebServer(_port));
    server->begin();
    MDNS.addService("http", "tcp", _port);
  }
  return _mdnsUp;
}

bool MdnsResponder::isUp(){
  return _mdnsUp;
}
void MdnsResponder::loop(){
  if(_mdnsUp){
    server->handleClient();
  }
}

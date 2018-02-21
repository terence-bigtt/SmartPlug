#include "ConfigButton.h"

void log(std::string msg){
  if (Serial && DBG_CB==1){
    Serial.println(msg.c_str());
  }
}

Callback::Callback(){
}

Callback::~Callback(){
}

Callback::Callback(void (* callback)()){
  setCallback(callback);

}

void Callback::setCallback(void (*fn)()){
  _callback = fn;
}

void Callback::activate(){
  _active = true;
}

void Callback::deactivate(){
  _active = false;
}

bool Callback::isActive(){
  return _active;
}

void Callback::run(){
  _callback();
}


void ConfigButton::setPin(int pin){
  pinMode(pin, INPUT);
  _pin = pin;
}

void ConfigButton::setPressCallback(int duration, void (*fn)()){
  Serial.print("CB::setting up callback for press of ");
  Serial.println(duration * PRESS_PERIOD);
  Callback callback(fn);
  callback.activate();
  pressCallbackMap.insert(std::make_pair(duration, callback));
}

void ConfigButton::setClickCallback(int nClick, void (*fn)()){
  Serial.print("CB::setting up callback for ");
  Serial.print(nClick);
  Serial.print(" clicks");
  Callback callback(fn);
  callback.activate();
  clickCallbackMap.insert(std::make_pair(nClick, callback));
}

void ConfigButton::setActive(int state){
  this->_active = state;
}

ConfigButton::ConfigButton(){
}

ConfigButton::ConfigButton(int pin){
  setPin(pin);
}

ConfigButton::ConfigButton(int pin, int active){
  setPin(pin);
  setActive(active);
}

ConfigButton::~ConfigButton(){
}

void ConfigButton::activeLoop(){
  _pressedAt = millis();
  _pressDuration = _pressedAt - _releasedAt;
  _clickRegistered = false;
}

void ConfigButton::inactiveLoop(){
  _releasedAt = millis();
  _lastPressedSince = _releasedAt - _pressedAt;
}

void ConfigButton::resetButton(){
  _nClick = 0;
  _pressDuration = 0;
  _lastPressedSince = 0;
}

bool ConfigButton::isClick(){
  return _pressDuration> DEBOUNCE && _pressDuration<PRESS_PERIOD;
}

bool ConfigButton::clickTimedOut(){
 return  _lastPressedSince > CLICK_TIMEOUT;
}

bool ConfigButton::isPress(){
  return _pressDuration >= PRESS_PERIOD;
}

void ConfigButton::loop(){
  _pressing= digitalRead(_pin) == _active;
  if(_pressing){
    activeLoop();
  }
  else{
    inactiveLoop();
    if(isClick() && !_clickRegistered){
          _nClick++;
          _clickRegistered = true;
          Serial.print("N Clicks: ");
          Serial.println(_nClick);
          Serial.print("pressDuration: ");
          Serial.println(_pressDuration);
          Serial.print("Last pressed since: ");
          Serial.println(_lastPressedSince);
      }
      if (_nClick!=0 && clickTimedOut()){
        Serial.print("CB::click detected, number of clicks: ");
        Serial.println(_nClick);
        Serial.print("pressDuration: ");
        Serial.println(_pressDuration);
        Serial.print("Last pressed since: ");
        Serial.println(_lastPressedSince);
        runClickCallback(_nClick);
        resetButton();
      }
      if (isPress()){
        int value = (int)(_pressDuration / PRESS_PERIOD);
        Serial.print("CB::press detected, value: ");
        Serial.println(value);
        runPressCallback(value);
        resetButton();
      }
    }
  }

bool ConfigButton:: runPressCallback(int duration){
  if (duration !=0){
    if(pressCallbackMap.find(duration) != pressCallbackMap.end()){
      Serial.println("Found press callback to call");
      pressCallbackMap[duration].run();
      return true;
    }
    Serial.println("Didn't find press callback");
    return false;
  }
  return false;
}

bool ConfigButton:: runClickCallback(int nClick){
  if (nClick !=0){
    if(clickCallbackMap.find(nClick) != clickCallbackMap.end()){
      Serial.println("Found click callback to call");
      clickCallbackMap[nClick].run();
      return true;
    }
      Serial.println("Didn't find click callback");
      return false;
    }
    return false;
}

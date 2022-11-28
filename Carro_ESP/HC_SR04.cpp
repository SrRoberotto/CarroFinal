#include "HC_SR04.h"

//HC_SR04 *HC_SR04::_instance=NULL;
HC_SR04 *HC_SR04::_instance(NULL);

HC_SR04::HC_SR04(int trigger, int echo, int interrupt, int max_dist)
    : _trigger(trigger), _echo(echo), _int(interrupt), _maxd(max_dist), _finished(false)
{
  if(_instance==0) _instance=this;    
}

void HC_SR04::begin(){
  pinMode(_trigger, OUTPUT);
  digitalWrite(_trigger, LOW);
  pinMode(_echo, INPUT);  
  attachInterrupt(_int, _echo_isr, CHANGE);
}

void HC_SR04::start(){
  _finished=false;
  _end = 0;
  digitalWrite(_trigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(_trigger, LOW);  
}

void HC_SR04::stop(){
  _end=micros();
  _finished=true;
}

unsigned int HC_SR04::getRange(bool units){
  int result = (_end-_start)/((units)?58:148);
  return (result > _maxd) ? _maxd : (result > 0) ? result : 0;
}

unsigned int HC_SR04::getTime(){
  if (_end > 0){
    return ( _end - _start);
  } else {
    return (micros() - _start);
  }
}


void HC_SR04::_echo_isr(){
  HC_SR04* _this=HC_SR04::instance();
  
  switch(digitalRead(_this->_echo)){
    case HIGH:
      _this->_start=micros();
      break;
    case LOW:
      _this->_end=micros();
      _this->_finished=true;
      break;
  }   
}

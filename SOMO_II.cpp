/*
  SOMO_II.cpp - Library for playing MP3 files with the 4D Systems SOMO II
  Created by DJ Harrigan, September 9, 2014.
  Released into the public domain.
*/

#include "Arduino.h"
#include "SoftwareSerial.h"
#include "SOMO_II.h"

SOMO_II::SOMO_II(SoftwareSerial *ss, uint8_t busyPin){
  somoSerial = ss;
  _pin = busyPin;
}

boolean SOMO_II::begin(){// baud rate is fixed at 9600 for the SOMO_II
  
  somoSerial->begin(9600);
  pinMode(_pin, INPUT_PULLUP);
  //let's test our connection!
  return somoCommand(PLAY_SOURCE, FEEDBACK, 0, SOURCE_USD);// send a quick command to test, this could be any SOMO_II function
 
}

uint8_t SOMO_II::somoCommand(uint8_t tempCommand, uint8_t tempFeedback, uint8_t para1, uint8_t para2){

	char inByte;
  char inCommand;
  uint8_t inData;//what, if anything, the somo returns in the second parameter
	uint16_t checkSum;
  char tempResponse[9];
	checkSum = 0xFFFF - (tempCommand + tempFeedback + para1 + para2) + 1;
        
        //parse the buffer first, flushing somo serial port from song/init/source activity data
        while (somoSerial->available()){
          //read the response from the SOMO
          somoSerial->readBytesUntil(END, tempResponse, 8);
          inCommand = tempResponse[1];
          inData = tempResponse[4];
          switch (inCommand){//this switchcase is reserved for future use, doesn't return anything yet
            case SOURCE_INSERT:
              if (inData == SOURCE_USB){
                //USB flash drive inserted
              } else if (inData == SOURCE_USD){
                //uSD card inserted
              }
            break;
            case SOURCE_REMOVED:
              if (inData == SOURCE_USB){
                //USB flash drive removed
              } else if (inData == SOURCE_USD){
                //uSD card removed
              }
            break;
            case USB_DONE:
              //USB has finished playing song(inData)
            break;
            case USD_DONE:
              //uSD has finished playing song(inData)
            break;
            case SOMO_STARTUP:
              switch(inData){
                case STARTUP_NO_MEDIA:
                break;
                case STARTUP_USB_TRUE:
                break;
                case STARTUP_USD_TRUE:
                break;
                case STARTUP_USB_AND_USD_TRUE:
                break;
              }
            break;
          }
        }
        
        //send the command string
        somoSerial->write(START);
        somoSerial->write(tempCommand);
        somoSerial->write(tempFeedback);
        somoSerial->write(para1);
        somoSerial->write(para2);
        somoSerial->write(highByte(checkSum));
        somoSerial->write(lowByte(checkSum));
        somoSerial->write(END);

        somoSerial->readBytesUntil(END, tempResponse, 8);//read the response from the SOMO
        inCommand = tempResponse[1];
        inData = tempResponse[4];
        
        switch (inCommand){
          case QUERY_VOLUME:
          case QUERY_EQ:
          case QUERY_TRACKS_USB:
          case QUERY_TRACKS_USD:
          case QUERY_CUR_TRACK_USB:
          case QUERY_CUR_TRACK_USD:
          case QUERY_TRACKS_IN_FOLDER:
          case SOMO_NAK:// in data can be the following error codes (NAK_BUSY, NAK_ASLEEP, NAK_SERIAL, NAK_CHECKSUM, NAK_SCOPE, NAK_NO_FILE)
            return inData;
          break;
          case SOMO_ACK:
            return true;
          break;
          default:
            //Serial.println("No response");
            return false;
          break;
        }//END SWITCH

}

boolean SOMO_II::volumeUp(){
  return somoCommand(VOLUME_PLUS, FEEDBACK, 0, 0);
}

boolean SOMO_II::volumeDown(){
  return somoCommand(VOLUME_MINUS, FEEDBACK, 0, 0);
}

boolean SOMO_II::setVolume(uint8_t volume){
  if (volume < MIN_VOLUME){ volume = MIN_VOLUME;}
  if (volume > MAX_VOLUME){ volume = MAX_VOLUME;} 
  return somoCommand(VOLUME_SET, FEEDBACK, 0, volume);
}

uint8_t SOMO_II::getVolume(){
  return somoCommand(QUERY_VOLUME, FEEDBACK, 0, 0);
}

boolean SOMO_II::playNext(){
  return somoCommand(NEXT, FEEDBACK, 0, 0);
}

boolean SOMO_II::playPrevious(){
  return somoCommand(PREVIOUS, FEEDBACK, 0, 0);
}

boolean SOMO_II::playFolderTrack(uint8_t folder, uint8_t track){
  return somoCommand(FOLDER_AND_TRACK_NUMBER, FEEDBACK, folder, track);
}
    
boolean SOMO_II::playTrack(uint8_t track){
  return somoCommand(PLAY_TRACK_NUM, FEEDBACK, 0, track);
}

boolean SOMO_II::repeatTrack(){
  return somoCommand(REAPEAT_A_TRACK, FEEDBACK, 0, 0);
}

boolean SOMO_II::play(){
  return somoCommand(PLAY, FEEDBACK, 0, 0);
}

boolean SOMO_II::pause(){
  return somoCommand(PAUSE, FEEDBACK, 0, 0);
}

boolean SOMO_II::stop(){
  return somoCommand(STOP, FEEDBACK, 0, 0);
}

boolean SOMO_II::setSource(uint8_t source){
  return somoCommand(PLAY_SOURCE, FEEDBACK, 0, source);
}

boolean SOMO_II::setEQ(uint8_t eq){
 return somoCommand(SET_EQ, FEEDBACK, 0, eq); 
}

boolean SOMO_II::setMode(uint8_t mode){
  switch (mode){
    case SINGLE_PLAY:
      return somoCommand(SINGLE_PLAY, FEEDBACK, 0, 0);
    break;
    case CONTINUOUS:
      return somoCommand(CONTINUOUS, FEEDBACK, 0, 0);
    break;
    //case REPEAT_CURRENT:
      //return somoCommand(REPEAT_CURRENT, FEEDBACK, 0, 0);
    //break;
    case RANDOM_TRACK:
      return somoCommand(RANDOM_TRACK, FEEDBACK, 0, 0);
    break;
  }
}

boolean SOMO_II::sleep(){
  return somoCommand(SLEEP, FEEDBACK, 0, 0);
}

boolean SOMO_II::reset(){
  return somoCommand(RESET, FEEDBACK, 0, 0);
}

boolean SOMO_II::isBusy(){
  return(digitalRead(_pin));
}

uint8_t SOMO_II::getEQ(){
  return somoCommand(QUERY_EQ, FEEDBACK, 0, 0);
}

uint8_t SOMO_II::getTracksUSB(){
  return somoCommand(QUERY_TRACKS_USB, FEEDBACK, 0, 0);
}

uint8_t SOMO_II::getTracksSD(){
  return somoCommand(QUERY_TRACKS_USD, FEEDBACK, 0, 0);
}

uint8_t SOMO_II::getTracksInFolder(uint8_t folder){
  return somoCommand(QUERY_TRACKS_IN_FOLDER, FEEDBACK, 0, folder);
}

// Low level stuff, allows the class to inherit the stream functions
inline int SOMO_II::available(void) {
  return somoSerial->available();
}

inline size_t SOMO_II::write(uint8_t x) {
  return somoSerial->write(x);
}

inline int SOMO_II::readBytesUntil(char stop, char* array, int length) {
  return somoSerial->readBytesUntil(stop, array, length);
}

inline int SOMO_II::read(void) {
  return somoSerial->read();
}

inline int SOMO_II::peek(void) {
  return somoSerial->peek();
}

inline void SOMO_II::flush() {
  somoSerial->flush();
}
/*
  SOMO_II.h - Library for playing MP3 files with the 4D Systems SOMO II
  Created by DJ Harrigan, September 9, 2014.
  Released into the public domain.
*/

#ifndef SOMO_II_h
#define SOMO_II_h

#include "Arduino.h"
#include "SoftwareSerial.h"

//General Codes
#define START      	0x7E
#define END        	0xEF
#define FEEDBACK   	0x01
#define NO_FEEDBACK 0x00
  //Command Codes
#define NEXT					0x01
#define PREVIOUS    			0X02
#define PLAY_TRACK_NUM   		0x03
#define VOLUME_PLUS				0x04
#define VOLUME_MINUS    		0x05
#define VOLUME_SET      		0x06
#define SET_EQ					0x07
#define REAPEAT_A_TRACK			0x08
#define PLAY_SOURCE				0x09
#define SLEEP					0x0A
#define RESET					0x0C
#define PLAY					0x0D
#define PAUSE					0x0E
#define FOLDER_AND_TRACK_NUMBER	0x0F
#define CONTINUOUS				0x11
#define STOP					0x16
#define RANDOM_TRACK			0x18
//#define REPEAT_CURRENT			0x19
#define SINGLE_PLAY				0x19//just send another single play command to repeat a song
#define QUERY_VOLUME			0x43
#define QUERY_EQ				0x44
#define QUERY_TRACKS_USB		0x47
#define QUERY_TRACKS_USD		0x48
#define QUERY_CUR_TRACK_USB		0x4B
#define QUERY_CUR_TRACK_USD		0x4C
#define QUERY_TRACKS_IN_FOLDER	0x4E
//Feedback Codes
#define SOMO_ACK 	0x41
#define SOMO_NAK 	0x40
//Misc Feedback Codes
#define SOURCE_INSERT		0x3A
#define SOURCE_REMOVED		0x3B
#define USB_DONE 			0x3C
#define USD_DONE 			0x3D
#define SOMO_STARTUP    	0x3F
//Negative Acknowledge Parameters
#define NAK_BUSY 	0x01
#define NAK_ASLEEP  0X02
#define NAK_SERIAL  0x03
#define NAK_CHKSUM  0x04
#define NAK_SCOPE   0x05
#define NAK_NOFILE  0x06
//Startup Data Parameters
#define STARTUP_NO_MEDIA			0x00
#define STARTUP_USB_TRUE   			0x01
#define STARTUP_USD_TRUE			0X02
#define STARTUP_USB_AND_USD_TRUE 	0x03
//Misc Parameters
#define MIN_VOLUME   0x00
#define MAX_VOLUME	 0x1E
#define EQ_NORMAL 	 0x00
#define EQ_POP		 0X01
#define EQ_ROCK		 0x02
#define EQ_JAZZ		 0x03
#define EQ_CLASSIC	 0x02
#define EQ_BASS		 0x03
#define SOURCE_USB   0X01
#define SOURCE_USD   0x02

class SOMO_II : public Stream {

	public:

		SOMO_II(SoftwareSerial *, uint8_t b);
		boolean begin();

		boolean volumeUp();
		boolean volumeDown();
		boolean setVolume(uint8_t volume);
		uint8_t getVolume();

		boolean playNext();
		boolean playPrevious();
		boolean playFolderTrack(uint8_t folder, uint8_t track);
		boolean playTrack(uint8_t track);
		boolean repeatTrack();
		boolean play();
		boolean pause();
		boolean stop();

		boolean setSource(uint8_t source);
		boolean setEQ(uint8_t eq);
		boolean setMode(uint8_t mode);
		boolean sleep();
		//boolean wakeUp(); Use setSource() command to wake up module
		boolean reset();

		boolean isBusy();
		uint8_t getEQ();
		uint8_t getTracksUSB();
		uint8_t getTracksSD();
		uint8_t getTracksInFolder(uint8_t folder);

		int available(void);
  		size_t write(uint8_t x);
  		int readBytesUntil(char stop, char* array, int length);
  		int read(void);
  		int peek(void);
  		void flush();

	private:

		uint8_t somoCommand(uint8_t tempCommand, uint8_t tempFeedback, uint8_t para1, uint8_t para2);
		uint8_t _pin;
		SoftwareSerial *somoSerial;

};

#endif
/**
 * Motion_GPS.h
 * @author:   Will Patton 
 * @url:      https://github.com/willpatton 
 * @license:  MIT License


	Gets GPS messages and echos them to a serial console and/or OLED U8G2 display.
	
	Uses U8G2
	
 */

#ifndef __MOTION_GPS_H
#define __MOTION_GPS_H

#include <Arduino.h>
#include <Adafruit_GPS.h>


//GPS
// what's the name of the hardware serial port?
#define GPSSerial Serial1

// Connect to the GPS on the hardware port
Adafruit_GPS GPS(&GPSSerial);
     
// Set GPSECHO to 'false' to turn off echoing the GPS data to the Serial console
// Set to 'true' if you want to debug and listen to the raw GPS sentences
#define GPSECHO true


//OLED U8G2 256x64 SSD1322 
#include <U8g2lib.h>
extern U8G2_SSD1322_NHD_256X64_2_4W_HW_SPI u8g2;


//SCREEN
#define SCR_GPS 1
#define SCR_DATETIME 2
#define SCR_MPH 3


//CLASS
class CGPS
{

public:
	//prototypes
	CGPS();
	void setup_gps();
	void detect_gps(); 			//is gps hardware present
	void loop_gps(int screen);  //what screen to display
	void gps_text();

private:

	//TIMERS
	uint32_t timer_gps;
	uint32_t counter_gps_bytes_sec;
	uint32_t counter_loop_gps = 0;

	//SCREENS
	uint16_t screen_gps = SCR_GPS; 

	//STRINGS
	String lat;					//Arduino style string handlers
	String lon;

};


#endif
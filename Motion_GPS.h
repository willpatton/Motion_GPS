/**
 * Motion_WiFi.h
 * @author:   Will Patton 
 * @url:      http://willpatton.com 
 * @license:  MIT License
 */

#ifndef __MOTION_GPS_H
#define __MOTION_GPS_H

#include <Arduino.h>
#include <Adafruit_GPS.h>


//#define GPS true
//SCREEN
#define SCR_GPS 1
#define SCR_DATETIME 2
#define SCR_MPH 3


class CGPS
{

public:
	//prototypes
	CGPS();
	void setup_gps();
	void detect_gps(); //is gps hardware present
	void loop_gps(int screen);  //what screen to display
	void gps_text();

private:

	volatile int screen_gps = SCR_GPS; 

	uint32_t timer_gps;

	String lat;
	String lon;

	int counter_gps_bytes_sec;
	int counter_loop_gps = 0;



};



#endif
/**
 * Test code for Ultimate GPS Using Hardware Serial (e.g. GPS Flora or FeatherWing)
 * https://learn.adafruit.com/adafruit-ultimate-gps-featherwing/basic-rx-tx-test
 *
 * This code shows how to listen to the GPS module via polling. Best used with
 * Feathers or Flora where you have hardware Serial and no interrupt
 *
 * Tested and works great with the Adafruit GPS FeatherWing
 * ------> https://www.adafruit.com/products/3133
 * 
 * 
 * // To generate your own sentences, check out the MTK command datasheet and use a checksum calculator
 * // such as the awesome http://www.hhhh.org/wiml/proj/nmeaxor.html
 * 
 */
/*
GPS message formats

GGA - Time, position and fix type data.
GSA - GPS receiver operating mode, active satellites used in the position solution and DOP values.
GSV - The number of GPS satellites in view satellite ID numbers, elevation, azimuth, and SNR values.
RMC - Time, date, position, course and speed data. Recommended Minimum Navigation Information.
VTG - Course and speed information relative to the ground.
*/


#include "Motion_GPS.h"

// what's the name of the hardware serial port?
#define GPSSerial Serial1

// Connect to the GPS on the hardware port
Adafruit_GPS GPS(&GPSSerial);
     
// Set GPSECHO to 'false' to turn off echoing the GPS data to the Serial console
// Set to 'true' if you want to debug and listen to the raw GPS sentences
#define GPSECHO true


#include <U8g2lib.h>
extern U8G2_SSD1322_NHD_256X64_2_4W_HW_SPI u8g2;



/**
 * constructor
 */
CGPS::CGPS(){

}

/**
 * detect if GPS hardware is present
 */
bool detect_gps(){

  //TODO detect hardware

  return false;
}

/**
 * 
 */
void CGPS::setup_gps()
{
  //CONSOLE BAUD
  //Serial.begin(115200);
  //while (!Serial); 
  Serial.print("Setup GPS... ");

  //GPS BAUD
  // 9600 NMEA is the default baud rate for Adafruit MTK GPS. 
  // Will's MTK3329 default is 115200
  GPS.begin(115200);

  //COMMANDS
  // uncomment this line to turn on RMC (recommended minimum) and GGA (fix data) including altitude
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  // uncomment this line to turn on only the "minimum recommended" data
  //GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
  // For parsing data, we don't suggest using anything but either RMC only or RMC+GGA since
  // the parser doesn't care about other sentences at this time
  
  // UPDATE RATE
  // Note that these only control the rate at which the position is echoed, 
  // For the parsing code to work nicely and have time to sort thru the data, and
  // print it out we don't suggest using anything higher than 1 Hz
  
  //PMTK_SET_NMEA_UPDATE_200_MILLIHERTZ  //// Once every 5 seconds, 200 millihertz.
  //PMTK_SET_NMEA_UPDATE_1HZ
  //PMTK_SET_NMEA_UPDATE_5HZ
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ); //update rate .2, 1, 5, 10Hz

  // FIX RATE
  // To speed up the position fix you must also send one of the position fix rate commands below too.
  //PMTK_API_SET_FIX_CTL_100_MILLIHERTZ // Once every 10 seconds, 100 millihertz.
  //PMTK_API_SET_FIX_CTL_200_MILLIHERTZ // Once every 5 seconds, 200 millihertz.
  //PMTK_API_SET_FIX_CTL_1HZ 
  //PMTK_API_SET_FIX_CTL_5HZ 
  // Can't fix position faster than 5 times a second!
   GPS.sendCommand(PMTK_API_SET_FIX_CTL_5HZ); //fix rate - 1 or 5Hz. Choose 5 HZ for speedometer
  

  //ANTENNA
  // Request updates on antenna status, comment out to keep quiet
  //GPS.sendCommand(PGCMD_ANTENNA);
  // wait for commands to take effect
  //delay(1000);
  
  // Ask for firmware version
  //GPSSerial.println(PMTK_Q_RELEASE);

  Serial.println("COMPLETE.");
  timer_gps = millis();
}


/**
 * loop gps
 * real time continuous reading of GPS messages data
 * run over and over again
 */
void CGPS::loop_gps(int screen) 
{
    screen_gps = screen;

    //time this fn
    int timer_fn = micros();
    
    // read data from the GPS in the 'main loop'
    char c = GPS.read();
    // if you want to debug, this is a good time to do it!
    if (GPSECHO){
      if (c) {
        //if(c == '$'){
        //  Serial.println();
        //}
        Serial.print(c);       
      }
    }
    // if a sentence is received, we can check the checksum, parse it...
    if (GPS.newNMEAreceived()) {
      // a tricky thing here is if we print the NMEA sentence, or data
      // we end up not listening and catching other sentences!
      // so be very wary if using OUTPUT_ALLDATA and trytng to print out data
      String str_gps = GPS.lastNMEA(); // this also sets the newNMEAreceived() flag to false
      if (!GPS.parse(GPS.lastNMEA())) {// this also sets the newNMEAreceived() flag to false
        return; // we can fail to parse a sentence in which case we should just wait for another
      }
      //DEBUG- output raw msg to tft as it happens
      //tft display - no fix msgs are about 165ms for 39 byte msg and 194ms for 49 byte msg.  Longer if fix (more bytes).
      //update rates of 2Hz (500ms) may work, but 5Hz (200ms) may not work (too fast) .
      /*if(debug){      
        //Serial.println(str_gps);  //also echo to serial
        tft.setCursor(0,190);
        tft.print(GPS.lastNMEA()); tft.print("          ");    
        tft.setCursor(20,260);
        tft.print("timer_fn "); 
        tft.print(String(((micros() - timer_fn) * 0.001), 1));  //msec
        tft.print(" msec"); 
        tft.print(" bytes: ");    
        counter_gps_bytes_sec = str_gps.length();
        tft.print(String(counter_gps_bytes_sec)); tft.print(" ");  //bytes   
      }*/
      //yield();
      //gps_text();
    }

    //OVERFLOW - if millis() or timer wraps around, we'll just reset it
    if (timer_gps > millis()) {timer_gps = millis();}

    //OLED REFRESH DISPLAY
    // 2Hz - periodically print out the current stats
    if((millis() - timer_gps > 480)) {    
      timer_gps = millis();       // reset gps timer

        //Serial print using raw GPS structure
      if(0){
        Serial.print("Date: ");
        Serial.print("20");Serial.print(GPS.year, DEC);Serial.print('-');
        Serial.print(GPS.month, DEC);Serial.print('-');
        Serial.print(GPS.day, DEC);
        Serial.print(" Time: ");
        Serial.print(GPS.hour, DEC); Serial.print(':');
        Serial.print(GPS.minute, DEC); Serial.print(':');
        Serial.print(GPS.seconds, DEC); Serial.print('.');
        Serial.println(GPS.milliseconds);
        Serial.print("Satellites: "); Serial.print((int)GPS.satellites);
        Serial.print(" Fix: "); Serial.print((int)GPS.fix);
        Serial.print(" quality: "); Serial.println((int)GPS.fixquality);   
        if(GPS.fix) { 
          Serial.print("Location: ");
          Serial.print(GPS.latitude, 4); Serial.print(GPS.lat);
          Serial.print(", ");
          Serial.print(GPS.longitude, 4); Serial.println(GPS.lon);
          Serial.print("Speed (knots): "); Serial.println(GPS.speed);
          Serial.print("Angle: "); Serial.println(GPS.angle);
          Serial.print("Altitude: "); Serial.println(GPS.altitude);
          //Serial.print("Satellites: "); Serial.println((int)GPS.satellites);
        }//end if fix
      }

      gps_text();
  }


  counter_loop_gps++;

  //yield();
  //wcp debug
  //return;
}


/**
 GPS text
*/
unsigned long timer_text_sec = 0;
void CGPS::gps_text() {

  unsigned long timer_text = micros();

  if(micros() - timer_text_sec < 1000000){
   return;
  }
  timer_text_sec = micros();
   
  //force speed for debug
  //GPS.speed = 77;
        
  String day = String(GPS.day, DEC);       //time
  String month = String(GPS.month, DEC);
  String year = String(GPS.year, DEC);
  String str_date = String("20" + year + "-" + month + "-" + day );
  
  String hour = String(GPS.hour, DEC);       //time
  String minute = String(GPS.minute, DEC);
  String seconds = String(GPS.seconds, DEC);
  String milliseconds = String(GPS.milliseconds / 100, DEC);
  String str_time = String(hour + ":" + minute + ":" + seconds );   //" GMT"
  str_time = String(str_time + + "." + milliseconds + " GMT          ");

  String sats = String((int)GPS.satellites);       //num of sats
  String str_sats = String(sats + " sats");
  //fix
  String fix = String((int)GPS.fix);            //fix quality
  String str_fix = String(fix + " fix");
  //fix quality
  String fixq = String((int)GPS.fixquality);       //fix quality
  String str_fixq = String(fixq + "q");

  //DEBUG
  //Serial.println("hello gps_text()");
  //return;
  lat = String(String(GPS.lat) + " " + String(GPS.latitude, 4) + "         ");
  lon = String(String(GPS.lon) + " " + String(GPS.longitude, 4) + "         ");

  //lat = String("55.66");
  //lon = String("77.88");

  //english
  String str_speed;
  String str_alt;
  if(1){
    String spd = String((int)(GPS.speed * 1.15078));  //speed (convert knots to mph)
    str_speed = String(spd); 
    String str_spd_units("mph      ");
    String alt = String(GPS.altitude * 3.28084, 0);   //feet
    str_alt = String(alt + " ft    "); 
  }
  //metric
  else {
    String spd = String((int)(GPS.speed * 1.852));  //speed (convert knots to kph)
    str_speed = String(spd);
    String str_spd_units("kph        ");
    String alt = String(GPS.altitude, 0);         //meters
    str_alt = String(alt + "m       "); 
  }


  //heading
  //GPS.angle = 0; //debug
  //GPS.angle = 180; //debug
  String ang;
  String str_ang("---");
  if(GPS.angle >= 0){
    ang = String(GPS.angle, 0);       //heading.  Values should not be negative???
    str_ang = String(ang); //+ " head"     //"°"
  }
    
  String str_heading = String("--");
  int angle = GPS.angle;
  //Serial.println();
  //Serial.println(angle);
  if(angle <= 15 || angle >= 345) {str_heading = String("N");}    //0, 360 NORTH
  if(angle > 15 && angle <= 30) {str_heading = String("NNE");} 
  if(angle > 30 && angle <= 60) {str_heading = String("NE");}    //45
  if(angle > 60 && angle < 75) {str_heading = String("ENE");} 
  if(angle >= 75 && angle <= 105) {str_heading = String("E");}    //90 EAST
  if(angle > 105 && angle <= 120) {str_heading = String("ESE");} 
  if(angle > 120 && angle <= 150) {str_heading = String("SE");}  //135 
  if(angle > 150 && angle < 165) {str_heading = String("SSE");} 
  if(angle >= 165 && angle <= 195) {str_heading = String("S");}   //180 SOUTH
  if(angle > 195 && angle <= 210) {str_heading = String("SSW");} 
  if(angle > 210 && angle <= 240) {str_heading = String("SW");}  //225
  if(angle > 240 && angle < 255) {str_heading = String("WSW");} 
  if(angle >= 255 && angle <= 285) {str_heading = String("W");}   //270 WEST
  if(angle > 285 && angle <= 300) {str_heading = String("WNW");}
  if(angle > 300 && angle <= 330) {str_heading = String("NW");}  //315
  if(angle > 330 && angle < 345) {str_heading = String("NNW");} 


  //SPEED MPH/KPH
  if(screen_gps == SCR_MPH){

    //Serial
    Serial.print("\nHello MPH ");
    Serial.print("speed:  ");
    Serial.print(str_speed.c_str()); 
    Serial.println(" mph");


    u8g2.firstPage();
    do {   
      u8g2.setFont(u8g2_font_helvB08_tr);
      //tft.println(2,30, scrn.c_str());  //screen number
      //u8g2.setFont(u8g2_font_helvB18_tr);

      //u8g2.setFont(u8g2_font_helvR14_tf);
      u8g2.setCursor(0,14);   
      u8g2.println("Speed ");
      //tft.setTextSize(8);

      u8g2.setFont(u8g2_font_helvB18_tr);
      u8g2.setCursor(120,40);
      u8g2.print(str_speed.c_str());  //speed
      //wcp - u8g2.setFont(u8g2_font_helvB10_tr);
      //tft.setCursor(92,30);
      //tft.setTextSize(2);
      u8g2.setFont(u8g2_font_helvB08_tr);
      u8g2.setCursor(220,60);
      u8g2.println("mph");  //speed
    } while (u8g2.nextPage()); 

  }
  
  //GPS
  if(screen_gps == SCR_GPS){

    Serial.println("\nHello GPS Position ");
    Serial.print("Sats: ");Serial.println(str_sats.c_str());
    Serial.print("Date: ");
    Serial.print(str_date.c_str()); 
    Serial.print(" Time: ");
    Serial.println(str_time.c_str());
    
    u8g2.firstPage();
    do { 
      if(0) { //!GPS.fix
        u8g2.setFont(u8g2_font_helvB08_tr);
        u8g2.setCursor(0,10); 
        u8g2.println("Waiting for GPS..."); 
        u8g2.setCursor(0,20); 
        u8g2.println("Sats: "); 
        u8g2.println(str_sats.c_str());  
        return;
      }
      if(1){ //GPS.fix
        u8g2.setFont(u8g2_font_helvB08_tr);
        u8g2.setCursor(0,10); 
        u8g2.println("GPS Position");
        u8g2.setCursor(0,20); 
        u8g2.println(str_sats.c_str());  
        u8g2.setCursor(0,30);  
        u8g2.println(str_fixq.c_str());  
        u8g2.setCursor(80,20); 
        u8g2.println(lat.c_str()); 
        u8g2.setCursor(80,30); 
        u8g2.println(lon.c_str());  
        u8g2.setCursor(80,40); 
        u8g2.println(str_alt.c_str());

        //heading  
        u8g2.setCursor(80,50); 
        u8g2.print(str_heading.c_str());  //2,30,    //HEADING LETTERS
        u8g2.print(" ");
        u8g2.print(str_ang.c_str()); //100,30,   //HEADING ANGLE
        //TODO DEGREE SYMBOL
        //degree symbol is 176
        ////wcp - u8g2.setFont(u8g2_font_unifont_t_symbols);  //best if used with font ending in "f" or "e"
        ////wcp - u8g2.drawGlyph(16, 30, 176);  // dec 9731/hex 2603 Snowman
        const char DEGREE_SYMBOL[] = { 0xB0, '\0' };
        //u8g2.drawUTF8(120, 30, DEGREE_SYMBOL);
        u8g2.print(String(DEGREE_SYMBOL));

        //speed
        //u8g2.print("   ");   
        u8g2.setCursor(80,60);   
        u8g2.print(str_speed.c_str());
        u8g2.println(" mph  "); 
        u8g2.println("        ");  
        
        //date and time
        u8g2.setCursor(160,20); 
        u8g2.println(str_date.c_str()); 
        u8g2.setCursor(160,30); 
        u8g2.println(str_time.c_str());    

        //SPINNER
        u8g2.setCursor(230,60);  
        if(counter_loop_gps & 0x01){
          u8g2.println("\\"); 
        } else {
          u8g2.println("/"); 
        }
 
      } 
    } while (u8g2.nextPage() ); 
  }   

  //DATE, TIME, STOPWATCH
  //Astronomical clock - see https://en.wikipedia.org/wiki/Astronomical_clock
  //time of day
  //calendar and zodiac
  //moon
  //hour lines
  //aspects
  //dragon hand - eclipse prediction and lunar nodes
  //TODO find formulas
  // http://edwilliams.org/sunrise_sunset_algorithm.htm
  if(screen_gps == SCR_DATETIME){

    //Serial
    Serial.print("\nHello Datetime: ");
    Serial.print(str_date.c_str()); Serial.print(" ");
    Serial.println(str_time.c_str());

    //OLED
    u8g2.firstPage();
    do { 
      u8g2.setFont(u8g2_font_helvB08_tr);
      //u8g2.setCursor(0,14);  
      //u8g2.drawStr(0,14,"GPS Date/Time String\n");
      u8g2.setCursor(0,10);   
      u8g2.println("GPS Date/Time");
      //u8g2.println(" Spc ");
      //tft.fillRect(0,36,179,34, 40);
      u8g2.setCursor(0,20); 
      u8g2.println(str_date.c_str());
      u8g2.setCursor(0,30); 
      u8g2.println(str_time.c_str());
      //u8g2.drawStr(0,145,str_time.c_str());
    } while (u8g2.nextPage()); 
  }

  //tft.setCursor(20,280);
  //tft.println(String("timer_text: ") + String((micros() - timer_text) * 0.001,1) + String(" msec"));

 
}
// NTP Clock based on SeeedStudio's Squama Ethernet board with PoE: 
//     https://www.seeedstudio.com/Squame-Ethernet-Arduino-W5500-Ethernet-Board-with-PoE-p-4822.html
// and SeedStudio's Grove 0.54" Red Quad Alphanumeric Display
//     https://www.seeedstudio.com/Grove-0-54-Red-Quad-Alphanumeric-Display-p-4032.html
//
// Wiring is straightforward: Plug Squama Board into Display using Grove Cable.  Plug Squama Board into PoE Ethernet Switch Port.
//
// IMPORTANT: Do NOT connect USB power to board if ethernet is plugged into a PoE switchport.  Unexpected results (incuding frying your
// computer or the Squama Board) may result with two separate power feeds!
// Either plug the Squama board into a NON-PoE Port on your switch during development, or use a power-switched USB hub between you
// computer and the Squama board. 
//
// Lloyd W. Taylor, ltaylor@netelder.com, 15 August 2021
// Grateful thanks to all the folks who have provided libraries and examples to make this all work!


#include "TimeLib.h"   // https://github.com/PaulStoffregen/Time
#include "NTPClient.h" // https://www.arduino.cc/reference/en/libraries/ntpclient/
#include "Ethernet.h"
#include <EthernetUdp.h>
#include <SPI.h>
#include <Wire.h>
#include "grove_alphanumeric_display.h" // https://github.com/Seeed-Studio/Seeed_Alphanumeric_Display_HT16K33

#define DEBUG 0 // Set to 1 to enable serial console output.

Seeed_Digital_Tube tube;

// Configuration Options.  Review and adjust as appropriate.

// char NTPServer[] = "192.168.0.1"; // Your private NTP server (best for frequent updates (less than every 10 minutes)
char NTPServer[] = "0.pool.ntp.org"; // NTP Pool servers (https://www.ntppool.org/en/use.html).  (10 minute minimum update frequency)

const long timeZoneStandardTime = -25200; // offset in seconds from UTC. -25200 is US PST.  
long utcOffsetInSeconds = timeZoneStandardTime; // global variable for NTPClient offset. Daylight Savings Time offset is handled by isDST function
const long ntpPollDelay = 600000; // NTP poll delay in mS - 600,000 is 10 minutes

byte mac[] = {
  0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02   // Default for Seeed Squama Ethernet boards.  MUST be changed if you have more than one on your network.
};

// Fallback addresses.  Will only be used if DHCP fails.  Set to addresses that are correct for your network.

IPAddress ip(192, 168, 1, 177);
IPAddress myDns(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 0, 0);

// END of Configuration Options


#define BUF_SIZE  10
char message[BUF_SIZE] = "";

// Define NTP Client to get time
EthernetUDP ntpUdp;
NTPClient timeClient(ntpUdp, NTPServer, utcOffsetInSeconds, ntpPollDelay);

// Are we in Daylight Savings Time or not?

int isDST(int day, int month, int dow)
{
    // return 3600 (one hour in seconds) if we're in Daylight Savings time.
    //January, february, and december are out.
    if (month < 3 || month > 11) { return 0; }
    //April to October are in
    if (month > 3 && month < 11) { return 3600; }
    int previousSunday = day - dow;
    //In march, we are DST if our previous sunday was on or after the 8th.
    if (month == 3 && previousSunday >= 8) { return 3600; }
    //In november we must be before the first sunday to be dst.
    //That means the previous sunday must be before the 1st.
    if (previousSunday <= 0) { return 3600; }
}


void setup() {
  Serial.begin(115200);
  delay(2000);
  Wire.begin();
  
  tube.setTubeType(TYPE_4, TYPE_4_DEFAULT_I2C_ADDR);
  tube.setBrightness(15);
  tube.setBlinkRate(BLINK_OFF);

  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
      while (true) {
        delay(1); // do nothing, no point running without Ethernet hardware
      }
    }
    if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }
    // initialize the Ethernet device not using DHCP:
    Ethernet.begin(mac, ip, myDns, gateway, subnet);
  }
  // print your local IP address:
  Serial.print("My IP address: ");
  Serial.println(Ethernet.localIP());
  
  timeClient.begin();
  delay(1000);
  timeClient.update();
  Serial.println(timeClient.getFormattedTime());
}

void loop() {
  int hours, minutes, seconds;
  String hoursStr, minuteStr, secondStr;

  utcOffsetInSeconds = timeZoneStandardTime + isDST(day(), month(), weekday()); //Check for Daylight Savings Time
  timeClient.setTimeOffset(utcOffsetInSeconds);
  
  # if DEBUG 
    Serial.print(timeZoneStandardTime);
    Serial.print(":");
    Serial.println(utcOffsetInSeconds);  
  # endif
    
  if (timeClient.update()){  // Returns FALSE if update failed
    hours = timeClient.getHours();
    minutes = timeClient.getMinutes();
    seconds = timeClient.getSeconds();
  
    hoursStr = hours < 10 ? "0" + String(hours) : String(hours);
    minuteStr = minutes < 10 ? "0" + String(minutes) : String(minutes);
    secondStr = seconds < 10 ? "0" + String(seconds) : String(seconds);
  }
  else {
    hoursStr = "00";  // If you see "00:00" on the display, the NTP update has failed.  Check your ethernet/internet connection.
    minuteStr = "00";
  }

//  (" " + hoursStr + ":" + minuteStr).toCharArray(message, 7); // four digits with separator
  (hoursStr + minuteStr).toCharArray(message, 5); // four digits


  #if DEBUG
    Serial.println(message);
  #endif

  
  tube.displayString(message);
  tube.setPoint(1, 1);
  tube.display();
  delay(500);
  
  tube.setPoint(0, 0);
  tube.display();
  delay(500);
  
}

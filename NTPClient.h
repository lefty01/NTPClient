#pragma once

#include "Arduino.h"

#include <Udp.h>
#include <time.h>
#include <Time.h>

#define SEVENZYYEARS 2208988800UL
#define NTP_PACKET_SIZE 48
#define NTP_DEFAULT_LOCAL_PORT 123

//convenient constants for dstRules
enum week_t {Last, First, Second, Third, Fourth};
enum dow_t {Sun=1, Mon, Tue, Wed, Thu, Fri, Sat};
enum month_t {Jan=1, Feb, Mar, Apr, May, Jun, Jul, Aug, Sep, Oct, Nov, Dec};

//structure to describe rules for when daylight/summer time begins,
//or when standard time begins.
struct TimeChangeRule
{
  char abbrev[6];    //five chars max
  uint8_t week;      //First, Second, Third, Fourth, or Last week of the month
  uint8_t dow;       //day of week, 1=Sun, 2=Mon, ... 7=Sat
  uint8_t month;     //1=Jan, 2=Feb, ... 12=Dec
  uint8_t hour;      //0-23
  int offset;        //offset from UTC in minutes
};

struct DateTime {
  int dt_seconds;
  int dt_minutes;
  int dt_hours;
  int dt_date;
  int dt_month;
  int dt_year;
};

class NTPClient {
private:
  UDP*          _udp;
  bool          _udpSetup       = false;

  TimeChangeRule myDST;    //Daylight time = UTC - 4 hours
  TimeChangeRule mySTD;     //Standard time = UTC - 5 hours

  const char*   _poolServerName = "pool.ntp.org"; // Default time server
  IPAddress     _poolServerIP;
  int           _port           = NTP_DEFAULT_LOCAL_PORT;
  long          _timeOffset     = 0;

  unsigned long _updateInterval = 60000;  // In ms

  unsigned long _currentEpoc    = 0;      // In s
  unsigned long _lastUpdate     = 0;      // In ms

  byte          _packetBuffer[NTP_PACKET_SIZE];

  String        dayStrings[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

  void          sendNTPPacket();

  void calcTimeChanges(int yr);
  time_t toTime_t(TimeChangeRule r, int yr);
  TimeChangeRule _dst;    //rule for start of dst or summer time for any year
  TimeChangeRule _std;    //rule for start of standard time for any year
  time_t _dstUTC;         //dst start for given/current year, given in UTC
  time_t _stdUTC;         //std time start for given/current year, given in UTC
  time_t _dstLoc;         //dst start for given/current year, given in local time
  time_t _stdLoc;         //std time start for given/current year, given in local time

public:
  NTPClient(UDP& udp);
  NTPClient(UDP& udp, long timeOffset);
  NTPClient(UDP& udp, const char* poolServerName);
  NTPClient(UDP& udp, const char* poolServerName, long timeOffset);
  NTPClient(UDP& udp, const char* poolServerName, long timeOffset, unsigned long updateInterval);
  NTPClient(UDP& udp, IPAddress poolServerIP);
  NTPClient(UDP& udp, IPAddress poolServerIP, long timeOffset);
  NTPClient(UDP& udp, IPAddress poolServerIP, long timeOffset, unsigned long updateInterval);
  NTPClient(UDP& udp, int timeOffset, TimeChangeRule myDST, TimeChangeRule mySTD);
  /**
   * Set time server name
   *
   * @param poolServerName
   */
  void setPoolServerName(const char* poolServerName);

  /**
   * Starts the underlying UDP client with the default local port
   */
  void begin();

  /**
   * Starts the underlying UDP client with the specified local port
   */
  void begin(int port);

  /**
   * This should be called in the main loop of your application. By default an update from the NTP Server is only
   * made every 60 seconds. This can be configured in the NTPClient constructor.
   *
   * @return true on success, false on failure
   */
  bool update();

  /**
   * This will force the update from the NTP Server.
   *
   * @return true on success, false on failure
   */
  bool forceUpdate();

  int getDay() ;
  int getHours() ;
  int getMinutes() ;
  int getSeconds() ;

  /**
   * Get date time as a struct which contains
   * Year, Month, Date, Hours, Minutes, Seconds
   */
  DateTime getDateTime();

  /**
   * Return the date time as a String with the given format (Ex: %Y/%m/%d %H:%M:%S)
   */
  String getFormattedDateTime(const char* dateTimeFormat);

  /**
   * Changes the time offset. Useful for changing timezones dynamically
   */
  void setTimeOffset(int timeOffset);

  /**
   * Set the update interval to another frequency. E.g. useful when the
   * timeOffset should not be set in the constructor
   */
  void setUpdateInterval(unsigned long updateInterval);

  /**
   * @return time formatted like `hh:mm:ss`
   */
  String getFormattedTime();

  /**
   * @return time in seconds since Jan. 1, 1970
   */
  unsigned long getEpochTime();
  time_t toLocal(time_t utc);
  boolean utcIsDST(time_t utc);
  boolean locIsDST(time_t local);
  void setZones(TimeChangeRule dstStart, TimeChangeRule stdStart);

  /**
   * Stops the underlying UDP client
   */
  void end();
};

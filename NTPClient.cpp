/**
 * The MIT License (MIT)
 * Copyright (c) 2015 by Fabrice Weinberg
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "NTPClient.h"

NTPClient::NTPClient(UDP& udp) {
  this->_udp            = &udp;
}

NTPClient::NTPClient(UDP& udp, long timeOffset) {
  this->_udp            = &udp;
  this->_timeOffset     = timeOffset;
}

NTPClient::NTPClient(UDP& udp, const char* poolServerName) {
  this->_udp            = &udp;
  this->_poolServerName = poolServerName;
}

NTPClient::NTPClient(UDP& udp, IPAddress poolServerIP) {
  this->_udp            = &udp;
  this->_poolServerIP   = poolServerIP;
  this->_poolServerName = NULL;
}

NTPClient::NTPClient(UDP& udp, const char* poolServerName, long timeOffset) {
  this->_udp            = &udp;
  this->_timeOffset     = timeOffset;
  this->_poolServerName = poolServerName;
}

NTPClient::NTPClient(UDP& udp, IPAddress poolServerIP, long timeOffset){
  this->_udp            = &udp;
  this->_timeOffset     = timeOffset;
  this->_poolServerIP   = poolServerIP;
  this->_poolServerName = NULL;
}

NTPClient::NTPClient(UDP& udp, const char* poolServerName, long timeOffset, unsigned long updateInterval) {
  this->_udp            = &udp;
  this->_timeOffset     = timeOffset;
  this->_poolServerName = poolServerName;
  this->_updateInterval = updateInterval;
}

NTPClient::NTPClient(UDP& udp, IPAddress poolServerIP, long timeOffset, unsigned long updateInterval) {
  this->_udp            = &udp;
  this->_timeOffset     = timeOffset;
  this->_poolServerIP   = poolServerIP;
  this->_poolServerName = NULL;
  this->_updateInterval = updateInterval;
}

NTPClient::NTPClient(UDP& udp, int timeOffset,
		     TimeChangeRule myDST,
		     TimeChangeRule mySTD) {
  this->_udp            = &udp;
  this->setZones(myDST, mySTD);
  this->_timeOffset     = timeOffset;
}


void NTPClient::setZones(TimeChangeRule dstStart, TimeChangeRule stdStart) {
    _dst = dstStart;
    _std = stdStart;
}

/*----------------------------------------------------------------------*
 * Determine whether the given UTC time_t is within the DST interval    *
 * or the Standard time interval.                                       *
 *----------------------------------------------------------------------*/
boolean NTPClient::utcIsDST(time_t utc)
{
  //recalculate the time change points if needed
  if (year(utc) != year(_dstUTC)) calcTimeChanges(year(utc));

  if (_stdUTC > _dstUTC)    //northern hemisphere
    return (utc >= _dstUTC && utc < _stdUTC);
  else                      //southern hemisphere
    return !(utc >= _stdUTC && utc < _dstUTC);
}

/*----------------------------------------------------------------------*
 * Determine whether the given Local time_t is within the DST interval  *
 * or the Standard time interval.                                       *
 *----------------------------------------------------------------------*/
boolean NTPClient::locIsDST(time_t local)
{
  //recalculate the time change points if needed
  if (year(local) != year(_dstLoc)) calcTimeChanges(year(local));

  if (_stdLoc > _dstLoc)    //northern hemisphere
    return (local >= _dstLoc && local < _stdLoc);
  else                      //southern hemisphere
    return !(local >= _stdLoc && local < _dstLoc);
}

/*----------------------------------------------------------------------*
 * Convert the given UTC time to local time, standard or                *
 * daylight time, as appropriate.                                       *
 *----------------------------------------------------------------------*/
time_t NTPClient::toLocal(time_t utc)
{
  //recalculate the time change points if needed
  if (year(utc) != year(_dstUTC)) calcTimeChanges(year(utc));

  if (utcIsDST(utc))
    return utc + _dst.offset * SECS_PER_MIN;
  else
    return utc + _std.offset * SECS_PER_MIN;
}

/*----------------------------------------------------------------------*
 * Calculate the DST and standard time change points for the given      *
 * given year as local and UTC time_t values.                           *
 *----------------------------------------------------------------------*/
void NTPClient::calcTimeChanges(int yr)
{
    _dstLoc = toTime_t(_dst, yr);
    _stdLoc = toTime_t(_std, yr);
    _dstUTC = _dstLoc - _std.offset * SECS_PER_MIN;
    _stdUTC = _stdLoc - _dst.offset * SECS_PER_MIN;
}

/*----------------------------------------------------------------------*
 * Convert the given DST change rule to a time_t value                  *
 * for the given year.                                                  *
 *----------------------------------------------------------------------*/
time_t NTPClient::toTime_t(TimeChangeRule r, int yr)
{
  tmElements_t tm;
  time_t t;
  uint8_t m, w; //temp copies of r.month and r.week

  m = r.month;
  w = r.week;
  if (w == 0) {      // Last week = 0
    if (++m > 12) {  // for "Last", go to the next month
      m = 1;
      yr++;
    }
    w = 1;           // and treat as first week of next month, subtract 7 days later
  }

  tm.Hour = r.hour;
  tm.Minute = 0;
  tm.Second = 0;
  tm.Day = 1;
  tm.Month = m;
  tm.Year = yr - 1970;
  t = makeTime(tm); //first day of the month, or first day of next month for "Last" rules
  t += (7 * (w - 1) + (r.dow - weekday(t) + 7) % 7) * SECS_PER_DAY;
  if (r.week == 0) t -= 7 * SECS_PER_DAY; //back up a week if this is a "Last" rule
  return t;
}





void NTPClient::begin() {
  this->begin(NTP_DEFAULT_LOCAL_PORT);
}

void NTPClient::begin(int port) {
  this->_port = port;

  this->_udp->begin(this->_port);

  this->_udpSetup = true;
}

bool NTPClient::forceUpdate() {
  #ifdef DEBUG_NTPClient
    Serial.println("Update from NTP Server");
  #endif

  // flush any existing packets
  while(this->_udp->parsePacket() != 0)
    this->_udp->flush();

  this->sendNTPPacket();

  // Wait till data is there or timeout...
  byte timeout = 0;
  int cb = 0;
  do {
    delay ( 10 );
    cb = this->_udp->parsePacket();
    if (timeout > 100) return false; // timeout after 1000 ms
    timeout++;
  } while (cb == 0);

  this->_lastUpdate = millis() - (10 * (timeout + 1)); // Account for delay in reading the time

  this->_udp->read(this->_packetBuffer, NTP_PACKET_SIZE);

  unsigned long highWord = word(this->_packetBuffer[40], this->_packetBuffer[41]);
  unsigned long lowWord = word(this->_packetBuffer[42], this->_packetBuffer[43]);
  // combine the four bytes (two words) into a long integer
  // this is NTP time (seconds since Jan 1 1900):
  unsigned long secsSince1900 = highWord << 16 | lowWord;

  this->_currentEpoc = secsSince1900 - SEVENZYYEARS;

  return true;  // return true after successful update
}

bool NTPClient::update() {
  if ((millis() - this->_lastUpdate >= this->_updateInterval)     // Update after _updateInterval
    || this->_lastUpdate == 0) {                                // Update if there was no update yet.
    if (!this->_udpSetup) this->begin();                         // setup the UDP client if needed
    return this->forceUpdate();
  }
  return false;   // return false if update does not occur
}

// unsigned long NTPClient::getEpochTime_() const {
//   return this->_timeOffset +   // User offset
//          this->_currentEpoc + // Epoc returned by the NTP server
//          ((millis() - this->_lastUpdate) / 1000); // Time since last update
// }
unsigned long NTPClient::getEpochTime() {
  return ((this->_timeOffset + locIsDST(toLocal(this->_currentEpoc))) * 60 * 60) + // User offset
    this->_currentEpoc + // Epoc returned by the NTP server
    ((millis() - this->_lastUpdate) / 1000); // Time since last update
}

int NTPClient::getDay()  {
  return (((this->getEpochTime()  / 86400L) + 4 ) % 7); //0 is Sunday
}
int NTPClient::getHours()  {
  return ((this->getEpochTime()  % 86400L) / 3600);
}
int NTPClient::getMinutes()  {
  return ((this->getEpochTime() % 3600) / 60);
}
int NTPClient::getSeconds()  {
  return (this->getEpochTime() % 60);
}

DateTime NTPClient::getDateTime()  {
  struct tm * ts;
  time_t rawTime = this->getEpochTime();
  ts = localtime(&rawTime);
  DateTime dt = { ts->tm_sec, ts->tm_min, ts->tm_hour,
		  ts->tm_mday, (ts->tm_mon + 1), (ts->tm_year + 1900) };
  return dt;
}

String NTPClient::getFormattedDateTime(const char* dateTimeFormat)  {
  struct tm * ts;
  time_t rawTime = this->getEpochTime();
  ts = localtime(&rawTime);
  char buf[64];
  strftime(buf, sizeof(buf), dateTimeFormat, ts);
  return String(buf);
}

String NTPClient::getFormattedTime() {
  unsigned long rawTime = this->getEpochTime();
  unsigned long hours = (rawTime % 86400L) / 3600;
  String hoursStr = hours < 10 ? "0" + String(hours) : String(hours);

  unsigned long minutes = (rawTime % 3600) / 60;
  String minuteStr = minutes < 10 ? "0" + String(minutes) : String(minutes);

  unsigned long seconds = rawTime % 60;
  String secondStr = seconds < 10 ? "0" + String(seconds) : String(seconds);

  return hoursStr + ":" + minuteStr + ":" + secondStr;
}

void NTPClient::end() {
  this->_udp->stop();

  this->_udpSetup = false;
}

void NTPClient::setTimeOffset(int timeOffset) {
  this->_timeOffset     = timeOffset;
}

void NTPClient::setUpdateInterval(unsigned long updateInterval) {
  this->_updateInterval = updateInterval;
}

void NTPClient::setPoolServerName(const char* poolServerName) {
    this->_poolServerName = poolServerName;
}

void NTPClient::sendNTPPacket() {
  // set all bytes in the buffer to 0
  memset(this->_packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  this->_packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  this->_packetBuffer[1] = 0;     // Stratum, or type of clock
  this->_packetBuffer[2] = 6;     // Polling Interval
  this->_packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  this->_packetBuffer[12]  = 49;
  this->_packetBuffer[13]  = 0x4E;
  this->_packetBuffer[14]  = 49;
  this->_packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  if  (this->_poolServerName) {
    this->_udp->beginPacket(this->_poolServerName, this->_port);
  } else {
    this->_udp->beginPacket(this->_poolServerIP, this->_port);
  }
  this->_udp->write(this->_packetBuffer, NTP_PACKET_SIZE);
  this->_udp->endPacket();
}

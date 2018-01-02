#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>

Sd2Card card;
SdVolume volume;
SdFile root;
File writeToCard;

//char rate[14] = {0xB5, 0x62, 0x06, 0x08, 0x06, 0x00, 0xE8, 0x03, 0x01, 0x00, 0x01, 0x00, 0x01, 0x39}; // 1Hz
char rate[14] = {0xB5, 0x62, 0x06, 0x08, 0x06, 0x00, 0xC8, 0x00, 0x01, 0x00, 0x01, 0x00, 0xDE ,0x6A}; // 5 Hz

char baudrate[28] = {0xB5, 0x62, 0x06, 0x00, 0x14, 0x00, 0x01, 0x00, 0x00, 0x00, 0xD0, 0x08, 0x00, 0x00, 0x80, 0x25, 0x00, 0x00, 0x07, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA2, 0xB5}; //9600
//char baudrate[28] = { 0xB5, 0x62, 0x06, 0x00, 0x14, 0x00, 0x01, 0x00, 0x00, 0x00, 0xD0, 0x08, 0x00, 0x00, 0xC0, 0x12, 0x00, 0x00, 0x07, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0xCF, 0xE4}; //4800
//char baudrate[28] = { 0xB5, 0x62, 0x06, 0x00, 0x14, 0x00, 0x01, 0x00, 0x00, 0x00, 0xD0, 0x08, 0x00, 0x00, 0x00, 0x4B, 0x00, 0x00, 0x07, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x48, 0x57}; //19200

char timepulse[28] = {0xB5, 0x62, 0x06, 0x07, 0x14, 0x00, 0x40, 0x0D, 0x03, 0x00, 0xA0, 0x86, 0x01, 0x00, 0x01, 0x01, 0x00, 0x00, 0x34, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD1, 0xCA}; // 5 Hz
/*
 Assembly:

SD card reader:
 * MOSI - pin 11 on Arduino Uno/Duemilanove/Diecimila
 * MISO - pin 12 on Arduino Uno/Duemilanove/Diecimila
 * CLK - pin 13 on Arduino Uno/Duemilanove/Diecimila
 * CS - pin 4 (depends on your SD card shield or module).

GPS module:
 * VCC - 5V
 * GND - GND
 * TXD - 3
 * RXD - 5
 * PPS
 */
 
static const int RXPin = 5, TXPin = 3; //GPS
static const uint32_t GPSBaud = 9600;
const int chipSelect = 4; // SD - was 4 in sample

// The TinyGPS++ object
TinyGPSPlus gps;
// The serial connection to the GPS device
SoftwareSerial ss(RXPin, TXPin);

void setup()
{


  // START OUR SERIAL DEBUG PORT
  ss.begin(9600);
  //ss.write("$PUBX,41,1,0007,0003,19200,0*25\r\n");
  //ss.write(baudrate, 28);
  ss.write(rate, 14);
  ss.write(timepulse, 28);
  ss.begin(GPSBaud);
  ss.flush();
  
  Serial.begin(9600);
  SD.begin(4);
  
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  

  Serial.print(F("Testing TinyGPS++ library v. ")); Serial.println(TinyGPSPlus::libraryVersion());;
  Serial.print("\nInitializing SD card...");
  if (!card.init(SPI_HALF_SPEED, chipSelect)) {
    Serial.println("initialization failed. Things to check:");
    Serial.println("* is a card inserted?");
    Serial.println("* is your wiring correct?");
    Serial.println("* did you change the chipSelect pin to match your shield or module?");
    return;
  } else {
    Serial.println("Wiring is correct and a card is present.");
  }

  
  // Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
  if (!volume.init(card)) {
    Serial.println("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
    return;
  }

  // print the type and size of the first FAT-type volume
  Serial.println("Files found on the card (name, date and size in bytes): ");
  
  root.openRoot(volume);

  // list all files in the card with date and size
  root.ls(LS_R | LS_DATE | LS_SIZE);
}
  
void loop()
{
  // This sketch displays information every time a new sentence is correctly encoded.
  while (ss.available() > 0)
    if (gps.encode(ss.read()))
      displayInfo();

  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
    while(true);
  }
}

void displayInfo()
{

  Serial.print(F("Location: ")); 
  if (gps.location.isValid())
  {
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.print(gps.location.lng(), 6);
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.print(F("  Date/Time: "));
  if (gps.date.isValid())
  {
    Serial.print(gps.date.month());
    Serial.print(F("/"));
    Serial.print(gps.date.day());
    Serial.print(F("/"));
    Serial.print(gps.date.year());
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.print(F(" "));
  if (gps.time.isValid())
  {
    if (gps.time.hour() < 10) Serial.print(F("0"));
    Serial.print(gps.time.hour());
    Serial.print(F(":"));
    if (gps.time.minute() < 10) Serial.print(F("0"));
    Serial.print(gps.time.minute());
    Serial.print(F(":"));
    if (gps.time.second() < 10) Serial.print(F("0"));
    Serial.print(gps.time.second());
    Serial.print(F("."));
    if (gps.time.centisecond() < 10) Serial.print(F("0"));
    Serial.print(gps.time.centisecond());
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.println();
  
writeToCard = SD.open("GPSDATA.TXT", FILE_WRITE);
if (writeToCard) {
    if (gps.location.isValid()) //location to card
  {
    writeToCard.print(gps.location.lat(), 6);
    writeToCard.print(F(","));
    writeToCard.print(gps.location.lng(), 6);
    writeToCard.print(F(","));
  }
  else
  {
    writeToCard.print(F("INVALID"));
    writeToCard.print(F(","));
    writeToCard.print(F("INVALID"));
    writeToCard.print(F(","));
  }

  if (gps.date.isValid()) //date to card
  {
    writeToCard.print(gps.date.day());
    writeToCard.print(F("/"));
    writeToCard.print(gps.date.month());
    writeToCard.print(F("/"));
    writeToCard.print(gps.date.year());
    writeToCard.print(F(","));
  }
  else
  {
    writeToCard.print(F("INVALID"));
    writeToCard.print(F(","));
  }

  
   if (gps.time.isValid()) //time to card
  {
    if (gps.time.hour() < 10) writeToCard.print(F("0"));
    writeToCard.print(gps.time.hour());
    writeToCard.print(F(":"));
    if (gps.time.minute() < 10) writeToCard.print(F("0"));
    writeToCard.print(gps.time.minute());
    writeToCard.print(F(":"));
    if (gps.time.second() < 10) writeToCard.print(F("0"));
    writeToCard.print(gps.time.second());
    writeToCard.print(F("."));
    if (gps.time.centisecond() < 10) writeToCard.print(F("0"));
    writeToCard.print(gps.time.centisecond());
    writeToCard.print("\r\n");
  }
  else
  {
    writeToCard.print(F("INVALID"));
    writeToCard.print("\r\n");
  }
  writeToCard.close();
}
}



#define Version "RF_frameworkV"
#define ver 2 // version 
#define debugOn
#define forceInitEeprom false
#include <HomeAutomationBytesCommands.h> // commands specifications
#include <EEPROM.h>
#include <RF433Link.h>

#define addrEepromStation 0   // address of station address
#define addrEepromGateway 1  // address of gateway address
#define addrEepromSenderPin 2
#define addrEepromreceiverPin 3
uint8_t senderPin = 3;
#define PPTPin 4
uint8_t receiverPin = 5;
#define speedLink 2000
#define nbRetry 4
uint8_t stationAddress = 0x01;
uint8_t gatewayAddress = 0xfe;
#define diagJustReboot 0
#define diagTimeUpToDate 2
uint8_t diagByte = 0b000000101;
byte data[20];
uint8_t *Pdata = &data[0]; // pointer to the data zone
#define configPIN MOSI     // when high enter in configuration mode- MISO PIN can easely be set with a switch on ISCP connector
//RF433Link rfLink(stationAddress, gatewayAddress, senderPin, receiverPin, speedLink,nbRetry);
RF433Link rfLink(speedLink);
unsigned long lastStatusSentTime = 0;
unsigned long lastIndicatorsSentTime = 10000;
unsigned long lastToSheetSentTime = 20000;
unsigned long timeSendRegister = 30000;
unsigned long timeSendDatabase = 40000;
unsigned long timeTimeRequested = 25000;
unsigned long lastTimeUpdated ;
unsigned long  lastUpdateClock;
uint8_t value0 = 0;
#define registerSize 2
unsigned long  updateCycle = 5; // in minutes
uint8_t Registers[registerSize] = {updateCycle, 0x00};
#define updatableIndicatorsNumber 1
unsigned int updatableIndicatorsIndex[updatableIndicatorsNumber] = {18};
int updatableIndicatorsValues[updatableIndicatorsNumber] = {0};
//boolean timeUpToDate = false;
unsigned long alertSentSince = 0;
unsigned int alertIdentifierLevel1 = 0;
unsigned int alertIdentifierLevel2 = 0;
unsigned int* alertId;
boolean newOpenedAlert = false;
boolean newClosedAlert = false;
//boolean newTempOpenedAlert = false;
//boolean newTempClosedAlert = false;
#define defaultAlertInterval 5
//unsigned int alertInterval=90;
unsigned int countAlertSent = 0;
#define TimeMode
//#define RTCMode

#ifdef RTCMode
#include <RTClib.h>
RTC_DS1307 RTC;
#define
#endif

#ifdef TimeMode
#include <TimeLib.h>

#endif
//#if defined
#define MonthList "JanFebMarAprMayJunJulAugSepOctNovDec"  // do not change can not be localized
//#endif

#include <CCS811.h>
#include <MQ2.h>
/*
   IIC address default 0x5A, the address becomes 0x5B if the ADDR_SEL is soldered.
*/
CCS811 sensor;
int pin = 0; //change this to the pin that you use
float lpg = 0, co = 0, smoke = 0;

MQ2 mq2(pin); //instance (true=with serial output enabled)

void setup() {
  Serial.begin(38400);            // use serial for debug only with Arduino that has multiple serial interfaces
  delay(1000);
  Serial.print("start:");
  Serial.print(Version);
  Serial.println(ver);
  pinMode(configPIN, INPUT_PULLUP);
  if (!digitalRead(configPIN) || forceInitEeprom) {
    UpdateEeprom();
  }
  stationAddress = EEPROM.read(addrEepromStation);
  gatewayAddress = EEPROM.read(addrEepromGateway);
  senderPin = EEPROM.read(addrEepromSenderPin);
  receiverPin = EEPROM.read(addrEepromreceiverPin);
  Serial.print("gateway:");
  Serial.print(gatewayAddress, HEX);
  Serial.print(" RF_sub address:");
  Serial.print(stationAddress, HEX);
//  stationAddress=0x0a;
  Serial.print(" station:");
  Serial.println(GetUnsignedValue(gatewayAddress, stationAddress));
  #define receiveBroadcast true
  rfLink.SetParameters(stationAddress, gatewayAddress, senderPin, receiverPin, PPTPin,  nbRetry, receiveBroadcast);
  rfLink.Start();
  Serial.print("tx status:");
  Serial.println(rfLink.Tx_active(), HEX);
  randomSeed(analogRead(0));
}

void loop() {
  /*

  */
  delay(1);
  //Pdata=&data[0];

  rfLink.RetrySend();
  ReceiveRF();

  if ((millis() > lastStatusSentTime +  (unsigned long)(Registers[0]) * 60000) || (millis() > lastStatusSentTime + 60000 && bitRead(diagByte, diagJustReboot))) {
    Serial.println("send status");
    lastStatusSentTime = millis();
    data[0] = 0x00; // reserved for futur usage
    data[1] = statusResponse;
    data[2] = diagByte;
    data[3] = ver;
    int minuteSinceReboot = millis() / 60000;
    data[4] = uint8_t(minuteSinceReboot / 256);
    data[5] = uint8_t(minuteSinceReboot);
    rfLink.SendData(Pdata, 5, diagByte);
    AffTime();
  }
  if (millis() > lastIndicatorsSentTime + (unsigned long)(Registers[0]) * 60000) {
    Serial.print("send indicators value:0x");
    lastIndicatorsSentTime = millis();
    data[0] = 0x00; // reserved for futur usage
    data[1] = indicatorsRequest;
    data[2] = value0;
    int value1 = analogRead(0);
    Serial.print(" ");
    Serial.print(value1);
    Serial.print(" ");
    data[3] = uint8_t(value1 / 256);
    data[4] = uint8_t(value1);
    data[5] = highByte(updatableIndicatorsValues[0]);
    data[6] = lowByte(updatableIndicatorsValues[0]);
    Serial.print(data[5], HEX);
    Serial.print("-");
    Serial.println(data[6], HEX);
    rfLink.SendData(Pdata, 6);
    value0++;
    AffTime();
  }
  if (millis() > lastToSheetSentTime + (unsigned long)(Registers[0]) * 60000) {
    Serial.println("send to sheet");
    lastToSheetSentTime = millis();
#define nbValues 2
    int values[nbValues];
    values[0] = value0;
    values[1] = analogRead(0);
    SendToGoogleSheet(nbValues, values);
  }
  if (millis() > timeSendDatabase + (unsigned long)(Registers[0]) * 60000) {
    Serial.println("send to database");
    timeSendDatabase = millis();

#define nbValues 2
    int values[nbValues];
    values[0] = value0;
    values[1] = analogRead(0);
    SendToDatabase(nbValues, 0, values);
  }
  if (millis() > timeSendRegister + (unsigned long)(Registers[0]) * 60000) {
    Serial.println("send registers");
    timeSendRegister = millis();
    SendRegisters();
  }
  if ( bitRead(diagByte, diagTimeUpToDate) && millis() > timeTimeRequested + 60000) {
    Serial.println("request time");
    timeTimeRequested = millis();
    SendRequestTime();
  }
  else if (!bitRead(diagByte, diagTimeUpToDate) && millis() > lastUpdateClock + 60 * 60000) {
    bitWrite(diagByte, diagTimeUpToDate, 1);
  }
  if (bitRead(diagByte, diagJustReboot)) {
    if (millis() > 2 * 60000) {
      bitWrite(diagByte, diagJustReboot, 0);
    }
  }
}

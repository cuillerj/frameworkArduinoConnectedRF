

#define Version "RF_frameworkV"
#define ver 1 // version 
#define debugOn
#define forceInitEeprom false
#include <HomeAutomationBytesCommands.h> // commands specifications

#include <EEPROM.h>
#include <RF433Link.h>
#define addrEepromStation 0   // address of station address
#define addrEepromGateway 1  // address of gateway address
#define addrEepromSenderPin 2
#define addrEepromReveiverPin 3
uint8_t senderPin = 3;
uint8_t reveiverPin = 5;
#define speedLink 2000
#define nbRetry 4
uint8_t stationAddress = 0x01;
uint8_t gatewayAddress = 0xfb;
#define diagTimeUpToDate 2
uint8_t diagByte = 0b000000100;
byte data[20];
uint8_t *Pdata = &data[0]; // pointer to the data zone
#define configPIN MOSI     // when high enter in configuration mode- MISO PIN can easely be set with a switch on ISCP connector
//RF433Link rfLink(stationAddress, gatewayAddress, senderPin, reveiverPin, speedLink,nbRetry);
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
#define TimeMode
//#define RTCMode

#ifdef RTCMode
#include <RTClib.h>
RTC_DS1307 RTC;
#define TimeOn
#endif

#ifdef TimeMode
#include <TimeLib.h>
#define TimeOn
#endif
//#if defined TimeOn
#define MonthList "JanFebMarAprMayJunJulAugSepOctNovDec"  // do not change can not be localized
//#endif

void setup() {
  Serial.begin(38400);            // use serial for debug only with Arduino that has multiple serial interfaces
  delay(1000);
  Serial.print("start:");
  Serial.print(Version);
  Serial.println(ver);
  pinMode(configPIN, INPUT_PULLUP);
  if (!digitalRead(configPIN)||forceInitEeprom) {
    UpdateEeprom();
  }
  stationAddress = EEPROM.read(addrEepromStation);
  gatewayAddress = EEPROM.read(addrEepromGateway);
  senderPin = EEPROM.read(addrEepromSenderPin);
  reveiverPin = EEPROM.read(addrEepromReveiverPin);
  Serial.print("gateway:");
  Serial.print(gatewayAddress, HEX);
  Serial.print(" RF_sub address:");
  Serial.print(stationAddress, HEX);
  Serial.print(" station:");
  Serial.println(GetUnsignedValue(gatewayAddress, stationAddress));
  rfLink.SetParameters(stationAddress, gatewayAddress, senderPin, reveiverPin, nbRetry);
  rfLink.Start();
  randomSeed(analogRead(0));
}

void loop() {
  /*

  */
  delay(100);
  //Pdata=&data[0];

  rfLink.RetrySend();
  ReceiveRF();

  if (millis() > lastStatusSentTime +  int(Registers[0]) * 60000) {
    Serial.println("send status");
    lastStatusSentTime = millis();
    data[0] = 0x00; // reserved for futur usage
    data[1] = statusResponse;
    data[2] = diagByte;
    data[3] = ver;
    int minuteSinceReboot = millis() / 60000;
    data[4] = uint8_t(minuteSinceReboot / 256);
    data[5] = uint8_t(minuteSinceReboot);
    rfLink.SendData(Pdata, 5);
  }
  if (millis() > lastIndicatorsSentTime + int(Registers[0]) * 60000) {
    Serial.print("send indicators value:0x");
    lastIndicatorsSentTime = millis();
    data[0] = 0x00; // reserved for futur usage
    data[1] = indicatorsRequest;
    data[2] = value0;
    int value1 = analogRead(A0);
    data[3] = uint8_t(value1 / 256);
    data[4] = uint8_t(value1);
    data[5] = highByte(updatableIndicatorsValues[0]);
    data[6] = lowByte(updatableIndicatorsValues[0]);
    Serial.print(data[5], HEX);
    Serial.print("-");
    Serial.println(data[6], HEX);
    rfLink.SendData(Pdata, 6);
    value0++;
  }
  if (millis() > lastToSheetSentTime + int(Registers[0]) * 60000) {
    Serial.println("send to sheet");
    lastToSheetSentTime = millis();
#define nbValues 2
    int values[nbValues];
    values[0] = value0;
    values[1] = analogRead(A0);
    SendToGoogleSheet(nbValues, values);
  }
  if (millis() > timeSendDatabase + int(Registers[0]) * 60000) {
    Serial.println("send to database");
    timeSendDatabase = millis();

#define nbValues 2
    int values[nbValues];
    values[0] = value0;
    values[1] = analogRead(A0);
    SendToDatabase(nbValues, 0, values);
  }
  if (millis() > timeSendRegister + int(Registers[0]) * 60000) {
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
}

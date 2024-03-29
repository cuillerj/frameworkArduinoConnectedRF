void UpdateEeprom() {
  Serial.print("update addresses in eeprom station-gateway from:");
  Serial.print(EEPROM.read(addrEepromStation));
  Serial.print("-");
  Serial.print(EEPROM.read(addrEepromGateway));
  Serial.print("-");
  Serial.print(EEPROM.read(addrEepromSenderPin));
  Serial.print("-");
  Serial.print(EEPROM.read(addrEepromreceiverPin));
  Serial.print(" to: ");
  Serial.print(stationAddress);
  Serial.print("-");
  Serial.print(gatewayAddress);
  Serial.print(" senderPin:");
  Serial.print(senderPin);
  Serial.print(" receiverrPin:");
  Serial.print(receiverPin);
  Serial.println(" in 30 secondes-");
  delay(30000);
  EEPROM.update(addrEepromStation, stationAddress);
  delay(100);
  EEPROM.update(addrEepromGateway, gatewayAddress);
  delay(100);
  EEPROM.update(addrEepromSenderPin, senderPin);
  delay(100);
  EEPROM.update(addrEepromreceiverPin, receiverPin);
  Serial.println(" remove configPIN or set forceInitEeprom to false and restart");
  while(true);
}

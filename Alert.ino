void SendAlert(boolean AlertOn, boolean level1, boolean updated, byte categoryEvent) {
  Serial.print("SendAlert On:");
  Serial.print(AlertOn);
  Serial.print("-");
  Serial.print(level1);
  Serial.print(" cat");
  Serial.print(categoryEvent);
  Serial.print(" number:");
  if (level1) {
    Serial.println(alertIdentifierLevel1);
  }
  else {
    Serial.println(alertIdentifierLevel2);
  }

  uint8_t messType = 0x03;
  if (AlertOn)
  {
    messType = 0x01;
  }
  data[0] = 0x00; // reserved for futur usage
  data[1] = alertRequest;
  data[2] = 0x01; //
  // ******************************************************************************
  //  ajouter ci-dessous les datas specifiques et modifier datalen
  if (level1) {
    data[3] = alertIdentifierLevel1 / 256; // identifier
    data[4] = alertIdentifierLevel1; // identifier
  }
  else {
    data[3] = alertIdentifierLevel2 / 256; // identifier
    data[4] = alertIdentifierLevel2; // identifier
  }

  data[5] = 0x01; // status actual
  if (updated) {
    data[6] = 0x02; // msgType update
  }
  else {
    data[6] = messType; // msgType alert
  }

  data[7] = categoryEvent; // category event
  if (messType == 0x03) {
    data[8] = 0x04; // urgency immediate
    data[9] = 0x04; // severity severe
    data[10] = 0x05; // responseType monitor
  }
  else {
    data[8] = 0x02; // urgency immediate
    data[9] = 0x04; // severity severe
    data[10] = 0x06; // responseType asses
  }
  rfLink.SendData(Pdata, 10);
  lastStatusSentTime = millis() - Registers[0] * 60000 + 2000;
  timeSendDatabase = timeSendDatabase - 2000;
  lastStatusSentTime = lastStatusSentTime - 2000;
  countAlertSent++;
  // pendingNumber=pendingNumber+1;
  // ******************************************* conserver tel quel le code entre les ********************
}

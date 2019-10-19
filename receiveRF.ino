void ReceiveRF() {
  if (rfLink.HaveAMessge()) {
    receivedData rc = rfLink.Receive();
    uint8_t *Pdata = rc.data;
    uint8_t  receiveLen = rc.dataLen;
    for (int i = 0; i < receiveLen; i++) {
      Serial.print(Pdata[i], HEX);
      Serial.print(".");
    }
    Serial.println();
    unsigned indicatorId = Pdata[1] * 256 + Pdata[2];
    int value = Pdata[4] * 256 + Pdata[5];
    if (Pdata[4] == 0x2d) {
      value = -value;
    }
    switch (Pdata[0]) {
      case indicatorsRequest:
        Serial.print("update indicator");
        for (int i = 0; i < updatableIndicatorsNumber; i++) {
          if (indicatorId == updatableIndicatorsIndex[i])
          {
            updatableIndicatorsValues[i] = value;
            Serial.print(" 18:");
            Serial.print(value);
            break;
          }
          Serial.println();
        }
        break;
      case timeUpdateResponse: {
          /*
            convert byte input to char date and time to setup clock
            input[0] id j number, [1] month number, [2] year number minus 2000, [3] hour number, [4]minute number, [5] second number
          */
#define firstDataBytePosition 1
          Serial.print ("time response ");
          Serial.println(Pdata[firstDataBytePosition + 1]);

          char DateToInit[15] = "xxx xx 20xx";
          char TimeToInit[9] = "xx: xx: xx";
          lastUpdateClock = millis();

          if (Pdata[1] > 0 && Pdata[firstDataBytePosition + 1] < 13)
          {
            DateToInit[0] = MonthList[3 * (Pdata[firstDataBytePosition + 1] - 1)];
            DateToInit[1] = MonthList[3 * (Pdata[firstDataBytePosition + 1] - 1) + 1];
            DateToInit[2] = MonthList[3 * (Pdata[firstDataBytePosition + 1] - 1) + 2];
            DateToInit[4] = uint8_t(Pdata[firstDataBytePosition ] / 10 + 48);
            DateToInit[5] = uint8_t((Pdata[firstDataBytePosition ] - (Pdata[firstDataBytePosition ] / 10) * 10) + 48);
            DateToInit[9] = uint8_t(Pdata[firstDataBytePosition + 3] / 10 + 48); //
            DateToInit[10] = uint8_t((Pdata[firstDataBytePosition + 3] - (Pdata[firstDataBytePosition + 3] / 10) * 10) + 48);
            TimeToInit[0] = uint8_t(Pdata[firstDataBytePosition + 4] / 10 + 48);
            TimeToInit[1] = uint8_t((Pdata[firstDataBytePosition + 4] - (Pdata[firstDataBytePosition + 4] / 10) * 10) + 48);
            TimeToInit[3] = uint8_t(Pdata[firstDataBytePosition + 6] / 10 + 48);
            TimeToInit[4] = uint8_t((Pdata[firstDataBytePosition + 6] - (Pdata[firstDataBytePosition + 6] / 10) * 10) + 48);
            TimeToInit[6] = uint8_t(Pdata[firstDataBytePosition + 7] / 10 + 48);
            TimeToInit[7] = uint8_t((Pdata[firstDataBytePosition + 7] - (Pdata[firstDataBytePosition + 7] / 10) * 10) + 48);
            bitWrite(diagByte, diagTimeUpToDate, 0);
            lastUpdateClock = millis();
            Serial.println("setting time");

#if defined RTCMode
            RTC.adjust(DateTime(DateToInit, TimeToInit));
#endif
#if defined TimeMode
            int hh = Pdata[firstDataBytePosition + 4];
            int mn = Pdata[firstDataBytePosition + 6];
            int ss = Pdata[firstDataBytePosition + 7];
            int jj = Pdata[firstDataBytePosition];
            int mm = Pdata[firstDataBytePosition + 1] ;
            int aa = 2000 + Pdata[firstDataBytePosition + 3];
            setTime(hh, mn, ss, jj, mm,  aa);
#endif

            Serial.print("set time ");
            AffTime();

          }
          break;
        }
      case updateRegistersRequest: {
#if defined(debugOn)
          Serial.println("updateRegisterRequest");

#endif
          for (int i = 0; i < floor(receiveLen - firstDataBytePosition ) / 2; i++) {
            if (Pdata[firstDataBytePosition + 2 * i] >= 0 && Pdata[firstDataBytePosition + 2 * i] < registerSize)
            {
              Registers[Pdata[firstDataBytePosition + 2 * i]] = Pdata[firstDataBytePosition + 2 * i + 1];
#if defined(debugOn)
              Serial.print("id ");
              Serial.print(i);
              Serial.print(" value: 0x");
              Serial.println(Registers[i], HEX);
#endif
            }
          }

          timeSendRegister = millis() - 1000;
          //    SendRegisters();
          break;
        }
      default: {
          Serial.print("unknown command:");
          Serial.println(Pdata[0]);
          break;
        }
    }
  }
}

void SendToGoogleSheet(uint8_t number, int values[])
{
  uint8_t idx = 1;
  number = min(number, (maxDataLen - 2) / 2);
  data[0] = 0x00; // reserved for futur usage
  data[1] = insertDataInSheetRequest;
  for (int j = 0; j < number; j++)
  {
    byte *PretByte = FormatIntBytes(insertDataInSheetRequest, values[j]); // format data according to the command type
    for (int i = 1; i <= *PretByte; i++) {
      data[i + idx] = *(PretByte + i);  //  data tot the frame
    }
    Serial.print("sh len:");
    Serial.print(*PretByte);
    idx = idx + *PretByte;
    Serial.print("-");
    Serial.println(idx);
  }
  rfLink.SendData(Pdata, idx);
}
void SendToDatabase(uint8_t number, uint8_t type, int values[])
{
  uint8_t idx = 2;
  number = min(number, (maxDataLen - 2) / 2);
  data[0] = 0x00; // reserved for futur usage
  data[1] = insertDataInDatabaseRequest;
  data[2] = type; // type of measurment
  for (int j = 0; j < number; j++)
  {
    byte *PretByte = FormatIntBytes(insertDataInDatabaseRequest, values[j]); // format data according to the command type
    for (int i = 1; i <= *PretByte; i++) {
      data[i + idx] = *(PretByte + i);  //  data to the frame
    }
    Serial.print("db len:");
    Serial.print(*PretByte);
    idx = idx + *PretByte;
    Serial.print("-");
    Serial.println(idx);
  }
  rfLink.SendData(Pdata, idx);
}
void SendRegisters()
{
  data[0] = 0x00; // reserved for futur usage
  data[1] = registersResponse;
  uint8_t framLen = 2;
  for (int i = 0; i < registerSize; i++) {
    data[2 + 2 * i] = uint8_t(i);  // register number
    data[2 + 2 * i] = Registers[i]; // register value
    framLen = framLen + 2;
    if (framLen > maxDataLen - 2) {       // test not over maximal frem size
      break;
    }
  }
  rfLink.SendData(Pdata, framLen);
}
void SendRequestTime()
{
  data[0] = 0x00; // reserved for futur usage
  data[1] = timeUpdateRequest;
  uint8_t framLen = 2;
  rfLink.SendData(Pdata, framLen);
}
byte * FormatIntBytes(uint8_t type, int value)
{
#define retLen 4
  static uint8_t retByte[retLen];
  switch (type)
  {
    case indicatorsRequest:
      {
        {
          /*
             in this case data format is a sequence of  0xcdef signed 2 bytes int
          */
          retByte[0] = retLen - 2;
          if (value < 0)
          {
            value = -value;
            retByte[1] = uint8_t(((value & 0x7f00) >> 8) | 0x80); // add negative bit
          }
          else
          {
            retByte[1] = uint8_t(value  >> 8); // add negative bit
          }
          retByte[2] = uint8_t(value & 0x00ff );
          break;
        }
      }
    case insertDataInDatabaseRequest:
    case insertDataInSheetRequest:
      {
        /*
           in this case data format is a sequence of 0xab (sign character of the value) 0xcdef (non signed 2 bytes int)
        */
        retByte[0] = retLen - 1;
        if (value < 0)
        {
          retByte[1] = 0x2d;
        }
        else
        {
          retByte[1] = 0x2b;
        }
        retByte[2] = uint8_t((value & 0xff00) >> 8);
        retByte[3] = uint8_t(value & 0x00ff );
        break;
      }
  }
  return retByte;
}

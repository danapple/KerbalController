

struct HandShakePacket
{
  byte id;
  byte M1;
  byte M2;
  byte M3;
};

//struct LogPacket
//{
//  byte id;
//  char message[200];
//};

//LogPacket LPacket;

HandShakePacket HPacket;

void Handshake(){
  
  HPacket.id = HSPid;
  HPacket.M1 = 3;
  HPacket.M2 = 1;
  HPacket.M3 = 4;
  KSPBoardSendData(details(HPacket));
  
}

void InitTxPackets() {
  
  CPacket.id = Cid;
  CPacket.seq = 0;
  
  //LPacket.id = 200;
}


//String toString(int n)
//{
//  char numstr[21];
//  sprintf(numstr, "%d", n);
//  return numstr;
// 
//}

//String toHex(int n)
//{
//  char numstr[21];
//  sprintf(numstr, "0x%X", n);
//  return numstr;
 
//}

uint8_t rx_len = 0;
uint8_t rx_array_inx = 0;  //index for RX parsing buffer
byte buffer[256]; //address for temporary storage and parsing buffer

boolean gotBE = false;
boolean gotEF = false;

int KSPBoardReceiveData() {
  int id = -1;
  while (rx_len == 0 && Serial.available() > 0) 
  {
    uint8_t readByte = Serial.read();
    //HostLog("header readByte = " + toHex(readByte));
    if (!gotBE)
    {
      gotBE = readByte == 0xBE;
    }
    else if (!gotEF)
    {
      gotEF = readByte == 0xEF;
      gotBE &= gotEF;
    }
    else
    {
      rx_len = readByte;
      rx_array_inx = 0;
      gotBE = false;
      gotEF = false;
    }
  }

  if (rx_len != 0)
  {
    while (Serial.available() > 0 && rx_array_inx < rx_len)
    {
      uint8_t readByte = Serial.read();
      //HostLog("payload readByte = " + toHex(readByte));
      buffer[rx_array_inx++] = readByte;
    }

    if (Serial.available() > 0 && rx_array_inx == rx_len)
    {
      uint8_t in_CS = Serial.read();
      
      uint8_t calc_CS = rx_len;
      for (int i = 0; i < rx_len; i++){
        calc_CS ^= buffer[i];
      } 
      //HostLog("calc_CS = " + toString(calc_CS) + ", in_CS = " + toString(in_CS));
      
      if (calc_CS == in_CS)
      {
        uint8_t * address;
        int structSize = 0;
      
        id = buffer[0];
        switch(id) {
          case HSPid:
            structSize = sizeof(HPacket);   
            address = (uint8_t*)&HPacket;     
            break;
          case VDid:
            structSize = sizeof(VData);   
            address = (uint8_t*)&VData;   
            break;
          default:
            id = -1;  
        }

        if (id > -1 && structSize == rx_len && structSize > 0 && structSize <= 256) 
        {
          memcpy(address, buffer, structSize);
        }
        else
        {
          id = -1;
        }
      }
      rx_len = 0;
      rx_array_inx = 0;
    }
  }

  return id;
}
/*
void HostLog (String myText)
{
  strncpy(LPacket.message, myText.c_str(), 200);
  LPacket.id = 200;
  KSPBoardSendData(details(LPacket));
}
*/
void KSPBoardSendData(uint8_t * address, uint8_t len){
  if (writeByte(0xBE) < 1)
  {
    return;
  }
  if (writeByte(0xEF) < 1)
  {
    return;
  }
  if (writeByte(len) < 1)
  {
    return;
  }
  uint8_t CS = len;

  for(int i = 0; i < len; i++){
    byte b = *(address+i);
    CS ^= b;
    if (writeByte(b) < 1)
    {
      return;
    }
  }

  writeByte(CS);
  
}

byte writeByte(byte b)
{
  int attempt = 0;
  while (attempt++ < 200 && Serial.availableForWrite() < 1)
  {
    delay(1);
  }
  if (attempt >= 200)
  {
    return 0;  
  }
  Serial.write(b);
  return 1;
}

void controlsInit() {
  pinMode(pTHROTTLE, INPUT);
  pinMode(pTX, INPUT);
  pinMode(pTY, INPUT);
  pinMode(pTZ, INPUT);
  pinMode(pRX, INPUT);
  pinMode(pRY, INPUT);
  pinMode(pRZ, INPUT);
  pinMode(pPOWER, INPUT_PULLUP);
  pinMode(pTB, INPUT_PULLUP);
  pinMode(pRB, INPUT_PULLUP);
  pinMode(pMODE, INPUT_PULLUP);
  pinMode(pLCDx, INPUT_PULLUP);
  pinMode(pLCDy, INPUT_PULLUP);
  pinMode(pLCDz, INPUT_PULLUP);
  pinMode(pSAS, INPUT_PULLUP);
  pinMode(pRCS, INPUT_PULLUP);
  pinMode(pABORTARM, INPUT_PULLUP);
  pinMode(pABORT, INPUT);
  pinMode(pARM, INPUT_PULLUP);
  pinMode(pSTAGE, INPUT_PULLUP);
  pinMode(pSTAGELED, OUTPUT);
  pinMode(pABORTLED, OUTPUT);
  pinMode(pLIGHTS, INPUT_PULLUP);
  pinMode(pLIGHTSLED, OUTPUT);
  pinMode(pLADDER, INPUT_PULLUP);
  pinMode(pLADDERLED, OUTPUT);
  pinMode(pSOLAR, INPUT_PULLUP);
  pinMode(pSOLARLED, OUTPUT);
  pinMode(pCHUTES, INPUT_PULLUP);
  pinMode(pCHUTESLED, OUTPUT);
  pinMode(pGEARS, INPUT_PULLUP);
  pinMode(pGEARSLED, OUTPUT);
  pinMode(pGEARDOWNDISAGREE, OUTPUT);
  pinMode(pGEARUPDISAGREE, OUTPUT);
  pinMode(pBRAKES, INPUT_PULLUP);
  pinMode(pBRAKESLED, OUTPUT);
  pinMode(pACTION1, INPUT_PULLUP);
  pinMode(pACTION1LED, OUTPUT);
  pinMode(pACTION2, INPUT_PULLUP);
  pinMode(pACTION2LED, OUTPUT);
  pinMode(pACTION3, INPUT_PULLUP);
  pinMode(pACTION3LED, OUTPUT);
  pinMode(pACTION4, INPUT_PULLUP);
  pinMode(pACTION4LED, OUTPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(pendingPacketLEDPin, OUTPUT);

}

void testLED(int pin, int testDelay)
{
  digitalWrite(pin, HIGH);
  delay(testDelay);
  digitalWrite(pin, LOW);
}

void testLEDS(int testDelay){
  testLED(pendingPacketLEDPin, testDelay);
  testLED(pABORTLED, testDelay);
  testLED(pSTAGELED, testDelay);
  testLED(pLIGHTSLED, testDelay);
  testLED(pLADDERLED, testDelay);
  testLED(pSOLARLED, testDelay);
  testLED(pCHUTESLED, testDelay);
  testLED(pGEARSLED, testDelay);
  testLED(pBRAKESLED, testDelay);
  testLED(pACTION1LED, testDelay);
  testLED(pACTION2LED, testDelay);
  testLED(pACTION3LED, testDelay);
  testLED(pACTION4LED, testDelay);
  testLED(pGEARDOWNDISAGREE, testDelay);
  testLED(pGEARUPDISAGREE, testDelay);

  if (HAVE_BARS)
  {
    //prepare the shift register
    digitalWrite(dataPin, LOW);
    digitalWrite(clockPin, LOW);
    digitalWrite(latchPin, LOW);
  
    inputBytes[0] = B11111111;
    inputBytes[1] = B11111111;
    inputBytes[2] = B11111111;
    inputBytes[3] = B11111111;
    inputBytes[4] = B11111111;
    inputBytes[5] = B11111111;
    inputBytes[6] = B11111111;
    //loop through the input bytes
    for (int j=0; j<=6; j++){
      byte inputByte = inputBytes[j];
      //Serial.println(inputByte);
      shiftOut(dataPin, clockPin, MSBFIRST, inputByte);
    }
    
    //latch the values in when done shifting
    digitalWrite(latchPin, HIGH); 
    
    delay(testDelay);
    
    //prepare the shift register
    digitalWrite(dataPin, LOW);
    digitalWrite(clockPin, LOW);
    digitalWrite(latchPin, LOW);
  
    inputBytes[0] = B00000000;
    inputBytes[1] = B00000000;
    inputBytes[2] = B00000000;
    inputBytes[3] = B00000000;
    inputBytes[4] = B00000000;
    inputBytes[5] = B00000000;
    inputBytes[6] = B00000000;
    //loop through the input bytes
    for (int j=0; j<=6; j++){
      byte inputByte = inputBytes[j];
      //Serial.println(inputByte);
      shiftOut(dataPin, clockPin, MSBFIRST, inputByte);
    }
    
    //latch the values in when done shifting
    digitalWrite(latchPin, HIGH); 
  }
}

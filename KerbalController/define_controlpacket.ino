//Define what controls to send to KSP

//check if it is time to send a control packet
void send_control_packet() {
  unsigned long now = millis();
  controlTime = now - controlTimeOld;
  if (controlTime > CONTROLREFRESH) {
    controlTimeOld = now;
    define_control_packet();
  }
}

unsigned long sentRequest = 0;
unsigned long stageLedTimeOld = 0;
unsigned long abortLedTimeOld = 0;

//define the structure of a control packet
struct ControlPacket {
  //the following controls can be sent to the KSPSerialIO plugin (defined by the plugin)
  byte id;
  byte seq;
  byte MainControls;                  //SAS RCS Lights Gears Brakes Precision Abort Stage (see enum)
  byte Mode;                          //0 = stage, 1 = docking, 2 = map
  unsigned int ControlGroup;          //action groups 1-10
  byte NavballSASMode;                //AutoPilot mode
  byte AdditionalControlByte1;
  int Pitch;                          //-1000 -> 1000
  int Roll;                           //-1000 -> 1000
  int Yaw;                            //-1000 -> 1000
  int TX;                             //-1000 -> 1000
  int TY;                             //-1000 -> 1000
  int TZ;                             //-1000 -> 1000
  int WheelSteer;                     //-1000 -> 1000
  int Throttle;                       //    0 -> 1000
  int WheelThrottle;                  //    0 -> 1000
};

//Create an instance of a control packet
ControlPacket CPacket;

//macro used to generate the control packet (also used for the handshake packet)
#define details(name) (uint8_t*)&name,sizeof(name)

//Enumeration of MainControls
#define SAS       7
#define RCS       6
#define LIGHTS    5
#define GEARS     4
#define BRAKES    3
#define PRECISION 2
#define ABORT     1
#define STAGE     0

// Enumberation of ControlGroups
#define LADDER    5
#define SOLAR     6
#define CHUTES    7
#define ACTION1   1
#define ACTION2   2
#define ACTION3   3
#define ACTION4   4

boolean sendCommand = false;
boolean fixStates = false;
unsigned long lastStateFix = 0;
//Main controls uses enum above, e.g. MainControls(SAS,true);

//Enumeration of SAS Modes
#define SMOFF           0
#define SMSAS           1
#define SMPrograde      2
#define SMRetroGrade    3
#define SMNormal        4
#define SMAntinormal    5
#define SMRadialIn      6
#define SMRadialOut     7
#define SMTarget        8
#define SMAntiTarget    9
#define SMManeuverNode  10

//SAS mode uses enum above, e.g. setSASMode(SMPrograde);
void setSASMode(byte m) {
  CPacket.NavballSASMode &= B11110000;
  CPacket.NavballSASMode += m;
}

//Enumeration of Navball Target Modes
#define NAVBallIGNORE   0
#define NAVBallORBIT    1
#define NAVBallSURFACE  2
#define NAVBallTARGET   3

//Navball mode uses enum above, e.g. setNavBallMode(NAVBallSURFACE);
void setNavballMode(byte m) {
  CPacket.NavballSASMode &= B00001111;
  CPacket.NavballSASMode += m << 4;
}

int PIN_STATE[60];

void initPinStates()
{
  for (int pin = 2; pin < 60; pin++) {
    PIN_STATE[pin] = readPinState(pin);
  }
}

void recordPin(int pin, boolean newPinState) 
{
   PIN_STATE[pin] = newPinState;
}

void MainControls(byte n, boolean s) {
    if (s)
      CPacket.MainControls |= (1 << n);       // forces nth bit of x to be 1.  all other bits left alone.
    else
      CPacket.MainControls &= ~(1 << n);      // forces nth bit of x to be 0.  all other bits left alone.
    sendCommand = true;
}

void ClearMainControls() 
{
  ClearMainControls(STAGE);
}

void ClearMainControls(int n) {
  CPacket.MainControls &= ~(1 << n);
}

//Control groups (action groups) uses an integer to refer to a custom action group, e.g. ControlGroup(5,true);
void ControlGroups(byte n, boolean s) {

  if (s)
    CPacket.ControlGroup |= (1 << n);       // forces nth bit of x to be 1.  all other bits left alone.
  else
    CPacket.ControlGroup &= ~(1 << n);      // forces nth bit of x to be 0.  all other bits left alone.
  sendCommand = true;
}

// For buttons that toggle a state
boolean positiveEdge(int readPin) 
{
  boolean oldPinState = PIN_STATE[readPin];
  boolean newPinState = readPinState(readPin);
  return oldPinState == false && newPinState == true;
}

boolean negativeEdge(int readPin) 
{
  boolean oldPinState = PIN_STATE[readPin];
  boolean newPinState = readPinState(readPin);
  return oldPinState == true && newPinState == false;
}

boolean readPinState(int readPin) {
  return !digitalRead(readPin);
}


boolean buttonPressLocal(int readPin, boolean state) 
{
  boolean pEdge = positiveEdge(readPin);
  recordPin(readPin, readPinState(readPin));
  if (pEdge)
  {
    return !state;
  }
  else {
    return state;
  }
}

// For buttons that toggle a state
void buttonPress(int readPin, int writePin, boolean state) 
{
  boolean pEdge = positiveEdge(readPin);
  recordPin(readPin, readPinState(readPin));
  if (pEdge) {
    MainControls(writePin, !state);
  }
}


// For buttons that cause an action
void buttonPress(int readPin, int writePin) 
{
  boolean pEdge = positiveEdge(readPin);
  recordPin(readPin, readPinState(readPin));
  if (pEdge) {
    MainControls(writePin, true);
  }
}

// For switches that induce a state upon being thrown
void triggerMainState(int readPin, int writePin, boolean state, boolean fix) 
{
  boolean pinState = readPinState(readPin);
  if (!state && positiveEdge(readPin)) 
  {
    MainControls(writePin, true);
  }
  else if (state && negativeEdge(readPin)) 
  {
    MainControls(writePin, false);
  }
  else if (fix && fixStates && state != pinState)
  {
    MainControls(writePin, pinState);
  }
  recordPin(readPin, pinState);
  
}

boolean matchControlState(int readPin, int writePin, boolean state) 
{
  boolean pinState = readPinState(readPin);
  if (pinState != state) 
  {
    ControlGroups(writePin, pinState);
  }
}

int readAnalog(int pin, int negStart, int negDead, int posDead, int posLimit)
{
  int readValue = analogRead(pin);
  int returnValue = 0;
  if (readValue >= 530) 
  {
    returnValue = constrain(map(readValue, negStart, negDead, -1000, 0), -1000, 0);
  }
  else if(readValue <= 470)
  {
    returnValue = constrain(map(readValue, posDead, posLimit, 0, 1000), 0, 1000);
  }
  return returnValue;
}

int readXY(int pin)
{
  return readAnalog(pin, 1023, 530, 470, 0);
}

int readZ(int pin)
{
  return readAnalog(pin, 0, 40, 60, 500);
}

void define_control_packet() {
  unsigned long now = millis();
  if (Connected)
  {
    if (lastStateFix == 0)
    {
      lastStateFix = now;
    }
    fixStates = (now - lastStateFix)  > 500;
    if (fixStates) 
    {
      lastStateFix = now;
    }
    if (CPacket.seq == ackseq)
    {
      // clear non-stateful actions once the host acknowledges the command
      ClearMainControls();
    }
    sendCommand = false;

    triggerMainState(pGEARS, GEARS, gears_on, false);
    buttonPress(pSAS, SAS, sas_on);
    buttonPress(pLIGHTS, LIGHTS, lights_on);
    
    triggerMainState(pRCS, RCS, rcs_on, true);

    matchControlState(pLADDER, LADDER, ladder_on);
    matchControlState(pSOLAR, SOLAR, solar_on);
    matchControlState(pCHUTES, CHUTES, chutes_on);

    matchControlState(pACTION1, ACTION1, action1_on);
    matchControlState(pACTION2, ACTION2, action2_on);
    matchControlState(pACTION3, ACTION3, action3_on);
    matchControlState(pACTION4, ACTION4, action4_on);
 
    boolean stageArmed = readPinState(pARM);
    if (stageArmed) 
    {
      buttonPress(pSTAGE, STAGE);
      
      int stageLedTime = now - stageLedTimeOld;
      if (stageLedTime > stageLedBlinkTime) {
        stageLedTimeOld = now;
        stage_led_on = !stage_led_on;
      }
    }
    else 
    {
      stageLedTimeOld = 0;
      stage_led_on = false;
    }

    boolean abortArmed = readPinState(pABORTARM);

    if (abortArmed) 
    {
      if (abort_on)
      {
        abort_led_on = true;
      }
      else
      {
        buttonPress(pABORT, ABORT);
        
        int abortLedTime = now - abortLedTimeOld;
        if (abortLedTime > abortLedBlinkTime) {
          abortLedTimeOld = now;
          abort_led_on = !abort_led_on;
        }
      }
    }
    else if (negativeEdge(pABORTARM))
    {
      abortLedTimeOld = 0;
      abort_led_on = false;
      MainControls(ABORT, false);
    }
    recordPin(pABORTARM, abortArmed);

    if (HAVE_THROTTLE) 
    {
      CPacket.Throttle = constrain(map(analogRead(pTHROTTLE),30,990,0,1023),0,1000);
    }
    if (HAVE_JOYSTICKS) 
    {
      rocket_mode = buttonPressLocal(pRB, rocket_mode);
      //rotation joystick button toggles flight control modes
      //if(!digitalRead(pRB) && !rb_prev){rb_on = !rb_on; rb_prev = true;}
      //if(digitalRead(pRB) && rb_prev){rb_prev = false;}
      
      if(rocket_mode){
        //rocket mode
        CPacket.Yaw = readXY(pRX);
        CPacket.Pitch = readXY(pRY);
        CPacket.Roll = readZ(pRZ);

        CPacket.TX = readXY(pTX);
        CPacket.TX = readXY(pTY);
        CPacket.TZ = readZ(pTZ);
      }
      else {
        //airplane mode
        CPacket.Roll = readXY(pRX);
        CPacket.Pitch = readXY(pRY);
        CPacket.Yaw = readXY(pTX);
        CPacket.TX = 0;
        CPacket.TY = 0;
        CPacket.TZ = 0;
      }
    }
 
    if (sendCommand) {
      sentRequest = 0;
      lastStateFix = now;
      CPacket.seq = (CPacket.seq + 1) % 200;
    }

    if (CPacket.seq != ackseq) 
    {
      if (now - sentRequest > 200) {
        pendingPacket = true;
        KSPBoardSendData(details(CPacket)); 
        sentRequest = now;
      }
    }
    else {
      pendingPacket = false;
    }

  }
}

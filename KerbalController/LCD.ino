void initLCD()
{
  if (HAVE_LCD == 0)
  {
    return;
  }
  mySerial.begin(9600); //LCD connection
  delay(500);           //wait for LCD boot
  
  //write to LCD
  clearLCD();
  writeLCD("KerbalController");
  jumpToLineTwo();
  writeLCD("booting...");
  delay(100);

}

void clearLCD ()
{
  if (HAVE_LCD == 0) 
  {
    return;
  }
  //clear the LCD by writing all spaces
  jumpToStart();
  mySerial.write("                ");
  mySerial.write("                ");
  jumpToStart();
}

void jumpToStart ()
{
  if (HAVE_LCD == 0) 
  {
    return;
  }
  //jump to the start of the first line on the LCD
  mySerial.write(254);
  mySerial.write(128);
}

void jumpToLineTwo ()
{
  if (HAVE_LCD == 0) 
  {
    return;
  }
  //jump to the start of the second line on the LCD
  mySerial.write(254);
  mySerial.write(192);
}

void writeLCD (char myText[])
{
  if (HAVE_LCD == 0) 
  {
    return;
  }
  //write text to the LCD
  mySerial.write(myText); 
}

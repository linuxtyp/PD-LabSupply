
/*
MIT License

Copyright (c) 2020 Ryan Ma

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include <Arduino.h>
#include <AP33772BETA.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>

 // On Trinket or Gemma, suggest changing this to 1

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS 1 // Popular NeoPixel ring size

#define maxCurrent 5000.00//mA
#define MinCurrent 50//mA
#define CurrentSteps 15//mA
#define maxVoltage 20000//mV
#define minVoltage 5000//mV
#define VoltageSteps 5//mV

/* old version
#define SwitchVA 14
#define SwitchOut 22
#define ButtonUp 2
#define ButtonDown 3
#define ADCHall A0
#define Pin5V 8
#define Pin9V 9
#define Pin12V 10
#define Pin15V 11
#define Pin20V 12
#define PinPSU 13
#define LED 0
*/

//new version
#define SwitchVA 21
#define SwitchOut 23
#define ButtonUp 20
#define ButtonSel 19
#define ButtonDown 18
#define ADCHall A2
#define Pin5V 6
#define Pin9V 7
#define Pin12V 8
#define Pin15V 9
#define Pin20V 10
#define PinPSU 11
#define LED 22




#define SDA 4
#define SCL 5


Adafruit_NeoPixel pixels(NUMPIXELS, LED, NEO_GRB + NEO_KHZ800);


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

AP33772 usbpd; // Automatically wire to Wire0,

bool state = 0;
int voltageSteps=VoltageSteps;
int voltages[]={5,9,12,15,20};
int maxVspec=0;
int minVspec=0;
int maxAspec=0;
bool ppsspec=true;

float MaxVoltage = maxVoltage;
float MaxCurrent = maxCurrent; 
float MinVoltage = minVoltage;

float adcHallOffset=0;
float adcHallOffsets[5];
bool overcurrent=true;//if plugged in while output onn still no current flow
bool cc = false;//current control








// 'logov1trans', 128x64px
const unsigned char bitmap [] PROGMEM = {
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x3f, 0xfe, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x0f, 0xfe, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x07, 0xfe, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x03, 0xfe, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x01, 0xfc, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x01, 0xfc, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xf0, 0x07, 0xf8, 0x00, 0xfc, 0x01, 0xff, 0x00, 0x1f, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xf0, 0x07, 0xfc, 0x00, 0xfc, 0x01, 0xff, 0x80, 0x1f, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xf0, 0x07, 0xfe, 0x00, 0xfc, 0x01, 0xff, 0x80, 0x1f, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xe0, 0x07, 0xfe, 0x00, 0xf8, 0x01, 0xff, 0x80, 0x1f, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xe0, 0x0f, 0xfe, 0x00, 0xf8, 0x01, 0xff, 0x80, 0x1f, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xe0, 0x0f, 0xfe, 0x00, 0xf8, 0x03, 0xff, 0x80, 0x1f, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xe0, 0x0f, 0xfe, 0x00, 0xf8, 0x03, 0xff, 0x80, 0x1f, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xe0, 0x0f, 0xfc, 0x00, 0xf8, 0x03, 0xff, 0x80, 0x1f, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xc0, 0x0f, 0xfc, 0x00, 0xf8, 0x03, 0xff, 0x80, 0x3f, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xc0, 0x1f, 0xf8, 0x01, 0xf0, 0x03, 0xff, 0x80, 0x3f, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xc0, 0x1f, 0xe0, 0x01, 0xf0, 0x07, 0xff, 0x00, 0x3f, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x03, 0xf0, 0x07, 0xff, 0x00, 0x3f, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x03, 0xf0, 0x07, 0xff, 0x00, 0x3f, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x07, 0xf0, 0x07, 0xff, 0x00, 0x7f, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 0x0f, 0xf0, 0x07, 0xff, 0x00, 0x7f, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 0x1f, 0xe0, 0x07, 0xff, 0x00, 0x7f, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 0x3f, 0xe0, 0x0f, 0xfe, 0x00, 0x7f, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 0xff, 0xe0, 0x0f, 0xfe, 0x00, 0x7f, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 0x03, 0xff, 0xe0, 0x0f, 0xfe, 0x00, 0x7f, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x7f, 0xff, 0xe0, 0x0f, 0xfc, 0x00, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0x00, 0x3f, 0xff, 0xff, 0xe0, 0x0f, 0xfc, 0x00, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0x00, 0x7f, 0xff, 0xff, 0xc0, 0x0f, 0xf8, 0x01, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0x00, 0x7f, 0xff, 0xff, 0xc0, 0x1f, 0xc0, 0x01, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0x00, 0x7f, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0x00, 0x7f, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xfe, 0x00, 0x7f, 0xff, 0xff, 0x80, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xfe, 0x00, 0x7f, 0xff, 0xff, 0x80, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xfe, 0x00, 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xfe, 0x00, 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xfe, 0x00, 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xfe, 0x00, 0xff, 0xff, 0xff, 0x80, 0x00, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xfb, 0xff, 0xfd, 0xff, 0x0f, 0xff, 0xff, 0xff, 0xe7, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xfb, 0xff, 0xf9, 0xfe, 0x07, 0xff, 0xff, 0xff, 0xe7, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xfb, 0xff, 0xf9, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xe7, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xfb, 0xfe, 0x39, 0x3c, 0xfe, 0xf7, 0xa7, 0xd9, 0xe6, 0xf7, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xf3, 0xf8, 0x38, 0x1c, 0x7e, 0x77, 0x03, 0x80, 0xe6, 0x73, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xf3, 0xf3, 0xb8, 0xce, 0x1e, 0x67, 0x19, 0x8c, 0xee, 0x67, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xf3, 0xf3, 0x39, 0xcf, 0x8e, 0xe7, 0x39, 0x9c, 0xcf, 0x6f, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xf3, 0xe7, 0x39, 0xdf, 0xce, 0xe7, 0x39, 0x9c, 0xcf, 0x0f, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xf3, 0xe6, 0x3b, 0x9f, 0xcc, 0xe7, 0x73, 0x9d, 0xcf, 0x1f, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xf3, 0xe2, 0x31, 0x19, 0x8c, 0xc7, 0x33, 0x99, 0xcf, 0x1f, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xf0, 0x30, 0x30, 0x3c, 0x1e, 0x07, 0x07, 0x83, 0xcf, 0xbf, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x7f, 0xbf, 0xff, 0x3f, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x7f, 0xbf, 0xff, 0x7f, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x7f, 0xbf, 0xfe, 0x7f, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};




const int bitmapallArray_LEN = 1;
const unsigned char* bitmapallArray[1] = {
  bitmap
};





void setup()
{
  Wire.setSDA(SDA);
  Wire.setSCL(SCL);
  Wire.begin();

  
  //usbpd.begin(); // Start pulling the PDOs from power supply
  //Reset to request PDO again
  //delay(2000);
  //usbpd.printPDO();


  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  //display.display();
  display.clearDisplay();
  display.drawBitmap(0,0,bitmap,128,64,WHITE);
  display.display();

  pixels.begin();
  pixels.clear(); 
  pixels.setPixelColor(0, pixels.Color(0, 0, 0));
  pixels.show();
  //delay(4000); // Pause for 2 seconds
  // Clear the buffer
  
  // Draw a single pixel in white
  //display.drawPixel(10, 10, WHITE);

  //pinMode(LED_BUILTIN, OUTPUT); // Built in LED
  // Dipswitch
  pinMode(Pin5V,INPUT_PULLUP);
  pinMode(Pin9V,INPUT_PULLUP);
  pinMode(Pin12V,INPUT_PULLUP);
  pinMode(Pin15V,INPUT_PULLUP);
  pinMode(Pin20V,INPUT_PULLUP);
  pinMode(PinPSU,INPUT_PULLUP);
  //Buttons
  pinMode(ButtonUp,INPUT_PULLUP);
  pinMode(ButtonDown,INPUT_PULLUP);
  pinMode(ButtonSel,INPUT_PULLUP);
  //Switch
  pinMode(SwitchVA,INPUT_PULLUP);
  pinMode(SwitchOut,INPUT_PULLUP);
  //Current
  pinMode(ADCHall,INPUT);
  
  //LED
  //pinMode(LED,OUTPUT);

  Serial.begin(115200);
  delay(1000); //Delay is need for the IC to obtain PDOs
  usbpd.begin(); // Start pulling the PDOs from power supply
  usbpd.off();
  //usbpd.printPDO();
  //Serial.println(usbpd.printPDO());
    
  //digitalWrite(LED_BUILTIN, HIGH);

  //delay(10000);



  delay(3500);
  printRatings();
  HallCalibrate();
  delay(5000);
}

void rgbLED(int r, int g, int b)
{
  pixels.clear();
  pixels.setPixelColor(0,pixels.Color(r,g,b));
  pixels.show();
}
void printRatings()
{
  
  float specs[4]={0,0,0,0};
  String specsS[4]={"","","",""};
  usbpd.printPDO(specs);
  for(int i = 0; i < 4; i++)
  {
    specsS[i]=String(specs[i]);
  }
  display.clearDisplay();
  display.setTextSize(1); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  if(specs[3]!=0)//PPS Support
  {
    ppsspec=true;
    maxVspec=specs[2];
    minVspec=specs[1];
    maxAspec=specs[3];

    //display.setCursor(0, 0);
    //display.println("Source PDO Num: " + specsS[0]);
    display.setCursor(0, 0);
    display.println("PPS Supported");
    display.setCursor(0, 10);
    display.println("Min Voltage: " + specsS[1]);
    display.setCursor(0, 20);
    display.println("Max Voltage: " + specsS[2]);
    display.setCursor(0, 30);
    display.println("Max Current: " + specsS[3]);
    MaxVoltage=int(maxVspec*1000);
    MinVoltage=int(minVspec*1000);
    MaxCurrent=int(maxAspec*1000);
    
  }else if (specs[0]!=0)//fixed Voltage
  {
    ppsspec=false;
    maxVspec=specs[1];
    maxAspec=specs[2];
    //display.setCursor(0, 0);
    //display.println("Source PDO Num: " + specsS[0]);
    display.setCursor(0, 0);
    display.println("No PPS Support");
    display.setCursor(0, 10);
    display.println("Voltage: " + specsS[1]);
    display.setCursor(0, 20);
    display.println("Max Current: " + specsS[2]);
    MaxCurrent=int(specsS[2].toFloat()*1000);
    MaxVoltage=float(maxVspec*1000);
  }else
  {
    ppsspec=false;
    //MaxVoltage=5000;
    //MinVoltage=5000;
    //MaxCurrent=5000;
    display.setCursor(0, 0);
    display.println("No PowerDelivery\nSupported");  
  }
  
  
  //Serial.println(usbpd.printPDO());
  display.display();
}
void info(int tv, int v, int ti ,float i, int t)
{
  if (tv <  21){
    tv=tv*1000;
  }
  String setV = String(tv);
  String voltage = String(v);
  String maxC = String(ti);
  String current = String(i);
  String temp = String(t);
  String power = "0";
  if(i>0.06)
  {
    //power = String((i/1000)*v);
    power = String((i)*(v/1000));
  }else{
    current="0";
  }
  String load = "inf";
  if(i>0.06)
  {
    load = String((v/1000)/(i));
  }

  //Serial.println("SetV: " + setV + " Voltage: " + voltage + " current: " + current + " Temp: " + temp);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  if(digitalRead(SwitchVA) || cc==true){
    display.drawFastHLine(0, 29, 53,SSD1306_WHITE);
  }else{
    display.drawFastHLine(0, 9, 36,SSD1306_WHITE);
  }
  display.setCursor(0, 0);
  display.println("Set V: "+ setV + "mV");
  display.setCursor(0, 10);
  display.println("Actual V: "+ voltage+"mV");
  display.setCursor(0, 20);
  display.println("Max Curr.: "+ maxC+"mA");
  display.setCursor(0, 30);
  display.println("Current: "+ current+"A");
  display.setCursor(0, 40);
  display.println("Power: "+ power+"W");
  display.setCursor(0, 50);
  display.println("Load: "+ load+" Ohm");
  display.setCursor(100, 0);
  display.println(temp+"C");
  display.setCursor(100,50);//Out On Off
  if (overcurrent==true){
    display.println("OC!");
  }
  else if (digitalRead(SwitchOut)==false){
    display.println("ON");
  }else{
    display.println("OFF");
  }
  display.setCursor(100, 40);
  if(cc==true){
    display.println("CC");
  }else{
    display.println("VC");
  }


  //display.setCursor(0, 40);
  //display.println(usbpd.printPDO());
  //Serial.println(usbpd.printPDO());
  
  
  display.display();

}
void HallCalibrate()
{
  //float temp=0;
  //for(int i = 0; i<100; i++)
  //{
  //  temp+=(3.3*analogRead(ADCHall))/1024;
  //  delay(1);
  //}
  //adcHallOffset=temp/100;
  usbpd.off();
  //display.clearDisplay();
  //display.setTextSize(1); // Draw 2X-scale text
  //display.setTextColor(SSD1306_WHITE);
  //display.setCursor(0, 10);
  //display.println("Calibrating...");
  //display.display();
  delay(400);
  for(int i = Pin5V; i < PinPSU; i++){
    usbpd.setVoltage(voltages[i-Pin5V]*1000);
    float temp=0;
    for (int j = 0; j<100; j++){
      temp+=(3.3*analogRead(ADCHall))/1024;
      delay(1);
    }
    adcHallOffsets[i-Pin5V]=temp/100;
    Serial.println(adcHallOffsets[i-Pin5V]);
    //display.clearDisplay();
    //display.setTextSize(1); // Draw 2X-scale text
    //display.setTextColor(SSD1306_WHITE);
    //display.setCursor(0, (i-Pin5V)*10);
    //display.println(adcHallOffsets[i-Pin5V]);
    //display.display();
  }
}
float readCurrentController()
{
  float temp=0;
  for(int i = 0; i < 20; i++)
  {
    temp+=usbpd.readCurrent();
  }
  return temp/(20000);
}
float readCurrent(int voltage)
{
  float vAverage=0;
  float temp=0;
  for(int i = 0; i < 20; i++)
  {
      int curr=analogRead(ADCHall);
      //temp=curr;
      //Serial.println(curr);
      float v=(3.3*curr)/1024;
      //Serial.println("Real: " + String(v));
      temp=v;
      //v=v-adcHallOffset;//v-2.575;
      int index = 0;
      for (int j = 0; j < 5; j++){
        if (roundToCustomValues(int(voltage/1000))==voltages[j]){
          index=j;
          break;
        }
      }
      //Serial.print("voltage: ");
      //Serial.println(roundToCustomValues(int(voltage/1000)));
      //Serial.print("Number: ");
      //Serial.println(index);
      //Serial.println(adcHallOffsets[index]);
      if (int(voltage/1000)>=5 && int(voltage/1000)<9){
        index=1;
      }
      v=adcHallOffsets[index]-v;
      //Serial.println("V:"+String(v));
      vAverage+=v/0.4;
  }
  //Serial.println("A: "+String((vAverage/20)-0.18));
  //return temp;
  //Serial.println(temp);
  return (vAverage/20);
  //return readCurrentController();
}
void LEDupdate(int PSUVoltage)
{
  //Serial.println(PSUVoltage);
  if (overcurrent==false && digitalRead(SwitchOut) == false && (roundto(readVoltage(),20) < PSUVoltage-200 || roundto(readVoltage(),20) > PSUVoltage+200)){
    usbpd.on();
    rgbLED(255,255,0);
  }else if (overcurrent==false && roundto(readVoltage(),20)>4000 && digitalRead(SwitchOut) == false)
  {
    usbpd.on();
    rgbLED(0,255,0);
  }else
  {
    usbpd.off();
    rgbLED(0,0,0);
  }
}

int roundto(int num, int roundto) {
    int remainder = num % roundto;
    if (remainder != 0) {
        return num + (roundto - remainder);
    }
    return num;
}

int roundToCustomValues(int value) {
  // Define the custom rounding values
  int roundingValues[] = {5, 9, 12, 15, 20};
  int closestValue = roundingValues[0]; // Initialize with the first value

  // Find the closest rounding value
  for (int i = 1; i < 5; i++) {//4
    //Serial.println(i);
    if (abs(value - roundingValues[i]) < abs(value - closestValue)) {
      closestValue = roundingValues[i];
    }
  }

  // Return the rounded value
  return closestValue;
}


int readVoltage()
{
  
  int temp=0;
  for(int i = 0; i < 20; i++)
  {
    temp+=usbpd.readVoltage();
  }
  return int(temp/20);
}

void overcurrentCheck(int mCurrent,int PSUVoltage)
{
  if(readCurrent(PSUVoltage)*1000>mCurrent) //|| overcurrent=true)
  {
    usbpd.off();
    overcurrent=true;
    rgbLED(255,0,0);
  }else if(overcurrent==true && digitalRead(SwitchOut)==true && readCurrent(PSUVoltage)*1000<mCurrent)
  {
    overcurrent=false;
  }else if(overcurrent == false)
  {
    LEDupdate(PSUVoltage);
  }
}
int PSUVoltage=5000;
int oldPSUVoltage=0;
float PSUCurrent=MaxCurrent;
int oldPSUCurrent=0;
int threshhold=40;
int timercounter=0;
int buttonPress[] = {false, false};
void loop()
{
  //display.clearDisplay();
  //display.setTextSize(1); // Draw 2X-scale text
  //display.setTextColor(SSD1306_WHITE);
  //display.setCursor(0, 30);
  //display.println("Max Current: " + String(MaxCurrent));
  //display.setCursor(0, 40);
  //display.println("Max Current: " + String(PSUCurrent));
  //display.display();
  //delay(2000);
  if (digitalRead(ButtonSel)==false && ppsspec == true){
    cc = !cc;
    delay(200);
    Serial.println("CC");
  }
  if(cc==true){//current control Mode
    
    while (digitalRead(ButtonSel)){//While not switches to VC
      int curr=readCurrent(PSUVoltage)*1000;
      if (digitalRead(SwitchOut)==false){
        usbpd.on();
        if (curr-0.25*curr>PSUCurrent && PSUVoltage>=MinVoltage+15*voltageSteps){//35% positive deviation
          PSUVoltage-=15*voltageSteps;
        }else if (curr-0.15*curr>PSUCurrent && PSUVoltage>=MinVoltage+6*voltageSteps){//25% positive deviation
          PSUVoltage-=6*voltageSteps;
        }else if(curr-0.06*curr>PSUCurrent && PSUVoltage>=MinVoltage+voltageSteps){//8% positive deviation
          PSUVoltage-=voltageSteps;

        

        }else if (curr+0.25*curr<PSUCurrent && PSUVoltage<MaxVoltage-15*voltageSteps){//35% negative deviation
          PSUVoltage+=15*voltageSteps;
          Serial.println("Far off");       
        }else if (curr+0.15*curr<PSUCurrent && PSUVoltage<MaxVoltage-6*voltageSteps){//25% negative deviation
          PSUVoltage+=6*voltageSteps;  
        }else if(curr+0.06*curr<PSUCurrent && PSUVoltage<MaxVoltage-voltageSteps){//8% negative deviation
          PSUVoltage+=voltageSteps;
          Serial.println("Close");
        }
        usbpd.setVoltage(PSUVoltage);
      }
      else
      {
        usbpd.off();
      }
      

      //Serial.println("Change max Current");
      if(digitalRead(ButtonUp)==false && PSUCurrent<MaxCurrent-CurrentSteps)
      {
        PSUCurrent+=CurrentSteps;
      }
      if(digitalRead(ButtonDown)==false && PSUCurrent>=MinCurrent+CurrentSteps)
      {
        PSUCurrent-=CurrentSteps;
      }
      if (PSUCurrent!=oldPSUCurrent)
      {
        //usbpd.setMaxCurrent(PSUCurrent);
        oldPSUCurrent=PSUCurrent;
      }
      
      info(PSUVoltage,roundto(readVoltage(),20),roundto(PSUCurrent,50),readCurrent(PSUVoltage),usbpd.readTemp());
    }
    cc=false;
    delay(200);
  }
  if(digitalRead(SwitchVA)==true)//Current Settings
  {
    //Serial.println("Change max Current");
    if(digitalRead(ButtonUp)==false && PSUCurrent<MaxCurrent-CurrentSteps)
    {
      PSUCurrent+=CurrentSteps;
    }
    if(digitalRead(ButtonDown)==false && PSUCurrent>=MinCurrent+CurrentSteps)
    {
      PSUCurrent-=CurrentSteps;
    }
    if (PSUCurrent!=oldPSUCurrent)
    {
      //usbpd.setMaxCurrent(PSUCurrent);
      oldPSUCurrent=PSUCurrent;
    }
    int vol = 0;
    for(int i = Pin5V; i <= PinPSU;i++)//So that voltage control works in cc mode
    {

      if(digitalRead(i)==false && i!=PinPSU && voltages[i-Pin5V]*1000<=MaxVoltage && voltages[i-Pin5V]*1000>=MinVoltage)//set fixed voltage
      {
        usbpd.setVoltage(voltages[i-Pin5V]*1000);
        vol = voltages[i-Pin5V]*1000;
        //info(voltages[i-Pin5V],roundto(readVoltage(),20),roundto(PSUCurrent,50),readCurrent(),usbpd.readTemp());
        overcurrentCheck(roundto(PSUCurrent,50),voltages[i-Pin5V]*1000);
        break;
      }
    }
    if (vol != 0){
      info(vol,roundto(readVoltage(),20),roundto(PSUCurrent,50),readCurrent(vol),usbpd.readTemp());
      overcurrentCheck(roundto(PSUCurrent,50),readCurrent(vol));
    }else{
      info(roundto(PSUVoltage,20),roundto(readVoltage(),20),roundto(PSUCurrent,50),readCurrent(PSUVoltage),usbpd.readTemp());
      overcurrentCheck(roundto(PSUCurrent,50),roundto(PSUVoltage,20));  
    }
  }
  else 
  {
    if (timercounter>threshhold+(VoltageSteps-voltageSteps)*500)
    {
      voltageSteps+=10;
    }else if (timercounter < threshhold)
    {
      voltageSteps=VoltageSteps;
    }
    for(int i = Pin5V; i <= PinPSU;i++)
    {
      //oldPSUVoltage=0;
      if (digitalRead(i)==false && i==PinPSU && ppsspec==false){
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 0);
        display.println("Fixed Voltages only\non this Power\nSupply!");
        display.display();
      }else if(digitalRead(i)==false && i==PinPSU)
      {
        Serial.println("PSU Mode");
        if(digitalRead(ButtonUp)==false && PSUVoltage<MaxVoltage-voltageSteps)
        {
          PSUVoltage+=voltageSteps;
          timercounter++;
          buttonPress[0]=true;
        }
        if(digitalRead(ButtonDown)==false && PSUVoltage>=MinVoltage+voltageSteps)
        {
          PSUVoltage-=voltageSteps;
          timercounter++;
          buttonPress[1]=true;
        }
        if ((digitalRead(ButtonUp)==true && buttonPress[0]==true) | (digitalRead(ButtonDown)==true && buttonPress[1]==true))
        {
          timercounter=0;
          voltageSteps=VoltageSteps;
          buttonPress[0]=false;
          buttonPress[1]=false;
        }
        if (PSUVoltage!=oldPSUVoltage)//set voltage PPS
        {
          usbpd.setVoltage(PSUVoltage);
          oldPSUVoltage=PSUVoltage;
        }
        info(roundto(PSUVoltage,20),roundto(readVoltage(),20),roundto(PSUCurrent,50),readCurrent(PSUVoltage),usbpd.readTemp());
        overcurrentCheck(roundto(PSUCurrent,50),roundto(PSUVoltage,20));
      }
      else 
      if(digitalRead(i)==false && i!=PinPSU && voltages[i-Pin5V]*1000<=MaxVoltage && voltages[i-Pin5V]*1000>=MinVoltage)//set fixed voltage
      {
        /*if (PSUVoltage!=oldPSUVoltage)//set voltage PPS
        {
          usbpd.setVoltage(voltages[i-Pin5V]*1000);
          oldPSUVoltage=voltages[i-Pin5V]*1000;
        }*/
        usbpd.setVoltage(voltages[i-Pin5V]*1000);
        info(voltages[i-Pin5V],roundto(readVoltage(),20),roundto(PSUCurrent,50),readCurrent(voltages[i-Pin5V]*1000),usbpd.readTemp());
        overcurrentCheck(roundto(PSUCurrent,50),voltages[i-Pin5V]*1000);
        break;
      }
    }
  }
}



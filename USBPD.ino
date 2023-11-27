

#include <Arduino.h>
#include <AP33772.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>
//extern TwoWire Wire1;

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN        0 // On Trinket or Gemma, suggest changing this to 1

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS 1 // Popular NeoPixel ring size

#define maxCurrent 5000//mA
#define MinCurrent 50//mA
#define CurrentSteps 15//mA
#define maxVoltage 20000//mV
#define minVoltage 5000//mV
#define VoltageSteps 5//mV

#define SwitchVA 14
#define SwitchOut 100
#define ButtonUp 2
#define ButtonDown 3
#define ADCHall A0
#define Pin5V 8
#define Pin9V 9
#define Pin12V 10
#define Pin15V 11
#define Pin20V 12
#define PinPSU 13

#define SDA 4
#define SCL 5


Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);


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

int MaxCurrent = maxCurrent; 
int MaxVoltage = maxVoltage;
int MinVoltage = minVoltage;

float adcHallOffset=0;

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
  display.display();
  delay(2000); // Pause for 2 seconds
  // Clear the buffer
  display.clearDisplay();
  // Draw a single pixel in white
  display.drawPixel(10, 10, WHITE);

  pinMode(LED_BUILTIN, OUTPUT); // Built in LED
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
  //Switch
  pinMode(SwitchVA,INPUT_PULLUP);
  //Current
  pinMode(ADCHall,INPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  Serial.begin(115200);
  delay(1000); //Delay is need for the IC to obtain PDOs
  usbpd.begin(); // Start pulling the PDOs from power supply
  //usbpd.printPDO();
  //usbpd.setVoltage(9000);
  //Serial.println(usbpd.printPDO());
    


    
  usbpd.setVoltage(5000); //Set voltage at 12V
  usbpd.setMaxCurrent(50);
    
  pinMode(23, OUTPUT); // Load Switch
  digitalWrite(LED_BUILTIN, HIGH);

  pixels.begin();
  pixels.clear(); 
  //pixels.setPixelColor(0, pixels.Color(255, 255, 255));

  pixels.show();
  




  printRatings();
  HallCalibrate();
  delay(5000);
}


void printRatings()
{
  float specs[4]={0,0,0,0};
  String specsS[4]={"","","",""};
  usbpd.printPDO(specs);
  Serial.println("he");
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

    display.setCursor(0, 0);
    display.println("Source PDO Num: " + specsS[0]);
    display.setCursor(0, 10);
    display.println("PPS Supported");
    display.setCursor(0, 20);
    display.println("Min Voltage: " + specsS[1]);
    display.setCursor(0, 30);
    display.println("Max Voltage: " + specsS[2]);
    display.setCursor(0, 40);
    display.println("Max Current: " + specsS[3]);
    MaxVoltage=int(maxVspec*1000);
    MinVoltage=int(minVspec*1000);
    MaxCurrent=int(maxAspec*1000);
  }else if (specs[0]!=0)//fixed Voltage
  {
    ppsspec=false;
    maxVspec=specs[1];
    maxAspec=specs[2];
    display.setCursor(0, 0);
    display.println("Source PDO Num: " + specsS[0]);
    display.setCursor(0, 10);
    display.println("No PPS Support");
    display.setCursor(0, 20);
    display.println("Voltage: " + specsS[1]);
    display.setCursor(0, 30);
    display.println("Max Current: " + specsS[2]);
    MaxVoltage=int(maxVspec*1000);
    MaxCurrent=int(maxAspec*1000);
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
  String setV = String(tv);
  String voltage = String(v);
  String maxC = String(ti);
  String current = String(i);
  String temp = String(t);
  String power = "0";
  if(i>0.15)
  {
    power = String((i/1000)*v);
  }
  String load = "inf";
  if(i>0.15)
  {
    load = String((v/1000)/(i));
  }

  //Serial.println("SetV: " + setV + " Voltage: " + voltage + " current: " + current + " Temp: " + temp);
  display.clearDisplay();
  display.setTextSize(1); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Set V: "+ setV);
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
  //display.setCursor(0, 40);
  //display.println(usbpd.printPDO());
  //Serial.println(usbpd.printPDO());
  
  
  display.display();

}
void HallCalibrate()
{
  float temp=0;
  for(int i = 0; i<100; i++)
  {
    temp+=(3.3*analogRead(A0))/1024;
  }
  adcHallOffset=temp/100;
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
float readCurrent()
{
  float vAverage=0;
  float temp=0;
  for(int i = 0; i < 20; i++)
  {
      int curr=analogRead(A0);
      //temp=curr;
      //Serial.println(curr);
      float v=(3.3*curr)/1024;
      //Serial.println("Real: " + String(v));
      temp=v;
      v=v-adcHallOffset;//v-2.575;
      //Serial.println("V:"+String(v));
      vAverage+=v/0.2;
  }
  //Serial.println("A: "+String((vAverage/20)-0.18));
  //return temp;
  return (vAverage/20);
  //return readCurrentController();
}

int roundto(int num, int roundto) {
    // Calculate the remainder when dividing by 40
    int remainder = num % roundto;

    // If remainder is non-zero, round up to the next multiple of 40
    if (remainder != 0) {
        return num + (roundto - remainder);
    }

    // Already a multiple of 40, no rounding needed
    return num;
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
int PSUVoltage=5000;
int oldPSUVoltage=0;
int PSUCurrent=10;
int oldPSUCurrent=0;
int threshhold=40;
int timercounter=0;
int buttonPress[] = {false, false};
void loop()
{
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
      usbpd.setMaxCurrent(PSUCurrent);
      oldPSUCurrent=PSUCurrent;
    }
    info(roundto(PSUVoltage,20),roundto(usbpd.readVoltage(),20),roundto(PSUCurrent,50),readCurrent(),usbpd.readTemp());    
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
    for(int i = 8; i <= 13;i++)
    {
      oldPSUVoltage=0;
      if(digitalRead(i)==false && i==13)
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
        if (PSUVoltage!=oldPSUVoltage)
        {
          usbpd.setVoltage(PSUVoltage);
          oldPSUVoltage=PSUVoltage;
        }
        info(roundto(PSUVoltage,20),roundto(usbpd.readVoltage(),20),roundto(PSUCurrent,50),readCurrent(),usbpd.readTemp());
      }
      else 
      if(digitalRead(i)==false && i!=PinPSU && voltages[i-8]*1000<=MaxVoltage && voltages[i-8]*1000>=MinVoltage)
      {
        usbpd.setVoltage(voltages[i-8]*1000);
        info(voltages[i-8],roundto(usbpd.readVoltage(),20),roundto(PSUCurrent,50),readCurrent(),usbpd.readTemp());
        break;
      }
    }
  }
}

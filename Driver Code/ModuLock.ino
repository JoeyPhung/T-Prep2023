#define REMOTEXY_MODE__ESP32CORE_WIFI_POINT
#include <WiFi.h>
#include <RemoteXY.h>
#include <Adafruit_NeoPixel.h> //neopixel
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_LIS3DH.h> //accelerometer
#include <Adafruit_Sensor.h> 

#define PIN 33 // input pin Neopixel is attached to 
#define NUMPIXELS 2 // number of neopixels in Ring
#define BUZZER 15 //piezo pin output
#define ANALOG 34 // for the manual wire cutting
#define LIS3DH_CLK 22  //scl
#define LIS3DH_MISO 14 //sdo
#define LIS3DH_MOSI 23 //sda
#define LIS3DH_CS 32 //cs pin
#define PIN_SWITCH_1 13 //switch
#define PIN_BUTTON_1 12 //deactivate
#define PIN_BUTTON_2 27 //sos

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800); //initalizes neopixel
Adafruit_LIS3DH lis = Adafruit_LIS3DH(LIS3DH_CS, LIS3DH_MOSI, LIS3DH_MISO, LIS3DH_CLK); //initializes accelerometer

// RemoteXY connection settings 
#define REMOTEXY_WIFI_SSID "RemoteXY"
#define REMOTEXY_WIFI_PASSWORD "12345678"
#define REMOTEXY_SERVER_PORT 6377

// RemoteXY configurate  
#pragma pack(push, 1)
uint8_t RemoteXY_CONF[] =   // 134 bytes
  { 255,3,0,11,0,127,0,16,177,1,2,0,19,21,24,13,36,24,31,31,
  79,78,0,79,70,70,0,1,0,35,70,23,23,136,31,0,129,0,17,15,
  28,5,31,68,105,115,97,114,109,47,65,114,109,0,67,4,1,46,61,9,
  31,26,11,129,0,33,63,25,5,31,68,101,97,99,116,105,118,97,116,101,
  0,129,0,19,39,24,6,31,76,111,103,32,66,111,120,0,129,0,12,2,
  39,8,31,77,111,100,117,76,111,99,107,0,1,0,5,70,24,24,1,31,
  0,129,0,9,63,15,5,31,80,65,78,73,67,0 };
  
// this structure defines all the variables and events of your control interface 
struct {
    // input variables
  uint8_t switch_1; // =1 if switch ON and =0 if OFF 
  uint8_t button_1; // =1 if button pressed, else =0 
  uint8_t button_2; // =1 if button pressed, else =0 
    // output variables
  char text_1[11];  // string UTF8 end zero 
    // other variable
  uint8_t connect_flag;  // =1 if wire connected, else =0 
} RemoteXY;
#pragma pack(pop)

/////////////////////////////////////////////
//           END RemoteXY include          //
/////////////////////////////////////////////

int redColor = 0;
int greenColor = 0;
int blueColor = 0;

void setup() {
  RemoteXY_Init (); 
  Serial.begin(921600);
  pinMode (PIN_SWITCH_1, OUTPUT);
  pinMode (PIN_BUTTON_1, OUTPUT);
  pinMode (PIN_BUTTON_2, OUTPUT);
  pinMode (PIN, OUTPUT);
  pinMode (BUZZER, OUTPUT);
  pinMode (ANALOG, INPUT);
  //pinMode(PHOTOCELLPIN, INPUT);

  pixels.begin();
  while (!Serial) delay(10);   
  Serial.println("LIS3DH test!");
  if (! lis.begin(0x18)) {   // change this to 0x19 for alternative i2c address
    Serial.println("Couldnt start");
    while (1) yield();
  }

  Serial.println("LIS3DH found!");
  lis.setRange(LIS3DH_RANGE_2_G);   //range of accelerometer: 2, 4, 8 or 16 G!
  Serial.print("Range = "); Serial.print(2 << lis.getRange());
  Serial.println("G");
  // lis.setDataRate(LIS3DH_DATARATE_50_HZ); //sets the data rate of the accelerometer
  Serial.print("Data rate set to: ");
  switch (lis.getDataRate()) {
    case LIS3DH_DATARATE_1_HZ: Serial.println("1 Hz"); break;
    case LIS3DH_DATARATE_10_HZ: Serial.println("10 Hz"); break;
    case LIS3DH_DATARATE_25_HZ: Serial.println("25 Hz"); break;
    case LIS3DH_DATARATE_50_HZ: Serial.println("50 Hz"); break;
    case LIS3DH_DATARATE_100_HZ: Serial.println("100 Hz"); break;
    case LIS3DH_DATARATE_200_HZ: Serial.println("200 Hz"); break;
    case LIS3DH_DATARATE_400_HZ: Serial.println("400 Hz"); break;
    case LIS3DH_DATARATE_POWERDOWN: Serial.println("Powered Down"); break;
    case LIS3DH_DATARATE_LOWPOWER_5KHZ: Serial.println("5 Khz Low Power"); break;
    case LIS3DH_DATARATE_LOWPOWER_1K6HZ: Serial.println("16 Khz Low Power"); break;
  }
  lightoff();
}

int eventstart = 0; // flag to check if this is the first time the code sees the armed switch turned on
float xstart = 0; //variables to store the initial acceleration values
float ystart = 0;
float zstart = 0;
int alarmon = 0; //flag to check if alarm is on

void loop(){ 
  RemoteXY_Handler ();
  digitalWrite(PIN_SWITCH_1, (RemoteXY.switch_1==0)?LOW:HIGH);
  digitalWrite(PIN_BUTTON_1, (RemoteXY.button_1==0)?LOW:HIGH);
  digitalWrite(PIN_BUTTON_2, (RemoteXY.button_2==0)?LOW:HIGH);

  if (digitalRead(PIN_SWITCH_1) == 1){ // if switch is on
    if (eventstart == 0){ //if this is the first time the system sees the armed switch is turned on
      strcpy(RemoteXY.text_1,"Armed!");
      sensors_event_t event;
      lis.getEvent(&event);
      xstart = event.acceleration.x;//takes in the intial acceleration values and stores them
      ystart = event.acceleration.y;
      zstart = event.acceleration.z;
      rgb(255,0,0); //neopixel flashes red for 0.1 seconds
      delay(100);
      lightoff();
      eventstart = 1;
    }
    sensoranalysis(); //checks the sensors if armed
    if(analogRead(ANALOG) == 0){ //checks if wire is cut
      strcpy(RemoteXY.text_1,"Wire Cut!");
      alarm();
    }
  }
  if (digitalRead(PIN_BUTTON_2) == 1){ //checks if the panic button has been pressed
    disco();
    strcpy(RemoteXY.text_1,"PANIC");
  }
  if (RemoteXY.button_1==1){ //checks if the disarm button 
    button1on();
  }
  if(digitalRead(PIN_SWITCH_1) == 0){ //checks if the switch is disarmed
    if (eventstart == 1){
      strcpy(RemoteXY.text_1,"Disarmed!");
      rgb(0,255,0); //flashes green when deactivated
      delay(100);
      antialarm();
      lightoff();
      eventstart = 0; //resets the alarm flag and the switch flag
      alarmon = 0;
    }
  }
  // use the RemoteXY structure for data transfer
  // do not call delay() 
}

void button1on(){
  strcpy(RemoteXY.text_1,"Deactivated!");
  rgb(0,0,255); //flashes blue when deactivated
  delay(100);
  antialarm();
  alarmon = 0; //resets the alarm flag
}

//Sets off the alarm
void alarm(){
  tone(BUZZER, 2000); //turns on piezo buzzer
  setColor(); //randomizes color
	rgb(redColor, greenColor, blueColor); //outputs random color to rgb
  delay(50);
}

//Turns off buzzer and light
void antialarm(){
  noTone(BUZZER);
  lightoff();
}

//Randomly sets the value of RedColor, GreenColor, BlueColor
void setColor(){
  redColor = random(0, 255);
  greenColor = random(0,255);
  blueColor = random(0, 255);
}

//takes in the current acceleration and gets the magnitude of that vector
void sensoranalysis(){
  sensors_event_t event;
  lis.getEvent(&event);
  float x = event.acceleration.x - xstart; //compares the current acceleration with the one intially stored
  float y = event.acceleration.y - ystart;
  float z = event.acceleration.z - zstart;
  float xyz = sqrt(x * x + y * y + z * z); //magnitude of the acceleration vector
  if (alarmon == 0){ //to check if the alarm has already been set, prevents case 3 from being overwritten by case 3
    if(xyz >= 8){ //case 3: alarm and message
      alarm();
      alarmon++;
      strcpy(RemoteXY.text_1,"Level 3 movement");
    }
    else if(xyz < 8 && xyz >= 4){ //case 2: short alarm and message
      alarm();
      delay(50);
      antialarm();
      strcpy(RemoteXY.text_1,"Level 2 movement");
    }
    else if(xyz <4 && xyz > 2){ //case 1: send a message
      strcpy(RemoteXY.text_1,"Level 1 movement");
    }
  }
  /*int darkness = 0; //code to determine if the lock is in a dark area
  darkness = analogRead(PHOTOCELLPIN);
  if (darkness > 2250){ //if the light value is above a certain point, the user will be allerted with a warning message
    strcpy(RemoteXY.text_1,"Getting Dark, Move bike");
    rgb (255, 255, 255);
    Serial.println(brightness)
  }*/
}

void lightoff(){ //Turns off the neopixels
  rgb(0,0,0);
}

void rgb(int red, int green, int blue){ //displays a given set of rgb values to the neopixel
  for (int i = 0; i < NUMPIXELS; i++){
    pixels.setPixelColor(i, pixels.Color(red, green, blue)); 
    pixels.setBrightness(255);
    pixels.show(); 
  }
}

void disco(){ //function to activate disco lights
  tone(BUZZER, 2000);
  while (true){ 
    RemoteXY_Handler (); //this is to check if the deactivate button has been pressed
    digitalWrite(PIN_BUTTON_1, (RemoteXY.button_1==0)?LOW:HIGH);
    setColor(); //color randomizer loop
	  rgb(redColor, greenColor, blueColor);
    delay(50);
    if(digitalRead(PIN_BUTTON_1) == 1){
      button1on();
      return;
    }
  }
}
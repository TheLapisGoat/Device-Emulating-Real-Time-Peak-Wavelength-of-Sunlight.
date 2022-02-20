//all the modes done except the auto mode along with the wifi part___wav to rgb function, time in min functions are added___complete manual input output and logic is done
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Keypad.h>
#include <TimeLib.h> //Library Time by Paul Stoffregen. Allow the functionality of setting time and getting current time value.
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define REDPIN 14        //GPIO pins on which RGB are connected
#define GREENPIN 12
#define BLUEPIN 13
#define D 5               //delay for rgb output
#define OLED_RESET     -1 // Reset pin # (-1 since sharing Node MCU reset pin)
#define SCREEN_ADDRESS 0x3c //Address of i2c display

WiFiClient client;

const byte ROWS = 4;
const byte COLS = 3;
char Time[4];               //user time input (manual mode)
int oppMode = 0;    //Mode device is currently opperating in
int min_day_span, min_mid_day;  //Dayspan time and mid day time in min (auto mode)
char key;                   //main keypad input variable for selecting modes
int rgb_manual[3];          //RGB values for manual output
int rgb_auto[3];            //RGB values for auto output
double colour_wav_slope, colour_wav_auto; //wavelength slope and value at time t (auto mode) 
double colour_wav_manual;
byte rowPins[ROWS] = {10, 9, 16, 0};        //pins on which keypad is connected
byte colPins[COLS] = {2, 15, 3}; 
char keys[ROWS][COLS] = 
{
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );   //Object of keypad

class Time_of_Day                                               //Declares class containing time values
{
 public:
    int minutes;
    
    void set(int h, int m)
    {
      minutes = h*60 + m;
    }
};

Time_of_Day sunrise, sunset;               //Instances of Time_of_Day to store time values.

void EnableWiFi()                                              // Connects to WiFi
{
  WiFi.mode(WIFI_STA);
  WiFi.begin("TP-Link_BF97", "Crzdip@90");                     //Enter ssid, password as strings)
  Serial.print("Connecting to WiFi");
  while(WiFi.status() != WL_CONNECTED)
  {
    Serial.print('.');
    delay(200);
  }
  Serial.print("IP Address ="); 
  Serial.print(WiFi.localIP());                                //Prints IP Adress
}

void GetData(int arr1[2], int arr2[2], int arr3[2])            //Get arrays for sunrise, sunset and current time in h,min from APIs
{
  HTTPClient http;                                             //Declares HTTP clients
  HTTPClient http1;
  HTTPClient http2;
  delay(500);

  http.begin(client, "http://ip-api.com/json/");    
  
  int httpCode = http.GET();
  if(httpCode < 0)  
   {
    Serial.println("I don't feel so good, httpCode < 0"); 
    while(true);                                              
  }
  String payload1 = http.getString();                          //String containing lat and long data of location of device
  int lat_index = payload1.indexOf("lat");
  int lon_index = payload1.indexOf("lon");
  String lat = payload1.substring(lat_index+5, lat_index+12);
  String lon = payload1.substring(lon_index+5, lon_index+12);
  delay(500);
  http.end();
 
  http1.begin(client, "http://api.sunrise-sunset.org/json?lat="+lat+"&lng="+lon);
  
  int httpCode1 = http1.GET();
  if(httpCode < 0)                                            
  {
    Serial.println("I don't feel so good, httpCode < 0"); 
    while(true);                                             
  }
  String payload2 = http1.getString();                        //String containing sunrise, sunset data
  int Sunrise_index = payload2.indexOf("sunrise");
  int Sunset_index = payload2.indexOf("sunset");
  int colon_index = payload2.indexOf(':', Sunrise_index+10);
  
  String Sunrise_time1 = payload2.substring(Sunrise_index+10, colon_index);
  String Sunrise_time2 = payload2.substring(colon_index+1, colon_index+3);
  arr1[0] = Sunrise_time1.toInt(); arr1[1] = Sunrise_time2.toInt();
    if( payload2.substring(colon_index+7, colon_index+9) == "PM" && arr1[0] != 12)         //Convert PM to 24h data
    {
      arr1[0] += 12;
    }
  
  colon_index = payload2.indexOf(':', Sunset_index+9);
  String Sunset_time1 = payload2.substring(Sunset_index+9, colon_index);
  String Sunset_time2 = payload2.substring(colon_index+1, colon_index+3);
  arr2[0] = Sunset_time1.toInt(); arr2[1] = Sunset_time2.toInt();
   if( payload2.substring(colon_index+7, colon_index+9) == "PM"  && arr2[0] != 12)          //Convert PM to 24h data
    {
      arr2[0] += 12;
    }

  delay(500);
  http1.end();
 
  
  http2.begin(client, "http://worldtimeapi.org/api/ip");
  
  httpCode = http2.GET();
  if(httpCode < 0)                                            
  {
    Serial.println("I don't feel so good, httpCode < 0"); 
    while(true);                                             
  }
  String payload3 = http2.getString();                                                      //Contains IST current time and utc offset
  int Time_index = payload3.indexOf("datetime");
  int utc_offset_index = payload3.indexOf("utc_offset");
  String New_time1 = payload3.substring(Time_index+22, Time_index+24);
  String New_time2 = payload3.substring(Time_index+25, Time_index+27);
  arr3[0] = New_time1.toInt(); arr3[1] = New_time2.toInt();
  
  String utc_offset_h = payload3.substring(utc_offset_index+14, utc_offset_index+16);
  String utc_offset_m = payload3.substring(utc_offset_index+17, utc_offset_index+19);
  int utc_h = utc_offset_h.toInt();
  int utc_m = utc_offset_m.toInt();

  delay(500);
  http2.end();
 
  
  arr1[0] += utc_h; arr1[1] += utc_m;
  arr2[0] += utc_h; arr2[1] += utc_m;
  if( arr1[1] >= 60)                                                        //Checks if min value is greater than 60
  {
    arr1[0]++; arr1[1] -= 60;
  }
  delay(250);
  if( arr2[1] >= 60)                                                        //Checks if min value is greater than 60
  {
    arr2[0]++; arr2[1] -= 60;
  }
  delay(250);
  if( arr1[0] >= 24)                                                        //Checks if hour values is greater than 24
  {
    arr1[0] -= 24;
  }
  delay(250); 
  if( arr2[0] >= 24)                                                        //Checks if hour values is greater than 24
  {
    arr2[0] -= 24;
  }
  
  delay(500);
}

int current_time()                                                          //Returns current time in terms of minutes
{
  return hour()*60 + minute();
}


void Wav_to_RGB (double wavelength, int arr[3])                            //Takes wavelength, pointer to an array. Converts wavelength to RGB values and edits array to that. Source : Prof Dan Bruton, http://www.physics.sfasu.edu/astro/color/spectra.html 
{                                                                          // Original code written in FORTRAN, last updated 20Feb 1996
    double gamma = 0.8;                                                    //Input wavelength b/w 380 and 750nm, visible spectrum of light
    double R, G, B, attenuation;
    
    if (wavelength >= 380 && wavelength <= 440) 
    {
        attenuation = 0.3 + 0.7 * (wavelength - 380) / 60.0;
        R = pow(((-(wavelength - 440) / 60.0 ) * attenuation), gamma);
        G = 0.0;
        B =pow((1.0 * attenuation), gamma);
    }
    else if (wavelength >= 440 && wavelength <= 490)
    {
        R = 0.0;
        G = pow(((wavelength - 440) / 50.0 ), gamma);
        B = 1.0;
    }
    else if (wavelength >= 490 && wavelength <= 510)
    {
        R = 0.0;
        G = 1.0;
        B = pow((-(wavelength - 510) / 20.0 ), gamma);
    }
    else if (wavelength >= 510 && wavelength <= 580)
    {
        R = pow(((wavelength - 510) / 70.0 ), gamma);
        G = 1.0;
        B = 0.0;
    }
    else if (wavelength >= 580 && wavelength <= 645)
    {
        R = 1.0;
        G = pow((-(wavelength - 645) / 65.0 ), gamma);
        B = 0.0;
    }
    else if (wavelength >= 645 && wavelength <= 750)
    {
        attenuation = 0.3 + 0.7 * (750 - wavelength) / 105.0 ;
        R = pow((1.0 * attenuation), gamma);
        G = 0.0;
        B = 0.0;
    }
    else
    {
        R = 0.0;
        G = 0.0;
        B = 0.0;
    }
    
    int R1 = R * 255; /*Gives RGB values in range of 0-255*/
    if ( R1 > 255)  /*Checks if RGB value is greater than 255*/
    R1 = 255;
    int G1 = G * 255;
    if ( G1 > 255)
    G1 = 255;
    int b1 = B * 255; /*B1 is some pre-existing arduino object, used b1 instead*/
    if ( b1 > 255)
    b1 = 255;
    arr[0]=R1; arr[1]=G1; arr[2]=b1;
    delay(1000);
}

void modesInterface_TextStyle()   //sets text style to size 1 and clears the existing display output along with setting cursor at (0,0)
{
    display.clearDisplay();
    display.display();
    display.setTextSize(1);             // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE);        
    display.setCursor(0,0);
}

void modesInterface()  /*displays the main modes interface, won't work without modesInterface_TextStyle both functions are seprate since it allows to print any other message than main interface*/
{
    display.setCursor(10,10);
    display.println(F("# to turnoff"));       
    display.setCursor(10,20);             
    display.println(F("1. Auto"));           
    display.setCursor(10,30);             
    display.println(F("4. Manual"));           
    display.setCursor(10,40);             
    display.println(F("7. Sleep"));           
    display.setCursor(10,50);             
    display.println(F("*. Work"));           
    display.display();
    display.display();
}

void entervalidtime()  //displays the error message when invalid time input is given inside the manual input interface
{
    display.setTextSize(1);             
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(20,50);
    display.println(F("Enter Valid Time"));
    display.display();
    delay(1000);
}

void usertimeinput()
{        
    int a, b, c, d;                       
    char keytime = '\0'; 
    for (int data_count=0; data_count <= 3; data_count++)   //loop to get time input from user
    {
        while (keytime == '\0')    //won't proceed ahead unless a input from keypad is given within 4 second for each input, if while was not present user won't get any time to give input
        {
            keytime = keypad.getKey();
        }

        Time[data_count] = keytime;   //stores time input in array

        display.setTextSize(2);             // Draw 2X-scale text
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(data_count*20+20,27);    //cursor position according to index of array
        display.print(Time[data_count]);
        if(data_count == 1)
        {
          display.setCursor(50,27);
          display.println(':');
        }
        display.display(); 
        keytime = '\0';
    }
        String temp12 = String(Time[0]);
        a = temp12.toInt();        //converts the char input from the keypad to int form
        temp12 = String(Time[1]);
        b = temp12.toInt();  
        temp12 = String(Time[2]);
        c = temp12.toInt(); 
        temp12 = String(Time[3]);
        d = temp12.toInt(); 
        
        int manual_hr = a*10+b;
        int manual_min = c*10+d;
        
    delay(500);            
    display.clearDisplay();
    modesInterface_TextStyle();
    modesInterface();
    display.setCursor(20,0);
    display.println(F("Time set!"));    //if complete input is given by user displays time set message
    display.display();    
    display.clearDisplay();
    
    setTime(manual_hr, manual_min, 0, 0, 0, 0);    //sets clock according to input time     
}

void automode()                             //Sets time and calculates wavelength parameters for auto mode
{
    modesInterface_TextStyle();
    display.println(F("Selected mode Auto"));        
    modesInterface();
    
    EnableWiFi();                           //Enables Wifi
    int a[2], b[2], c[2];                   //Arrays for h, min. a: Sunrise, b: Sunset, c: Current time IST)
    GetData(a, b, c);                       //Gives arrays h and min values.
    sunrise.set(a[0], a[1]); sunset.set(b[0], b[1]);
    min_day_span = sunset.minutes - sunrise.minutes;
    min_mid_day = (sunset.minutes+sunrise.minutes)/2;
    colour_wav_slope = 621.80/min_day_span;
    setTime(c[0], c[1], 0, 0, 0, 0);         //Set time
}


void modergb(String c)               //displays the interface in a particular mode along with giving appropriate analog output for RGB input string is the name of mode
{
    int r,g,b;                       //RGB output values
    display.clearDisplay();
    modesInterface_TextStyle();
    display.print(F("Current mode:")); 
    display.println(c);
    modesInterface();
    if (c == "Sleep")
    {
          r = 255, g = 112, b = 0;
    }
    if (c == "Work")
    {
          r = 255, g = 244, b = 248;
    }
    if (c == "Manual")
    {
          r = rgb_manual[0], g = rgb_manual[1], b = rgb_manual[2];
    }
    if (c == "Turnoff")
    {
          r = 0, g = 0, b = 0;       
    }
    if (c == "Auto")
    {
          r = rgb_auto[0], g = rgb_auto[1], b = rgb_auto[2];       
    }
    analogWrite(REDPIN, r);
    analogWrite(GREENPIN, g);
    analogWrite(BLUEPIN, b);
    delay(D);
}

void setup() 
{
    pinMode(REDPIN, OUTPUT);
    pinMode(GREENPIN, OUTPUT);
    pinMode(BLUEPIN, OUTPUT);

    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3c))    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    {
        Serial.println(F("SSD1306 allocation failed"));
        for(;;); // Don't proceed, loop forever
    }

    modesInterface_TextStyle();               //main screen
    display.setCursor(0,0);                   //coordinates of point from which printing on display starts total pixels -> (128,64)    
    display.println(F("DIY TEAM 7"));         //prints any variable or text
    display.display();                        //prints all the things in buffer
    modesInterface(); 
}

void loop()
{

    Serial.begin(9600);
    key = keypad.getKey();        //takes input from keypad
    
    if (key)                      //won't enter the if unless a key is pressed
    {
        if (key == '7')
        {
            String s = "Sleep";
            modergb(s);
            oppMode = 0;
        }
        if (key == '*')
        {
            String s = "Work";
            modergb(s);
            oppMode = 0;
        }
        if (key == '1')
        {
            automode();
            oppMode = 2;
        }
        if (key == '4')
        {
            modesInterface_TextStyle();        
            display.setCursor(0,0);
            display.println(F("Enter time in HH : MM"));
            display.display();
            usertimeinput();
            oppMode = 1;
        }        
    }

    if (oppMode == 1)           //If oppMode is in Manual
    {
        int Min = current_time();                          //time in minutes
    
        if ( Min <= 780 && Min >= 360 )                //gives wavelength according to the time of day the sunrise(6am) and sunset(7pm) time for manual input are perdefined
          {
              colour_wav_manual = 750 - 0.79718*(Min-360);
          }
        else if ( Min >= 780 && Min <= 1140 )
          {
              colour_wav_manual = 750 - 0.79718*(1140-Min);
          }
        else
          {
              colour_wav_manual = 750;
          }

        delay(1000);
        Wav_to_RGB(colour_wav_manual, rgb_manual);   //calls the wav to rgb function to get rgb data according to the wavelength
        String s = "Manual";
        modergb(s);                                  //give required output on the display and to the rgb pins
    } 
    else if (oppMode == 2)
    {
        if ( current_time() <= min_mid_day && current_time() >= sunrise.minutes )     //Calculate wavelength as at a particular time
          {
              colour_wav_auto = 750 - colour_wav_slope*(current_time()-sunrise.minutes);
          }
        else if ( current_time() >= min_mid_day && current_time() <= sunset.minutes )
          {
              colour_wav_auto = 750 - colour_wav_slope*(sunset.minutes-current_time());
          }
        else
          {
              colour_wav_auto = 750;
          }
        delay(1000);
        Wav_to_RGB(colour_wav_auto, rgb_auto);                                             //Call function to get RGB value from wavelength
        String s = "Auto";
        modergb(s);                                  //give required output on the display and to the rgb pins
    }
    
}

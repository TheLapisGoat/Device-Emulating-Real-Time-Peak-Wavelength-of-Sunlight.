#include <TimeLib.h> /*Library Time by Paul Stoffregen. Allow the functionality of setting time and getting current time value.*/
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

WiFiClient client;

class Time_of_Day                             /*Declare class containing time values*/
{
 public:
    int minutes;
    
    void set(int h, int m)
    {
      minutes = h*60 + m;
    }
};

int current_time() /* Returns current time in terms of minutes*/
{
  return hour()*60 + minute();
}

void Wav_to_RGB (double wavelength, int arr[3]) /*Takes wavelength, pointer to an array. Converts wavelength to RGB values and edits array to that. Source : Prof Dan Bruton, http://www.physics.sfasu.edu/astro/color/spectra.html */ 
    {                                                                                                                                               /* Original code written in FORTRAN, last updated 20Feb 1996*/
    double gamma = 0.8;                         /*Input wavelength b/w 380 and 750nm, visible spectrum of light*/
    double R, G, B, attenuation;
    if (wavelength >= 380 && wavelength <= 440) {
        attenuation = 0.3 + 0.7 * (wavelength - 380) / 60.0;
        R = pow(((-(wavelength - 440) / 60.0 ) * attenuation), gamma);
        G = 0.0;
        B =pow((1.0 * attenuation), gamma);
        }
    else if (wavelength >= 440 && wavelength <= 490) {
         R = 0.0;
         G = pow(((wavelength - 440) / 50.0 ), gamma);
         B = 1.0;
        }
    else if (wavelength >= 490 && wavelength <= 510) {
         R = 0.0;
         G = 1.0;
         B = pow((-(wavelength - 510) / 20.0 ), gamma);
        }
    else if (wavelength >= 510 && wavelength <= 580) {
         R = pow(((wavelength - 510) / 70.0 ), gamma);
         G = 1.0;
         B = 0.0;
        }
    else if (wavelength >= 580 && wavelength <= 645) {
         R = 1.0;
         G = pow((-(wavelength - 645) / 65.0 ), gamma);
         B = 0.0;
        }
    else if (wavelength >= 645 && wavelength <= 750) {
         attenuation = 0.3 + 0.7 * (750 - wavelength) / 105.0 ;
         R = pow((1.0 * attenuation), gamma);
         G = 0.0;
         B = 0.0;
        }
    else {
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


void EnableWiFi()                                       /* Connects to WiFi*/
{
  WiFi.mode(WIFI_STA);
  WiFi.begin("TP-Link_BF97", "Crzdip@90");                     /*Enter ssid, password as strings)*/
  Serial.print("Connecting to WiFi");
  while(WiFi.status() != WL_CONNECTED)
  {
    Serial.print('.');
    delay(200);
  }
  Serial.print("IP Address ="); 
  Serial.print(WiFi.localIP());                     /*Prints IP Adress*/
}

void GetData(int arr1[2], int arr2[2], int arr3[2])
{
  HTTPClient http; /*Declares HTTP clients*/
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
  String payload1 = http.getString();       /*String containing lat and long data of location of device*/
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
  String payload2 = http1.getString();                        /*String containing sunrise, sunset data */
  int Sunrise_index = payload2.indexOf("sunrise");
  int Sunset_index = payload2.indexOf("sunset");
  int colon_index = payload2.indexOf(':', Sunrise_index+10);
  
  String Sunrise_time1 = payload2.substring(Sunrise_index+10, colon_index);
  String Sunrise_time2 = payload2.substring(colon_index+1, colon_index+3);
  arr1[0] = Sunrise_time1.toInt(); arr1[1] = Sunrise_time2.toInt();
    if( payload2.substring(colon_index+7, colon_index+9) == "PM" && arr1[0] != 12)         /*Convert PM to 24h data*/
    {
      arr1[0] += 12;
    }
  
  colon_index = payload2.indexOf(':', Sunset_index+9);
  String Sunset_time1 = payload2.substring(Sunset_index+9, colon_index);
  String Sunset_time2 = payload2.substring(colon_index+1, colon_index+3);
  arr2[0] = Sunset_time1.toInt(); arr2[1] = Sunset_time2.toInt();
   if( payload2.substring(colon_index+7, colon_index+9) == "PM"  && arr2[0] != 12)        /*Convert PM to 24h data*/
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
  String payload3 = http2.getString();                        /*Contains IST current time and utc offset*/
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
  if( arr1[1] >= 60)                          /*Checks if min value is greater than 60*/
  {
    arr1[0]++; arr1[1] -= 60;
  }
  delay(250);
  if( arr2[1] >= 60)                        /*Checks if min value is greater than 60*/
  {
    arr2[0]++; arr2[1] -= 60;
  }
  delay(250);
  if( arr1[0] >= 24)                        /*Checks if hour values is greater than 24*/
  {
    arr1[0] -= 24;
  }
  delay(250); 
  if( arr2[0] >= 24)                         /*Checks if hour values is greater than 24*/
  {
    arr2[0] -= 24;
  }
  
  delay(500);
}




Time_of_Day sunrise, sunset;               /*Sunset, Sunrise containers.*/
bool restart = true;
int min_day_span, min_mid_day;
double colour_wav_slope, colour_wav;





void setup() {
  Serial.begin(115200);
  
}


void loop() 
{
  if (restart)
  {
    EnableWiFi();
    int a[2], b[2], c[2]; /* Arrays for h, min. a: Sunrise, b: Sunset, c: Current time IST)*/
    GetData(a, b, c);   /*Gives arrays h and min values.*/
    sunrise.set(a[0], a[1]); sunset.set(b[0], b[1]);
    min_day_span = sunset.minutes - sunrise.minutes;
    min_mid_day = (sunset.minutes+sunrise.minutes)/2;
    colour_wav_slope = 621.80/min_day_span;
    setTime(c[0], c[1], 0, 0, 0, 0);      /*Set time*/
    restart = false;
  }

  if ( current_time() <= min_mid_day && current_time() >= sunrise.minutes )     /*Calculate wavelength as at a particular time*/
  {
    colour_wav = 750 - colour_wav_slope*(current_time()-sunrise.minutes);
  }
  else if ( current_time() >= min_mid_day && current_time() <= sunset.minutes )
  {
    colour_wav = 750 - colour_wav_slope*(sunset.minutes-current_time());
  }
  else
  {
    colour_wav = 750;
  }
  delay(1000);
  int arr[3];
  Wav_to_RGB(colour_wav, arr);  /*Call function to get RGB value from wavelength*/
  Serial.println(arr[0]);  Serial.println(arr[1]);  Serial.println(arr[1]);
}

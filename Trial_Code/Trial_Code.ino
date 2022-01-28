#include <TimeLib.h>

class Time_of_Day
{
 public:
    int minutes;
    
    void set(int h, int m)
    {
      minutes = h*60 + m;
    }
};

int current_time()
{
  return hour()*60 + minute();
}

void Wav_to_RGB (double wavelength, int arr[3])
    {
    double gamma = 0.8;
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
    int R1 = R * 255;
    if ( R1 > 255)
    R1 = 255;
    int G1 = G * 255;
    if ( G1 > 255)
    G1 = 255;
    int b1 = B * 255; //B1 is some pre-existing arduino object.
    if ( b1 > 255)
    b1 = 255;
    arr[0]=R1; arr[1]=G1; arr[2]=b1;
    }




Time_of_Day sunrise, sunset;
int a, b, c, d, e, f;
bool restart = false;
int min_day_span, min_mid_day;
double colour_wav_slope, colour_wav;





void setup() {
  // put your setup code here, to run once:
}


void loop() 
{
  if (restart == true)
  {
    sunrise.set(a, b); sunset.set(c, d);
    min_day_span = sunset.minutes - sunrise.minutes;
    min_mid_day = (sunset.minutes+sunrise.minutes)/2;
    colour_wav_slope = 621.80/min_day_span;
    setTime(e, f, 0, 0, 0, 0);
    restart = false;
  }

  if ( current_time() <= min_mid_day && current_time() >= sunrise.minutes )
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

  int arr[3];
  Wav_to_RGB(colour_wav, arr);
  
}

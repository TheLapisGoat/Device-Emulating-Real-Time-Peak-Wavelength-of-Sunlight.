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

Time_of_Day sunrise, sunset;
int a, b, c, d, e, f;
bool restart = false;
int min_day_span, min_mid_day;
float colour_temp_slope, colour_temp;





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
    colour_temp_slope = 9400.0/min_day_span;
    setTime(e, f, 0, 0, 0, 0);
    restart = false;
  }

  if ( current_time() <= min_mid_day && current_time() >= sunrise.minutes )
  {
    colour_temp = 1900 + colour_temp_slope*(current_time()-sunrise.minutes);
  }
  else if ( current_time() >= min_mid_day && current_time() <= sunset.minutes )
  {
    colour_temp = 1900 + colour_temp_slope*(sunset.minutes-current_time());
  }
  else
  {
    colour_temp = 1900;
  }  
  
}

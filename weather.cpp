#include "led-matrix.h"
#include "graphics.h"

#include <unistd.h>
#include <math.h>
#include <stdio.h>
#include <functional>
#include <cstdlib>
#include <iostream>
#include <cstring>
#include <ctime>
#include <signal.h>
#include <syslog.h>
#include <sys/time.h>
#include <fstream>

#include "icons.h"
#include "animation.h"

using namespace std;

using rgb_matrix::GPIO;
using rgb_matrix::RGBMatrix;
using rgb_matrix::Canvas;


volatile bool interrupt_received = false;
static void InterruptHandler(int signo) {
	 syslog( LOG_NOTICE, "interrupt handler ");
	  interrupt_received = true;
}

void usage()
{
	printf("Usage: weather icon currentTemp[int] minTemp[int] maxTempi[int] \n");
	exit (1);
}

void DrawIcon(Canvas *canvas, const uint8_t icon[], float brightness )
{
     int l =0;
     for( int y=0; y <=31; y++ )
     {
         for ( int x = 0; x < 32; x++ )
         {
             canvas->SetPixel(x, y, brightness * icon[l++] , brightness * icon[l++], brightness * icon[l++]);
         }
     }
}

string readFile(string filename)
{
   //cout << " reading " << filename << endl;
   ifstream ifs(filename.c_str());
   string content( (istreambuf_iterator<char>(ifs) ), (istreambuf_iterator<char>() ) );

   return content;
}


int main(int argc, char *argv[]) 
{
  setlogmask( LOG_UPTO (LOG_NOTICE));
  GPIO io;
  if (!io.Init())
    return 1;


  std::string font_type = "./pongnumberfont.bdf";
  rgb_matrix::Font font;
  if (!font.LoadFont(font_type.c_str())) 
  {
	cout <<  "Couldn't load font " << font_type << std::endl;
        return 1;
  }

  Canvas *canvas = new RGBMatrix(&io, 32, 1);

  printf("hello\n");

  string currentTemp = "12";
  string minTemp     = "4";
  string maxTemp     = "24";
  string icon        = "";
  string dailycovidCases  = "";
  string activeCovidCases = "";
  string nextISSPass = "";

  for( int t=1; t < 9; t++)
  {
	switch(t)
	{
		case 1: // icon
			icon = readFile("/tmp/currenticon");
		break;

		case 2: // current temp
			currentTemp = readFile("/tmp/currenttemp");
		break;

		case 3: // min temp
			minTemp = readFile("/tmp/currentmin");
		break;

		case 4: // max temp
			maxTemp = readFile("/tmp/currentmax");
		break;

		case 5: // brightness
		break;

		case 6:
			dailycovidCases = readFile("/tmp/currentcovid");
		break;
		case 7:
			activeCovidCases = readFile("/tmp/activecovid");
		break;
		case 8:
			nextISSPass = readFile("/tmp/isspassutc");
		break;

	}
  }

  minTemp     = std::to_string( std::stoi(minTemp)); 
  maxTemp     = std::to_string( std::stoi(maxTemp));
  currentTemp = std::to_string( std::stoi(currentTemp));


  	float brightnessCorrection = 1.0;
	time_t t = time(0);
	struct tm* now = localtime(&t);

	if (now->tm_hour < 7 || now->tm_hour > 22 )
		brightnessCorrection = 0.2;


  	float brightness = 0.2 * brightnessCorrection;
	canvas->Clear();
	if( icon == "01" )
	DrawIcon( canvas, icon_01d, brightness );
	if ( icon == "02" )
	DrawIcon( canvas, icon_02d, brightness );
	if ( icon == "03" )
        DrawIcon( canvas, icon_03d, brightness );
        if ( icon == "04" )
        DrawIcon( canvas, icon_04d, brightness );
        if ( icon == "09" )
        DrawIcon( canvas, icon_09d, brightness );
        if (icon == "10" )
        DrawIcon( canvas, icon_10d, brightness );
        if ( icon == "11" )
        DrawIcon( canvas, icon_11d, brightness );
        if ( icon == "13" )
        DrawIcon( canvas, icon_13d, brightness );
        if ( icon == "50" )
        DrawIcon( canvas, icon_50d, brightness );
		
	// draw min temp
	rgb_matrix::Color minColor(brightness * 51, brightness * 153, brightness * 255);
	rgb_matrix::DrawText(canvas, font, 0, 32, minColor, minTemp.c_str());

	//draw current temp
	rgb_matrix::Color currentColor(brightness * 100, brightness * 100 , brightness * 100);
	rgb_matrix::DrawText(canvas, font, 11 ,32, currentColor, currentTemp.c_str() );

	// draw max temp
        rgb_matrix::Color maxColor(brightness * 255, brightness * 51 , brightness * 0);
	rgb_matrix::DrawText(canvas, font, 22 ,32, maxColor,  maxTemp.c_str() );


	sleep(10);

	//rgb_matrix::Color covidColor(brightness * 169, brightness * 238, brightness * 73 );
        rgb_matrix::Color covidColor( 169 * brightnessCorrection, 238 * brightnessCorrection, 73 * brightnessCorrection);


        canvas->Clear();
	DrawIcon( canvas, icon_covid_19, 0.5 * brightnessCorrection );
	
	// active cases
	printf("active cases %s\n", activeCovidCases.c_str());
	rgb_matrix::DrawText(canvas, font, 0,28, covidColor, activeCovidCases.c_str() );

	//draw current covid
        printf("daily cases %s\n" , dailycovidCases.c_str());
        rgb_matrix::DrawText(canvas, font, 19 ,28, covidColor,  dailycovidCases.c_str() );

	sleep(10);

	printf("isspass : %s\n",nextISSPass.c_str());

	if ( nextISSPass.length() > 0 )
	{
		long nextUtc = std::atol(nextISSPass.c_str());
		time_t utc_now = time( NULL );
		printf( " %ld  >= %ld \n" , utc_now, nextUtc );
		if( utc_now <= nextUtc )
		{
			canvas->Clear();
			for( int index = 0; index < animation_frames; index++)
			{
				time_t tt_Dif = nextUtc - utc_now; 
				int nSeconds            = tt_Dif % 60;  tt_Dif /= 60;
				int nMinutes            = tt_Dif % 60;  tt_Dif /= 60;
				int nHours              = tt_Dif % 24;  tt_Dif /= 24;
				int nDays               = tt_Dif;
											                        
				string output;
				if( nDays > 0 )
				{       
				          output += std::to_string(nDays);
				          output += " ";
				}
				else    
				{       
				          output += "  ";
				}
				if( nHours <= 9 ) 
				          output += "0";
				output += std::to_string(nHours) + " " ;
				if ( nMinutes <= 0) 
				          output += "0";
				output += std::to_string(nMinutes);
				if ( nSeconds <= 9 )
				          output += "0";
				output += std::to_string(nSeconds);

				DrawIcon(canvas, animation_icons[index], 1.0);
	                        rgb_matrix::DrawText(canvas, font, 0, 32, currentColor, output.c_str());
							
				usleep(100000);
				utc_now = time( NULL );
			}
			
		}
	}




  	canvas->Clear();
  	delete canvas;
  	return 0;
}


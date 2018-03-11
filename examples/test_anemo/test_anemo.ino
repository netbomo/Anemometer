
#include "Arduino.h"
#include "Anemometer.h"
	Anemometer anemo1(0);
	Anemometer anemo2(1);

bool isRunning = false;
const unsigned char measure_max = 4;
unsigned char measure=0;

//The setup function is called once at startup of the sketch
void setup()
{
// Add your initialization code here


	anemo1.set_enable(true);
	anemo1.set_factor(1);
	anemo1.set_offset(0);
	anemo1.clear(measure_max);

	anemo2.set_enable(true);
	anemo2.set_factor(1);
	anemo2.set_offset(0);
	anemo2.clear(measure_max);

	Serial.begin(115200);
	Serial.println("Test the anemometer library");


}

// The loop function is called in an endless loop
void loop()
{
//Add your repeated code here

		if(anemo1.flag_anemo()){		// if timer 3 overflow, it's time to read value
			isRunning = false;
			anemo1.read_value(measure);
			anemo2.read_value(measure);
			measure++;
			anemo1.reset_flag();
		}
		else if(measure>measure_max){	// else if measure > measure_max, it's need to reset measure
			anemo1.calc_average(measure_max);	// do averages
			anemo2.calc_average(measure_max);

			char temp[25];
			anemo1.print_data_array();			// and print data
			Serial.print(anemo1.print(temp));
			Serial.print(" ");
			Serial.println(anemo1.print_average(3, temp));

			anemo2.print_data_array();
			Serial.print(anemo2.print(temp));
			Serial.print(" ");
			Serial.println(anemo2.print_average(3, temp));

			measure = 0;
		}
		else if(isRunning==false){			// else if no measure is running it's time for a new
			isRunning = true;
			Serial.println("New measures :");
			anemo1.start();
			Serial.print("Start");
		}


}

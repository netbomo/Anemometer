/**
 *******************************************************************************
 *******************************************************************************
 *
 *	License :
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * 
 *******************************************************************************
 *******************************************************************************
 *
 *
 *    @file   Anemometer.cpp
 *    @author gilou
 *    @date   24 mai 2017
 *    @brief  The anemometer class permit to read wind speed from frequencies on timer1 and timer0, it use the timer 3 with is overflow to stop measure

 */

#include "Arduino.h"

#include "Anemometer.h"

bool Anemometer::flag_anemoReady = false;

/******************************************************************************
 * Class constructor and destructor
 */
Anemometer::Anemometer(unsigned char id):m_id(id) {
	// initialize variables
	m_average = 0;

	// load eeprom config
	m_eeprom_addr = 50 + m_id*50;		/**< This calc permit to store sensor data each 16 bytes after the EEPROM_OFFSET(30) */
	load_param();

noInterrupts();
	if(m_id==0){
		// Clear All timer0 register
		TCCR0A = 0;
		TCCR0B = 0;
		TIMSK0 = 0;
		TCNT0 = 0;
	}
	else if(m_id==1){
		// Clear All timer1 register
		TCCR1A = 0;
		TCCR1B = 0;
		TCCR1C = 0;
		TIMSK1 = 0;
		TCNT1 = 0;
	}

	// Clear timer3 register
	TCCR3A = 0;
	TCCR3B = 0;
	TCCR3C = 0;
	TIMSK3 = 0;
	interrupts();
}

Anemometer::~Anemometer() {
}


/******************************************************************************
 * Sensor's methods
 */
//This method clear data array.
void Anemometer::clear(unsigned char measure_max){
	for(unsigned char i = 0; i < measure_max; ++i){
		m_data[i] = 0;
	}
}

// The start method is static because it's start measure on the two anemometers.Connect external pins on timers 0 and 12 and save millis in m_start member.
void Anemometer::start(){
	noInterrupts();
	TCNT0 = 0;	// reset the timer 0 value register
	TCNT1 = 0;	// reset the timer 1 value register
	TCNT3 = 0;	// reset the timer 3 value register

	TCCR0B = _BV(CS02) | _BV(CS01) | _BV(CS00);	// use external clock on rising edge
	TCCR1B = _BV(ICES1) | _BV(CS12) | _BV(CS11) | _BV(CS10);  //ICNC1: noise canceler, ICES1: capture on rising edge, CS1[2:0] external clock on rising edge

	TCCR3A = 0;		// Overwrite arduino definition
	TCCR3B = (1 << CS32) | (1 << CS30);	// CCS3[2:0] use 1024 prescaler to start the timer3
	TIMSK3 = (1 << TOIE1);	// start to count for TNCT3 timer
	interrupts();
}

// This method read the sensor's value
void Anemometer::read_value(unsigned char measure_number){
	if(is_enable()){
		double tmp_value = 0;

		if(m_id==0)
		{
			if(TCNT0!=0)						// Control to not devide 0
			{
				tmp_value = double(TCNT0/  TIMER3_OVF_PERIODE);
				//tmp_value = double(TCNT0*1000 /  m_period);	// Convert pulse in hertz
			}
			else{
				tmp_value =0;
			}
		}
		else if(m_id==1)
		{
			if(TCNT1!=0)							// Control to not devide 0
			{
				tmp_value = double(TCNT1/  TIMER3_OVF_PERIODE);
				//tmp_value = double(TCNT1*1000 /  m_period);	// Convert pulse in hertz
			}
			else{
				tmp_value =0;
			}
		}
		m_data[measure_number]=(tmp_value * m_factor) + m_offset; 	// apply the factor and offset before save the data
		if(m_data[measure_number] == m_offset) m_data[measure_number] = 0;	// Remove false calc result
	}
}

// This method calculate the average from the data array.
void Anemometer::calc_average(unsigned char measure_max){
	if(is_enable()){
		m_average=0;
		for(int i =0; i< measure_max;++i)
		{
			m_average+=m_data[i];
		}
		m_average/= measure_max;
	}
}


// The print method print in string "Anemo id"
char* Anemometer::print(char *string){
	if(m_enable){
		char temp_char[6];
		 strcpy(string,"Anemo ");
		 strcat(string,itoa(m_id+1,temp_char,10));
	}

	return string;	// return
}

// This method print the sensor configuration for the sensor. It's a good idea to overload this function to do it more explicit for each sensor.
// todo re implemente print_config
void Anemometer::print_config(){
	Serial.println("Anemometer :");
	Serial.print("	*");Serial.print(m_id+3); Serial.print("1 enable :	");Serial.print(m_enable);Serial.println("			enable : 1, disable : 0");
	Serial.print("	*");Serial.print(m_id+3); Serial.print("2 factor:	");Serial.print(m_factor);Serial.println("		can be a float value. ex: 42.42");
	Serial.print("	*");Serial.print(m_id+3); Serial.print("3 offset:	");Serial.print(m_offset);Serial.println("		can be a float value. ex: 42.42");
}

void Anemometer::config(char *stringConfig){
	uint8_t item = stringConfig[2]-'0';	// convert item in char

	double arg_f = atof(stringConfig + 4);	// convert the second part, the value in double to cover all possibilities.
	unsigned char arg_uc = (unsigned char)arg_f;
	switch (item) {
		case 1:	// enable or disable anemo1
			if(arg_uc==0)m_enable = false;	// disable
			else m_enable = true;				// enable
			update_param();
		break;
		case 2:	// Set factor value
			m_factor = arg_f;
			update_param();
			break;
		case 3:	// Set offset value
			m_offset = arg_f;
			update_param();
			break;
		default:
			Serial.print("Bad request : ");Serial.println(item);
	}

}

// Use this method for debugging or calibration accuracy
void Anemometer::print_data_array()const{
	char temp[20];
	Serial.print("\r\n");
	for(int i=0;i<10;i++)
	{
		Serial.print(i);Serial.print('	');Serial.print(dtostrf(m_data[i],0,3,temp));Serial.print("\r\n");
	}
}

/******************************************************************************
 * sens_param management
 */
// Load saved parameters for sensors from the eeprom
void Anemometer::load_param(){
	m_factor = eeprom_read_float((float*)m_eeprom_addr);
	m_offset = eeprom_read_float((float*)m_eeprom_addr+5);

	if(eeprom_read_byte((unsigned char*)m_eeprom_addr+10)==0) m_enable = false;
	else m_enable = true;
}

// Update saved parameters for sensors in the eeprom
void Anemometer::update_param(){
	eeprom_update_float((float*)m_eeprom_addr,m_factor);
	eeprom_update_float((float*)m_eeprom_addr+5,m_offset);
	if(m_enable==true)	eeprom_update_byte((unsigned char*)m_eeprom_addr+10,1);
	else eeprom_update_byte((unsigned char*)m_eeprom_addr+10,0);
}

// Initialize the eeprom memory and the sens_param array
// todo initialize_param
void Anemometer::initialize_param(){

}

ISR(TIMER3_OVF_vect)
{
	TCCR0B = 0;		// stop Timer0
	TCCR1B = 0;		// stop Timer1
	TCCR3B = 0;		// stop Timer3

	Anemometer::flag_anemoReady = 1;	// Set the flag
}



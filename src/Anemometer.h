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
 *    @file   Anemometer.h
 *    @author gilou
 *    @date   24 mai 2017
 *    @brief  The anemometer class permit to read wind speed from frequencies on timer1 and timer0, it use the timer 3 with is overflow to stop measure
 *
 */
#ifndef ANEMOMETER_H_
#define ANEMOMETER_H_

/**
 * \brief The anemometer class permit to read wind speed from frequencies on timer1 and timer0, it use the timer 3 with is overflow to stop measure
 *
 * A prerequies is to use the Arduino improvement from : https://github.com/netbomo/arduino_core_improvement
 * This improvement add the possibility to use Timer2 instead of Timer 0 for micro and millis functions
 */
class Anemometer {
public:
	/******************************************************************************
	 * Constructor and destructor
	 ******************************************************************************/
	/**
	 * Constructor
	 * @param id use a number as id, use to differenced multiple sensors and for eeprom address
	 * For Anemometer class : id need to be 0 or 1 (timer reference)
	 */
	Anemometer(unsigned char id);

	/**
	 * class destructor
	 */
	virtual ~Anemometer();

	static bool flag_anemoReady;


	/******************************************************************************
	 * setters
	 ******************************************************************************/
	inline void set_factor(double factor) {m_factor=factor; update_param();}

	inline void set_offset(double offset) {m_offset=offset; update_param();}

	inline void set_enable(unsigned char enable) {m_enable=enable; update_param();}

	inline void reset_flag(){flag_anemoReady = false;}


	/******************************************************************************
	 * getters
	 ******************************************************************************/
	inline unsigned char is_enable()const{return m_enable;}

	inline bool flag_anemo()const{return flag_anemoReady;}

	inline float get_average()const{return m_average;}


	/******************************************************************************
	 * Sensor's methods
	 ******************************************************************************/
	/**
	 * This method clear data array.
	 * @param measure_max data array max limit
	 */
	void clear(unsigned char measure_max);

	/**
	 * The start method start measure on the two anemometer if they are enable. Connect external pins on timers 0 and 1. Run timer 3
	 */
	static void start();

	/**
	 * This method read the sensor's value
	 * @param measure_number the m_data array's index
	 */
	void read_value(unsigned char measure_number);

	/**
	* \brief This method calculate the average from the data array.
	*/
	void calc_average(unsigned char measure_max);

	/**
	 * The print method print in string "Anemo id"
	 * @return return string pointer
	 */
	char* print(char *string);

	/**
	 * \brief This method print the sensor configuration for the sensor. It's a good idea to overload this function to do it more explicit for each sensor.
	 */
	void print_config();

	/**
	 * The config method permit to update parameters
	 * @param stringConfig
	 */
	void config(char *stringConfig);

	/**
	 * \brief Use this method for debugging or calibration accuracy
	 */
	void print_data_array()const;

	/******************************************************************************
	 * sens_param management
	 ******************************************************************************/
	/**
	 * \brief Load saved parameters for sensors from the eeprom
	 * \return void
	 */
	void load_param();

	/**
	 * \brief Update saved parameters for sensors in the eeprom
	 * \return void
	 */
	void update_param();

	/**
	 * \brief Initialize the eeprom memory
	 * \return void
	 */
	void initialize_param();

	/**
	 * Static variables declaration
	 */
	static const int measure_max = 10; /// todo do it more flexible and global
	const double TIMER3_OVF_PERIODE = (double) 65536/15625.0; /// 15625 constant to convert from 1/fclock*1024(prescaler) in second /// todo give the possibility to chose time

private:

	unsigned char m_id;						/**< this the sensor's id in the Eeprom::sensor_counter */

	bool m_enable;					/**< If enable is off, the sensor is not process */

	unsigned char m_eeprom_addr;				/**< This is the eeprom address calc from the m_id and the EEPROM_OFFSET */

	double m_data[measure_max]={0.0};			/**< this is the data array where witch data are put and from where the average is done. */

	double m_average;						/**< there is the result from the m_data average */

	double m_factor;							/** this is the sensor's factor. it initial value is 1, but if the sensor data need to be multiply, change the factor value. */

	double m_offset;							/** this is the sensor's offset. It initial value is zero. Change it if you want add an offset to the sensor's data. */
};

#endif /* ANEMOMETER_H_ */

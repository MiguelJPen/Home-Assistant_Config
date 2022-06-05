#include "esphome.h"

class BarrierSensor : public PollingComponent, public BinarySensor {
public:
	const int PINVector[6] = { 32, 33, 34, 35, 36, 39};
	BinarySensor *Barriers[6] { new BinarySensor(), new BinarySensor(), new BinarySensor(), new BinarySensor(), new BinarySensor(), new BinarySensor()};
	BinarySensor *Tampers[6] { new BinarySensor(), new BinarySensor(), new BinarySensor(), new BinarySensor(), new BinarySensor(), new BinarySensor()};
	// This two is for no updating in every loop
	// false: free barrier, true: cannot see each other;
	bool barr[6] = {false, false, false, false, false, false};
	// false: tamper closed, true: tamper opened;
	bool tamp[6] = {false, false, false, false, false, false};

	// Constructor
	BarrierSensor() : PollingComponent(125) { ; }; // Update time

/* ADC readings v voltage
 *  y = -0.000000000009824x3 + 0.000000016557283x2 + 0.000854596860691x + 0.065440348345433
 // Polynomial curve match, based on raw data thus:
 *   464     0.5
 *  1088     1.0
 *  1707     1.5
 *  2331     2.0
 *  2951     2.5 
 *  3775     3.0
 *  
 */

	void setup() override { 
	  analogSetAttenuation(ADC_11db); // To read up to 3.9V in ESP32 (1V = 1088)
	}

	void update() override {
		int value;
		
		for (int i = 0; i < 6; ++i){
			value = analogRead(PINVector[i]);
			if (value <= 200 && tamp[i] == false){ // If the voltage is less than 0.5V
			    delay(50);
			    value = analogRead(PINVector[i]);
			    if (value <= 1024) {
    				tamp[i] = true;
    				Tampers[i]->publish_state(tamp[i]);
				}
			}
			else if (value > 200 && value <= 2100 && barr[i] == false){ // If the voltage is 0.5V < v < 2V
				if (tamp[i] == true){
					tamp[i] = false;
					Tampers[i]->publish_state(tamp[i]);
				}
				delay(50);
			    value = analogRead(PINVector[i]);
			    if (value > 1024 && value <= 2000) {
    				barr[i] = true;
    				Barriers[i]->publish_state(barr[i]);
				}
			}
			else if (value > 2100 && barr[i] == true){ // If the voltage is over 2V
				if (tamp[i] == true){
					tamp[i] = false;
					Tampers[i]->publish_state(tamp[i]);
				}
    			barr[i] = false;
    			Barriers[i]->publish_state(barr[i]);
			}
	}
  }
};
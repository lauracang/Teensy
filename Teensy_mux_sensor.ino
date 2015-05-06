// This code is for a fabric grid sensor of given dimensions.
// 'GROUND_SIZE' and 'POWER_SIZE' must be specified as the grid rows
// and columns

// define grid size (GROUND_SIZE is ground-only side, POWER_SIZE is
// analog - VCC split side)

const int POWER_SIZE = 16;
const int GROUND_SIZE = 16;

// sensor_sample_t encapsulates a sensor sample
typedef struct {
	uint32_t header;
	uint32_t time;
	uint16_t values[POWER_SIZE][GROUND_SIZE];
	uint8_t checksum;
} sensor_sample_t;

// forward declarations
void sample_grid(sensor_sample_t *buf);
void set_pin_group1(int pin0, uint8_t val);
void set_pin_group2(int pin0, uint8_t val);

/*

Initialize the program.

1. The serial driver is confugred with 115200 baud rate.
2. Pin 0 is configured as input for ADC.
3. Pins 1-4 are configured for output select for the power mux.
4. Pins 5-8 are configured for output select for the ground mux.
5. Pins 9-10 are set to output low for the NEN signal on the muxes.

*/
void setup() {

	// start serial driver
	Serial.begin(115200);

	// set ADC A0 pin to input mode
	pinMode(A1, INPUT);
	digitalWrite(A1, LOW);
	// set mux1 channel select pins to output (see schematic)
        pinMode(2, OUTPUT);
	pinMode(A0, OUTPUT);
	pinMode(7, OUTPUT);
	pinMode(8, OUTPUT);
	// set mux2 channel select pins to output
	pinMode(6, OUTPUT);
	pinMode(A6, OUTPUT);
	pinMode(A7, OUTPUT);
	pinMode(5, OUTPUT);
	// set enable pins A9-10 pins to output
	pinMode(9, OUTPUT);
	pinMode(10, OUTPUT);
	digitalWrite(9, LOW);
	digitalWrite(10, LOW);
}

/*

Take samples and send results over serial.

*/
void loop() {

	// allocate static buffer
	static sensor_sample_t buf = { 0xffffffff };

	// sample the grid
	sample_grid(&buf);

	// send data
	Serial.write((uint8_t *)&buf, sizeof(buf));

	// http://arduino.cc/en/Serial/Flush
	// don't wait for transmission to finish
	// /* Serial.flush(); */

	// Optional wait to slow data collection
	 delay(100); // delay 100 ms
}

/*

Sample the pressure grid.

1. Save the current time.
2. Select the outputs on the muxes.
3. Sample the output voltage.
4. Calculate the checksum.

@param buf The sensor sample structure to save data to

*/
void sample_grid(sensor_sample_t *buf) {

	// save current time, truncated to 32-bits
	buf->time = (uint32_t)millis();
	// reset checksum
	buf->checksum = buf->time;

	// sample sensors
	for (uint8_t x = 0; x < POWER_SIZE; x++) {

		// select power mux output channel
		set_pin_group1(2, x & 0x07);

		for (uint8_t y = 0; y < GROUND_SIZE; y++) {

			// select ground mux output channel
			set_pin_group2(6, y & 0x07);

			// sample voltage
			buf->values[x][y] = analogRead(A1);

			// update checksum
			buf->checksum += buf->values[x][y];
		}
	}

	// all done
}

/*

Set the value of a group of 3 pins.

@param pin0 The first pin number
@param val A binary-encoded value to output

*/
void set_pin_group1(int pin0, uint8_t val) {
	digitalWrite(pin0, (val & 0x01) ? HIGH : LOW);
	digitalWrite(A0, (val & 0x02) ? HIGH : LOW);
	digitalWrite(7, (val & 0x04) ? HIGH : LOW);
	digitalWrite(8, (val & 0x08) ? HIGH : LOW);
}

void set_pin_group2(int pin0, uint8_t val) {
	digitalWrite(pin0, (val & 0x01) ? HIGH : LOW);
	digitalWrite(A6, (val & 0x02) ? HIGH : LOW);
	digitalWrite(A7, (val & 0x04) ? HIGH : LOW);
	digitalWrite(5, (val & 0x08) ? HIGH : LOW);
}

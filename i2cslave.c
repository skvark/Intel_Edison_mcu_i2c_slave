#include "i2cslave.h"
#include "mcu_api.h"

unsigned char DATA_REGISTER;

void init() {
	gpio_setup(SDAGPIO, 0);
	gpio_setup(SCLGPIO, 0);
	gpio_register_interrupt(SDAGPIO, 0, int_SDA);
}

int SDA() {
	return gpio_read(SDAGPIO);
}

int SCL() {
	return gpio_read(SCLGPIO);
}

void drive_SDA_high() {
	gpio_write(SDAGPIO, 1);
}

void drive_SDA_low() {
	gpio_write(SDAGPIO, 0);
}

/**
* Interrupt handler for SDA falling edge.
*
* Checks if start condition is present and
* either exits the ISR or continues to receive
* the address.
*
* After that send or receives according to the R/W bit.
*/
int int_SDA()
{
	char retval;

	// SDA low and SCL is high == start condition

	if (SCL()) {
		retval = I2C_START_DETECTED;
		debug_print(DBG_INFO, "start!\n");
	} else {
		return IRQ_HANDLED;
	}

	while(retval == I2C_START_DETECTED) {

		retval = read_slave_byte();  // read address byte

		if ((retval & 0xfffe) != I2C_SLAVE_ADDR) {
			break;  // Wrong address or stop
		}

		// Receive or send data according to the R/W bit of the address byte
		if (retval & 1) {
			debug_print(DBG_INFO, "send\n");
			retval = send_data();
		} else {
			debug_print(DBG_INFO, "receive\n");
			retval = receive_data();
		}

	}
	return IRQ_HANDLED;
}

/**
* Called always after start.
* Reads the address byte and sends ACK if address matches.
* @param addrMatch
*/
char read_slave_byte() {

	int i;
	char value = 0;

	for (i = 0; i < 8; ++i) {

		debug_print(DBG_INFO, "scl 0\n");
		while (!SCL());
		debug_print(DBG_INFO, "scl 1\n");
		value = (value << 1) | SDA();

		while (SCL()) {

			// If SDA changes while SCL is high, it's a
			// stop (low to high) or start (high to low) condition.
			if ((value & 1) != SDA())  {

				if (SDA()) {
					debug_print(DBG_INFO, "stop\n");
					return I2C_STOP_DETECTED;
				} else {
					debug_print(DBG_INFO, "start\n");
					return I2C_START_DETECTED;
				}

			}
		}
	}

	// Send ACK
	if ((value & 0xfe) == I2C_SLAVE_ADDR) {
		debug_print(DBG_INFO, "ACK\n");
		drive_SDA_low();
		while (!SCL());
		while (SCL());
		drive_SDA_high();

	// Address is not right
	} else {
		debug_print(DBG_INFO, "addr mismatch\n");
		return I2C_ADDR_MISMATCH;
	}

	return value;
}

/**
* Send data if master has issued read
*/
int send_data() {

	int i;
	for(i = 0; i < 8; ++i) {

		while(SCL());

		if((DATA_REGISTER >> (7 - i)) & 1) {
			drive_SDA_high();
		} else {
			drive_SDA_low();
		}
		while(!SCL());
	}

	while(SCL());
	drive_SDA_high();
	while(!SCL());

	if(!SDA()) {
		return I2C_START_DETECTED;
	} else {
		return I2C_STOP_DETECTED;
	}
}

/**
* Receive data if master has issued write
*/
int receive_data() {

	int retval = 0;

	int i;
	for (i = 0; i < 8; ++i) {

		while (!SCL());
		DATA_REGISTER = (DATA_REGISTER << 1) | SDA();

		while (SCL()) {

			if ((DATA_REGISTER & 1) != SDA())  {
				if (SDA()) {
					return I2C_STOP_DETECTED;
				} else {
					return I2C_START_DETECTED;
				}
			}
		}
	}

	// send ACK

	drive_SDA_low();
	while (!SCL());
	while (SCL());
	drive_SDA_high();

	return I2C_START_DETECTED;

}


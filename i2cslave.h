#ifndef I2CSLAVE_H_
#define I2CSLAVE_H_

// MRAA pin 7 == GP20 == GPIO-20 == I2C-1-SDA
// MRAA pin 19 == GP19 == GPIO-19 == I2C-1-SCL
#define SDAGPIO 20
#define SCLGPIO 19

#define I2C_SLAVE_ADDR 0x25
#define I2C_START_DETECTED 0x01
#define I2C_STOP_DETECTED 0x02
#define I2C_ADDR_MISMATCH 0x03

void init();

int SDA();
int SCL();

void drive_SDA_high();
void drive_SDA_low();

int int_SDA();
char read_slave_byte();

int send_data();
int receive_data();


#endif /* I2CSLAVE_H_ */

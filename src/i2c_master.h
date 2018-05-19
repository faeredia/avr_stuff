
//https://github.com/g4lvanix/I2C-master-lib
/*
extended somewhat by myself.

incoming device addresses should ALWAYS be 7bit address
The library will handle left shift and read/write bit sets.

call i2c_addr8_to_addr7() to get a 7 bit address if the ds only gives you an 8
(can be either the read or write address, this just right shifts by one bit.)

The old i2c_readRegxx and i2c_writeRegxx are kept for compatibility.
will be depracated shortly.
the i2c_read_xx and i2c_write_xx are the easiest way to read and write to
a device if you know the data size you are receiving/sending.

*/

#ifndef I2C_MASTER_H
#define I2C_MASTER_H

#define I2C_READ 0x01
#define I2C_WRITE 0x00

uint8_t i2c_init(void);
uint8_t i2c_addr7_to_addr8(uint8_t addr7);
uint8_t i2c_start(uint8_t address);
uint8_t i2c_write(uint8_t data);
uint8_t i2c_read_ack(void);
uint8_t i2c_read_nack(void);
uint8_t i2c_transmit(uint8_t address, uint8_t* data, uint16_t length);
uint8_t i2c_receive(uint8_t address, uint8_t* data, uint16_t length);
uint8_t i2c_writeReg(uint8_t devaddr, uint8_t regaddr, uint8_t* data, uint16_t length);
uint8_t i2c_readReg(uint8_t devaddr, uint8_t regaddr, uint8_t* data, uint16_t length);
void i2c_stop(void);

uint8_t i2c_write8(uint8_t devaddr, uint8_t regaddr, uint8_t data);

uint8_t i2c_read8(uint8_t devaddr, uint8_t regaddr);
uint16_t i2c_read16(uint8_t devaddr, uint8_t regaddr);


#endif // I2C_MASTER_H
Some random stuff for avr  
  
## Libraries in the src/ folder:  
    uart        -   Initialising a uart connection with avr  
    i2c_master  -   For communicating over TWI/I2C  
    libBMP180   -   Used with i2c_master for reading pressure and temperature from a Bosch BMP180 sensor  
  
weather.c is a basic demonstration for using these libraries.  
  
## Makefile
This can be reused elsewhere.  
Some useful commands:  
    make all        -   Compile the program with avr-gcc  
    make upload     -   Uploads the compiled .hex to arduino on /dev/ttyACM0  
    make listen     -   Use picocom to listen over uart  
    make log        -   pipes arudino uart stream to data.csv  
    make clean      -   Tidy the project  
  
Chain make commands together for a good one liner:
    make all upload listen
  
## Scripts
Useful scripts  
    reset.py        -   Easy way to reset the attached arduino (used when calling)  

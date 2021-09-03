//build_flags = -D USB_AUDIO

#include <Arduino.h>
#include <SPI.h>

//#define LOWEST_CONVST_DURATION 20 // from datasheet, 20ns

const int clockSPIspeed = 37500000; //37.5Mhz (1/16 of 600MHz)

//Teensy 4.0 pins definition
const byte SCKx = 13;
const byte MISOx = 9;
const byte CONVSTx = 10; //  CS/CONVST/CNV
const byte SDOx = 11; //ADC data ready flag (low)

//Sound definition
int SampleFrequency = 500000; //500kHz

//Variable
bool data_ready = 0;
short int byte_adc[2] = {0,0};
short int ADCValue = (short)0;

void busy_trigger() //ADC have finished
{
	data_ready = 1;
}

void SPIreadRegister(short *byte_adc, short ADCValue) {
// send a value of 0 to read the next byte returned:
//detachInterrupt(digitalPinToInterrupt(SDO), busy_trigger, FALLING); //attach busy interrupt, FALLING
detachInterrupt(digitalPinToInterrupt(SDOx)); //attach busy interrupt, FALLING
byte_adc[0] = SPI.transfer(0x00); //Read MSB
byte_adc[1] = SPI.transfer(0x00); //Read LSB

// convert to 16 bit
ADCValue = byte_adc[0] << 8 | byte_adc[1];
attachInterrupt(digitalPinToInterrupt(SDOx), busy_trigger, FALLING); //attach busy interrupt, FALLING
}

void setup() 
{
	//Set pin direction
	pinMode(SCKx,OUTPUT);
	pinMode(MISOx,INPUT);
	pinMode(CONVSTx,OUTPUT);
	pinMode(SDOx,INPUT);
	
	
	//Set the signal to trigger the ADC	
	analogWriteFrequency (CONVSTx, SampleFrequency) ;
	analogWrite (CONVSTx, 3) ;  //Becarefull, have to be high for 20ns at least. 7.6 from https://www.ti.com/lit/ds/symlink/ads8319.pdf
	  

	//Set SPI 
	SPI.setMISO(MISOx);
	SPI.setSCK(SCKx);
	SPI.begin();
	SPI.beginTransaction(SPISettings(clockSPIspeed, MSBFIRST, SPI_MODE0));


	//Set interrupt ADC finished
	attachInterrupt(digitalPinToInterrupt(SDOx), busy_trigger, FALLING); //attach busy interrupt, FALLING
	
	
	//Serial com
	Serial.begin(115200);
}
  
void loop() {
	if(data_ready == 1)
	{
		SPIreadRegister(byte_adc, ADCValue); //calls SPI read function
		Serial.println(ADCValue); //increment data counter
		data_ready = 0; //reset read flag
	}
}

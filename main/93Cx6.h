/*
  93Cx6.h - Library for the Three-wire Serial EEPROM chip
*/
#ifndef _93CX6_H_
#define _93CX6_H_

#define EEPROM_MODE_UNKNOWN    0
#define EEPROM_MODE_8BIT	1
#define EEPROM_MODE_16BIT	 2

#define    LOW			  0
#define    HIGH			   1

typedef struct
{
	int16_t _CS;
	int16_t _SK;
	int16_t _DI;
	int16_t _DO;
	bool _ew;
	int16_t _org;		 // eeprom Organization(8bit/16bit)
	int16_t _model;		   // eeprom model(46/56/66/76/86)
	int16_t _bytes;		   // eeprom memory size(byte)
	int16_t _addr;		  // number of address bit
	uint16_t _mask;    // address mask
} EEPROM_T;


int eeprom_open(EEPROM_T * dev, int16_t model, int16_t org, int16_t GPIO_CS, int16_t GPIO_SK, int16_t GPIO_DI, int16_t GPIO_DO);
void eeprom_ew_disable(EEPROM_T * dev);
void eeprom_ew_enable(EEPROM_T * dev);
bool eeprom_is_ew_enabled(EEPROM_T * dev);
void eeprom_erase_all(EEPROM_T * dev);
void eeprom_erase(EEPROM_T * dev, uint16_t addr);
void eeprom_write_all(EEPROM_T * dev, uint16_t value);
void eeprom_write(EEPROM_T * dev, uint16_t addr, uint16_t value);
uint16_t eeprom_read(EEPROM_T * dev, uint16_t addr);
void send_bits(EEPROM_T * dev, uint16_t value, int len);
void wait_ready(EEPROM_T * dev);
int getBytesByModel(int bit, int model);
int getAddrByModel(int bit, int model);
uint16_t getMaskByModel(int bit, int model);

#endif


/*
  93Cx6.c - Library for the Three-wire Serial EEPROM chip
*/
#include <stdio.h>
#include <unistd.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "93Cx6.h"

#define DELAY_CS	0
#define DELAY_READ	  1
#define DELAY_WRITE    1
#define DELAY_WAIT	  1

#define _DEBUG_ 0

enum OP { // Operations
	CONTROL	= 0x00,
	WRITE	= 0x01,
	READ	= 0x02,
	ERASE	= 0x03
};
enum CC { // Control Codes
	EW_DISABLE	= 0x00,
	WRITE_ALL	= 0x01,
	ERASE_ALL	= 0x02,
	EW_ENABLE	= 0x03
};

// Open Memory Device
// model:EEPROM model(46/56/66/76/86)
// org:Organization Select(1=8Bit/2=16Bit)
int eeprom_open(EEPROM_T * dev, int16_t model, int16_t org, int16_t GPIO_CS, int16_t GPIO_SK, int16_t GPIO_DI, int16_t GPIO_DO)
{
	ESP_LOGI(__FUNCTION__, "GPIO_CS=%d",GPIO_CS);
	//gpio_pad_select_gpio( GPIO_CS );
	gpio_reset_pin( GPIO_CS );
	gpio_set_direction( GPIO_CS, GPIO_MODE_OUTPUT );
	gpio_set_level( GPIO_CS, LOW );

	ESP_LOGI(__FUNCTION__, "GPIO_SK=%d",GPIO_SK);
	//gpio_pad_select_gpio( GPIO_SK );
	gpio_reset_pin( GPIO_SK );
	gpio_set_direction( GPIO_SK, GPIO_MODE_OUTPUT );
	
	ESP_LOGI(__FUNCTION__, "GPIO_DI=%d",GPIO_DI);
	//gpio_pad_select_gpio( GPIO_DI );
	gpio_reset_pin( GPIO_DI );
	gpio_set_direction( GPIO_DI, GPIO_MODE_OUTPUT );

	ESP_LOGI(__FUNCTION__, "GPIO_DO=%d",GPIO_DO);
	//gpio_pad_select_gpio( GPIO_DO );
	gpio_reset_pin( GPIO_DO );
	gpio_set_direction( GPIO_DO, GPIO_MODE_INPUT );

	dev->_CS = GPIO_CS;
	dev->_SK = GPIO_SK;
	dev->_DI = GPIO_DI;
	dev->_DO = GPIO_DO;
	dev->_ew = false;
	dev->_org = org;
	dev->_model = model;
	dev->_bytes = getBytesByModel(dev->_org, dev->_model);
	dev->_addr = getAddrByModel(dev->_org, dev->_model);
	dev->_mask = getMaskByModel(dev->_org, dev->_model);
	ESP_LOGI(__FUNCTION__, "dev->_bytes=%d dev->_addr=%d dev->_mask=%x", dev->_bytes, dev->_addr, dev->_mask);
	return dev->_bytes;
};


// Erase/Write Enable
// Required Clock Cycle : 9-13
void eeprom_ew_enable(EEPROM_T *dev)
{
	gpio_set_level(dev->_CS, HIGH);
	usleep(DELAY_CS);
	send_bits(dev, HIGH, 1);
	//uint16_t value = CONTROL<<dev->_addr | EW_ENABLE<<(dev->_addr-2);
	//ESP_LOGI(__FUNCTION__, "eeprom_ew_enable value=%04x", value);
	send_bits(dev, CONTROL<<dev->_addr | EW_ENABLE<<(dev->_addr-2), dev->_addr + 2);
	gpio_set_level(dev->_CS, LOW);
	dev->_ew = true;
};

// Erase/Write Disable
// Required Clock Cycle : 9-13
void eeprom_ew_disable(EEPROM_T *dev)
{
	gpio_set_level(dev->_CS, HIGH);
	usleep(DELAY_CS);
	send_bits(dev, HIGH, 1);
	send_bits(dev, CONTROL<<dev->_addr | EW_DISABLE<<(dev->_addr-2), dev->_addr + 2);
	gpio_set_level(dev->_CS, LOW);
	dev->_ew = false;
}

// Check Erase/Write Enable
bool eeprom_is_ew_enabled(EEPROM_T *dev)
{
	return dev->_ew;
}

// Erase All Memory
// Required Clock Cycle : 9-13
void eeprom_erase_all(EEPROM_T *dev)
{
	if(!eeprom_is_ew_enabled(dev)) return;
	gpio_set_level(dev->_CS, HIGH);
	usleep(DELAY_CS);
	send_bits(dev, HIGH, 1);
	//uint16_t value = CONTROL<<dev->_addr | ERASE_ALL<<(dev->_addr-2);
	//ESP_LOGI(__FUNCTION__, "eeprom_erase_all value=%04x", value);
	send_bits(dev, CONTROL<<dev->_addr | ERASE_ALL<<(dev->_addr-2), dev->_addr + 2);
	gpio_set_level(dev->_CS, LOW);
	wait_ready(dev);
}

// Erase Byte or Word
// Required Clock Cycle : 9-13
void eeprom_erase(EEPROM_T *dev, uint16_t addr)
{
	if(!eeprom_is_ew_enabled(dev)) return;
	gpio_set_level(dev->_CS, HIGH);
	usleep(DELAY_CS);
	send_bits(dev, HIGH, 1);
	if(dev->_org == EEPROM_MODE_16BIT) {
		send_bits(dev, ERASE<<dev->_addr | (addr & dev->_mask), dev->_addr + 2);
	} else {
		send_bits(dev, ERASE<<dev->_addr | (addr & dev->_mask), dev->_addr + 2);
	}
	
	gpio_set_level(dev->_CS, LOW);
	wait_ready(dev);
}


// Write All Memory with same Data
// Required Clock Cycle : 25-29
void eeprom_write_all(EEPROM_T *dev, uint16_t value)
{
	if(!eeprom_is_ew_enabled(dev)) return;
	gpio_set_level(dev->_CS, HIGH);
	usleep(DELAY_CS);
	send_bits(dev, HIGH, 1);
	send_bits(dev, CONTROL<<dev->_addr | WRITE_ALL<<(dev->_addr-2), dev->_addr + 2);
	if(dev->_org == EEPROM_MODE_16BIT) {
		send_bits(dev, 0xFFFF & value, 16);
	} else {
		send_bits(dev, 0xFF & value, 8);
	}
	gpio_set_level(dev->_CS, LOW);
	wait_ready(dev);
}

// Write Data to Memory
// Required Clock Cycle : 25-29
void eeprom_write(EEPROM_T *dev, uint16_t addr, uint16_t value)
{
	if(!eeprom_is_ew_enabled(dev)) return;
	gpio_set_level(dev->_CS, HIGH);
	usleep(DELAY_CS);
	send_bits(dev, HIGH, 1);
	if(dev->_org == EEPROM_MODE_16BIT) {
		send_bits(dev, WRITE<<dev->_addr | (addr & dev->_mask), dev->_addr + 2);
		send_bits(dev, 0xFFFF & value, 16);
	} else {
		send_bits(dev, WRITE<<dev->_addr | (addr & dev->_mask), dev->_addr + 2);
		send_bits(dev, 0xFF & value, 8);
	}
	gpio_set_level(dev->_CS, LOW);
	wait_ready(dev);
}

// Read Data from Memory
uint16_t eeprom_read(EEPROM_T *dev, uint16_t addr)
{
	uint16_t val = 0;
	gpio_set_level(dev->_CS, HIGH);
	usleep(DELAY_CS);
	send_bits(dev, HIGH, 1);	// Start bit
	
	int amtBits;
	if(dev->_org == EEPROM_MODE_16BIT) {
		send_bits(dev, READ<<dev->_addr | (addr & dev->_mask), dev->_addr + 2);
		amtBits = 16;
	} else {
		send_bits(dev, READ<<dev->_addr | (addr & dev->_mask), dev->_addr + 2);
		amtBits = 8;
	}
	// Read bits
	for(int i = amtBits; i>0; i--) {
		gpio_set_level(dev->_SK, HIGH);
		usleep(DELAY_READ);
		int in = gpio_get_level(dev->_DO) ? 1 : 0;
		gpio_set_level(dev->_SK, LOW);
		usleep(DELAY_READ);
		val |= (in << (i-1));
	}
	gpio_set_level(dev->_CS, LOW);
	return val;
}


void send_bits(EEPROM_T *dev, uint16_t value, int len)
{
	ESP_LOGD(__FUNCTION__, "send_bits value=0x%04x len=%d",value, len);
	for(int i = len-1; i>=0; i--)
	{
		bool toSend = (value & 1<<i);
		// Send bit
		if (toSend) gpio_set_level(dev->_DI, HIGH);
		if (!toSend) gpio_set_level(dev->_DI, LOW);
		usleep(DELAY_WRITE);
		gpio_set_level(dev->_SK, HIGH);
		usleep(DELAY_WRITE);
		gpio_set_level(dev->_SK, LOW);
		usleep(DELAY_WRITE);
	}
}

void wait_ready(EEPROM_T *dev)
{
	//Wait until action is done.
	gpio_set_level(dev->_CS, HIGH);
	while(gpio_get_level(dev->_DO) != HIGH) {
		usleep(DELAY_WAIT);
		//vTaskDelay(1);
	}
	gpio_set_level(dev->_CS, LOW);
}


int getBytesByModel(int org, int model)
{
	int byte = 0;
	if (org == EEPROM_MODE_8BIT) byte = 128;
	if (org == EEPROM_MODE_16BIT) byte = 64;
	if (model == 56) byte = byte * 2;	 // 256/128
	if (model == 66) byte = byte * 4;	 // 512/256
	if (model == 76) byte = byte * 8;	 // 1024/256
	if (model == 86) byte = byte * 16;	  // 2048/1024
	return byte;
}

int getAddrByModel(int org, int model)
{
	int addr = 0;
	if (org == EEPROM_MODE_8BIT) addr = 7;
	if (org == EEPROM_MODE_16BIT) addr = 6;
	if (model == 56) addr = addr + 2;	 // 9/8
	if (model == 66) addr = addr + 2;	 // 9/8
	if (model == 76) addr = addr + 4;	 // 11/10
	if (model == 86) addr = addr + 4;	 // 11/10
	return addr;
}

uint16_t getMaskByModel(int org, int model)
{
	uint16_t mask = 0;
	if (org == EEPROM_MODE_8BIT) mask = 0x7f;
	if (org == EEPROM_MODE_16BIT) mask = 0x3f;
	if (model == 56) mask = (mask<<2) + 0x03;	 // 0x1ff/0xff
	if (model == 66) mask = (mask<<2) + 0x03;	 // 0x1ff/0xff
	if (model == 76) mask = (mask<<4) + 0x0f;	 // 0x7ff/0x3ff
	if (model == 86) mask = (mask<<4) + 0x0f;	 // 0x7ff/0x3ff
	return mask;
} 


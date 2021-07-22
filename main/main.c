#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

#include "93Cx6.h"

static const char *TAG = "MAIN";

void dump(char * title, int bits, uint16_t *dt, uint16_t n) {
	int clm = 0;
	uint16_t data;
	uint16_t saddr =0;
	uint16_t eaddr =n-1;

	printf("-------------------- [%s] --------------------\n", title);
	uint16_t addr;
	for (addr = saddr; addr <= eaddr; addr++) {
		data = dt[addr];
		if (clm == 0) {
			printf("%05x: ",addr);
		}

		if (bits == 8)	printf("%02x ",data);
		if (bits == 16) printf("%04x ",data);

		clm++;
		if (bits == 8 && clm == 16) {
			printf("\n");
			clm = 0;
		}
		if (bits == 16 && clm == 8) {
			printf("\n");
			clm = 0;
		}
	}
	printf("-------------------- [%s] --------------------\n", title);
}

void org8Mode(EEPROM_T * dev, int bytes) {
	uint16_t mem_addr;
	uint16_t data;
	uint16_t rdata[128];
	int i;

	// erase/write enable
	eeprom_ew_enable(dev);
	ESP_LOGD(TAG, "eeprom_ew_enable");

	// erase all memory
	eeprom_erase_all(dev);
	ESP_LOGD(TAG, "eeprom_erase_all");

	// write same data
	eeprom_write_all(dev, 0x00);
	ESP_LOGD(TAG, "eeprom_write_all");

#if 0
	// read first blcok 8bit mode
	memset(rdata, 0, sizeof(rdata));
	for(i=0;i<128;i++) {
		mem_addr = i;
		rdata[i] = eeprom_read(dev, mem_addr);
	}
	dump("8bit:address 0-128", 8, rdata, 128);
#endif

	// write first blcok 8bit mode
	for(i=0;i<128;i++) {
		mem_addr = i;
		data = i;
		eeprom_write(dev, mem_addr, data);
	}

	// read first blcok 8bit mode
	memset(rdata, 0, sizeof(rdata));
	for(i=0;i<128;i++) {
		mem_addr = i;
		rdata[i] = eeprom_read(dev, mem_addr);
	}
	dump("8bit:address 0-128", 8, rdata, 128);

	// write last blcok 8bit mode
	for(i=0;i<128;i++) {
		mem_addr = bytes- i;
		data = 128-i;
		eeprom_write(dev, mem_addr, data);
	}

	// read last blcok 8bit mode
	memset(rdata, 0, sizeof(rdata));
	for(i=0;i<128;i++) {
		mem_addr = bytes - i;
		rdata[i] = eeprom_read(dev, mem_addr);
	}
	char title[64];
	sprintf(title,"8bit:address %d-%d", bytes-128, bytes);
	//dump("8bit:address 0-128", 8, rdata, 128);
	dump(title, 8, rdata, 128);

}

void org16Mode(EEPROM_T * dev, int bytes) {
	uint16_t mem_addr;
	uint16_t data;
	uint16_t rdata[128];
	int i;

	// erase/write enable
	eeprom_ew_enable(dev);

	// erase all memory
	eeprom_erase_all(dev);

	// write same data
	eeprom_write_all(dev, 0x00);

#if 0
	// read first blcok 16bit mode
	memset(rdata, 0, sizeof(rdata));
	for(i=0;i<64;i++) {
		mem_addr = i;
		rdata[i] = eeprom_read(dev, mem_addr);
	}
	dump("16bit:address 0-64", 16, rdata, 64);
#endif

	// write first blcok 16bit mode
	for(i=0;i<64;i++) {
		mem_addr = i;
		data = i + 0xFF00;
		eeprom_write(dev, mem_addr, data);
	}

	// read last blcok 16bit mode
	memset(rdata, 0, sizeof(rdata));
	for(i=0;i<64;i++) {
		mem_addr = i;
		rdata[i] = eeprom_read(dev, mem_addr);
	}
	dump("16bit:address 0-64", 16, rdata, 64);

	// write last blcok 16bit mode
	for(i=0;i<64;i++) {
		mem_addr = bytes - i;
		data = 0xFFFF - i;
		eeprom_write(dev, mem_addr, data);
	}

	// read last blcok 16bit mode
	memset(rdata, 0, sizeof(rdata));
	for(i=0;i<64;i++) {
		mem_addr = bytes - i;
		rdata[i] = eeprom_read(dev, mem_addr);
	}
	char title[64];
	sprintf(title,"16bit:address %d-%d", bytes-64, bytes);
	//dump("16bit:address 0-64", 16, rdata, 64);
	dump(title, 16, rdata, 64);

}


void app_main(void)
{
	// set EEPROM memory size
	int eeprom_model = 0;
#if CONFIG_93C46
	eeprom_model = 46;
#endif
#if CONFIG_93C56
	eeprom_model = 56;
#endif
#if CONFIG_93C66
	eeprom_model = 66;
#endif
#if CONFIG_93C76
	eeprom_model = 76;
#endif
#if CONFIG_93C86
	eeprom_model = 86;
#endif
	ESP_LOGI(TAG, "eeprom_model=%d",eeprom_model);

	// set EEPROM organization
	int eeprom_org;
	int bits;
#if CONFIG_ORG_BYTE
	eeprom_org = EEPROM_MODE_8BIT;
	bits = 8;
#endif
#if CONFIG_ORG_WORD
	eeprom_org = EEPROM_MODE_16BIT;
	bits = 16;
#endif

	// open device
	EEPROM_T dev;
	int eeprom_bytes = eeprom_open(&dev, eeprom_model, eeprom_org, CONFIG_CS_GPIO, CONFIG_SK_GPIO, CONFIG_DI_GPIO, CONFIG_DO_GPIO);
	if (eeprom_org == EEPROM_MODE_8BIT) {
		ESP_LOGI(TAG, "EEPROM chip=93C%02d, %dBit Organization, Total=%dBytes",eeprom_model, bits, eeprom_bytes);
	} else {
		ESP_LOGI(TAG, "EEPROM chip=93C%02d, %dBit Organization, Total=%dWords",eeprom_model, bits, eeprom_bytes);
	}

	if (eeprom_org == EEPROM_MODE_8BIT) org8Mode(&dev, eeprom_bytes);
	if (eeprom_org == EEPROM_MODE_16BIT) org16Mode(&dev, eeprom_bytes);
}



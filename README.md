# esp-idf-93Cx6
Three-Wire Serial EEPROM Driver for esp-idf.

# Configure
You have to set this config value with menuconfig.   
- CONFIG_MODEL   
- CONFIG_CS_GPIO   
- CONFIG_SK_GPIO   
- CONFIG_DI_GPIO   
- CONFIG_DO_GPIO   

![config-1](https://user-images.githubusercontent.com/6020549/79813215-fae74080-83b5-11ea-8b74-5f7e92977763.jpg)
![config-2](https://user-images.githubusercontent.com/6020549/79813213-fa4eaa00-83b5-11ea-8be8-ee6195dc1491.jpg)
![config-3](https://user-images.githubusercontent.com/6020549/79813211-f91d7d00-83b5-11ea-86ab-d44a1e82d637.jpg)
![config-4](https://user-images.githubusercontent.com/6020549/79813216-fae74080-83b5-11ea-904f-587c6cea2a1c.jpg)

# Installation for ESP32
```
git clone https://github.com/nopnop2002/esp-idf-93Cx6
cd esp-idf-93Cx6
idf.py set-target esp32
idf.py menuconfig
idf.py flash
```

# Installation for ESP32-S2
```
git clone https://github.com/nopnop2002/esp-idf-93Cx6
cd esp-idf-93Cx6
idf.py set-target esp32s2
idf.py menuconfig
idf.py flash
```

# Installation for ESP32-C3
```
git clone https://github.com/nopnop2002/esp-idf-93Cx6
cd esp-idf-93Cx6
idf.py set-target esp32c3
idf.py menuconfig
idf.py flash
```

# Interface

It look like SPI.   
But CS is ACTIVE HIGH.   
And data bit is not always 8bit.   

---

# Memory size vs Organization

The 93Cx6 memory is organized either as bytes (x8) or as words (x16).   
If Organization Select (ORG) is left unconnected (or connected to VCC) the x16 organization is selected.   
When Organization Select (ORG) is connected to Ground (VSS) the x8 organization is selected.    

|Device|Number of Bits|Number of 8-bit Bytes|Number of 16-bit Words|
|:---|:---|:---|:---|
|93C46|1024|128|64|
|93C56|2048|256|128|
|93C66|4096|512|256|
|93C76|8192|1024|512|
|93C86|16384|2048|1024|

---

# API

```
// Open Memory Device
// model:EEPROM model(46/56/66/76/86)
// org:Organization Select(1=8Bit/2=16Bit)
int eeprom_open(EEPROM_T * dev, int16_t model, int16_t org, int16_t GPIO_CS, int16_t GPIO_SK, int16_t GPIO_DI, int16_t GPIO_DO)

// Erase/Write Enable
void eeprom_ew_enable(EEPROM_T *dev)

// Erase/Write Disable
void eeprom_ew_disable(EEPROM_T *dev);

// Check Erase/Write Enable
bool eeprom_is_ew_enabled(EEPROM_T *dev);

// Erase All Memory
void eeprom_erase_all(EEPROM_T *dev);

// Erase Byte or Word
void eeprom_erase(EEPROM_T *dev, uint16_t addr)

// Write All Memory with same Data
void eeprom_write_all(EEPROM_T *dev, uint16_t value)

// Write Data to Memory
void eeprom_write(EEPROM_T *dev, uint16_t addr, uint16_t value)

// Read Data from Memory
uint16_t eeprom_read(EEPROM_T *dev, uint16_t addr)
```

---

# Wireing for x8 Organization

|93Cx6||ESP32|
|:-:|:-:|:-:|
|CS|--|GPIO12(*1)|
|SK|--|GPIO13(*1)|
|DI|--|GPIO14(*1)|
|DO|--|GPIO15(*1)(*2)|
|GND|--|GND|
|ORG|--|GND|
|DC|--|N/C|
|Vcc|--|5V(*2)|

(*1)   
You can change any GPIO using menuconfig.   

(*2)   
**It's insufficient in 3.3V Power supply.**   
**You have to supply 5V.**   
**So you have to shift level about DO line.**   

![esp32-93cx6-byte](https://user-images.githubusercontent.com/6020549/79865683-855f8c80-8416-11ea-9291-8c6f1168bf5a.jpg)

# Wireing for x16 Organization

|93Cx6||ESP32|
|:-:|:-:|:-:|
|CS|--|GPIO12(*1)|
|SK|--|GPIO13(*1)|
|DI|--|GPIO14(*1)|
|DO|--|GPIO15(*1)(*2)|
|GND|--|GND|
|ORG|--|5V(*2)|
|DC|--|N/C|
|Vcc|--|5V(*2)|

(*1)   
You can change any GPIO using menuconfig.   


(*2)   
**It's insufficient in 3.3V Power supply.**   
**You have to supply 5V.**   
**So you have to shift level about DO line.**   

![esp32-93cx6-word](https://user-images.githubusercontent.com/6020549/79865692-885a7d00-8416-11ea-95f6-ebd20640af7c.jpg)

---

# 93C46
![93c46-bytes](https://user-images.githubusercontent.com/6020549/79813724-67af0a80-83b7-11ea-9298-2411dc440067.jpg)

![93c46-word](https://user-images.githubusercontent.com/6020549/79813729-6978ce00-83b7-11ea-99a8-89958a521215.jpg)

# 93C56
![93c56-bytes](https://user-images.githubusercontent.com/6020549/79813752-7990ad80-83b7-11ea-9454-6c0f213855d0.jpg)

![93c56-word](https://user-images.githubusercontent.com/6020549/79813759-7bf30780-83b7-11ea-9c88-1dcf4a2bdb66.jpg)

# 93C66
![93c66-bytes](https://user-images.githubusercontent.com/6020549/79813777-8ca37d80-83b7-11ea-9c6d-48368bb2b6e2.jpg)

![93c66-word](https://user-images.githubusercontent.com/6020549/79813786-90370480-83b7-11ea-94d2-c9794da5e51a.jpg)

# 93C86
![93c86-bytes](https://user-images.githubusercontent.com/6020549/79813799-99c06c80-83b7-11ea-9ef5-4c3d2689dda5.jpg)

![93c86-word](https://user-images.githubusercontent.com/6020549/79813802-9c22c680-83b7-11ea-8021-bd04add299f9.jpg)

---
# Note
esp-idf provides a similar sample [here](https://github.com/espressif/esp-idf/tree/master/examples/peripherals/spi_master/hd_eeprom).   
But it only supports 93c46.   


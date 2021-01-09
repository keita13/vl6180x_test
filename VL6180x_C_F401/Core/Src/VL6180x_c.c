/*
 * VL6180x_c.c
 *
 *  Created on: Jan 7, 2021
 *      Author: keita
 */

#include "VL6180x_c.h"

void VL6180x_init(VL6180x* DataStruct,I2C_HandleTypeDef hi2c,uint8_t address) {

	//Input I2C setting
	DataStruct->hi2cx=hi2c;
	DataStruct->address=address;

	//initialize variable
	DataStruct->data=0;
	DataStruct->data_16=0;
	DataStruct->distance=0;
	DataStruct->alsCalculated=0.0f;

    /* REGISTER_TUNING_SR03_270514_CustomerView.txt */
	VL6180x_setRegister(DataStruct,VL6180X_SYSTEM_FRESH_OUT_OF_RESET, 0x00);
    VL6180x_setRegister(DataStruct,0x0207, 0x01);
    VL6180x_setRegister(DataStruct,0x0208, 0x01);
    VL6180x_setRegister(DataStruct,0x0096, 0x00);
    VL6180x_setRegister(DataStruct,0x0097, 0xfd);
    VL6180x_setRegister(DataStruct,0x00e3, 0x00);
    VL6180x_setRegister(DataStruct,0x00e4, 0x04);
    VL6180x_setRegister(DataStruct,0x00e5, 0x02);
    VL6180x_setRegister(DataStruct,0x00e6, 0x01);
    VL6180x_setRegister(DataStruct,0x00e7, 0x03);
    VL6180x_setRegister(DataStruct,0x00f5, 0x02);
    VL6180x_setRegister(DataStruct,0x00d9, 0x05);
    VL6180x_setRegister(DataStruct,0x00db, 0xce);
    VL6180x_setRegister(DataStruct,0x00dc, 0x03);
    VL6180x_setRegister(DataStruct,0x00dd, 0xf8);
    VL6180x_setRegister(DataStruct,0x009f, 0x00);
    VL6180x_setRegister(DataStruct,0x00a3, 0x3c);
    VL6180x_setRegister(DataStruct,0x00b7, 0x00);
    VL6180x_setRegister(DataStruct,0x00bb, 0x3c);
    VL6180x_setRegister(DataStruct,0x00b2, 0x09);
    VL6180x_setRegister(DataStruct,0x00ca, 0x09);
    VL6180x_setRegister(DataStruct,0x0198, 0x01);
    VL6180x_setRegister(DataStruct,0x01b0, 0x17);
    VL6180x_setRegister(DataStruct,0x01ad, 0x00);
    VL6180x_setRegister(DataStruct,0x00ff, 0x05);
    VL6180x_setRegister(DataStruct,0x0100, 0x05);
    VL6180x_setRegister(DataStruct,0x0199, 0x05);
    VL6180x_setRegister(DataStruct,0x01a6, 0x1b);
    VL6180x_setRegister(DataStruct,0x01ac, 0x3e);
    VL6180x_setRegister(DataStruct,0x01a7, 0x1f);
    VL6180x_setRegister(DataStruct,0x0030, 0x00);

    /* Recommended : Public registers - See data sheet for more detail */
    VL6180x_setRegister(DataStruct,0x0011, 0x10); /* Enables polling for New Sample ready when measurement completes */
    VL6180x_setRegister(DataStruct,0x010a, 0x30);/* Set the averaging sample period (compromise between lower noise and increased execution time) */
    VL6180x_setRegister(DataStruct,0x003f, 0x46);/* Sets the light and dark gain (upper nibble). Dark gain should not be changed.*/
    VL6180x_setRegister(DataStruct,0x0031, 0xFF);/* sets the # of range measurements after which auto calibration of system is performed */
    VL6180x_setRegister(DataStruct,0x0040, 0x63);/* Set ALS integration time to 100ms */
    VL6180x_setRegister(DataStruct,0x002e, 0x01);/* perform a single temperature calibration of the ranging sensor */

    /* Optional: Public registers - See data sheet for more detail */
    VL6180x_setRegister(DataStruct,0x001b, 0x09); /* Set default ranging inter-measurement period to 100ms */
    VL6180x_setRegister(DataStruct,0x003e, 0x31); /* Set default ALS inter-measurement period to 500ms */
    VL6180x_setRegister(DataStruct,0x0014, 0x24); /* Configures interrupt on New sample ready */

}

void getIdentification(VL6180x* DataStruct,VL6180xIdentification *temp){

  VL6180x_getRegister(DataStruct,VL6180X_IDENTIFICATION_MODEL_ID);
  temp->idModel = DataStruct->data;

  VL6180x_getRegister(DataStruct,VL6180X_IDENTIFICATION_MODEL_REV_MAJOR);
  temp->idModelRevMajor =DataStruct->data;

  VL6180x_getRegister(DataStruct,VL6180X_IDENTIFICATION_MODEL_REV_MINOR);
  temp->idModelRevMinor =DataStruct->data;

  VL6180x_getRegister(DataStruct,VL6180X_IDENTIFICATION_MODULE_REV_MAJOR);
  temp->idModuleRevMajor =DataStruct->data;
  VL6180x_getRegister(DataStruct,VL6180X_IDENTIFICATION_MODULE_REV_MINOR);
  temp->idModuleRevMinor =DataStruct->data;

  VL6180x_getRegister16bit(DataStruct,VL6180X_IDENTIFICATION_DATE_HI);
  temp->idDate =DataStruct->data_16;
  VL6180x_getRegister16bit(DataStruct,VL6180X_IDENTIFICATION_TIME_1);
  temp->idTime =DataStruct->data_16;
}

uint8_t changeAddress(VL6180x* DataStruct, uint8_t new_address){

  //NOTICE:  IT APPEARS THAT CHANGING THE ADDRESS IS NOT STORED IN NON-VOLATILE MEMORY
  // POWER CYCLING THE DEVICE REVERTS ADDRESS BACK TO 0X29

  if( DataStruct->address == new_address) return DataStruct->address;
  if( new_address > 127) return DataStruct->address;

   VL6180x_setRegister(DataStruct,VL6180X_I2C_SLAVE_DEVICE_ADDRESS, new_address);
   DataStruct->address=new_address<<1;
   VL6180x_getRegister(DataStruct,VL6180X_I2C_SLAVE_DEVICE_ADDRESS);
   return DataStruct->data;
}


void getDistance(VL6180x* DataStruct) {

  VL6180x_setRegister(DataStruct,VL6180X_SYSRANGE_START, 0x01); //Start Single shot mode
  HAL_Delay(10);
  VL6180x_getRegister(DataStruct,VL6180X_RESULT_RANGE_VAL);
  DataStruct->distance = DataStruct->data;
  VL6180x_setRegister(DataStruct,VL6180X_SYSTEM_INTERRUPT_CLEAR, 0x07);

}

void getAmbientLight(VL6180x* DataStruct,vl6180x_als_gain VL6180X_ALS_GAIN)
{
  //First load in Gain we are using, do it everytime incase someone changes it on us.
  //Note: Upper nibble shoudl be set to 0x4 i.e. for ALS gain of 1.0 write 0x46
  VL6180x_setRegister(DataStruct,VL6180X_SYSALS_ANALOGUE_GAIN, (0x40 | VL6180X_ALS_GAIN)); // Set the ALS gain

  //Start ALS Measurement
  VL6180x_setRegister(DataStruct,VL6180X_SYSALS_START, 0x01);

    //wait_ms(100); //give it time...
  HAL_Delay(100);
  VL6180x_setRegister(DataStruct,VL6180X_SYSTEM_INTERRUPT_CLEAR, 0x07);

  //Retrieve the Raw ALS value from the sensoe
  VL6180x_getRegister16bit(DataStruct,VL6180X_RESULT_ALS_VAL);
  unsigned int alsRaw = DataStruct->data_16;

  //Get Integration Period for calculation, we do this everytime incase someone changes it on us.
  VL6180x_getRegister16bit(DataStruct,VL6180X_SYSALS_INTEGRATION_PERIOD);
  unsigned int alsIntegrationPeriodRaw = DataStruct->data_16;
  float alsIntegrationPeriod = 100.0 / alsIntegrationPeriodRaw ;

  //Calculate actual LUX from Appnotes

  float alsGain = 0.0;

  switch (VL6180X_ALS_GAIN){
    case GAIN_20: alsGain = 20.0; break;
    case GAIN_10: alsGain = 10.32; break;
    case GAIN_5: alsGain = 5.21; break;
    case GAIN_2_5: alsGain = 2.60; break;
    case GAIN_1_67: alsGain = 1.72; break;
    case GAIN_1_25: alsGain = 1.28; break;
    case GAIN_1: alsGain = 1.01; break;
    case GAIN_40: alsGain = 40.0; break;
  }

//Calculate LUX from formula in AppNotes

  DataStruct->alsCalculated = (float)0.32 * ((float)alsRaw / alsGain) * alsIntegrationPeriod;

}


inline void VL6180x_getRegister(VL6180x* DataStruct,uint16_t registerAddr)
{
  uint8_t data_write[2];
  uint8_t data_read[1];
  data_write[0] = (registerAddr >> 8) & 0xFF; //MSB of register address
  data_write[1] = registerAddr & 0xFF; //LSB of register address

  HAL_I2C_Master_Transmit(&DataStruct->hi2cx, DataStruct->address, data_write, 2, 300);
  HAL_I2C_Master_Receive(&DataStruct->hi2cx, DataStruct->address, data_read, 1, 300);

  DataStruct->data=data_read[0];
}

inline void VL6180x_getRegister16bit(VL6180x* DataStruct,uint16_t registerAddr)
{
  uint8_t data_low;
  uint8_t data_high;

  uint8_t data_write[2];
  uint8_t data_read[2];
  data_write[0] = (registerAddr >> 8) & 0xFF; //MSB of register address
  data_write[1] = registerAddr & 0xFF; //LSB of register address
  HAL_I2C_Master_Transmit(&DataStruct->hi2cx, DataStruct->address, data_write, 2, 300);
  HAL_I2C_Master_Receive(&DataStruct->hi2cx, DataStruct->address, data_read, 2, 300);
  data_high = data_read[0]; //Read Data from selected register
  data_low = data_read[1]; //Read Data from selected register
  DataStruct->data_16 = (data_high << 8)|data_low;

}

inline void VL6180x_setRegister(VL6180x* DataStruct,uint16_t registerAddr, uint8_t data)
{
    uint8_t data_write[3];
    data_write[0] = (registerAddr >> 8) & 0xFF; //MSB of register address
    data_write[1] = registerAddr & 0xFF; //LSB of register address
    data_write[2] = data & 0xFF;
    HAL_I2C_Master_Transmit(&DataStruct->hi2cx, DataStruct->address, data_write, 3, 300);
}

inline void VL6180x_setRegister16bit(VL6180x* DataStruct,uint16_t registerAddr, uint16_t data)
{
	uint8_t data_write[4];
    data_write[0] = (registerAddr >> 8) & 0xFF; //MSB of register address
    data_write[1] = registerAddr & 0xFF; //LSB of register address
    data_write[2] = (data >> 8) & 0xFF;
    data_write[3] = data & 0xFF;
    HAL_I2C_Master_Transmit(&DataStruct->hi2cx, DataStruct->address, data_write, 4, 300);
}


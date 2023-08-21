// SHTC3 Temp/Humidity Breakout Test
#include "Wire.h"

#include <SparkFun_SHTC3.h>
#define SHTC3_SAMPLE_PERIOD 500
#define SHTC3_MAX_CLK_HZ 1000000

SHTC3 mySHTC3;
uint32_t shtc3_timestamp  = 0;

void setup()
{
  Serial.begin(500000);

  Wire.begin();
  Wire.setClock(SHTC3_MAX_CLK_HZ);
  
  SHTC3_Status_TypeDef result = mySHTC3.begin();
  
  Serial.println("Startup");

  bool crc_ok = mySHTC3.passIDcrc;
  if( crc_ok )
  {
    Serial.print("Device ID: 0b"); 
    Serial.println(mySHTC3.ID, BIN);  
  }

  bool setting_ok = false;

  // Clock stretching, RH first, Low power mode
  setting_ok = mySHTC3.setMode(SHTC3_CMD_CSE_RHF_LPM);

  // Clock stretching, RH first, Normal power mode
//  setting_ok = mySHTC3.setMode(SHTC3_CMD_CSE_RHF_LPM);

  // Polling, RH first, Low power mode
//  setting_ok = mySHTC3.setMode(SHTC3_CMD_CSD_RHF_LPM);

  shtc3_timestamp = millis();
}

void loop()
{
  if( millis() - shtc3_timestamp >= SHTC3_SAMPLE_PERIOD )
  {
    SHTC3_Status_TypeDef result = mySHTC3.update();

    float humidity = mySHTC3.toPercent();
    float temp = mySHTC3.toDegC();

    bool humidity_ok = mySHTC3.passRHcrc;
    bool temp_ok = mySHTC3.passTcrc;

    Serial.print("RH = "); 
    Serial.print(humidity);
    Serial.print("%"); 
    
    Serial.print(", T = "); 
    Serial.print(temp);
    Serial.println(" degC"); 

    if( !humidity_ok || !temp_ok )
    {
      Serial.println("FAIL CRC");
      delay(100);
    }

    shtc3_timestamp = millis();
  }
  
}

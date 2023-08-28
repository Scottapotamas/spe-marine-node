// LSM6DS0 Breakout Test
#include "Wire.h"

#include "SparkFunLSM6DSO.h"

// Datasheet implies 400kHz is max speed, 1M seems to work fine
#define LSM6DSO_MAX_CLK_HZ 1000000

LSM6DSO myIMU;
fifoData myFifo;
float imu_temp;

void setup()
{
  Serial.begin(500000);
  pinMode( LED_BUILTIN, OUTPUT );
  digitalWrite( LED_BUILTIN, HIGH );
  
  Wire.begin();
  Wire.setClock(LSM6DSO_MAX_CLK_HZ);
  
  if( !myIMU.begin() )
  {
    while(1)
    {
      Serial.println("Could not connect to IMU.");
      Serial.println("Freezing");
      delay(500);
    }
  }

  myIMU.softwareReset();
  
  myIMU.setFifoDepth(500);
  //setTSDecimation(); // FIFO_CTRL4
  
  myIMU.setAccelBatchDataRate(417); //417Hz
  myIMU.setAccelRange(8); //+-8g
  myIMU.setAccelDataRate(125);  //12.5Hz

  myIMU.setGyroBatchDataRate(417);
  myIMU.setGyroRange(500);
  myIMU.setGyroDataRate(125);
      
  myIMU.setFifoDepth(500);  //samples

//    FIFO_MODE_DISABLED       = 0x00,
//    FIFO_MODE_STOP_WHEN_FULL = 0x01,
//    FIFO_MODE_CONT_TO_FIFO   = 0x03,
//    FIFO_MODE_BYPASS_TO_CONT = 0x04,
//    FIFO_MODE_CONTINUOUS     = 0x06,
//    FIFO_MODE_BYPASS_TO_FIFO = 0x07,
  myIMU.setFifoMode(FIFO_MODE_CONT_TO_FIFO);  
    
}

void loop()
{

//    imu_temp = myIMU.readTempC();
//    Serial.print("Temp: ");
//    Serial.println(imu_temp, 2);

  uint16_t samples_waiting = myIMU.getFifoStatus();

  if( samples_waiting )
  {
    Serial.print("FIFO check: ");
    Serial.println(samples_waiting);
  }

  for( int32_t samples = samples_waiting; samples > 0; samples-- )
  {
    myFifo = myIMU.fifoRead(); // Get the data

    if( myFifo.fifoTag == ACCELEROMETER_DATA )
    {
      Serial.println("Accel:");
      Serial.print(" X = ");
      Serial.println(myFifo.xAccel, 3);
      Serial.print(" Y = ");
      Serial.println(myFifo.yAccel, 3);
      Serial.print(" Z = ");
      Serial.println(myFifo.zAccel, 3);
    }

    if( myFifo.fifoTag == GYROSCOPE_DATA )
    {
      Serial.println("Gyro:");
      Serial.print(" X = ");
      Serial.println(myFifo.xGyro, 3);
      Serial.print(" Y = ");
      Serial.println(myFifo.yGyro, 3);
      Serial.print(" Z = ");
      Serial.println(myFifo.zGyro, 3);
    }
  }

  // When we did read something out, update status afterwards
  if( samples_waiting )
  {
    samples_waiting = myIMU.getFifoStatus();
    Serial.print("FIFO read ended with: ");
    Serial.println(samples_waiting);
    Serial.println("");
  }

  // Wait long enough to use a few FIFO slots
  delay(500);
  digitalWrite( LED_BUILTIN, !digitalRead(LED_BUILTIN) );

}

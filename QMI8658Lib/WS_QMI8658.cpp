#include "WS_QMI8658.h"

#define I2C_SDA       11
#define I2C_SCL       12

SensorQMI8658 QMI;

IMUdata Accel;
IMUdata Gyro;



void QMI8658_Init()
{
  Wire.begin(I2C_SDA, I2C_SCL);                
  //Using WIRE !!
  if (!QMI.begin(Wire, QMI8658_L_SLAVE_ADDRESS, I2C_SDA, I2C_SCL)) {
      printf("Failed to find QMI8658 - check your wiring!\r\n");
      while (1) {
          delay(1000);
      }
  }
  printf("Device ID: %x\r\n",QMI.getChipID());    // Get chip id

  QMI.configAccelerometer(
      SensorQMI8658::ACC_RANGE_4G,      // ACC_RANGE_2G / ACC_RANGE_4G / ACC_RANGE_8G / ACC_RANGE_16G
      SensorQMI8658::ACC_ODR_1000Hz,    // ACC_ODR_1000H / ACC_ODR_500Hz / ACC_ODR_250Hz / ACC_ODR_125Hz / ACC_ODR_62_5Hz / ACC_ODR_31_25Hz / ACC_ODR_LOWPOWER_128Hz / ACC_ODR_LOWPOWER_21Hz / ACC_ODR_LOWPOWER_11Hz / ACC_ODR_LOWPOWER_3Hz    
      SensorQMI8658::LPF_MODE_0,        //LPF_MODE_0 (2.66% of ODR) / LPF_MODE_1 (3.63% of ODR) / LPF_MODE_2 (5.39% of ODR) / LPF_MODE_3 (13.37% of ODR)
      true);                            // selfTest enable
  QMI.configGyroscope(
      SensorQMI8658::GYR_RANGE_64DPS,   // GYR_RANGE_16DPS / GYR_RANGE_32DPS / GYR_RANGE_64DPS / GYR_RANGE_128DPS / GYR_RANGE_256DPS / GYR_RANGE_512DPS / GYR_RANGE_1024DPS
      SensorQMI8658::GYR_ODR_896_8Hz,   // GYR_ODR_7174_4Hz / GYR_ODR_3587_2Hz / GYR_ODR_1793_6Hz / GYR_ODR_896_8Hz / GYR_ODR_448_4Hz / GYR_ODR_224_2Hz / GYR_ODR_112_1Hz / GYR_ODR_56_05Hz / GYR_ODR_28_025H
      SensorQMI8658::LPF_MODE_3,        // LPF_MODE_0 (2.66% of ODR) / LPF_MODE_1 (3.63% of ODR) / LPF_MODE_2 (5.39% of ODR) / LPF_MODE_3 (13.37% of ODR)
      true);                            // selfTest enable


  // In 6DOF mode (accelerometer and gyroscope are both enabled),
  // the output data rate is derived from the nature frequency of gyroscope
  QMI.enableGyroscope();
  QMI.enableAccelerometer();
  
  QMI.dumpCtrlRegister();               // printf register configuration information
  printf("Read data now...\r\n");
}


void QMI8658_Loop()
{
    if (QMI.getDataReady()) {
        if (QMI.getAccelerometer(Accel.x, Accel.y, Accel.z)) {
            printf("ACCEL:  %f  %f  %f\r\n",Accel.x,Accel.y,Accel.z);
        }

        if (QMI.getGyroscope(Gyro.x, Gyro.y, Gyro.z)) {
            printf("GYRO:  %f  %f  %f\r\n",Gyro.x,Gyro.y,Gyro.z);
        }
        printf("\t\t>      %lu   %.2f ℃\n", QMI.getTimestamp(), QMI.getTemperature_C());
        printf("\r\n");
    }
}

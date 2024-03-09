#include "mbed.h"

#include "VL53L0X.h"

#include <cstdlib>
#include <iostream>


 int main(void)
 {
    // Create I2C instance
    DevI2C i2c(PB_11, PB_10);

    // Create DigitalOut instance
    DigitalOut pin(PC_6);

    // Create VL53L0X instance
    VL53L0X sensor(&i2c, &pin, PC_7);

    sensor.VL53L0X_on();

    // Initialize the sensor
    if (sensor.init(NULL) != 0) {
        printf("Error initializing sensor\n");
        exit(-1);
    }
    printf("Sensor Initialized\n");

    // Prepare the sensor for operation
    if (sensor.prepare() != 0) {
        printf("Error preparing sensor\n");
        exit(-1);
    }
    printf("Sensor Prepared\n");

    
    VL53L0X_DEV dev;
    if(sensor.vl53l0x_get_device(&dev)!=0){
        printf("Error get device\n");
        exit(-1);
    }
    printf("Device Gotten\n");

    if (sensor.start_measurement(OperatingMode::range_single_shot_polling, NULL) != 0) {
        printf("Error starting measurement\n");
        exit(-1);
    }
    printf("Measurement Started\n");

    // VL53L0X_RangingMeasurementData_t measurement;
    // while(1)
    // {
    //     if (sensor.get_measurement(OperatingMode::range_single_shot_polling, &measurement) != 0) {
    //         printf("Error getting measurement\n");
    //         exit(-1);
    //     }
    //     printf("Range: %d\n", measurement.RangeMilliMeter);
    //     ThisThread::sleep_for(2000ms);
    // }
    VL53L0X_RangingMeasurementData_t measurement;
    VL53L0X_HistogramMeasurementData_t histogram;
    while(1)
    {
        // if (sensor.VL53L0X_perform_single_measurement(dev)!=0){
        //     printf("Error getting measurement\n");
        //     exit(-1);
        // }

        sensor.get_measurement(OperatingMode::range_single_shot_polling, &measurement);
        // sensor.VL53L0X_get_histogram_measurement_data(dev, &histogram);

        printf("Range: %d\n", measurement.RangeMilliMeter);
        printf("Reflectance: %d\n", measurement.SignalRateRtnMegaCps);
        printf("Ambient Light: %d\n", measurement.AmbientRateRtnMegaCps);
        ThisThread::sleep_for(2000ms);
    }
 }


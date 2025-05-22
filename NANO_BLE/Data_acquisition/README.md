# Arduino NANO BLE 33 

In libraries you can find the needs to make this scrip run: 
    - ArduinoBLE -> As default. To send data through BLE
    - Arduino_LSM9DS1 -> Modified. Now this library access to the sample rate register, so the user can choose the sample rate of the accelerometer and the gyroscope. 
                        Check:      setAccelerometerSampleRate(int sampleRate);  
                                    void setGyroscopeSampleRate(int sampleRate);   

The code used to take data is in BLE_acc_gyro_notify. 


#include <ArduinoBLE.h>
#include <Wire.h>
#include <SPI.h>
#include <Arduino_LSM9DS1.h>
//**La libreria Arduino_LSM9DS1.h ha sido modificada para poder modificar el sample rate del acelerometro y del gyroscopo**//
#define DEBUG_MODE true

#define IMU_ADDRESS 0x6B  // Dirección del chip LSM9DS1 (acc + gyro)

typedef struct {
  float acc_x, acc_y, acc_z;
  float gyro_x, gyro_y, gyro_z;
} SensorData_t;



SensorData_t SensorData;
unsigned char buffer[sizeof(SensorData_t)];
void ReadData();
void SetUpIMU(int AccSampleRate, int GyroSampleRate);
BLEService imuService("0dd54ba6-6456-4cf9-87d3-e2b3319cb5d1"); // BLE Custom generation of UUID

// BLE LED Switch Characteristic - custom 128-bit UUID, read and writable by central
//BLEByteCharacteristic accCharacteristic("0dd54ba7-6456-4cf9-87d3-e2b3319cb5d1", BLERead | BLENotify, sizeof(SensorData)); //Lectura y actualizaciones 
BLECharacteristic accCharacteristic("0dd54ba7-6456-4cf9-87d3-e2b3319cb5d1", 
                                    BLERead | BLENotify, sizeof(SensorData));

//BLEByteCharacteristic switchCharacteristic("2A57", BLERead | BLEWrite);



//BLEByteCharacteristic gyrCharacteristic("0dd54ba8-6456-4cf9-87d3-e2b3319cb5d1", BLERead | BLENotify, sizeof(float) *3); //Lectura y actualizaciones 



void setup() {
  
  Serial.begin(9600);
  Serial.println("Puerto serie iniciado...");
  
  // set LED's pin to output mode
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);         // All leds on to make enough power consumption to turn on the battery
  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDB, OUTPUT);
  digitalWrite(LEDR, LOW);
  digitalWrite(LEDG, LOW);
  digitalWrite(LEDB, LOW);

  //begin IMU initialization
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while(1);
  }
  delay(5000);
  SetUpIMU(10,10);
  delay(5000);
  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy failed!");

    while (1);
  }

  // set advertised local name and service UUID:
  BLE.setLocalName("Nano 33 BLE Sense");
  BLE.setAdvertisedService(imuService);

  // add the characteristic to the service
  imuService.addCharacteristic(accCharacteristic);
  //imuService.addCharacteristic(switchCharacteristic);

  // add service
  BLE.addService(imuService);

  // set the initial value for the characteristic:
  //switchCharacteristic.writeValue(0);

  // start advertising
  BLE.advertise();

  Serial.println("BLE LED Peripheral");
}
//char hexString[4];  // Para almacenar el valor en hexadecimal como cadena

void loop() {
  // listen for Bluetooth® Low Energy peripherals to connect:
  BLEDevice central = BLE.central();

  // if a central is connected to peripheral:
  if (central) {
    Serial.print("Connected to central: ");
    // print the central's MAC address:
    Serial.println(central.address());

    // while the central is still connected to peripheral:
    while (central.connected()) {
      if(IMU.accelerationAvailable() && IMU.gyroscopeAvailable()){
        ReadData();
        memcpy(buffer, &SensorData, sizeof(SensorData_t));
        //accCharacteristic.writeValue(buffer, sizeof(SensorData_t));
        accCharacteristic.writeValue((uint8_t*)&buffer, sizeof(SensorData_t));
       //delay(1000);
      }
  }
    Serial.print(F("Disconnected from central: "));
    Serial.println(central.address());

  }

}



void ReadData(){
  IMU.readAcceleration(SensorData.acc_x, SensorData.acc_y, SensorData.acc_z);
  IMU.readGyroscope(SensorData.gyro_x, SensorData.gyro_y, SensorData.gyro_z); 
}

void SetUpIMU(int AccSampleRate, int GyroSampleRate)
{
  IMU.setContinuousMode(); 
  IMU.setAccelerometerSampleRate(AccSampleRate);
  IMU.setGyroscopeSampleRate(GyroSampleRate);
  /*Serial.print("Acceleration Sample Rate: "); Serial.println(IMU.accelerationSampleRate());
  Serial.print("Gyroscope Sample Rate: "); Serial.println(IMU.gyroscopeSampleRate());
  Serial.println("IMU inicializado correctamente.");*/

}


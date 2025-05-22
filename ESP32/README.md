# ESP32


## 📡 Data Reception

The data reception is handled using an ESP32, programmed with PlatformIO in VSCode.

The initial plan was to receive the data via Bluetooth and store it on the computer using the FTP protocol using FreeRTOS library with the two cores of the ESP32. However, during the first test run, the data tracking failed — nearly 50% of the data was lost due to delays between reception and transmission. This happened because FTP takes approximately one second to open the server connection and send the data, while the accelerometer generates new data every 100 ms. Combined with the fact that the chosen movements last less than 2 seconds, this resulted in an unacceptable data loss rate.

To solve this, I switched the communication protocol to MQTT, which is significantly faster. I set up a local MQTT broker on my computer using Mosquitto. With this setup, data is published to the server with an average delay of only ~15 ms, eliminating data loss. It also includes a python script to take the data from the broker and save it in .csv

### 🔗 Communication Protocols

- 📁 [FTP Protocol Implementation](./FTP_protocol/)
- 📁 [MQTT Protocol Implementation](./MQTT_protocol/)

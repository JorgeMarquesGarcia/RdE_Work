// FTPSEND.cpp
#include "FTPsend.h"
#include "Estructuras.h"
#include "WIFIF.h"



FTPsend::FTPsend(const char* ftp_server, int port, const char* ftp_user, const char* ftp_pass)
    : ftp((char*)ftp_server, port, (char*)ftp_user, (char*)ftp_pass, 5000, 0) {
    // Aquí puedes inicializar otros parámetros si lo necesitas
}

void FTPsend::setup() {
    // Configuración inicial del archivo
    //Serial.println("Configurando FTP...");
    ftp.OpenConnection();
    

    // Avisar al usuario que ingrese un nombre de archivo
    Serial.println("Por favor, ingrese un nombre de archivo (sin extensión):");

    // Espera a que el usuario ingrese un nombre de archivo válido
    while (Serial.available() == 0) {
    }
    if (Serial.available()) {
        filename = Serial.readStringUntil('\n');  // Lee el nombre del archivo desde el puerto serie
        filename.trim();  // Elimina espacios innecesarios
    }

    // Si el nombre del archivo está vacío, sigue esperando
    if (filename.length() == 0) {
        Serial.println("Nombre no válido. Por favor, ingrese un nombre de archivo:");
    }
    delay(100);  // Pequeña espera para no sobrecargar el loop
    

    // Añadir la extensión .csv
    filename += ".csv";

    // Mostrar el nombre del archivo
    //Serial.print("Archivo seleccionado: ");
    //Serial.println(namefile);

    // Crear el archivo en el servidor FTP
    ftp.InitFile("Type A");
    ftp.NewFile(filename.c_str());  // Usar c_str() para convertir String a const char*
    ftp.Write("acc_x,acc_y,acc_z,gyro_x,gyro_y,gyro_z\n");
    ftp.CloseFile();
}


void FTPsend::sendData(SensorData_t sensorData) {
    char csvLine[128];
    snprintf(csvLine, sizeof(csvLine), "%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",
             sensorData.acc_x, sensorData.acc_y, sensorData.acc_z,
             sensorData.gyro_x, sensorData.gyro_y, sensorData.gyro_z);
    
    // Enviar por puerto serie lo que se va a subir al FTP
    //Serial.print("Datos a subir al FTP: ");
    //Serial.println(csvLine);
    
    ftp.OpenConnection();
    ftp.InitFile("Type A");
    ftp.AppendFile((char*)filename.c_str());
    ftp.Write(csvLine);
    ftp.CloseFile();
    Serial.print("[FTPsend]"); Serial.println("Data out");

}

void FTPsend::closeConnection() {
    ftp.CloseConnection();
}

void FTPsend::sendDataBatch(SensorData_t* data, size_t size) {
  ftp.OpenConnection();
  ftp.InitFile("Type A");
  ftp.AppendFile((char*)filename.c_str());

  for (size_t i = 0; i < size; ++i) {
    char csvLine[128];
    snprintf(csvLine, sizeof(csvLine), "%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",
             data[i].acc_x, data[i].acc_y, data[i].acc_z,
             data[i].gyro_x, data[i].gyro_y, data[i].gyro_z);
    ftp.Write(csvLine);
  }

  ftp.CloseFile();
}

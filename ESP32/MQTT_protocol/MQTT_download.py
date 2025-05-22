import paho.mqtt.client as mqtt
import time
import csv
import pandas as pd
from datetime import datetime
import json
import os

# Configura el broker y el topic
broker_address = "localhost"  # o la IP si es otro equipo
broker_port = 1883

# Nombre del topic
topic = "cf_dataset/Marta_KPU"  # Puedes cambiar el nombre del topic aquí si lo deseas

# Crear o abrir el archivo CSV con el nombre del topic
csv_file = ""
df = pd.DataFrame()
# Abre el archivo CSV y escribe el encabezado si es necesario
# with open(csv_file, mode='w', newline='') as file:
#     writer = csv.writer(file)
#     writer.writerow(['Timestamp', 'Data'])  # Escribe el encabezado

# Inicia el cliente MQTT
client = mqtt.Client()

# Callback para cuando el cliente se conecta al broker
def on_connect(client, userdata, flags, rc):
    print(f"Conectado con código: {rc}")
    client.subscribe(topic)  # Suscribirse al topic especificado

# Callback para cuando llega un mensaje
def on_message(client, userdata, msg):
    print(f"Mensaje recibido: {msg.payload.decode()}")
    global df, csv_file
    try: 
        timestamp = time.strftime("%Y-%m-%d %H:%M:%S", time.gmtime())

        raw = msg.payload.decode('utf-8')
        fixed_json = raw.replace('""', '"').strip('"')
        data = json.loads(fixed_json)
        data["timestamp"] = timestamp

        df = pd.DataFrame([data])  # Crear un DataFrame a partir del mensaje recibido

        if not csv_file:
            topic_clean = msg.topic.replace('/', '_')
            csv_file = f"C:/Users\JooRg\OneDrive\Desktop\Máster electrónica\Sem 2\[01] Redes de sensores\[04] Trabajo\{topic_clean}.csv"

        else: 
            header = not os.path.exists(csv_file)
        
        df.to_csv(csv_file, mode='a', header=header, index=False)  # Guardar el DataFrame en el CSV
        print(f"Guardado: {data} en {csv_file}")
    except json.JSONDecodeError as e:
        print(f"Error al decodificar JSON: {e}")
    except Exception as e:
        print(f"Error inesperado: {e}")
    # Abre el archivo CSV en modo append para agregar nuevos datos sin sobrescribir el archivo
    # with open(csv_file, mode='a', newline='') as file:
    #     writer = csv.writer(file)
    #     writer.writerow([timestamp, msg.payload.decode()])  # Escribe los datos en el CSV

# Configurar los callbacks
client.on_connect = on_connect
client.on_message = on_message

# Conectar al broker
client.connect(broker_address, broker_port, keepalive=60)

# Iniciar el loop de MQTT
client.loop_start()  # Esto permite que el cliente gestione la conexión y la suscripción de manera asíncrona

# Espera mientras se reciben mensajes
try:
    while True:
        time.sleep(1)  # Mantiene el programa ejecutándose
except KeyboardInterrupt:
    print("Finalizando programa")
    client.loop_stop()

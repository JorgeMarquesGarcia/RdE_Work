#include <JMarques-project-1_inferencing.h>
#include <Arduino_LSM9DS1.h>
#include <ArduinoBLE.h>
#include <Wire.h>
#include <SPI.h>

#define CONVERT_G_TO_MS2 9.80665f
#define MAX_ACCEPTED_RANGE 2.0f

static bool debug_nn = false;
static uint32_t run_inference_every_ms = 200;
static rtos::Thread inference_thread(osPriorityLow);
static float buffer[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE] = { 0 };
static float inference_buffer[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];

// BLE
BLEService eiService("0dd54ba6-6456-4cf9-87d3-e2b3319cb5d1");
BLECharacteristic predictionCharacteristic("0dd54ba7-6456-4cf9-87d3-e2b3319cb5d1",
                                           BLERead | BLENotify, 32); // 32 chars buffer

void run_inference_background();
float ei_get_sign(float number) { return (number >= 0.0) ? 1.0 : -1.0; }

void setup()
{
    Serial.begin(115200);
    while (!Serial);
    Serial.println("Edge Impulse + BLE notification");

    // IMU init
    if (!IMU.begin()) {
        Serial.println("Failed to initialize IMU!");
        while (1);
    }
    Serial.println("IMU initialized");

    if (EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME != 3) {
        Serial.println("Model incompatible with IMU (not 3-axis)");
        return;
    }

    // BLE init
    if (!BLE.begin()) {
        Serial.println("BLE failed to start!");
        while (1);
    }
    BLE.setLocalName("EI-Crossfit");
    BLE.setAdvertisedService(eiService);
    eiService.addCharacteristic(predictionCharacteristic);
    BLE.addService(eiService);
    predictionCharacteristic.writeValue("Esperando...");

    BLE.advertise();
    Serial.println("BLE device is advertising");

    inference_thread.start(mbed::callback(&run_inference_background));
}

void loop()
{
    BLE.poll();

    // Inference data acquisition loop
    uint64_t next_tick = micros() + (EI_CLASSIFIER_INTERVAL_MS * 1000);
    numpy::roll(buffer, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, -3);

    IMU.readAcceleration(
        buffer[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE - 3],
        buffer[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE - 2],
        buffer[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE - 1]);

    for (int i = 0; i < 3; i++) {
        if (fabs(buffer[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE - 3 + i]) > MAX_ACCEPTED_RANGE) {
            buffer[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE - 3 + i] =
                ei_get_sign(buffer[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE - 3 + i]) * MAX_ACCEPTED_RANGE;
        }
    }

    buffer[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE - 3] *= CONVERT_G_TO_MS2;
    buffer[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE - 2] *= CONVERT_G_TO_MS2;
    buffer[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE - 1] *= CONVERT_G_TO_MS2;

    uint64_t time_to_wait = next_tick - micros();
    delay((int)(time_to_wait / 1000));
    delayMicroseconds(time_to_wait % 1000);
}

void run_inference_background()
{
    delay((EI_CLASSIFIER_INTERVAL_MS * EI_CLASSIFIER_RAW_SAMPLE_COUNT) + 100);

    ei_classifier_smooth_t smooth;
    ei_classifier_smooth_init(&smooth, 10, 7, 0.8, 0.3);

    while (1) {
        memcpy(inference_buffer, buffer, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE * sizeof(float));

        signal_t signal;
        int err = numpy::signal_from_buffer(inference_buffer, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
        if (err != 0) {
            Serial.printf("Signal error (%d)\n", err);
            continue;
        }

        ei_impulse_result_t result = { 0 };
        err = run_classifier(&signal, &result, debug_nn);
        if (err != EI_IMPULSE_OK) {
            Serial.printf("Classifier error (%d)\n", err);
            continue;
        }

        const char *prediction = ei_classifier_smooth_update(&smooth, &result);
        Serial.print("Prediction: ");
        Serial.println(prediction);

        // Send BLE notification
        if (BLE.connected()) {
            predictionCharacteristic.writeValue(prediction);
        }

        delay(run_inference_every_ms);
    }

    ei_classifier_smooth_free(&smooth);
}

#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "WIFI AP";
const char* password = "PASSWORD";
#include <Arduino.h>
#include "FS.h"
#include "SD_MMC.h"
#include "SPI.h"
#include "es8311.h"
//#include "Audio.h"
#include "Wire.h"
#include "ESP_I2S.h"

//ESP32-S3 IO Pin define
#define SD_SCK 38
#define SD_CMD 40
#define SD_D0 39
#define SD_D1 41
#define SD_D2 48
#define SD_D3 47

//I2S IO Pin define
#define I2S_MCK 4
#define I2S_BCK 5
#define I2S_DINT 6
#define I2S_DOUT 8
#define I2S_WS 7
#define AP_ENABLE 1
#define I2C_SCL 15       
#define I2C_SDA 16      
#define I2C_SPEED 400000 

I2SClass es8311_i2s;

void driver_es8311_init(void) {
  pinMode(AP_ENABLE, OUTPUT);
  digitalWrite(AP_ENABLE, LOW);

  Wire.begin(I2C_SDA, I2C_SCL, I2C_SPEED);

  es8311_i2s.setPins(I2S_BCK, I2S_WS, I2S_DOUT, I2S_DINT, I2S_MCK);
  if (!es8311_i2s.begin(I2S_MODE_STD, 16000, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO, I2S_STD_SLOT_LEFT)) {
    Serial.println("Failed to initialize I2S bus!");
  }
}

void setup()
{
  Serial.begin(115200);
  
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  while (!Serial) {
    delay(10);
  }

  Serial.println("Start initializing the audio device...");
  driver_es8311_init();
  if (es8311_codec_init() != ESP_OK) {
    Serial.println("ES8311 init failed!");
    return;
  }
  delay(3000);
  Serial.println("Initialization completed.");
}

void loop()
{
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi lost...");
    delay(1000);
    return;
  }

  uint8_t *wav_buffer;
  size_t wav_size;

  // --- 1. RECORD ---
  Serial.println("\n--- Listening... (3s) ---");
  es8311_set_voice_mute(true); 
  delay(50);
  wav_buffer = es8311_i2s.recordWAV(3, &wav_size);
  es8311_set_voice_mute(false); 

  // --- 2. UPLOAD & DOWNLOAD ---
  Serial.println("Thinking...");
  
  HTTPClient http;
  http.begin("http://192.168.10.101:8000/chat"); // <--- DOUBLE CHECK YOUR IP
  http.addHeader("Content-Type", "audio/wav");

  int httpCode = http.POST(wav_buffer, wav_size);

  // Free the recording buffer to make room for the playback buffer
  free(wav_buffer); 

  if (httpCode == 200) {
    // Get the size of the incoming audio file
    int len = http.getSize();
    Serial.printf("Downloading response: %d bytes\n", len);

    if (len > 0) {
      // Allocate memory for the response
      // NOTE: If response is huge (>4MB), this might fail without PSRAM logic
      uint8_t* audio_resp = (uint8_t*)ps_malloc(len); 
      
      if (audio_resp != NULL) {
        // Read the stream directly into memory
        WiFiClient *stream = http.getStreamPtr();
        
        int totalRead = 0;
        while (http.connected() && (totalRead < len)) {
          size_t size = stream->available();
          if (size) {
            int c = stream->readBytes(audio_resp + totalRead, size);
            totalRead += c;
          }
          delay(1);
        }

        // --- 3. PLAYBACK ---
        Serial.println("Speaking...");
        es8311_i2s.playWAV(audio_resp, totalRead);
        
        free(audio_resp);
      } else {
        Serial.println("Error: Not enough RAM for audio response!");
      }
    }
  } else {
    Serial.printf("Error: HTTP %d\n", httpCode);
    String errorMsg = http.getString();
    Serial.println(errorMsg);
  }

  http.end();
  Serial.println("Done.");
  delay(2000);
}

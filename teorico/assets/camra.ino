#include "Arduino.h"
#include "WiFi.h"
#include "esp_camera.h"
#include "soc/soc.h"           // Disable brownout problems
#include "soc/rtc_cntl_reg.h"  // Disable brownout problems
#include "driver/rtc_io.h"
#include <LittleFS.h>
#include <FS.h>
#include <Firebase_ESP_Client.h>
// Provide the token generation process info.
#include <addons/TokenHelper.h>

// WiFi credentials
const char* ssid = "iPhone de Erick";
const char* password = "esp32cam";

// Insert Firebase project API Key
#define API_KEY "AIzaSyA-2Rq5cyLy1YOrx63tltujlcFEC4P88iE"

// Insert Authorized Email and Corresponding Password
#define USER_EMAIL "miguelcarrera2003@outlook.com"
#define USER_PASSWORD "123445"

// Insert Firebase storage bucket ID e.g bucket-name.appspot.com
#define STORAGE_BUCKET_ID "esp32-fb6da.appspot.com"

// OV2640 camera module pins (CAMERA_MODEL_AI_THINKER)
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22
#define FLASH_LED_PIN     4

boolean takeNewPhoto = true;
int photoNumber = 0; // Counter to keep track of photo numbers

// Define Firebase Data objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig configF;

void fcsUploadCallback(FCS_UploadStatusInfo info);
bool taskCompleted = false;

void capturePhotoSaveLittleFS() {
  // Turn on the flash
  digitalWrite(FLASH_LED_PIN, HIGH);

  camera_fb_t* fb = NULL;
  // Skip first 3 frames (increase/decrease number as needed).
  for (int i = 0; i < 4; i++) {
    fb = esp_camera_fb_get();
    esp_camera_fb_return(fb);
    fb = NULL;
  }
    
  // Take a new photo
  fb = esp_camera_fb_get();  
  if (!fb) {
    Serial.println("Camera capture failed");
    digitalWrite(FLASH_LED_PIN, LOW); // Turn off the flash in case of failure
    delay(1000);
    ESP.restart();
  }

  // Increment photo number and prepare file path
  photoNumber++;
  String localPath = String("/image_") + (photoNumber < 10 ? "0" : "") + (photoNumber < 100 ? "00" : "") + String(photoNumber) + ".jpg";
  
  // Photo file name
  Serial.printf("Picture file name: %s\n", localPath.c_str());
  File file = LittleFS.open(localPath.c_str(), FILE_WRITE);

  // Insert the data in the photo file
  if (!file) {
    Serial.println("Failed to open file in writing mode");
  } else {
    file.write(fb->buf, fb->len); // payload (image), payload length
    Serial.print("The picture has been saved in ");
    Serial.print(localPath);
    Serial.print(" - Size: ");
    Serial.print(fb->len);
    Serial.println(" bytes");
  }
  // Close the file
  file.close();
  esp_camera_fb_return(fb);

  // Turn off the flash
  digitalWrite(FLASH_LED_PIN, LOW);

  // Prepare for upload
  String bucketPath = String("/Interior/image_") + (photoNumber < 10 ? "0" : "") + (photoNumber < 100 ? "00" : "") + String(photoNumber) + ".jpg";
  if (Firebase.ready() && !taskCompleted) {
    taskCompleted = true;
    Serial.print("Uploading picture... ");

    if (Firebase.Storage.upload(&fbdo, STORAGE_BUCKET_ID, localPath.c_str(), mem_storage_type_flash, bucketPath.c_str(), "image/jpeg", fcsUploadCallback)) {
      Serial.printf("\nDownload URL: %s\n", fbdo.downloadURL().c_str());
    } else {
      Serial.println(fbdo.errorReason());
    }
    taskCompleted = false; // Reset for next photo
  }
}

void initWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
}

void initLittleFS() {
  if (!LittleFS.begin(true)) {
    Serial.println("An Error has occurred while mounting LittleFS");
    ESP.restart();
  } else {
    delay(500);
    Serial.println("LittleFS mounted successfully");
  }
}

void initCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode = CAMERA_GRAB_LATEST;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 15;
    config.fb_count = 1;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    ESP.restart();
  }

  // //set flip
  // sensor_t * s = esp_camera_sensor_get();
  // if (s != NULL) {
  //   s->set_vflip(s, 1); // Flip image vertically
  //   s->set_hmirror(s, 1); // Mirror image horizontally
  // }

}

void setup() {
  Serial.begin(115200);
  initWiFi();
  initLittleFS();
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  initCamera();
  
  configF.api_key = API_KEY;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  configF.token_status_callback = tokenStatusCallback;

  Firebase.begin(&configF, &auth);
  Firebase.reconnectWiFi(true);

  pinMode(FLASH_LED_PIN, OUTPUT);
  digitalWrite(FLASH_LED_PIN, LOW); // Turn off the flash
}

void loop() {
  // Simplify the loop by directly calling the capture and upload function without a conditional check
  // This is because the conditional logic will now be handled inside the fcsUploadCallback
  if (takeNewPhoto && !taskCompleted) {
    capturePhotoSaveLittleFS(); // This now directly attempts to capture and save a new photo
    // The flags are managed inside capturePhotoSaveLittleFS and fcsUploadCallback to ensure proper sequence
  }
  delay(15000); // This ensures the ESP32 doesn't hammer the Firebase with continuous upload attempts
}

// The Firebase Storage upload callback function
void fcsUploadCallback(FCS_UploadStatusInfo info) {
    if (info.status == firebase_fcs_upload_status_complete) {
        Serial.println("Upload completed\n");
        takeNewPhoto = true; // Set this to true to allow another photo to be taken
        taskCompleted = false; // Reset this to allow another upload

        // After successful upload, delete the file to save space
        String localPath = String("/image_") + (photoNumber < 10 ? "0" : "") + (photoNumber < 100 ? "00" : "") + String(photoNumber) + ".jpg";
        if (LittleFS.remove(localPath.c_str())) {
            Serial.println("File deleted successfully");
        } else {
            Serial.println("Failed to delete file");
        }
    } else if (info.status == firebase_fcs_upload_status_error) {
        Serial.printf("Upload failed, %s\n", info.errorMsg.c_str());
        takeNewPhoto = true;
        taskCompleted = false;
    }
}
#include <WebServer.h>
#include <WiFiClient.h> 
#include <ESP32Servo.h>

#include <Firebase_ESP_Client.h> 
#include "addons/TokenHelper.h"
#include "esp_camera.h"
#include "esp_wifi.h"
#include <time.h> 

#include "SPIFFS.h"
#include "WebPage.h"

#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"


#include "soc/timer_group_reg.h"
#include "soc/timer_group_struct.h"
#define PWDN_GPIO_NUM   32
#define RESET_GPIO_NUM  -1
#define XCLK_GPIO_NUM    0
#define SIOD_GPIO_NUM   26
#define SIOC_GPIO_NUM   27
#define Y9_GPIO_NUM     35
#define Y8_GPIO_NUM     34
#define Y7_GPIO_NUM     39
#define Y6_GPIO_NUM     36
#define Y5_GPIO_NUM     21
#define Y4_GPIO_NUM     19
#define Y3_GPIO_NUM     18
#define Y2_GPIO_NUM      5
#define VSYNC_GPIO_NUM  25
#define HREF_GPIO_NUM   23
#define PCLK_GPIO_NUM   22

#define timezone 1 
#define flashPin 4
#define servoYPin 14
#define servoZPin 15
#define getDocumentInterval 3000

const char *SSIDName = "ESP32Camera"; 
const char *SSIDPass = "1234567890"; 

String networkName = "";
String networkPass = "";
String userEmail = "";
String userPass = "";

int framesizeCode = 3;
int settingsLength;
int firstHash;
int secondHash;
int thirdHash;
int startIndex;
int endIndex;
int photoCount;
int photoInterval;
int currentPhotoCount;

int YAxisRotate1;
int YAxisRotate2;
int YAxisRotate = 0;
int ZAxisRotate = 90;
int ZAxisRotate1 = 90;
int ZAxisRotate2 = 90;
int videoContrast1;
int videoContrast2; 
int videoBrightness1;
int videoBrightness2;
int videoSpecialEffect1;
int videoSpecialEffect2;
int videoResolution1 = 3;
int videoResolution2 = 3;
int videoWhiteBalanceMode1;
int videoWhiteBalanceMode2;

bool flipVideo1;
bool flipVideo2;
bool mirrorVideo1;
bool mirrorVideo2;

bool photoSavingStarted; 
bool turnOnFlash;
bool photoSaving;
bool changeVideoFlip;
bool changeVideoMirror;
bool changeYAxisRotate; 
bool changeZAxisRotate; 
bool changeVideoContrast; 
bool changeVideoResolution;
bool changeVideoBrightness; 
bool changeVideoSpecialEffect; 
bool changeVideoWhiteBalanceMode;

long imageSize; 
String imageName;
String imageName1; 
String imageName2;
bool changeImageName;
int imageCountPerMinute; 

String ESP32IP; 
String contentJson;
String HTMLContent;
String settings = "-"; 

String hours;
String minutes; 
String days; 
String month;
int year; 

bool streamIPSaved; 
bool serverRunning; 

unsigned long getDocumentMillis; 
unsigned long photoIntervalMillis; 


const char header[] = "HTTP/1.1 200 OK\r\n" \
                      "Access-Control-Allow-Origin: *\r\n" \
                      "Content-Type: multipart/x-mixed-replace; boundary=123456789000000000000987654321\r\n";
const char boundary[] = "\r\n--123456789000000000000987654321\r\n";
const char ctntType[] = "Content-Type: image/jpeg\r\nContent-Length: "; 
const int hdrLength = strlen(header); 
const int bdrLength = strlen(boundary); 
const int cntLength = strlen(ctntType);
char jpegBuffer[32];
int photoSize;
SemaphoreHandle_t httpRequestMutex; 
TaskHandle_t handleStreamTask; 
TaskHandle_t getDocumentTask; 
sensor_t * camera = NULL; 
camera_fb_t * fb = NULL;
WebServer server(80); 

Servo ServoY2;
Servo ServoZ2;

Servo ServoY; 
Servo ServoZ; 

FirebaseData firebaseData; 
FirebaseAuth firebaseAuth;
FirebaseJson firebaseJson; 
FirebaseConfig firebaseConfig;

bool setupCamera() {
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

  config.frame_size = FRAMESIZE_SVGA; // (UXGA|SXGA|XGA|SVGA|VGA|CIF|QVGA|HQVGA|QQVGA)
  config.jpeg_quality = 12;
  config.fb_count = 1; 

  esp_err_t err = esp_camera_init(&config); 
  if (err != ESP_OK) { 
    Serial.printf("Đã xảy ra lỗi camera sau: 0x%x ", err);
    return false;
  }

  camera = esp_camera_sensor_get();
  if (camera->id.PID == OV3660_PID) {
    camera->set_vflip(camera, 1); 
    camera->set_brightness(camera, 1); 
    camera->set_saturation(camera, -2); 
  }
  camera->set_framesize(camera, FRAMESIZE_SVGA); 
  delay(500); 
  return true; 
}
void handleRoot() {
  HTMLContent = WebPage; 
  server.send(200, "text/html", HTMLContent);

  if (server.hasArg("ssid_name")) { 
    networkName = server.arg("ssid_name"); 
    Serial.print("Tên WiFi: " + String(networkName)); 
  }
  if (server.hasArg("ssid_pass")) {
    networkPass = server.arg("ssid_pass"); 
    Serial.print("Mật khẩu WiFi: " + String(networkPass));
  }
  if (server.hasArg("user_email")) { 
    userEmail = server.arg("user_email");
    Serial.print("Email: " + String(userEmail));
  }
  if (server.hasArg("user_pass")) { 
    userPass = server.arg("user_pass");
    Serial.println("Pass: " + String(userPass));
  }

  if (networkName != "-" and networkPass != "-" and userEmail != "-" and userPass != "-") {
    Serial.println("Các thông số vận hành được chỉ định!");
    settings = networkName + "#" + networkPass + "#" + userEmail + "#" + userPass; 
    File settingsFile = SPIFFS.open("/Settings.txt", FILE_WRITE);

    if (settingsFile) {
      Serial.println("Tệp có thông số vận hành đã được mở thành công để ghi!");
    } else {
      Serial.println("Không thể mở tệp có thông số vận hành để ghi :(");
      return;
    }
    
    if (settingsFile.print(settings)) {
      Serial.println("Tệp đã được ghi thành công!");
      server.stop(); 
      WiFi.softAPdisconnect(true); 
    } else {
      Serial.println("Không thể ghi tập tin:(");
      return;
    }

    settingsFile.close(); 
  }
}


void handleMJPEGStream(void) {
  WiFiClient client = server.client(); 

  xSemaphoreTake(httpRequestMutex, portMAX_DELAY); 
  client.write(header, hdrLength); 
  client.write(boundary, bdrLength); 
  xSemaphoreGive(httpRequestMutex); 
  while (true) {
    if (!client.connected()) break; 
    if (fb) esp_camera_fb_return(fb); 
    fb = esp_camera_fb_get();

    photoSize = fb->len; 
    xSemaphoreTake(httpRequestMutex, portMAX_DELAY);
    client.write(ctntType, cntLength);
    sprintf(jpegBuffer, "%d\r\n\r\n", photoSize);
    client.write(jpegBuffer, strlen(jpegBuffer)); 
    client.write((char *)fb->buf, photoSize);
    client.write(boundary, bdrLength); 
    xSemaphoreGive(httpRequestMutex);
  }
}


void handleNotFound() {
  HTMLContent = HandleNotFound;  
  server.send(200, "text/html", HTMLContent); 
}


void savePhotoToStorage() {
  camera->set_framesize(camera, FRAMESIZE_VGA); 
  delay(500); 
  do { 
    Serial.println("Chúng tôi nhận được một bức ảnh ...");
    digitalWrite(flashPin, turnOnFlash); 
    delay(50);
    if (fb) esp_camera_fb_return(fb); 
    fb = esp_camera_fb_get(); 
    digitalWrite(flashPin, LOW);

    File photoFile = SPIFFS.open("/Photo.jpg", FILE_WRITE); 
    if (photoFile) { 
      photoFile.write(fb->buf, fb->len);
      imageSize = photoFile.size(); 
      Serial.println("Ảnh đã lưu vào SPIFFS! \nKích thước: " + String(imageSize) + " byte");
    } else {
      Serial.println("Không lưu được ảnh vào SPIFFS");
      return;
    }

    photoFile.close(); 
  } while (imageSize == 0);

  // Thiết lập kích thước khung hình lại (định nghĩa của hàm setFramesize() phải được cung cấp)
  // setFramesize(); 
  
  do {
    time_t now = time(nullptr);
    struct tm* p_tm = localtime(&now);
    hours = String(p_tm->tm_hour + 1);
    minutes = String(p_tm->tm_min); 
    days = String(p_tm->tm_mday);
    month = String(p_tm->tm_mon + 1);
    year = p_tm->tm_year + 1900; 
  } while (year == 1900 or year == 1970); 

  if (hours.toInt() < 10) { 
    hours = "0" + hours;
  }
  if (minutes.toInt() < 10) { 
    minutes = "0" + minutes; 
  }
  if (days.toInt() < 10) {
    days = "0" + days; 
  }
  if (month.toInt() < 10) { 
    month = "0" + month; 
  }

  changeImageName = !changeImageName; 
  if (changeImageName) { 
    imageName1 = "/" + String(hours) + ":" + String(minutes) + " " + String(days) + "." + String(month) + "." + String(year) + ".jpg";
  } else {
    imageName2 = "/" + String(hours) + ":" + String(minutes) + " " + String(days) + "." + String(month) + "." + String(year) + ".jpg";
  }
  if (imageName1 == imageName2) { 
    imageCountPerMinute++; 
    imageName = "/" + String(hours) + ":" + String(minutes) + " " + String(days) + "." + String(month) + "." + String(year) + " (" + String(imageCountPerMinute) + ").jpg";
  } else {
    imageCountPerMinute = 0;
    imageName = "/" + String(hours) + ":" + String(minutes) + " " + String(days) + "." + String(month) + "." + String(year) + ".jpg";
  }
  
  Serial.println("Tiêu đề ảnh: " + String(imageName) + "\nTải ảnh lên ...");

  if (Firebase.Storage.upload(&firebaseData, "esp32-cam-9182b.appspot.com", "/Photo.jpg", mem_storage_type_flash, ((firebaseAuth.token.uid).c_str() + imageName).c_str(), "image/jpg")) {
    Serial.println("Đã tải ảnh lên!");
    
    firebaseJson.set("fields/uploadPhoto/stringValue", (firebaseData.downloadURL() + " " + imageName.substring(1, imageName.length())).c_str());
    firebaseJson.toString(contentJson); 

    if (Firebase.Firestore.patchDocument(&firebaseData, "esp32-cam-9182b", "", ("users/" + firebaseAuth.token.uid).c_str(), contentJson.c_str(), "uploadPhoto")) {
      Serial.println("Thực tế là bức ảnh đã được tải lên được lưu!");
    } else {
      Serial.println("Không lưu được ảnh đã tải lên :(");
      return;
    }
  } else {
    Serial.println("Không thể tải ảnh lên :( Gây ra: " + String(firebaseData.errorReason()));
    
    firebaseJson.set("fields/uploadPhoto/stringValue", "uploadPhotoFailed");
    firebaseJson.toString(contentJson);
   
    if (Firebase.Firestore.patchDocument(&firebaseData, "esp32-cam-9182b", "", ("users/" + firebaseAuth.token.uid).c_str(), contentJson.c_str(), "uploadPhoto")) {
      Serial.println("Thực tế là không thể lưu ảnh tải lên!");
    } else {
      Serial.println("Không lưu được, không tải được ảnh lên :(");
      return;
    }
  }

  if (SPIFFS.remove("/Photo.jpg")) { 
    Serial.println("Đã xóa ảnh khỏi SPIFFS!");
  } else {
    Serial.println("Không thể xóa ảnh khỏi SPIFFS :(");
    return;
  }
}


void setFramesize() { 
  if (framesizeCode == 0) {
    camera->set_framesize(camera, FRAMESIZE_UXGA); 
  } else if (framesizeCode == 1) {
    camera->set_framesize(camera, FRAMESIZE_SXGA);
  } else if (framesizeCode == 2) {
    camera->set_framesize(camera, FRAMESIZE_XGA); 
  } else if (framesizeCode == 3) {
    camera->set_framesize(camera, FRAMESIZE_SVGA);
  } else if (framesizeCode == 4) {
    camera->set_framesize(camera, FRAMESIZE_VGA);
  } else if (framesizeCode == 5) {
    camera->set_framesize(camera, FRAMESIZE_CIF); 
  } else if (framesizeCode == 6) {
    camera->set_framesize(camera, FRAMESIZE_QVGA);
  }
}


String getField(String firebasePayload, String fieldName, String valueType) {
  if (valueType != "booleanValue") { 
    startIndex = firebasePayload.indexOf(valueType, firebasePayload.indexOf(fieldName)) + valueType.length() + 4; 
    endIndex = firebasePayload.indexOf("\"", firebasePayload.indexOf(valueType, firebasePayload.indexOf(fieldName)) + valueType.length() + 4); 
  } else {
    startIndex = firebasePayload.indexOf(valueType, firebasePayload.indexOf(fieldName)) + valueType.length() + 3;
    endIndex = firebasePayload.indexOf(" ", firebasePayload.indexOf(valueType, firebasePayload.indexOf(fieldName)) + valueType.length() + 3);
  }
  return firebasePayload.substring(startIndex, endIndex);
}


void SoftAPConnected(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.println("Máy khách đã kết nối với điểm WiFi!");
}

void setup() {
  esp_err_t set_ps = esp_wifi_set_ps(WIFI_PS_NONE); 
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); 

  ServoY.setPeriodHertz(50);
  ServoZ.setPeriodHertz(50);
 
  ServoY2.attach(2, 500, 2000);
  ServoZ2.attach(13, 500, 2000);
  ServoY.attach(servoYPin, 500, 2000);
  ServoZ.attach(servoZPin, 500, 2000); 
  delay(400);
  ServoY.write(0); 
  ServoZ.write(90); 

  Serial.begin(115200); 
  pinMode(flashPin, OUTPUT);
  digitalWrite(flashPin, LOW); 

  if (setupCamera()) { 
    Serial.println("Máy ảnh đã được khởi chạy thành công!");
  } else {
    Serial.println("Không khởi động được máy ảnh:(");
    ESP.restart(); 
  }

  if (SPIFFS.begin(true)) {
    Serial.println("Gắn SPIFFS đã thành công!");
  } else {
    Serial.println("Không thể gắn SPIFFS :(");
    return;
  }

  if (SPIFFS.exists("/Settings.txt")) { 
    File settingsFile = SPIFFS.open("/Settings.txt");
    if (settingsFile) {
      Serial.println("Tệp có thông số vận hành đã được mở thành công để đọc!");
    } else {
      Serial.println("Không thể mở tệp có thông số vận hành để đọc:(");
      return;
    }

    while (settingsFile.available()) {
      settings = settingsFile.readString();
      Serial.println("Nội dung tập tin: " + String(settings));

      settingsLength = settings.length();
      firstHash  = settings.indexOf('#');
      secondHash = settings.indexOf('#', firstHash + 1); 
      thirdHash  = settings.indexOf('#', secondHash + 1);

      networkName = settings.substring(0, firstHash);
      networkPass = settings.substring(firstHash + 1, secondHash);
      userEmail   = settings.substring(secondHash + 1, thirdHash); 
      userPass    = settings.substring(thirdHash + 1, settingsLength);
    }

    settingsFile.close();
  } else {
    networkName = "-";
    networkPass = "-"; 
    userEmail   = "-"; 
    userPass    = "-";
  }

  Serial.println("Tên mạng Wi-Fi: "  + String(networkName));
  Serial.println("Mật khẩu mạng WiFi: "    + String(networkPass));
  Serial.println("Email: "  + String(userEmail));
  Serial.println("Mật khẩu: " + String(userPass));

  if (settings == "-") {
    Serial.println("Chế độ cài đặt thông số vận hành đã bắt đầu!");
    WiFi.mode(WIFI_AP);
    delay(250); 
    WiFi.softAP(SSIDName, SSIDPass); 
    WiFi.onEvent(SoftAPConnected, SYSTEM_EVENT_AP_STACONNECTED); 
    IPAddress ServerIP = WiFi.softAPIP();

    server.on("/", handleRoot); 
    server.begin();

    Serial.println("Điểm phát sóng WiFi được ra mắt! Mật khẩu: " + String(SSIDPass));
    Serial.print("Địa chỉ máy chủ web: "); Serial.println(ServerIP);
  }

  httpRequestMutex = xSemaphoreCreateMutex();
   xTaskCreatePinnedToCore( 
    getDocumentTaskCode,   
    "getDocumentTask",     
    10000,                
    NULL,                  
    1,                     
    &getDocumentTask,      
    0);                         
  delay(500); 
}
void handleStreamTaskCode(void * taskParameters){
  Serial.println("Chạy tác vụ gửi luồng video trên kernel: " + String(xPortGetCoreID()));
  for (;;) { 
    if (serverRunning) { 
      server.handleClient();
    }
    TIMERG0.wdt_wprotect = TIMG_WDT_WKEY_VALUE;
    TIMERG0.wdt_feed = 1;
    TIMERG0.wdt_wprotect = 0;
  }
}


void getDocumentTaskCode(void * taskParameters){
  Serial.println("Chúng tôi chạy một tác vụ để lấy tài liệu trên kernel: " + String(xPortGetCoreID()));
  for (;;) { 
    if (settings != "-") { 
      if (WiFi.status() == WL_CONNECTED) { 
        
        if (Firebase.ready() && (millis() - getDocumentMillis >= getDocumentInterval || getDocumentMillis == 0)) {
          getDocumentMillis = millis();

          xSemaphoreTake(httpRequestMutex, portMAX_DELAY);
          
          if (Firebase.Firestore.getDocument(&firebaseData, "esp32-cam-9182b", "", ("users/" + firebaseAuth.token.uid).c_str())) {
            xSemaphoreGive(httpRequestMutex);
            if (!streamIPSaved) { 
              firebaseJson.set("fields/streamIP/stringValue", ("http://" + ESP32IP + "/mjpeg").c_str());
              firebaseJson.toString(contentJson); 
              if (Firebase.Firestore.patchDocument(&firebaseData, "esp32-cam-9182b", "", ("users/" + firebaseAuth.token.uid).c_str(), contentJson.c_str(), "streamIP")) {
                streamIPSaved = true; 
                Serial.println("Địa chỉ IP của luồng video đã được lưu!");
              } else {
                Serial.println("Không lưu được địa chỉ IP của luồng video :(");
              }
            }

            if ((getField(firebaseData.payload(), "streamStarted", "integerValue")).toInt() == 1) {
              if (!serverRunning) { 
                server.on("/mjpeg", HTTP_GET, handleMJPEGStream); 
                server.onNotFound(handleNotFound); 
                server.begin(); 
                serverRunning = true; 
                Serial.println("Địa chỉ luồng video: http://" + String(ESP32IP) + "/mjpeg");
                xTaskCreatePinnedToCore(
                  handleStreamTaskCode,  
                  "handleStreamTask",    
                  2048,                  
                  NULL,                 
                  1,                     
                  &handleStreamTask,    
                  1);                    
                delay(500);
                
                firebaseJson.set("fields/streamStarted/integerValue", 2);
                firebaseJson.toString(contentJson);
                if (Firebase.Firestore.patchDocument(&firebaseData, "esp32-cam-9182b", "", ("users/" + firebaseAuth.token.uid).c_str(), contentJson.c_str(), "streamStarted")) {
                  Serial.println("Việc luồng video đang chạy được lưu lại!");
                } else {
                  Serial.println("Không lưu được luồng video đang chạy:(");
                }
              }
            } else if ((getField(firebaseData.payload(), "streamStarted", "integerValue")).toInt() == 0) {
              if (serverRunning) {
                if (handleStreamTask != NULL) {
                  vTaskDelete(handleStreamTask);
                  delay(500);
                  Serial.println("Luồng video đã dừng!");
                }
                serverRunning = false; 
                server.stop();      
              }
            }

            turnOnFlash = (getField(firebaseData.payload(), "turnOnFlash", "booleanValue")).indexOf("true") != -1; 

            if ((getField(firebaseData.payload(), "uploadPhoto", "stringValue")).indexOf("takePhoto") != -1) {
              savePhotoToStorage(); 
            }

            if ((getField(firebaseData.payload(), "photoSaving", "booleanValue")).indexOf("true") != -1) { 
              if (!photoSavingStarted) { 
                photoInterval = (getField(firebaseData.payload(), "photoInterval", "integerValue")).toInt(); 
                photoCount = (getField(firebaseData.payload(), "photoCount", "integerValue")).toInt(); 
                photoSavingStarted = true; 
                photoSaving = true; 
                photoIntervalMillis = millis(); 
                Serial.println("Quá trình tự động tải ảnh lên đã bắt đầu!\Khoảng thời gian chụp ảnh: " + String(photoInterval) + "\nSố lượng ảnh: " + String(photoCount));
              }
            } else {
              photoSavingStarted = false; 
              photoSaving = false; 
            }

            if ((getField(firebaseData.payload(), "flashState", "booleanValue")).indexOf("true") != -1) { 
              digitalWrite(flashPin, HIGH); 
            } else {
              digitalWrite(flashPin, LOW);
            }

            changeVideoFlip = !changeVideoFlip;
            if (changeVideoFlip) { 
              flipVideo1 = (getField(firebaseData.payload(), "flipVideo", "booleanValue")).indexOf("true") != -1;
            } else {
              flipVideo2 = (getField(firebaseData.payload(), "flipVideo", "booleanValue")).indexOf("true") != -1;
            }
            if (flipVideo1 != flipVideo2) {
              camera->set_vflip(camera, (getField(firebaseData.payload(), "flipVideo", "booleanValue")).indexOf("true") != -1); 
              Serial.println("Cài đặt lật video: " + String((getField(firebaseData.payload(), "flipVideo", "booleanValue")).indexOf("true") != -1));
            }

            changeVideoMirror = !changeVideoMirror;
            if (changeVideoMirror) { 
              mirrorVideo1 = (getField(firebaseData.payload(), "mirrorVideo", "booleanValue")).indexOf("true") != -1;
            } else {
              mirrorVideo2 = (getField(firebaseData.payload(), "mirrorVideo", "booleanValue")).indexOf("true") != -1;
            }
            if (mirrorVideo1 != mirrorVideo2) { 
              camera->set_hmirror(camera, (getField(firebaseData.payload(), "mirrorVideo", "booleanValue")).indexOf("true") != -1);
              Serial.println("Thiết lập phản chiếu video: " + String((getField(firebaseData.payload(), "mirrorVideo", "booleanValue")).indexOf("true") != -1));
            }

            changeVideoResolution = !changeVideoResolution; 
            if (changeVideoResolution) { 
              videoResolution1 = (getField(firebaseData.payload(), "resolution", "integerValue")).toInt();
            } else {
              videoResolution2 = (getField(firebaseData.payload(), "resolution", "integerValue")).toInt();
            }
            if (videoResolution1 != videoResolution2) { 
              framesizeCode = (getField(firebaseData.payload(), "resolution", "integerValue")).toInt();
              setFramesize(); 
              Serial.println("Đặt độ phân giải video: " + String((getField(firebaseData.payload(), "resolution", "integerValue")).toInt()));
            }

            changeVideoBrightness = !changeVideoBrightness; 
            if (changeVideoBrightness) { 
              videoBrightness1 = (getField(firebaseData.payload(), "brightness", "integerValue")).toInt();
            } else {
              videoBrightness2 = (getField(firebaseData.payload(), "brightness", "integerValue")).toInt();
            }
            if (videoBrightness1 != videoBrightness2) { 
              camera->set_brightness(camera, (getField(firebaseData.payload(), "brightness", "integerValue")).toInt()); 
              Serial.println("Đặt độ sáng video: " + String((getField(firebaseData.payload(), "brightness", "integerValue")).toInt()));
            }

            changeVideoContrast = !changeVideoContrast; 
            if (changeVideoBrightness) { 
              videoContrast1 = (getField(firebaseData.payload(), "contrast", "integerValue")).toInt();
            } else {
              videoContrast2 = (getField(firebaseData.payload(), "contrast", "integerValue")).toInt();
            }
            if (videoContrast1 != videoContrast2) {
              camera->set_contrast(camera, (getField(firebaseData.payload(), "contrast", "integerValue")).toInt());
              Serial.println("Đặt độ tương phản video: " + String((getField(firebaseData.payload(), "contrast", "integerValue")).toInt()));
            }

            changeVideoSpecialEffect = !changeVideoSpecialEffect; 
            if (changeVideoSpecialEffect) { 
              videoSpecialEffect1 = (getField(firebaseData.payload(), "specialEffect", "integerValue")).toInt();
            } else {
              videoSpecialEffect2 = (getField(firebaseData.payload(), "specialEffect", "integerValue")).toInt();
            }
            if (videoSpecialEffect1 != videoSpecialEffect2) {
              camera->set_special_effect(camera, (getField(firebaseData.payload(), "specialEffect", "integerValue")).toInt()); 
              Serial.println("Cài đặt hiệu ứng video: " + String((getField(firebaseData.payload(), "specialEffect", "integerValue")).toInt()));
            }

            changeVideoWhiteBalanceMode = !changeVideoWhiteBalanceMode;
            if (changeVideoBrightness) { 
              videoWhiteBalanceMode1 = (getField(firebaseData.payload(), "whiteBalance", "integerValue")).toInt();
            } else {
              videoWhiteBalanceMode2 = (getField(firebaseData.payload(), "whiteBalance", "integerValue")).toInt();
            }
            if (videoWhiteBalanceMode1 != videoWhiteBalanceMode2) { 
              camera->set_wb_mode(camera, (getField(firebaseData.payload(), "whiteBalance", "integerValue")).toInt());
              Serial.println("Cài đặt chế độ cân bằng trắng: " + String((getField(firebaseData.payload(), "whiteBalance", "integerValue")).toInt()));
            }

            changeYAxisRotate = !changeYAxisRotate;
            if (changeYAxisRotate) {
              YAxisRotate1 = (getField(firebaseData.payload(), "YAxisRotate", "integerValue")).toInt();
            } else {
              YAxisRotate2 = (getField(firebaseData.payload(), "YAxisRotate", "integerValue")).toInt();
            }
            if (YAxisRotate1 != YAxisRotate2) { 
              if ((getField(firebaseData.payload(), "YAxisRotate", "integerValue")).toInt() > YAxisRotate) {
               
                for (; YAxisRotate <= (getField(firebaseData.payload(), "YAxisRotate", "integerValue")).toInt(); YAxisRotate++) {
                  ServoY.write(YAxisRotate); 
                  Serial.println("Xoay trong Y: " + String(YAxisRotate));
                  delay(15); 
                }
              } else {
                
                for (; YAxisRotate >= (getField(firebaseData.payload(), "YAxisRotate", "integerValue")).toInt(); YAxisRotate--) {
                  ServoY.write(YAxisRotate); 
                  Serial.println("Xoay trong Y: " + String(YAxisRotate));
                  delay(15); 
                }
              }
            }

            changeZAxisRotate = !changeZAxisRotate;
            if (changeZAxisRotate) {
              ZAxisRotate1 = (getField(firebaseData.payload(), "ZAxisRotate", "integerValue")).toInt();
            } else {
              ZAxisRotate2 = (getField(firebaseData.payload(), "ZAxisRotate", "integerValue")).toInt();
            }
            if (ZAxisRotate1 != ZAxisRotate2) {
              if ((getField(firebaseData.payload(), "ZAxisRotate", "integerValue")).toInt() > ZAxisRotate) { 
                
                for (; ZAxisRotate <= (getField(firebaseData.payload(), "ZAxisRotate", "integerValue")).toInt(); ZAxisRotate++) {
                  ServoZ.write(ZAxisRotate); 
                  Serial.println("Xoay trong Z: " + String(ZAxisRotate));
                  delay(15); 
                }
              } else {
                for (; ZAxisRotate >= (getField(firebaseData.payload(), "ZAxisRotate", "integerValue")).toInt(); ZAxisRotate--) {
                  ServoZ.write(ZAxisRotate);
                  Serial.println("Xoay trong Z: " + String(ZAxisRotate));
                  delay(15);
                }
              }
            }
          } else {
            Serial.println("Không nhận được tài liệu :(");
          }
        }
        if (photoSaving) { 
          if (millis() - photoIntervalMillis >= photoInterval * 60 * 1000) { 
            if (currentPhotoCount < photoCount) { 
              currentPhotoCount++;
              Serial.println("Số lượng ảnh đã tải lên: " + String(currentPhotoCount));
              savePhotoToStorage(); 
              photoIntervalMillis = millis(); 
            } else {
              Serial.println("Số lượng ảnh được chỉ định đã được tải lên!");
              photoSavingStarted = false;
              photoSaving = false; 
              
              firebaseJson.set("fields/photoSaving/booleanValue", false);
              firebaseJson.toString(contentJson); 
             
              if (Firebase.Firestore.patchDocument(&firebaseData, "esp32-cam-9182b", "", ("users/" + firebaseAuth.token.uid).c_str(), contentJson.c_str(), "photoSaving")) {
                Serial.println("Việc tắt tính năng tự động tải ảnh lên được lưu lại!");
              } else {
                Serial.println("Không lưu được vì tính năng tự động tải ảnh lên bị tắt:(");
              }
            }
          }
        }
      } else {
        WiFi.mode(WIFI_STA); 
        delay(250); 
        WiFi.begin((const char*)networkName.c_str(), (const char*)networkPass.c_str()); 
        Serial.println("Kết nối với" + String(networkName));

        if (WiFi.waitForConnectResult() == WL_CONNECTED) { 
          Serial.println("Đã kết nối với mạng WiFi!");
          IPAddress LocalIP = WiFi.localIP(); 
          for (int i = 0; i < 4; i ++) { 
            ESP32IP += i ? "." + String(LocalIP[i]) : String(LocalIP[i]);
          }
          Serial.println("Địa chỉ IP cục bộ: " + String(ESP32IP));

          configTime(timezone*3600, 3600, "pool.ntp.org"); 
          Serial.println("Tận dụng thời gian từ Internet ...");

          while(!time(nullptr)) {
            Serial.print("*");
            delay(500);
          }
          Serial.println("Đáp ứng nhận được!");

          firebaseConfig.api_key = "AIzaSyBenCQg1o495zHvn1vxdN5WEx1kQrj9LJg";
          firebaseAuth.user.email = (userEmail).c_str(); 
          firebaseAuth.user.password = (userPass).c_str(); 
          firebaseConfig.token_status_callback = tokenStatusCallback; 
          Firebase.begin(&firebaseConfig, &firebaseAuth); 
          Firebase.reconnectWiFi(false); 
        }
      }
    } else { 
      server.handleClient(); 
    }
    
    TIMERG0.wdt_wprotect = TIMG_WDT_WKEY_VALUE;
    TIMERG0.wdt_feed = 1;
    TIMERG0.wdt_wprotect = 0;
  }
}

void loop() {}

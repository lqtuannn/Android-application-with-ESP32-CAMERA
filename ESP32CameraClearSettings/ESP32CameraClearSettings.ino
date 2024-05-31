#include "SPIFFS.h" 


#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); 
  Serial.begin(115200); 
  delay(2500); 

  if (SPIFFS.begin(true)) { 
    Serial.println("Gắn SPIFFS đã thành công!");
  } else {
    Serial.println("Không thể gắn SPIFFS :(");
    return;
  }

  File settingsFile = SPIFFS.open("/Settings.txt", FILE_WRITE); 
  if (settingsFile) { 
    Serial.println("Tệp có thông số vận hành đã được mở thành công để ghi!");
  } else {
    Serial.println("Không thể mở tệp có thông số vận hành để ghi:(");
    return;
  }

  if (settingsFile.print("-")) { 
    Serial.println("Các thông số vận hành đã được thiết lập lại!");
  } else {
    Serial.println("Không thể thiết lập lại các thông số vận hành :(");
    return;
  }

  settingsFile.close(); 
}

void loop() {}

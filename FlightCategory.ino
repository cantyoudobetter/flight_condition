#include <FastLED.h>
#include <WiFi.h>
// #include "time.h"
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <TFT_eSPI.h> // Hardware-specific library
#include <SPI.h>
// #define NUM_LEDS = num_leds
#define DATA_PIN 2
// #include "Free_Fonts.h" // Include the header file attached to this sketch

TFT_eSPI tft = TFT_eSPI();                   // Invoke custom library with default width and height

unsigned long drawTime = 0;
const char* ssid       = "TAMU";
const char* password   = "tamu1993";

char* metarJson;
unsigned long timerDelay = 5000;

typedef struct {
  char airport[5];
  int led_id;
  int fc;
} ledArray;

ledArray lArray[] = { 
                      {"KSGR",0,0}, 
                      {"KHOU",1,0}, 
                      {"KDAL",2,0}, 
                      {"KAUS",3,0}, 
                      {"KSAT",4,0}, 
                      {"KEDC",5,0}, 
                      {"KLBX",6,0} 
                    };

const int num_leds = (int)sizeof(lArray)/sizeof(lArray[0]);
CRGB leds[num_leds];

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, num_leds);
  tft.begin();
  tft.setRotation(1);
}
void loop() {
  buildMetar();
  LightLEDs();
  delay(10000);
}


void LightLEDs() 
{
  for (int x = 0; x < num_leds; x++) {  
    if (lArray[x].fc == 0) leds[lArray[x].led_id] = CRGB::Green;
    if (lArray[x].fc == 1) leds[lArray[x].led_id] = CRGB::Blue;
    if (lArray[x].fc == 2) leds[lArray[x].led_id] = CRGB::Red;
  }
  FastLED.show();
}



void buildMetar()
{
  if(WiFi.status()== WL_CONNECTED){
    HTTPClient http;
    for (int x = 0; x < num_leds; x++) {
      // char[] airport = lArray[x].airport;
      String airport_str(lArray[x].airport);
      String serverPath = "https://api.checkwx.com/metar/"+airport_str+"/decoded?x-api-key=f953033170224830a32fcd07dc";
      http.begin(serverPath.c_str());
      int httpResponseCode = http.GET();
      
      if (httpResponseCode>0) {
        String str = http.getString(); 
        DynamicJsonDocument doc(2048);
        deserializeJson(doc, str);
        const char* fc = doc["data"][0]["flight_category"];
        lArray[x].fc = 0;
        if (strcmp(fc, "VFR") == 0)  lArray[x].fc = 0;
        if (strcmp(fc, "MVFR") == 0) lArray[x].fc = 1;
        if (strcmp(fc, "IFR") == 0)  lArray[x].fc = 2;

      } else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
    }

    Serial.println("DUMP: ");
    for (int x = 0; x < num_leds; x++) {
      String out = String(lArray[x].airport) + ":" + String(lArray[x].led_id) + ":" + String(lArray[x].fc);
      Serial.println(out);
    }
    tft.fillScreen(TFT_BLACK);
    int xpos =  40;
    int ypos = 40;
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(xpos, ypos);    // Set cursor near top left corner of screen
    tft.setTextSize(12);
    tft.drawString(String(lArray[1].fc), 10, 10);   // Select the orginal small GLCD font by using NULL or GLCD

  
    // Free resources
    http.end();
  } else {
    Serial.print("Not Connected");
  }

}
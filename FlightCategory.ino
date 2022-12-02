#include <FastLED.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <TFT_eSPI.h> // Hardware-specific library
#include <SPI.h>
#define DATA_PIN 2
// #include "Free_Fonts.h" // Include the header file attached to this sketch
String strs[30];
int StringCount = 0;

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
                      {"KLBX",0,4},
                      {"VOID",1,4},
                      {"KGLS",2,4}, 
                      {"KEFD",3,4}, 
                      {"KHOU",4,4}, 
                      {"KAXH",5,4}, 
                      {"KLVJ",6,4}, 
                      {"KHPY",7,4}, 
                      {"KIAH",8,4}, 
                      {"KDWH",9,4}, 
                      {"KCXO",10,4}, 
                      {"KGR3",11,4}, 
                      {"K00R",12,4}, 
                      {"VOID",13,4},
                      {"KUTS",14,4}, 
                      {"K51R",15,4}, 
                      {"KCFD",16,4}, 
                      {"KCLL",17,4}, 
                      {"K11R",18,4}, 
                      {"VOID",19,4},
                      {"KTME",20,4}, 
                      {"KIWS",21,4}, 
                      {"KSGR",22,4}, 
                      {"VOID",23,4},
                      {"KELA",24,4}, 
                      {"KARM",25,4}, 
                      {"KBYY",26,4}, 
                      {"KPSX",27,4}, 
                      {"KPKV",28,4}, 
                      {"KVCT",29,4}, 
                      {"VOID",30,4}, 
                      {"K2R9",31,4}, 
                      {"VOID",32,4}, 
                      {"KSAT",33,4}, 
                      {"KBAZ",34,4},
                      {"K50R",35,4}, 
                      {"KAUS",36,4}, 
                      {"KEDC",37,4}, 
                      {"KGTU",38,4},
                      {"VOID",39,4}, 
                      {"KTPL",40,4}, 
                      {"VOID",41,4}, 
                      {"VOID",42,4}, 
                      {"KRWV",43,4}, 
                      {"KGYB",44,4}, 
                      {"K3T5",45,4}, 
                      {"VOID",46,4}, 
                      {"KT85",47,4}, 
                      {"VOID",48,4},
                      {"K26R",49,4} 
                    };

//const int num_leds = 50;
const int num_leds = (int)sizeof(lArray)/sizeof(lArray[0]);
CRGB leds[num_leds];


void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  // WiFi.begin("Wokwi-GUEST", "", 6);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, num_leds);
  tft.begin();
  tft.setRotation(0);
  buildDisplay();
  buildMetar();
  LightLEDs();
  delay(1000*60*25);
  ESP.restart();
}



void LightLEDs() 
{
  for (int x = 0; x < 50; x++) {  
    leds[x] = CRGB::Green;
  }
  FastLED.show();
  delay(500);
  for (int x = 0; x < 50; x++) {  
    leds[x] = CRGB::Blue;
  }
  FastLED.show();
  delay(500);
  for (int x = 0; x < 50; x++) {  
    leds[x] = CRGB::Red;
  }
  FastLED.show();
  delay(500);
  for (int x = 0; x < 50; x++) {  
    leds[x] = CRGB(255, 0, 255);
  }
  FastLED.show();
  delay(500);


  for (int x = 0; x < num_leds; x++) {  
    if (lArray[x].fc == 0) leds[lArray[x].led_id] = CRGB(0, 255, 0);
    if (lArray[x].fc == 1) leds[lArray[x].led_id] = CRGB(0, 0, 255);
    if (lArray[x].fc == 2) leds[lArray[x].led_id] = CRGB(255, 0, 0);
    if (lArray[x].fc == 3) leds[lArray[x].led_id] = CRGB(255, 0, 255);
    if (lArray[x].fc > 3) leds[lArray[x].led_id] = CRGB::Black;
  }
  // for (int x = 0; x < 50; x++) {  
  //   leds[x] = CRGB::Green;
  // }

  FastLED.show();
}



void buildMetar()
{
  if(WiFi.status()== WL_CONNECTED){
    HTTPClient http;
    for (int x = 0; x < num_leds; x++) {
      if (lArray[x].airport != "VOID") {
        // char[] airport = lArray[x].airport;
        String airport_str(lArray[x].airport);
        // Serial.println(airport_str);
        String serverPath = "https://avwx.rest/api/metar/"+airport_str+"?options=&airport=true&reporting=true&format=json&remove=&filter=&onfail=cache&token=hcG1LPJ-tkO0P2LIj-9VlhqkbdZNfrSICga0jcltzwM";
        // String serverPath = "https://api.checkwx.com/metar/"+airport_str+"/decoded?x-api-key=f953033170224830a32fcd07dc";
        http.begin(serverPath.c_str());
        int httpResponseCode = http.GET();
        
        if (httpResponseCode>0) {
          String str = http.getString(); 
          DynamicJsonDocument doc(2048);
          deserializeJson(doc, str);
          // const int res_count = doc["results"];
          const int res_count = 1;
          // Serial.print("Res count:");
          // Serial.println(res_count);
          if (res_count > 0) {
            const char* fc = doc["flight_rules"];
            // const char* fc = doc["data"][0]["flight_category"];
            lArray[x].fc = 0;
            if (strcmp(fc, "VFR") == 0)  lArray[x].fc = 0;
            if (strcmp(fc, "MVFR") == 0) lArray[x].fc = 1;
            if (strcmp(fc, "IFR") == 0)  lArray[x].fc = 2;
            if (strcmp(fc, "LIFR") == 0)  lArray[x].fc = 3;

          }

        } else {
          lArray[x].fc = 4;
          Serial.print("Error code: ");
          Serial.println(httpResponseCode);
        }
      }
    }
    Serial.println("DUMP: ");
    for (int x = 0; x < num_leds; x++) {
      String out = String(lArray[x].airport) + ":" + String(lArray[x].led_id) + ":" + String(lArray[x].fc);
      Serial.println(out);
    }
    http.end();
  } else {
    Serial.print("Not Connected");
  }  

}

void buildDisplay()
{
  if(WiFi.status()== WL_CONNECTED){
    HTTPClient http;
    String serverPath = "https://avwx.rest/api/metar/KSGR?options=&airport=true&reporting=true&format=json&remove=&filter=&onfail=cache&token=hcG1LPJ-tkO0P2LIj-9VlhqkbdZNfrSICga0jcltzwM";

    http.begin(serverPath.c_str());
    int httpResponseCode = http.GET();
    Serial.print("HTTP:");
    Serial.println(httpResponseCode);
    if (httpResponseCode>0) {
      String str = http.getString(); 
      DynamicJsonDocument doc(2048);
      deserializeJson(doc, str);
      // const int res_count = doc["results"];
      const int res_count = 1;
      
      if (res_count > 0) {
        const char* raw = doc["raw"];
        String str = String(raw);
        Serial.print("METAR:");
        Serial.print(str);
        while (str.length() > 0)
        {
          int index = str.indexOf(' ');
          if (index == -1) // No space found
          {
            strs[StringCount++] = str;
            break;
          }
          else
          {
            strs[StringCount++] = str.substring(0, index);
            str = str.substring(index+1);
          }
        }
        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setTextSize (3);  
        // Show the resulting substrings
        int y_cnt = 0;
        for (int i = 2; i < StringCount; i++)
        {
          
          if (strs[i] == "AUTO") {continue;}
          if (strs[i].indexOf("Z") > 0)  {continue;}
          if (strs[i].indexOf("/") > 0)  {continue;}
          if (strs[i].indexOf("A") == 0)  {continue;}
          if (strs[i] == "RMK")  {break;}
          if (i > 6) {break;}
          
          if (strs[i].indexOf("KT") > 0)  {
            tft.setTextSize(4);
            tft.drawString(strs[i].substring(0,3), 10, 10);
            // Serial.print("DIR:");
            // Serial.println(strs[i].substring(0,3));
            tft.drawString(strs[i].substring(3), 10, 60);
            // Serial.print("WIND:");
            // Serial.println(strs[i].substring(3));

          } else {
            tft.setTextSize(3);            
            tft.drawString(strs[i], 10, (110+(30*y_cnt++)));
          }


             // Select the orginal small GLCD font by using NULL or GLCD


          Serial.print(i);
          Serial.print(": \"");
          Serial.print(strs[i]);
          Serial.println("\"");
        }


      }

    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }


    http.end();
  } else {
    Serial.print("Not Connected");
  }  

}

void loop() {
  // buildDisplay();
  // buildMetar();
  // LightLEDs();
  // delay(1000*60);
}

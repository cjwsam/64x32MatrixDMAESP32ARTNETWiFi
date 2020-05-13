/*
   Made by Christopher Willis Git (cjwsam)

   Note:
   This is a pre-release version of this software.  It is not yet ready for prime time and contains bugs (known and unknown).
   Please submit any bugs or code changes so they can be included into the next release.

*/

// Library Includes

#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"
#include <FS.h>
#include <EEPROM.h>
#include <WiFiManager.h>   //git clone https://github.com/tzapu/WiFiManager.git --branch development in library folder //edited thirdparty Libz 

#include <SmartMatrix3.h> // LED matrixx libary //edited thirdparty Libz 

#define FASTLED_INTERNAL //Disable (SPI) PRAGMA warnings in FastLED library
#include <FastLED.h> //Fast LED library for LED effects

#include "ArtnetWifiAsync.h" //artnetWifi thirdparty Libz
#include "SSD1306Wire.h" // legacy include: `#include "SSD1306.h"`

// Include custom passed and fail images in *.xbm
#include "images.h"

#include <Adafruit_GFX.h> //Ada Graphics library
#include <WiFi.h> //Wifi
#include <DNSServer.h>
#include <WebServer.h>
#include <HTTPClient.h>

#include <Fonts/Picopixel.h> //Tiny font

#include <SPI.h>
#include "FilenameFunctions.h"

const int numberOfChannels = 6144; // Total number of channels you want to receive (1 led = 3 channels)

// Initialize the OLED display using Wire library

SSD1306Wire  display(0x3c, 5, 4);

TaskHandle_t TaskA, TaskB;

bool shouldSaveConfig = false; //flag for saving data

//artnetCreation
ArtnetWifi artnet;

//******************************************************************************************************************

//Default Config

//My own bits
bool passed = false;
byte channelBuffer[numberOfChannels]; // Combined universes into a single array
bool sendFrame = 0;
bool universesReceived[numberOfChannels];

//WIFI Manager Bits
char WIFI_SSID[20] ;
char WIFI_PASS[20] ;

//artnet bits
int startUni ;
int endUni ;
int processOffset;

//******************************************************************************************************************
//DEFINE Print Debugging

#define DEBUG 1

#ifdef DEBUG
#define DEBUG_PRINT(x)     Serial.print (x)
#define DEBUG_PRINTDEC(x)  Serial.print (x, DEC)
#define DEBUG_PRINTHEX(x)  Serial.print (x,HEX)
#define DEBUG_PRINTLN(x)   Serial.println (x)
#define DEBUG_PRINTF(x,y)  Serial.printf( x,y)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTDEC(x)
#define DEBUG_PRINTHEX(x)
#define DEBUG_PRINTLN(x)
#define DEBUG_PRINTF(x,y)
#endif

//******************************************************************************************************************

// SmartMatrix matrixx Config

#define COLOR_DEPTH 24                  // known working: 24, 48 - If the sketch uses type `rgb24` directly, COLOR_DEPTH must be 24
const uint8_t kMatrixWidth = 64;        // known working: 32, 64, 96, 128
const uint8_t kMatrixHeight = 32;       // known working: 16, 32, 48, 64
const uint8_t kRefreshDepth = 48;       // known working: 24, 36, 48
const uint8_t kDmaBufferRows = 4;       // known working: 2-4, use 2 to save memory, more to keep from dropping frames and automatically lowering refresh rate
const uint8_t kPanelType =  SMARTMATRIX_HUB75_32ROW_MOD16SCAN   ;   // use SMARTMATRIX_HUB75_16ROW_MOD8SCAN for common 16x32 panels
const uint8_t kMatrixOptions = (SMARTMATRIX_OPTIONS_NONE);      // see http://docs.pixelmatix.com/SmartMatrix for options
const uint8_t kBackgroundLayerOptions = (SM_BACKGROUND_OPTIONS_NONE);
SMARTMATRIX_ALLOCATE_BUFFERS(matrix, kMatrixWidth, kMatrixHeight, kRefreshDepth, kDmaBufferRows, kPanelType, kMatrixOptions);
SMARTMATRIX_ALLOCATE_BACKGROUND_LAYER(backgroundLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kBackgroundLayerOptions);

const int defaultBrightness = 100 * (255 / 100); // full brightness 100*


//******************************************************************************************************************

// Config Wifi

WiFiManager wm;
String wifiSSID;
String wifiPSK;
WiFiClient espClient;

//******************************************************************************************************************

// Misc Setup Vars
String uniInfo = "N/A";

//******************************************************************************************************************

void sprintdiv ()
{
  DEBUG_PRINTLN("/***************************************************************/");
}

String getParam(String name)
{
  //read parameter from server, for customhmtl input
  String value;
  if (wm.server->hasArg(name)) {
    value = wm.server->arg(name);
  }
  return value;
}

void saveWifiCallback()
{



}

//gets called when WiFiManager enters configuration mode
void configModeCallback (WiFiManager *myWiFiManager)
{
  passed = false;
  DoColorTest();
  drawImageDemo(false);

}

void saveParamCallback()
{

  startUni = atol(getParam("startUniverse").c_str());
  endUni = atol(getParam("endUniverse").c_str());
  processOffset = atol(getParam("speedOffset").c_str());

  DEBUG_PRINTLN ("[EEPROM] DONE Writing EEPROM");
  Serial.println("PARAM startUniverse = " + getParam("startUniverse"));
  Serial.println("PARAM endUniverse = " + getParam("endUniverse"));
  Serial.println("PARAM processOffset = " + getParam("speedOffset"));


  SaveEEPROM();

}

void resetCallback()
{
  resetEEPROM();
}

void bindServerCallback() {
  wm.server->on("/custom", handleRoute);
  // wm.server->on("/info",handleRoute); // can override wm!
}

void handleRoute() {
  Serial.println("[HTTP] handle route");
  wm.server->send(200, "text/plain", "hello from user code");
}

void DoColorTest()
{

  backgroundLayer.fillScreen({0xff, 0, 0});
  backgroundLayer.drawString(((matrix.getScreenWidth() / 2) - 10), ((matrix.getScreenHeight() / 2) - 10), {0, 0, 0}, "RED");
  backgroundLayer.swapBuffers();
  delay(1500);

  // draw a box in green
  backgroundLayer.fillScreen({0, 0xff, 0});
  backgroundLayer.drawString(((matrix.getScreenWidth() / 2) - 10), ((matrix.getScreenHeight() / 2) - 10), {0, 0, 0}, "GREEN");
  backgroundLayer.swapBuffers();
  delay(1500);

  //draw a box blue;
  backgroundLayer.fillScreen({0, 0, 0xff});
  backgroundLayer.drawString(((matrix.getScreenWidth() / 2) - 10), ((matrix.getScreenHeight() / 2) - 10), {0, 0, 0}, "BLUE");
  backgroundLayer.swapBuffers();
  delay(1500);

  //draw a box white;
  backgroundLayer.fillScreen({0xff, 0xff, 0xff});
  backgroundLayer.drawString(((matrix.getScreenWidth() / 2) - 10), ((matrix.getScreenHeight() / 2) - 10), {0, 0, 0}, "WHITE");
  backgroundLayer.swapBuffers();
  delay(1500);

  if (passed)
  {
    backgroundLayer.fillScreen({0, 0xff, 0});
    backgroundLayer.drawString(((matrix.getScreenWidth() / 2) - 12), ((matrix.getScreenHeight() / 2) - 10), {0, 0, 0}, "PASSED,");
    backgroundLayer.drawString(((matrix.getScreenWidth() / 2) - 15), ((matrix.getScreenHeight() / 2) - 2), {0, 0, 0}, uniInfo.c_str() ); //displays the current universe settings for debugging and mapping
    backgroundLayer.swapBuffers();
    delay(3000); //extend delay for longer visual info ^^^
    backgroundLayer.fillScreen({0, 0, 0});
    backgroundLayer.swapBuffers();
  }
  else
  {
    backgroundLayer.fillScreen({0xff, 0, 0});
    backgroundLayer.drawString(((matrix.getScreenWidth() / 2) - 12), ((matrix.getScreenHeight() / 2) - 10), {0, 0, 0}, "ERROR,");
    backgroundLayer.drawString(((matrix.getScreenWidth() / 2) - 20), ((matrix.getScreenHeight() / 2) - 4), {0, 0, 0}, "RESTART OR,");
    backgroundLayer.drawString(((matrix.getScreenWidth() / 2) - 13), ((matrix.getScreenHeight() / 2) + 2), {0, 0, 0}, "CONFIG");
    backgroundLayer.swapBuffers();
    delay(500);
  }

  sprintdiv ();
  String checkResult;
  if (passed) {
    checkResult = "Passed";
  } else {
    checkResult = "Fail";
  }
  DEBUG_PRINTLN  ("[SelfTest] BootTime Test Completed : " +  checkResult);

  sprintdiv();

}

void setup_wifi()
{

  // wm.debugPlatformInfo();
  // wm.resetSettings();
  // wm.erase();

  wm.setClass("invert");

  WiFiManagerParameter custom_html("<p>Artnet & Speed Settings Page </p>"); // only custom html
  WiFiManagerParameter startUniverse("startUniverse", "Start Universe", "", 5);
  WiFiManagerParameter endUniverse("endUniverse", "End Universe", "", 5);
  WiFiManagerParameter speedOffset("speedOffset", "Process Priority", "", 5);

  // callbacks
  wm.setAPCallback(configModeCallback);
  wm.setWebServerCallback(bindServerCallback);
  wm.setSaveConfigCallback(saveWifiCallback);
  wm.setSaveParamsCallback(saveParamCallback);
  wm.setConfigResetCallback(resetCallback);


  //add all your parameters here
  wm.addParameter(&startUniverse);
  wm.addParameter(&endUniverse);
  wm.addParameter(&speedOffset);


  startUniverse.setValue(String(startUni).c_str(), 4);
  endUniverse.setValue(String(endUni).c_str(), 4);
  speedOffset.setValue(String(processOffset).c_str(), 8);


  std::vector<const char *> menu = {"wifi", "param", "info", "close", "sep", "erase", "restart", "exit"};
  wm.setMenu(menu); // custom menu, pass vector

  // set country
  wm.setCountry("US"); // setting wifi country seems to improve OSX soft ap connectivity, may help others as well

  // set channel
  wm.setWiFiAPChannel(13);

  // Set the connect timeout to 4 minutes
  wm.setTimeout(900);
  String ssid = "Chris's Config " + String(getChipID());
  if (!wm.autoConnect(ssid.c_str()))
  {
    // DEBUG_PRINTLN("Never Printed");
  }
  else
  {
    sprintdiv ();

    //if you get here you have connected to the WiFi
    Serial.println("[WiFi] Connected...yeey :)");
    passed = true;
    DoColorTest();
    drawImageDemo(true);
  }
}


void ReadEEPROM() {

  Serial.println("Reading EEPROM...");    //read configuration from FS json
  EEPROM.begin(8);
  startUni = EEPROM16_Read(0);
  endUni = EEPROM16_Read(2);
  processOffset = EEPROM16_Read(4);


  sprintdiv();
  DEBUG_PRINTLN ("[EEPROM] DONE Reading EEPROM");
  sprintdiv();

  Serial.println("Saved Values Where:");
  Serial.println("Start Universe : " + String(startUni));
  Serial.println("End Universe : " + String(endUni));
  Serial.println("Process Offset : " + String(processOffset));


}

void SaveEEPROM()
{
  DEBUG_PRINTLN ("Writing EEPROM");

  EEPROM16_Write(0, startUni);
  EEPROM16_Write(2, endUni);
  EEPROM16_Write(4, processOffset);


  DEBUG_PRINTLN ("DONE Writing EEPROM");

  sprintdiv ();

}

void resetEEPROM()
{

  DEBUG_PRINTLN ("[EEPROM] Erasing ");
  for (int i = 0 ; i < EEPROM.length() ; i++) {
    EEPROM.write(i, 0);
  }
  WiFi.disconnect(false, true); //Erase existing WIFI credentials

}

void EEPROM16_Write(uint8_t a, uint16_t b)
{
  EEPROM.begin(8);
  EEPROM.write(a, lowByte(b));
  EEPROM.write(a + 1, highByte(b));
  EEPROM.commit();
}

uint16_t EEPROM16_Read(uint8_t a)
{
  return word(EEPROM.read(a + 1), EEPROM.read(a));
}


uint32_t getChipID() {

  // uint32_t chipID;
  uint64_t macAddress = ESP.getEfuseMac();
  uint64_t macAddressTrunc = macAddress >> 40;
  return ( macAddressTrunc );
}


//On DMX *Artnet packet
void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data)
{
  sendFrame = 1;

  if (universe <= endUni && universe >= startUni)
  {

    universesReceived[universe - startUni] = 1; // Store which universe has got in

    for (int i = 0; i < length ; i++) // Load data into the channelBuffer arrey at the right spot
    {
      int bufferIndex = i + ((universe - startUni) * length);

      if (bufferIndex < numberOfChannels) // to verify
      {
        channelBuffer[bufferIndex] = byte(data[i]);
      }
    }

  }


  for (int i = 0; i < 12; i++) //check if all universes are there // must be 12 as start from 0-12 = 13 lol
  {
    if (universesReceived[i] == 0)
    {
      sendFrame = 0;
      break;
    }
  }


  if (sendFrame) //if all universes are there, send frame to display
  {

    int ii = 0;

    //Split Loading into 2 smaller loops *for some reason it works faster
    for (int16_t y = 0; y < kMatrixHeight / 2; y++)
      for (int16_t x = 0; x < kMatrixWidth; x++)
      {
        backgroundLayer.drawPixel(x, y, CRGB( channelBuffer[(ii * 3)] , channelBuffer[(ii * 3) + 1], channelBuffer[(ii * 3) + 2]));
        ii++;
      }
    for (int16_t y = 16; y < kMatrixHeight; y++)
      for (int16_t x = 0; x < kMatrixWidth; x++)
      {
        backgroundLayer.drawPixel(x, y, CRGB(channelBuffer[(ii * 3)] , channelBuffer[(ii * 3) + 1], channelBuffer[(ii * 3) + 2]));
        ii++;
      }

    backgroundLayer.swapBuffers(); //Display the loaded frame
    memset(universesReceived, 0, endUni);// Reset universeReceived to 0
  }
}

void drawImageDemo(bool allGood)
{
  // see http://blog.squix.org/2015/05/esp8266-nodemcu-how-to-create-xbm.html
  // on how to create xbm files

  delay(200);

  if (!allGood)
  {
    display.clear();
    DEBUG_PRINTLN ("Fail Checkup Triggered: ");
    display.drawXbm(34, 14, noGood_width, noGood_height, noGood_bits);
  }
  else
  {
    display.clear();
    DEBUG_PRINTLN ("Passed Checkup Triggered: ");
    display.drawXbm(34, 14, allGood_width, allGood_height, allGood_bits);
  }

  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.drawString(10, 128, String(millis()));
  // write the buffer to the display
  display.display();
}

void setup() {
  //Setup Serial
  Serial.begin(115200);
  delay(300);

  display.init();
  display.setFont(ArialMT_Plain_10);
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
  display.drawString(128 / 2, 64 / 2 - 10, "Booting Up :-)");
  display.display();


  matrix.addLayer(&backgroundLayer); //pretty self exp .. creates a new "background layer"
  matrix.begin(); //starts SmartMatrix
  matrix.setBrightness(defaultBrightness);
  backgroundLayer.enableColorCorrection(true); //Reduce blinding brightness

  delay(300);
  sprintdiv();
  ReadEEPROM();
  sprintdiv();
  delay(300);

  uniInfo = (String(startUni) + " TO " + String(endUni));

  DEBUG_PRINT ("Heap: ");
  DEBUG_PRINT (system_get_free_heap_size());
  DEBUG_PRINTLN();
  sprintdiv();

  DEBUG_PRINT  ("CHIP ID: ");
  DEBUG_PRINTLN (getChipID());

  sprintdiv();

  setup_wifi();

  xTaskCreatePinnedToCore(
    Task1,                  /* pvTaskCode */
    "Workload1",            /* pcName */
    30000,                   /* usStackDepth */
    NULL,                   /* pvParameters */
    processOffset,          /* uxPriority adjust based on udp reading speed requirements  */
    &TaskA,                 /* pxCreatedTask */
    0);                     /* xCoreID */


  delay (100);
  artnet.begin();
  artnet.setArtDmxCallback(onDmxFrame); //gets called if a frame arrives

  bool universesReceived[endUni];

}


void Task1( void * parameter )
{


  TIMERG0.wdt_wprotect = TIMG_WDT_WKEY_VALUE;  //required to avoid watchdog triggerd kernal panic
  TIMERG0.wdt_feed = 1;
  TIMERG0.wdt_wprotect = 1;

  for (;;)
  {

    artnet.read(); //check for new Artnet data *loops in perpetuity*
    vTaskDelay(7);
}
  yield();
 
  vTaskDelete(NULL);
}

void loop() {


  TIMERG0.wdt_wprotect = TIMG_WDT_WKEY_VALUE; //required to avoid watchdog triggerd kernal panic
  TIMERG0.wdt_feed = 1;
  TIMERG0.wdt_wprotect = 0;
  
vTaskDelay(1);
yield();
//vTaskDelete(NULL);


}

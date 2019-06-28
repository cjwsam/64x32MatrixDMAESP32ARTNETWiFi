/* This Code has been created by ME (chris) :) https://github.com/cjwsam :)
   This Code is ONLY for Personal/Educational Except By 2Entertain ONLY on Colorline Fantasy 
*/
#include <SmartMatrix3.h> // LED Matrix libary //edited thirdparty Libz 
#include <FastLED.h> //Library to convert R,G,B to single RGB24 
#include <WiFi.h> //Not necessary to declare in esp32 but for the backwards compatibiltiy to esp8622 change this among other
#include <ArtnetWifi.h> //artnetWifi thirdparty Libz
#include <WiFiUdp.h> //needed to handle Artnet broadcast based packets 

//matrixCreation
#define COLOR_DEPTH 24                  // known working: 24, 48 - If the sketch uses type `rgb24` directly, COLOR_DEPTH must be 24
const uint8_t kMatrixWidth = 64;        // known working: 32, 64, 96, 128
const uint8_t kMatrixHeight = 32;       // known working: 16, 32, 48, 64
const uint8_t kRefreshDepth = 24;       // known working: 24, 36, 48
const uint8_t kDmaBufferRows = 4;       // known working: 2-4, use 2 to save memory, more to keep from dropping frames and automatically lowering refresh rate
const uint8_t kPanelType =  SMARTMATRIX_HUB75_32ROW_MOD16SCAN   ;   // use SMARTMATRIX_HUB75_16ROW_MOD8SCAN for common 16x32 panels
const uint8_t kMatrixOptions = (SMARTMATRIX_OPTIONS_NONE);      // see http://docs.pixelmatix.com/SmartMatrix for options
const uint8_t kBackgroundLayerOptions = (SM_BACKGROUND_OPTIONS_NONE);
SMARTMATRIX_ALLOCATE_BUFFERS(matrixx, kMatrixWidth, kMatrixHeight, kRefreshDepth, kDmaBufferRows, kPanelType, kMatrixOptions);
SMARTMATRIX_ALLOCATE_BACKGROUND_LAYER(backgroundLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kBackgroundLayerOptions);

const int defaultBrightness = 100*(255/100);    // full brightness

//artnetCreation
ArtnetWifi artnet;
const int startUniverse = 0; // CHANGE FOR YOUR SETUP most software this is 1, some software send out artnet first universe as zero.
const int numberOfChannels = 6144;//6168; // Total number of channels you want to receive (1 led = 3 channels)
byte channelBuffer[numberOfChannels + 1]; // Combined universes into a single array
const int maxUniverses = (numberOfChannels / 512 + ((numberOfChannels % 512) ? 1 : 0));   // Check if we got all universes
bool universesReceived[maxUniverses];
bool sendFrame = 0;

// WiFi INFO
const char ssid[] = "Pretty Fly For A WiFi"; //"SLvi2000";
const char pwd[] =  "server13" ;//"SL9278234";

//On DMX *Artnet packet
void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data)
{ 
 
  sendFrame = 1;
 
 if (universe < maxUniverses) universesReceived[universe] = 1; // Store which universe has got in

  for (int i = 0; i < length; i++) // Load data into the channelBuffer arrey at the right spot 
  {
    int bufferIndex = i + ((universe - startUniverse) * length);
    if (bufferIndex < numberOfChannels) // to verify
      channelBuffer[bufferIndex] = byte(data[i]);
  }
  
  for (int i = 0; i < maxUniverses; i++) //check if all universes are there
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
  for (int16_t y=0;y<kMatrixHeight/2; y++)
  for (int16_t x=0;x<kMatrixWidth; x++)
  {
    backgroundLayer.drawPixel(x, y, CRGB( channelBuffer[(ii * 3)] , channelBuffer[(ii * 3) + 1], channelBuffer[(ii * 3) + 2]));
    ii++;
  } 
  for (int16_t y=16;y<kMatrixHeight; y++)
  for (int16_t x=0;x<kMatrixWidth; x++)
  {
    backgroundLayer.drawPixel(x, y, (CRGB(channelBuffer[(ii * 3)] , channelBuffer[(ii * 3) + 1], channelBuffer[(ii * 3) + 2])));
    ii++;
  } 
       
    backgroundLayer.swapBuffers(); //Display the loaded frame
    memset(universesReceived, 0, maxUniverses);// Reset universeReceived to 0   
  }
}


void setup() 
{

   //Wait for it lol (slowish boot)
   delay(3000);
   
   matrixx.addLayer(&backgroundLayer); //pretty self exp .. creates a new "background layer"
   matrixx.begin(); //starts SmartMatrix
   backgroundLayer.enableColorCorrection(true); //Reduce blinding brightness
   backgroundLayer.swapBuffers(); //Clear buffer loaded to error check 
 
  Serial.begin(115200); //Start Serial Output 
  Serial.print("Start"); 

 //WiFiConnectionInitialization
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pwd);
  while (WiFi.status() != WL_CONNECTED) { Serial.print("."); delay(500); }
  Serial.print("WiFi connected, IP = "); Serial.println(WiFi.localIP());

  //ArtnetInitialization
  artnet.begin();
  artnet.setArtDmxCallback(onDmxFrame); //gets called if a frame comnplete


 }

void loop() 
{
  artnet.read(); //check for new Artnet data *loops in perpetuity*
}

    
  

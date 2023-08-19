#include "Arduino.h"
#include <WiFi.h>
//Libraries for permenant data storage
#include "FS.h"
#include "SPIFFS.h"

#include <WiFiMulti.h>
#include "Ticker.h"
#include <Wire.h>
WiFiMulti wifiMulti;
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Wire.h>               // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h"        // legacy: #include "SSD1306.h"
#include "images.h"
#include "Electroniccats_PN7150.h"

#define PN7150_IRQ   (23)
#define PN7150_VEN   (19)
#define PN7150_ADDR  (0x28)
Electroniccats_PN7150 nfc(PN7150_IRQ, PN7150_VEN, PN7150_ADDR);    // creates a global NFC device interface object, attached to pins 7 (IRQ) and 8 (VEN) and using the default I2C address 0x28
RfIntf_t RfInterface;                                              //Intarface to save data for multiple tags
#define FORMAT_SPIFFS_IF_FAILED true

uint8_t mode = 1;                                                  // modes: 1 = Reader/ Writer, 2 = Emulation
// Replace with your network credentials
const char* ssid = "OnePlus 10 Pro 5G";
const char* password = "JerryTom";
bool ledState = 0;
const int ledPin = 2;
// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
bool is_new_message = false;
String final_message;
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include "SimpleTimer.h"
#include "HardwareSerial.h"
extern "C" {
#include "bep_host_if.h"
#include "com_common.h"
#include "platform.h"
#include "fpc_bep_types.h"
#include "fpc_hcp_common.h"
#include "fpc_com_result.h"
#include "fpc_hcp.h"
#include "platform.h"
#include "com_common.h"
#include "bep_host_if.h"
}
#include "AsyncUDP.h"
#include <SPI.h>
#define SCK_PIN   14
#define MISO_PIN  12
#define MOSI_PIN  13
#define SS_PIN    15
#define VIBRATE 33
#define shutdownpin 18
#define LED_1 26
#define LED_2 27
SPIClass SPI_T(HSPI);

// Display code
SSD1306Wire display(0x3c, SDA, SCL);
#define DEMO_DURATION 3000
String ad = " ";
String ad1 = " ";
typedef void (*Demo)(void);
int demoMode = 0;
int counter = 1;
// Displace code
int template_counter = 1;
   
  
static const int spiClk = 10000000;
SimpleTimer timer;
Ticker touchDetect;
Ticker touchStop;
// WiFi udp prototypes
void startudp();
void sendlock();
void parse_command(String read_string);
AsyncUDP udp;
void stop_timer() {
  touchDetect.detach();
  digitalWrite(LED_1, HIGH);
  digitalWrite(LED_2, HIGH);
}
void blink_touch() {
  digitalWrite(LED_1, !digitalRead(LED_1));
  digitalWrite(LED_2, !digitalRead(LED_2));
}
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP Web Server</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <style>
  html {
    font-family: Arial, Helvetica, sans-serif;
    text-align: center;
  }
  h1 {
    font-size: 1.5rem;
    color: white;
  }
  h3 {
    font-size: 0.8rem;
    color: white;
  }
  h2{
    font-size: 1.5rem;
    font-weight: bold;
    color: #143642;
  }
  .topnav {
    overflow: hidden;
    background-color: #143642;
  }
  body {
    margin: 0;
  }
  .content {
    padding: 30px;
    max-width: 600px;
    margin: 0 auto;
  }
  .card {
    background-color: #F8F7F9;;
    box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5);
    padding-top:10px;
    padding-bottom:20px;
  }
  .button {
    padding: 15px 50px;
    font-size: 24px;
    text-align: center;
    outline: none;
    color: #fff;
    background-color: #0f8b8d;
    border: none;
    border-radius: 5px;
    -webkit-touch-callout: none;
    -webkit-user-select: none;
    -khtml-user-select: none;
    -moz-user-select: none;
    -ms-user-select: none;
    user-select: none;
    -webkit-tap-highlight-color: rgba(0,0,0,0);
   }
   input {
    padding: 10px 40px;
    font-size: 12px;
    text-align: center;
    outline: none;
    color: #fff;
    background-color: #8d0f7d;
    border: none;
    border-radius: 5px;
   }
   /*.button:hover {background-color: #0f8b8d}*/
   .button:active {
     background-color: #0f8b8d;
     box-shadow: 2 2px #CDCDCD;
     transform: translateY(2px);
   }
   .state {
     font-size: 1.5rem;
     color:#8c8c8c;
     font-weight: bold;
   }
  </style>
<title>ESP Web Server</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<link rel="icon" href="data:,">
</head>
<body>
  <div class="topnav">
    <h1>WiFi Fingerprint Demo</h1>
    <h3>Press Key According To Your Need.</h1>
  </div>
  <div class="content">
    <div class="card">
      <h5>  A: Enroll Finger  </h5>
      <h5>  B: Capture and Identify Finger  </h5>
      <h5>  C: Remove all Templates  </h5>
      <h5>  H: Get Version  </h5>
      <h5>  I: Wait for Finger  </h5>
      <h5>  Q: Exit Program  </h5>
      <label for="fname">Input ::</label>
      <input type="text" id="fname" name="fname" autocomplete="off"><br><br>
      <p class="buttton" id="message_ws"> Welcome ! </p>
    </div>
  </div>
<script>
  var gateway = `ws://${window.location.hostname}/ws`;
  var websocket;
  window.addEventListener('load', onLoad);
  function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen    = onOpen;
    websocket.onclose   = onClose;
    websocket.onmessage = onMessage; // <-- add this line
  }
  function onOpen(event) {
    console.log('Connection opened');
  }
  function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
  }
  function onMessage(event) {
    document.getElementById('message_ws').innerHTML = event.data;
  }
  function onLoad(event) {
    initWebSocket();
    initButton();
  }
  function initButton() {
    document.getElementById('button').addEventListener('click', toggle);
  }
  function toggle(){
    websocket.send('toggle');
  }
    var input = document.getElementById("fname");
  // Execute a function when the user releases a key on the keyboard
  input.addEventListener("keyup", function(event) {
    // Number 13 is the "Enter" key on the keyboard
    if (event.keyCode === 13) {
      // Cancel the default action, if needed
      event.preventDefault();
      new_value = document.getElementById("fname").value;
      send_function(new_value);
      document.getElementById("fname").value = "";
    }
  });
function send_function(value_t){
  console.log(value_t);
  websocket.send(value_t);
}
</script>
</body>
</html>
)rawliteral";

//Function to write data into SPIFFS
void SPIFFS_write(fs::FS &fs, const char * path,int template_counter)
{
    Serial.printf("Writing %d to file: %s\r\n",template_counter, path);
    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("- failed to open file for writing");
        return;
    }
    if(file.print(template_counter)){
        Serial.println("- file written");
    } else {
        Serial.println("- write failed");
    }
    file.close();
    
}

//Function to read data from the SPIFFS
int SPIFFS_Read(fs::FS &fs, const char * path){
   int temp_count = 0 ;
   Serial.printf("Reading file: %s\r\n", path);

   File file = fs.open(path);
   
   if(!file || file.isDirectory()){
       Serial.println("− failed to open file for reading");
       return 0;
   }

   Serial.println("− read from file:");
   String content = file.readString();   
   temp_count = content.toInt();                                                                                                                                                                                                          
    printf("Last temp count is :%d\n",temp_count);
     file.close();
   return temp_count;
  
}

void SpiInit(void)
{
  pinMode(SS_PIN, OUTPUT);
  SPI_T.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);         //spi initialization  //CLK,MISO,MOIS,SS
}


void SpiWriteBurstReg(const uint8_t *buffer, uint16_t num)
{
  SPI_T.beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
  digitalWrite(SS_PIN, LOW);
  for (int i = 0; i < num; i++)
  {
    SPI_T.transfer(buffer[i]);
  }
  digitalWrite(SS_PIN, HIGH);
  SPI_T.endTransaction();
}

void SpiReadBurstReg( uint8_t *buffer, uint16_t num)
{
  SPI_T.beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
  digitalWrite(SS_PIN, LOW);
  for (int i = 0; i < num; i++)
  {
    buffer[i] = SPI_T.transfer(0x00);
    //    Serial.println(buffer[i],HEX);
  }
  digitalWrite(SS_PIN, HIGH);
  SPI_T.endTransaction();
}

char *port = NULL;
int speed = 1000;
int timeout = 5;
int index_t;
uint8_t buffer[512];
uint16_t size[2] = { 256, 256 };
fpc_com_chain_t hcp_chain;
void flush_serial() {
}
void IRAM_ATTR isr() {
  printf("\n ISR " );
}
void serial_delay() {
}
void new_function(uint16_t size, const uint8_t *data, uint32_t timeout, void *session)
{
  SpiWriteBurstReg(data, size);
}
void recv_function(uint16_t size, uint8_t *data_d, uint32_t timeout, void *session)
{
  uint16_t size_d = size;
  int i = 0;
  uint32_t timeout_millis = millis();
  bool is_timeout = true;
  while (1) {
    if (digitalRead(2) == 1) break;
  }
  SpiReadBurstReg(data_d, size);
}
static void help(void)
{
  fprintf(stderr, "BEP Host Communication Application\n");
  fprintf(stderr, "Syntax: bep_host_com [-s speed (khz)] [-t timeout] [-h (help)]\n");
}
static const char *result_string(fpc_bep_result_t result)
{
  switch (result) {
    case FPC_BEP_RESULT_OK:
      return "OK";
    case FPC_BEP_RESULT_GENERAL_ERROR:
      return "General Error";
    case FPC_BEP_RESULT_INTERNAL_ERROR:
      return "Internal Error";
    case FPC_BEP_RESULT_INVALID_ARGUMENT:
      return "Invalid Argument";
    case FPC_BEP_RESULT_NOT_IMPLEMENTED:
      return "Not Implemented";
    case FPC_BEP_RESULT_CANCELLED:
      return "Cancelled";
    case FPC_BEP_RESULT_NO_MEMORY:
      return "No Memory";
    case FPC_BEP_RESULT_NO_RESOURCE:
      return "No Resource";
    case FPC_BEP_RESULT_IO_ERROR:
      return "IO Error";
    case FPC_BEP_RESULT_BROKEN_SENSOR:
      return "Broken Sensor";
    case FPC_BEP_RESULT_WRONG_STATE:
      return "Wrong State";
    case FPC_BEP_RESULT_TIMEOUT:
      return "Timeout";
    case FPC_BEP_RESULT_ID_NOT_UNIQUE:
      return "Id Not Unique";
    case FPC_BEP_RESULT_ID_NOT_FOUND:
      return "Id Not Found";
    case FPC_BEP_RESULT_INVALID_FORMAT:
      return "Invalid Format";
    case FPC_BEP_RESULT_IMAGE_CAPTURE_ERROR:
      return "Image Capture Error";
    case FPC_BEP_RESULT_SENSOR_MISMATCH:
      return "Sensor Mismatch";
    case FPC_BEP_RESULT_INVALID_PARAMETER:
      return "Invalid Parameter";
    case FPC_BEP_RESULT_MISSING_TEMPLATE:
      return "Missing Template";
    case FPC_BEP_RESULT_INVALID_CALIBRATION:
      return "Invalid Calibration";
    case FPC_BEP_RESULT_STORAGE_NOT_FORMATTED:
      return "Storage Not Formatted";
    case FPC_BEP_RESULT_SENSOR_NOT_INITIALIZED:
      return "Sensor Not Initialized";
    case FPC_BEP_RESULT_TOO_MANY_BAD_IMAGES:
      return "Too Many Bad Images";
    case FPC_BEP_RESULT_NOT_SUPPORTED:
      return "Not Supported";
    case FPC_BEP_FINGER_NOT_STABLE:
      return "Finger Not Stable";
    case FPC_BEP_RESULT_NOT_INITIALIZED:
      return "Not Initialized";
    default:
      return "Unknown Error";
  }
  return NULL;
}
void PrintBuf(const byte * data, const uint32_t numBytes){ //Print hex data buffer in format
  uint32_t szPos;
  for (szPos=0; szPos < numBytes; szPos++)
  {
    Serial.print(F("0x"));
    ad1 = ad1 + F("0x"); 
    // Append leading 0 for small values
    if (data[szPos] <= 0xF)
      Serial.print(F("0"));
      ad1 = ad1 + F("0");
    Serial.print(data[szPos]&0xff, HEX);
    ad1 = ad1 +String(data[szPos]&0xff, HEX); 
    if ((numBytes > 1) && (szPos != numBytes - 1))
    {
      ad1 = ad1 + F(" ") ;
      Serial.print(F(" "));
    }
  }
  ad1 = ad1 + "l";
  Serial.println("l");
 // ws.textAll(ad1);
}
void displayCardInfo(RfIntf_t RfIntf){ //Funtion in charge to show the card/s in te field
  char tmp[16];
  while (1){
    switch(RfIntf.Protocol){  //Indetify card protocol
    case PROT_T1T:
    case PROT_T2T:
    case PROT_T3T:
    case PROT_ISODEP:
        Serial.print(" - POLL MODE: Remote activated tag type: ");
        Serial.println(RfIntf.Protocol);
        break;
    case PROT_ISO15693:
        Serial.println(" - POLL MODE: Remote ISO15693 card activated");
        break;
    case PROT_MIFARE:
        ws.textAll(" - POLL MODE: Remote MIFARE card activated");
        Serial.println(" - POLL MODE: Remote MIFARE card activated");
        digitalWrite(VIBRATE, HIGH);
        delay(100);
        digitalWrite(VIBRATE, LOW);
        break;
    default:
        Serial.println(" - POLL MODE: Undetermined target");
        return;
    }

    switch(RfIntf.ModeTech) { //Indetify card technology
      case (MODE_POLL | TECH_PASSIVE_NFCA):
          Serial.print("\tSENS_RES = ");
          ad = ad + "\tSENS_RES =  ";
          sprintf(tmp, "0x%.2X",RfIntf.Info.NFC_APP.SensRes[0]);
          Serial.print(tmp); Serial.print(" ");
          ad = ad +tmp+" ";
          sprintf(tmp, "0x%.2X",RfIntf.Info.NFC_APP.SensRes[1]);
          Serial.print(tmp); Serial.println(" ");
           ad = ad +tmp+ " ";
          Serial.print("\tNFCID = ");
          ad = ad + "\tNFCID =  ";
          PrintBuf(RfIntf.Info.NFC_APP.NfcId, RfIntf.Info.NFC_APP.NfcIdLen);
           ad = ad + ad1;
          if(RfIntf.Info.NFC_APP.SelResLen != 0) {
              Serial.print("\tSEL_RES = ");
              ad = ad + "\tSEL_RES =  ";
              sprintf(tmp, "0x%.2X",RfIntf.Info.NFC_APP.SelRes[0]);
              Serial.print(tmp); Serial.println(" ");
              ad = ad +tmp+" ";
          }
          ws.textAll(ad);
      break;
  
      case (MODE_POLL | TECH_PASSIVE_NFCB):
          if(RfIntf.Info.NFC_BPP.SensResLen != 0) {
              Serial.print("\tSENS_RES = ");
              PrintBuf(RfIntf.Info.NFC_BPP.SensRes,RfIntf.Info.NFC_BPP.SensResLen);
          }
          break;
  
      case (MODE_POLL | TECH_PASSIVE_NFCF):
          Serial.print("\tBitrate = ");
          Serial.println((RfIntf.Info.NFC_FPP.BitRate == 1) ? "212" : "424");
          
          if(RfIntf.Info.NFC_FPP.SensResLen != 0) {
              Serial.print("\tSENS_RES = ");
              PrintBuf(RfIntf.Info.NFC_FPP.SensRes,RfIntf.Info.NFC_FPP.SensResLen);
          }
          break;
  
      case (MODE_POLL | TECH_PASSIVE_15693):
          Serial.print("\tID = ");
          PrintBuf(RfIntf.Info.NFC_VPP.ID,sizeof(RfIntf.Info.NFC_VPP.ID));
          
          Serial.print("\ntAFI = ");
          Serial.println(RfIntf.Info.NFC_VPP.AFI);
          
          Serial.print("\tDSFID = ");
          Serial.println(RfIntf.Info.NFC_VPP.DSFID,HEX);
      break;
  
      default:
          break;
    }
    if(RfIntf.MoreTags) { // It will try to identify more NFC cards if they are the same technology
      if(nfc.ReaderActivateNext(&RfIntf) == NFC_ERROR) break;
    }
    else break;
  }
}
void notifyClients() {
  ws.textAll(String(ledState));
}
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  Serial.println("fn::handleWebSocketMessage");
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    String parse_string;
    for(int i = 0; i < len;i++)
    {
      parse_string += (char)data[i];
    }
    final_message = parse_string;
    is_new_message = true;
  }
}
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}
void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}
String processor(const String& var){
  Serial.println(var);
  if(var == "STATE"){
    if (ledState){
      return "ON";
    }
    else{
      return "OFF";
    }
  }
}
void send_ws_message(char *cr){
  Serial.println("fn::send_ws_message");
  String final_msg = String(cr);
  Serial.println(final_msg);
   ws.textAll(final_msg);
}
void setup() {
  delay(10000);
  pinMode(SCK_PIN, OUTPUT);
  pinMode(MOSI_PIN, OUTPUT);
  pinMode(MISO_PIN, INPUT);
  pinMode(SS_PIN, OUTPUT);
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  pinMode(VIBRATE, OUTPUT);
  pinMode(shutdownpin, OUTPUT);
  pinMode(25, OUTPUT);
  pinMode(2, INPUT_PULLUP);
  digitalWrite(shutdownpin, LOW);
  digitalWrite(25, HIGH);
  delay(10);
  digitalWrite(25, LOW);
  delay(10);
  Serial.begin(115200);
  digitalWrite(LED_1, HIGH);
  digitalWrite(LED_2, HIGH);
  SpiInit();
  delay(100);
  digitalWrite(VIBRATE, LOW);
  index_t = 1;
  speed = 10000;
  if (!platform_com_init(port, &speed, timeout)) {
    WRITE("Com initialization failed\n");
    exit(1);
  }

  //SPIFFS initilization begin
  if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){
        Serial.println("SPIFFS Mount Failed");
        return;
    }

  //last read count for template
  template_counter = SPIFFS_Read(SPIFFS,"/.count.txt");
  Serial.println("temp_count from setup :");
  Serial.println(template_counter);
  
  assign_function((void*)new_function);
  assign2_function((void*)recv_function);
  assign3_function((void*)serial_delay);
  assign_callback_forws((void*)send_ws_message);
  init_com_chain(&hcp_chain, buffer, size, NULL);
  WiFi.softAP("Chhavi_VEGG","touchDetect");
  wifiMulti.addAP("OnePlus 10 Pro 5G", "JerryTom");
  while(wifiMulti.run() != WL_CONNECTED) {
        Serial.println("WiFi not connected!");
        delay(1000);
  }
  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());
  initWebSocket();
  
  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  // Start server
  server.begin();
  touchDetect.attach(1, blink_touch);
  // Display 
//  display.init();
//  display.flipScreenVertically();
}
void loop() {
  ws.cleanupClients();
    uint16_t timeout;
    uint16_t sleep_counter;
    sleep_counter = 30;
  digitalWrite(ledPin, ledState);
  if(is_new_message){
    parse_command(final_message);
    is_new_message= false;
  }
}
void parse_command(String read_string){
  Serial.println("fn::parse_command");
  char cmd[100];
  fpc_bep_result_t res = FPC_BEP_RESULT_OK;
  fpc_bep_result_t result = FPC_BEP_RESULT_OK;
  uint16_t template_id;
  bool match;
  int a;
  read_string.toCharArray(cmd, read_string.length() + 1);
  switch (cmd[0]) {
    case 'A':
            res = bep_enroll_finger(&hcp_chain);
            template_id = template_counter;
            result = bep_save_template(&hcp_chain, template_id);
            if(result == FPC_BEP_RESULT_OK)
            ws.textAll("Template saved with ID :: "+String(template_counter));
            template_counter++;
            SPIFFS_write(SPIFFS,"/.count.txt",template_counter);//writes template count to SPIFFS
      break;
    case 'a':
            res = bep_enroll_finger(&hcp_chain);
            template_id = template_counter;
            result = bep_save_template(&hcp_chain, template_id);
            if(result == FPC_BEP_RESULT_OK)
            ws.textAll("Template saved with ID :: "+String(template_counter));
            template_counter++;
            SPIFFS_write(SPIFFS,"/.count.txt",template_counter);//writes template count to SPIFFS
      break;
    case 'b':
      res = bep_identify_finger(&hcp_chain, &template_id, &match);
      if (res == FPC_BEP_RESULT_OK) {
         if (match) {
            char ch[100];
            sprintf(ch, "Match with template id: %d\n", template_id);
            String display_text = "Approved! \n Employee ID:: "+String(template_id);
            ws.textAll(display_text);
            }else{
              String display_text = "Fingerprint Not Added ! Please Add Fingerprint First and Try Again";
              ws.textAll(display_text);
            }
             if(template_id < 2){
                sendlock();
                digitalWrite(VIBRATE, HIGH);
                delay(500);
                digitalWrite(VIBRATE, LOW);
                }
                }else{
                  String display_text = "Try Again!";
                  digitalWrite(VIBRATE, HIGH);
                  delay(500);
                  digitalWrite(VIBRATE, LOW);
                  delay(500);
                  digitalWrite(VIBRATE, HIGH);
                  delay(500);
                  digitalWrite(VIBRATE, LOW);
                  ws.textAll("No match\n");
                  }
      break;
    case 'B':
      res = bep_identify_finger(&hcp_chain, &template_id, &match);
      if (res == FPC_BEP_RESULT_OK) {
         if (match) {
            char ch[100];
            sprintf(ch, "Match with template id: %d\n", template_id);
            String display_text = "Approved! \n Employee ID:: "+String(template_id);
            ws.textAll(display_text);
            }else{
              String display_text = "Fingerprint Not Added ! Please Add Fingerprint First and Try Again";
              ws.textAll(display_text);
            }
            
             if(template_id < 2){
                sendlock();
                digitalWrite(VIBRATE, HIGH);
                delay(500);
                digitalWrite(VIBRATE, LOW);
             }
          } else {
           String display_text = "Try Again!";
            digitalWrite(VIBRATE, HIGH);
            delay(500);
            digitalWrite(VIBRATE, LOW);
            delay(500);
            digitalWrite(VIBRATE, HIGH);
            delay(500);
            digitalWrite(VIBRATE, LOW);
            ws.textAll("No match\n");
            }
       break;
    case 'c':
      res = bep_delete_template(&hcp_chain, REMOVE_ID_ALL_TEMPLATES);
      if(res == FPC_BEP_RESULT_OK)
        ws.textAll("All templated delted!");
      template_counter = 1;
     SPIFFS_write(SPIFFS,"/.count.txt",template_counter);//writes template count to SPIFFS
      break;
    case 'C':
      res = bep_delete_template(&hcp_chain, REMOVE_ID_ALL_TEMPLATES);
      if(res == FPC_BEP_RESULT_OK)
        ws.textAll("All templated delted!");
      template_counter = 1;
     SPIFFS_write(SPIFFS,"/.count.txt",template_counter);//writes template count to SPIFFS
      break;
    /*case 'd':
      
      template_id = template_counter;
      printf("temp id = %d & temp_count = %d\n",template_id,template_counter);
      res = bep_save_template(&hcp_chain, template_id);
      if(res == FPC_BEP_RESULT_OK)
        ws.textAll("Template saved with ID :: "+String(template_counter));
        template_counter++;
        SPIFFS_write(SPIFFS,"/.count.txt",template_counter);//writes template count to SPIFFS
      break;
    case 'D':
      template_id = template_counter;
      printf("temp id = %d & temp_count = %d\n",template_id,template_counter);
      res = bep_save_template(&hcp_chain, template_id);
      if(res == FPC_BEP_RESULT_OK)
        ws.textAll("Template saved with ID :: "+String(template_counter));
        template_counter++;
        SPIFFS_write(SPIFFS,"/.count.txt",template_counter);//writes template count to SPIFFS
      break;*/
    case 'f':
      res = bep_capture(&hcp_chain, 50000);
      break;
    case 'h': {
        char version[100];
        memset(version, 0, 100);
        res = bep_version(&hcp_chain, version, 99);
        if (res == FPC_BEP_RESULT_OK) {
          WRITE("%s\nChavvi Firmware Version : 0.2", version);
          ws.textAll("Chhavi Firmware Version : 0.2");
        }
        break;
      }
    case 'H': {
        char version[100];
        memset(version, 0, 100);
        res = bep_version(&hcp_chain, version, 99);
        if (res == FPC_BEP_RESULT_OK) {
          WRITE("%s\nChhavi Firmware Version : 0.2", version);
          ws.textAll("Chhavi Firmware Version : 0.2");
        }
        break;
      }
    case 'i': {
        uint16_t timeout;
        uint16_t sleep_counter;
        WRITE("Timeout: ");
        timeout = 10000;
        WRITE("Sleep polling interval (4-1020 ms): ");
        sleep_counter = 30;
        ws.textAll("Waiting for finger to detect!");
        res = bep_sensor_wait_for_finger(&hcp_chain, timeout, sleep_counter);
        flush_serial();
        if(res == FPC_BEP_RESULT_OK){
          touchDetect.attach(1, blink_touch);
          touchStop.once(10, stop_timer);
          ws.textAll("Finger Detected!");
          digitalWrite(VIBRATE, HIGH);
          delay(500);
          digitalWrite(VIBRATE, LOW);
        }else{
          Serial.println("Timeout!");
        }
        break;
    }
    case 'I': {
        uint16_t timeout;
        uint16_t sleep_counter;
        WRITE("Timeout: ");
        timeout = 10000;
        WRITE("Sleep polling interval (4-1020 ms): ");
        sleep_counter = 30;
        ws.textAll("Waiting for finger to detect!");
        res = bep_sensor_wait_for_finger(&hcp_chain, timeout, sleep_counter);
        flush_serial();
        if(res == FPC_BEP_RESULT_OK){
          touchDetect.attach(1, blink_touch);
          touchStop.once(10, stop_timer);
          ws.textAll("Finger Detected!");
          digitalWrite(VIBRATE, HIGH);
          delay(500);
          digitalWrite(VIBRATE, LOW);
        }else{
          Serial.println("Timeout!");
        }
        break;
      }
    case 'q':
      ws.textAll("Refresh Web Page To Continue!");
      ESP.restart();
      return;
    case 'Q':
      ws.textAll("Refresh Web Page To Continue!");
      ESP.restart();
      return;
    case 'T':
    ad = " ";
    ad1 = " ";
  Serial.println("Re-initializing...");
  while(!Serial);
  //Serial.println("\nDetect NFC tags with PN7150");
  
  //Serial.println("Initializing...");                
  if (nfc.connectNCI()) { //Wake up the board
    //Serial.println("Error while setting up the mode, check connections!");
    while (1);
  }
  
  if (nfc.ConfigureSettings()) {
    //Serial.println("The Configure Settings is failed!");
    while (1);
  }
  
  if(nfc.ConfigMode(mode)){ //Set up the configuration mode
    //Serial.println("The Configure Mode is failed!!");
    while (1);
  }
  nfc.StartDiscovery(mode); //NCI Discovery mode
  ws.textAll("Waiting for an Card ...");
  Serial.println("Waiting for an Card ...");
  if(!nfc.WaitForDiscoveryNotification(&RfInterface)){ // Waiting to detect cards
    displayCardInfo(RfInterface);
    switch(RfInterface.Protocol) {
      case PROT_T1T:
      case PROT_T2T:
      case PROT_T3T:
      case PROT_ISODEP:
          nfc.ProcessReaderMode(RfInterface, READ_NDEF);
          break;
      
      case PROT_ISO15693:
          break;
      
      case PROT_MIFARE:
          nfc.ProcessReaderMode(RfInterface, READ_NDEF);
          break;
      
      default:
          break;
    }
    
    //* It can detect multiple cards at the same time if they use the same protocol 
    if(RfInterface.MoreTags) {
        nfc.ReaderActivateNext(&RfInterface);
    }
    //* Wait for card removal 
    nfc.ProcessReaderMode(RfInterface, PRESENCE_CHECK);
    ws.textAll("CARD REMOVED!");
    Serial.println("CARD REMOVED!");
  }
  break;
    default:
      WRITE("\nUnknown command\n");
      ws.textAll("Unknown command");
  }
  if (res == FPC_BEP_RESULT_OK) {
    WRITE("\nCommand succeeded\n");    
  } else {
    ws.textAll("Command failed");
    ws.textAll("\nCommand failed with error code %s (%d)\n"+String(result_string(res), res));
    WRITE("\nCommand failed with error code %s (%d)\n", result_string(res), res);
  }
}

 
#include "Arduino.h"
#include <WiFi.h>
#include <WiFiMulti.h>
#include "Ticker.h"
#include <Wire.h>
WiFiMulti wifiMulti;
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Wire.h>               // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h"        // legacy: #include "SSD1306.h"
#include "images.h"
// Replace with your network credentials
const char* ssid = "SSID";
const char* password = "PWD";
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


#define LED_1 26
#define LED_2 27
SPIClass SPI_T(HSPI);

// Display code 

SSD1306Wire display(0x3c, SDA, SCL); 

#define DEMO_DURATION 3000
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
    font-size: 1.8rem;
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
    <h3>Press key according to your need.</h1>
  </div>
  <div class="content">
    <div class="card">
      <h5>  a: Enroll finger  </h5>
      <h5>  b: Capture and identify finger  </h5>
      <h5>  c: Remove all templates  </h5>
      <h5>  d: Save template  </h5>
      <h5>  h: Get version  </h5>
      <h5>  i: Wait for finger  </h5>
      <h5>  q: Exit program  </h5>
      <label for="fname">Input ::</label>
      <input type="text" id="fname" name="fname" autocomplete="off"><br><br>
      <p class="buttton" id="message_ws"> Toggle </p>
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
  //  if (Serial2.available()) {
  ////    Serial.println("Flushing data");
  //    while (Serial2.available()) {
  ////      Serial.println(Serial2.read(), HEX);
  //    };
  //  }

}

void IRAM_ATTR isr() {
  printf("\n ISR " );
}

void serial_delay() {
  //  Serial.println("fn::serial_delay");
  //  while (!Serial2.available()) {};
}

void new_function(uint16_t size, const uint8_t *data, uint32_t timeout, void *session)
{
  //  printf("new_function::size %d \n", size);
  //  for (int i = 0; i < size; i++)
  //  {
  //    printf("%02X \n", data[i]);
  //  }
  SpiWriteBurstReg(data, size);
}


void recv_function(uint16_t size, uint8_t *data_d, uint32_t timeout, void *session)
{
  uint16_t size_d = size;
  //  Serial.println("fn:recv_function");
  //  Serial.println(size);
  //  Serial.println(timeout);
  delay(10);
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

  pinMode(SCK_PIN, OUTPUT);
  pinMode(MOSI_PIN, OUTPUT);
  pinMode(MISO_PIN, INPUT);
  pinMode(SS_PIN, OUTPUT);
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);

  pinMode(VIBRATE, OUTPUT);

  pinMode(25, OUTPUT);
  pinMode(2, INPUT_PULLUP);
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

  //  if (speed > 10000) {
  speed = 10000;
  //  }

  if (!platform_com_init(port, &speed, timeout)) {
    WRITE("Com initialization failed\n");
    exit(1);
  }
  //  send_data_t();
  assign_function((void*)new_function);
  assign2_function((void*)recv_function);
  assign3_function((void*)serial_delay);
  assign_callback_forws((void*)send_ws_message);
  init_com_chain(&hcp_chain, buffer, size, NULL);
  timer.setInterval(1000, []() {
//    digitalWrite(LED_1, !digitalRead(LED_1));
//    digitalWrite(LED_2, !digitalRead(LED_2));
  });

  WiFi.softAP("Chhavi_VEGG","touchDetect");
  wifiMulti.addAP("VEGG_5", "sss3kk2aaaa4");
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
  display.init();

  display.flipScreenVertically();
//  display.setFont(ArialMT_Plain_10);
  
  showText("Chhavi",30,20);
//  testdrawchar();
}

void loop() {
  ws.cleanupClients();
  timer.run();
  digitalWrite(ledPin, ledState);
  if(is_new_message){
//    Serial.println(final_message);
    parse_command(final_message);
    is_new_message= false;
  }
//  char cmd[100];
//  fpc_bep_result_t res = FPC_BEP_RESULT_OK;
//  uint16_t template_id;
//  bool match;
//
//  platform_clear_screen();
//  WRITE("BEP Host Interface\n");
//  //        WRITE("[speed: %d]\n", speed);
//  WRITE("Timeout: %ds\n", timeout);
//  WRITE("-------------------\n\n");
//  WRITE("Possible options:\n");
//  WRITE("a: Enroll finger\n");
//  WRITE("b: Capture and identify finger\n");
//  WRITE("c: Remove all templates\n");
//  WRITE("d: Save template\n");
//  WRITE("e: Remove template\n");
//  WRITE("f: Capture image\n");
//  WRITE("g: Image upload\n");
//  WRITE("h: Get version\n");
//  WRITE("i: Wait for finger\n");
//  WRITE("q: Exit program\n");
//  WRITE("\nOption>> ");
//  while (!Serial.available()) {
//    timer.run();
//  }
//  timer.run();
//  String read_string = Serial.readString();
//  read_string.toCharArray(cmd, read_string.length() + 1);
//  switch (cmd[0]) {
//    case 'a':
//      res = bep_enroll_finger(&hcp_chain);
//      break;
//    case 'b':
//      res = bep_identify_finger(&hcp_chain, &template_id, &match);
//      if (res == FPC_BEP_RESULT_OK) {
//        if (match) {
//          WRITE("Match with template id: %d\n", template_id);
//        } else {
//          WRITE("No match\n");
//        }
//      }
//      break;
//    case 'c':
//      res = bep_delete_template(&hcp_chain, REMOVE_ID_ALL_TEMPLATES);
//      break;
//    case 'd':
//      WRITE("Template id: ");
//      while (!Serial.available()) {
//        timer.run();
//      }
//      read_string = Serial.readString();
//      read_string.toCharArray(cmd, read_string.length() + 1);
//      fgets(cmd, sizeof(cmd), stdin);
//      template_id = atoi(cmd);
//      res = bep_save_template(&hcp_chain, template_id);
//      break;
//    case 'e':
//      WRITE("Template id: ");
//      while (!Serial.available()) {
//        timer.run();
//      }
//      read_string = Serial.readString();
//      read_string.toCharArray(cmd, read_string.length() + 1);
//      fgets(cmd, sizeof(cmd), stdin);
//      template_id = atoi(cmd);
//      res = bep_delete_template(&hcp_chain, template_id);
//      break;
//    case 'f':
//      WRITE("Timeout: ");
//      fgets(cmd, sizeof(cmd), stdin);
//      res = bep_capture(&hcp_chain, 50000);
//      break;
//    case 'g': {
//        uint32_t size;
//        res = bep_image_get_size(&hcp_chain, &size);
//        if (res == FPC_BEP_RESULT_OK) {
//          uint8_t *buf = (uint8_t*)malloc(size);
//          if (buf) {
//            res = bep_image_get(&hcp_chain, buf, size);
//            if (res == FPC_BEP_RESULT_OK) {
//              FILE *f = fopen("image.raw", "wb");
//              if (f) {
//                fwrite(buf, 1, size, f);
//                fclose(f);
//                WRITE("Image saved as image.raw\n");
//              }
//            }
//          }
//
//        }
//        break;
//      }
//    case 'h': {
//        char version[100];
//        memset(version, 0, 100);
//        res = bep_version(&hcp_chain, version, 99);
//        if (res == FPC_BEP_RESULT_OK) {
//          WRITE("%s\n", version);
//        }
//        break;
//      }
//    case 'i': {
//        uint16_t timeout;
//        uint16_t sleep_counter;
//        WRITE("Timeout: ");
//        timeout = 60000;
//        WRITE("Sleep polling interval (4-1020 ms): ");
//        sleep_counter = 30;
//        res = bep_sensor_wait_for_finger(&hcp_chain, timeout, sleep_counter);
//        flush_serial();
//        //        printf("Waiting Out\n!");
//        //        res = receive_result_no_args(&hcp_chain);
//        break;
//      }
//    case 'q':
//      ESP.restart();
//      return;
//    default:
//      WRITE("\nUnknown command\n");
//  }
//  if (res == FPC_BEP_RESULT_OK) {
//    WRITE("\nCommand succeeded\n");
//  } else {
//    WRITE("\nCommand failed with error code %s (%d)\n", result_string(res), res);
//  }
//  WRITE("Press enter to continue...");
//  fgets(cmd, sizeof(cmd), stdin);
}

void parse_command(String read_string){
  Serial.println("fn::parse_command");
  char cmd[100];
  fpc_bep_result_t res = FPC_BEP_RESULT_OK;
  uint16_t template_id;
  bool match;
  read_string.toCharArray(cmd, read_string.length() + 1);
  switch (cmd[0]) {
    case 'a':
      res = bep_enroll_finger(&hcp_chain);
      break;
    case 'b':
      res = bep_identify_finger(&hcp_chain, &template_id, &match);
      if (res == FPC_BEP_RESULT_OK) {
         if (match) {
            char ch[100];
            sprintf(ch, "Match with template id: %d\n", template_id);
            String display_text = "Approved! \n Employee ID:: "+String(template_id);
            showText(display_text,0,0);
            timer.setTimeout(3000, [](){
              showText("Welcome!\n ",30,20);
              
            });
            
             if(template_id < 2){
                sendlock();
                digitalWrite(VIBRATE, HIGH);
                timer.setTimeout(50,[](){
                  digitalWrite(VIBRATE, LOW);
                });
             }
             ws.textAll(String(ch));
          } else {
           String display_text = "Try Again!";
           showText(display_text,30,20);
            timer.setTimeout(2000, [](){
              showText("Welcome!",30,20);
           });
            digitalWrite(VIBRATE, HIGH);
             timer.setTimeout(50,[](){
              digitalWrite(VIBRATE, LOW);
              timer.setTimeout(400,[](){
                  digitalWrite(VIBRATE, HIGH);
                  timer.setTimeout(50,[](){
                      digitalWrite(VIBRATE, LOW);
                  });    
              });
            });
            ws.textAll("No match\n");
          }
      }
    
      break;
    case 'c':
      res = bep_delete_template(&hcp_chain, REMOVE_ID_ALL_TEMPLATES);
      if(res == FPC_BEP_RESULT_OK)
        ws.textAll("All templated delted!");
      template_counter = 1;
      break;
    case 'd':
      template_id = template_counter;
      res = bep_save_template(&hcp_chain, template_id);
      if(res == FPC_BEP_RESULT_OK)
        ws.textAll("Template saved with ID :: "+String(template_counter));
      template_counter++;
      break;
    case 'f':
      res = bep_capture(&hcp_chain, 50000);
      break;
    case 'h': {
        char version[100];
        memset(version, 0, 100);
        res = bep_version(&hcp_chain, version, 99);
        if (res == FPC_BEP_RESULT_OK) {
          WRITE("%s\n", version);
          ws.textAll(String(version));
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
//          sendlock();
        }else{
          Serial.println("Timeout!");
        }
        break;
      }
    case 'q':
      ESP.restart();
      return;
    default:
      WRITE("\nUnknown command\n");
      ws.textAll("Unknown command");
  }
  if (res == FPC_BEP_RESULT_OK) {
//    ws.textAll("Command succeeded");
    WRITE("\nCommand succeeded\n");
  } else {
    ws.textAll("Command failed");
    WRITE("\nCommand failed with error code %s (%d)\n", result_string(res), res);
  }
}

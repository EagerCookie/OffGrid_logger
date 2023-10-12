#include <DebugMacros.h>
#include <HTTPSRedirect.h>


#include <SoftwareSerial.h>
//#include <EEPROM.h>>

#include <WebSocketsServer.h>

#include <ESP8266WiFi.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266WebServer.h>


#define UPD_ADDR 0;
//#define SEND_TO_SHEETS_ADDR 0;

WebSocketsServer webSocket = WebSocketsServer(81);


const int SEND_TO_SHEETS_ADDR = 0;
 
int update_interval;
bool send_to_sheets;
bool read_invertor;
bool read_BMS;


//const char* host = "webhook.site"; // Change to your script's domain
const char* host = "script.google.com"; 
const String key = "";            // SHOULD insert your key for Google Sheets logging
//const char* host = "webhook.site"; 
const int httpsPort = 443; // HTTPS port

HTTPSRedirect* client = nullptr;

//Rresponse size of QPIGS command is always constant. CRC byte could respresent special characters as a CHAR, so it needs to be processed.
String QPIGSdata = "(BBB.B CC.C DDD.D EE.E FFFF GGGG HHH III JJ.JJ KKK OOO TTTT EEEE UUU.U WW.WW PPPPP 76543210 QQ VV MMMMM 098Cn"; 
//byte QPIGSdata[109]; 
byte BMSdata[279]; //size depends on current configuration of battery. For example amount of cells 
String BMSdataString; // String representation of BMSdata byte array

#define rxOffGrid 4
#define txOffGrid 5

SoftwareSerial OffGridSerial (rxOffGrid, txOffGrid);

/*
#define rxBMS 12
#define txBMS 14

SoftwareSerial BMSSerial (rxBMS, txBMS);
*/
ESP8266HTTPUpdateServer httpUpdate;
ESP8266WebServer server(80); // create a web server on port 80


unsigned long previousMillis = 0; 
const long interval = 3000;



const char* ssid = ""; // replace with your Wi-Fi network name
const char* password = ""; // replace with your Wi-Fi password


void byteArrayToString(byte byteArray[], int length, String& result) {
  result = ""; // Clear the result string

  for (int i = 0; i < length; i++) {
    // Convert each byte to a two-character hexadecimal representation
    char hex[3]; // A buffer to hold the hexadecimal representation (2 characters + null terminator)
    sprintf(hex, "%02X", byteArray[i]);
    
    // Concatenate the hexadecimal value to the result string
    result += hex;
  }
}


void readQPIGS(String& response){
  //OffGridSerial.listen();
  byte QPIGS[8] = {0x51, 0x50, 0x49, 0x47, 0x53, 0xb7, 0xa9, 0x0d};
  OffGridSerial.write(QPIGS, 8);
  //QPIGS format  (BBB.B CC.C DDD.D EE.E FFFF GGGG HHH III JJ.JJ KKK OOO TTTT EEEE UUU.U WW.WW PPPPP 76543210 QQ VV MMMMM 098CRCc
 response = "";
 unsigned long requestStartTime = millis();
  
  byte QPIGSsize = 109;
  byte bytesRead = 0;
//  String response = "";
  
  //Comparing data size with recieved data needs to avoid Serial software 64 bytes length limit
  while (QPIGSsize>=bytesRead){
    
    //to avoid stucking in the endless cycle when connection was interrupted
/*  if((millis() - requestStartTime)> 5000){
      break;
      }*/
    while(OffGridSerial.available()>0){
      char symbol = (char)OffGridSerial.read();
      if(symbol >= 32 && symbol<=57) // filtering only printable characters
        response += symbol;
      bytesRead++;
  
    }
  }

  //return response;
  
}


void readBMS(byte* BMSdata){
  //BMSSerial.listen();
//BMSdata=[];
  int bytesRead= 0;
  const byte BMSRequest[21] = {0x4E, 0x57, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x68, 0x00, 0x00, 0x01, 0x29};
  
  
  Serial.write(BMSRequest, 21);
  //delay(30); //without delay dont cath incoming data , optimal is 20
//  int bytesRead=0;  
  unsigned long requestStartTime = millis();
    while (279>=bytesRead){
      if((millis() - requestStartTime)> 1000){
      break;
      }
  while(Serial.available()>0){
   /*
    if((millis() - requestStartTime)> 2000){
      break;
    }*/
    //BMSdata[1]=0x3;
   // int rlen = Serial.readBytes(BMSdata, 279);
    BMSdata[bytesRead] = Serial.read();
    //BMSdata[bytesRead] = 0x1;
    bytesRead ++;
  }  
    }
  //BMSdata[0]=bytes;
//return BMSdata;
}


void setParams(){
  if(server.hasArg("sendToSheets")){
       byte value=server.arg("sendToSheets").toInt();
        if (value==1){
        send_to_sheets = true;
       // EEPROM.write(SEND_TO_SHEETS_ADDR,1);
        }
        else{
          send_to_sheets = false;
         // EEPROM.write(SEND_TO_SHEETS_ADDR,0);
          }
        //EEPROM.commit();
        }
      server.send(200);
  }


void requestRawBMS(){
   server.send(200, "application/octet-stream", BMSdata, 279);
  }
void requestData() {
 
  
  server.send(200, "text/plain", QPIGSdata); // send the message to the client
}

void requestBMS() {
  //readBMS();
  //byteArrayToString(BMSdata, 279, BMSdataString);
  server.send(200, "text/plain", BMSdataString); // send the message to the client
}


void handleRoot() {
  server.send(200, "text/html", "<html><body><a href='/index.html'>UI</a><br></h1><a href='/update'>Update</a><br><a href='/setParams?sendToSheets=1'>EnableLog</a></body></html>"); // display a simple HTML page
}

void handleResource() {
  String resourcePath = server.uri(); // get the requested resource path
  String contentType = "text/plain"; // set the content type to plain text by default
  ////Serial1.println(resourcePath);


  if (resourcePath.endsWith(".html")) { // check if the resource is an HTML file
    contentType = "text/html"; // set the content type to HTML
  } else if (resourcePath.endsWith(".css")) { // check if the resource is a CSS file
    contentType = "text/css"; // set the content type to CSS
  } else if (resourcePath.endsWith(".js")) { // check if the resource is a JavaScript file
    contentType = "application/javascript"; // set the content type to JavaScript
  }
  
  if (SPIFFS.exists(resourcePath)) { // check if the resource exists in the internal memory
    File file = SPIFFS.open(resourcePath, "r"); // open the resource file in read mode
    server.streamFile(file, contentType); // stream the file to the client with the correct content type
    file.close(); // close the file
  } else { // if the resource does not exist
    server.send(404, "text/plain", "Resource not found"+resourcePath); // send a 404 Not Found status
  }
}







void setup() {

 // EEPROM.begin(64);
  //send_to_sheets=EEPROM.read(SEND_TO_SHEETS_ADDR);
  pinMode(rxOffGrid, INPUT);
  pinMode(txOffGrid, OUTPUT);
  OffGridSerial.begin(2400);

  pinMode(2, OUTPUT);
  digitalWrite(2,HIGH);

/*
  pinMode(rxBMS, INPUT);
  pinMode(txBMS, OUTPUT);
  BMSSerial.begin(115200);
*/
  Serial.begin(115200);
  
  WiFi.mode(WIFI_STA); // set ESP8266 in Station mode
  WiFi.disconnect();
  if (WiFi.SSID()) { // check if Wi-Fi credentials are stored
    WiFi.begin(ssid, password); // connect to Wi-Fi
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 10) { // make few attempts to connect
      delay(3000);
      attempts++;
    }
    if (WiFi.status() == WL_CONNECTED) {
      //Serial1.println(WiFi.localIP());
    } else {
      //Serial1.println("Failed to connect to stored Wi-Fi.");
      WiFi.disconnect(); // disconnect from Wi-Fi
    }
  } else {
    //Serial1.println("No stored Wi-Fi credentials found.");
    WiFi.mode(WIFI_AP); // set ESP8266 in Access Point mode
    WiFi.softAP(ssid, password); // start AP
    //Serial1.print("AP started with SSID: ");
    //Serial1.println(ssid);
    //Serial1.print("AP IP address: ");
    //Serial1.println(WiFi.softAPIP());
  }
  server.on("/", handleRoot); // handle requests to the root URL
  server.on("/data",requestData);  
  server.on("/bms",requestBMS);
  server.on("/rawBms",requestRawBMS);
  server.on("/setParams",setParams);
  server.onNotFound(handleResource); // handle requests to all other URLs

  httpUpdate.setup(&server);
  server.begin(); // start the server
  //Serial1.println("Web server started");



  // Use HTTPSRedirect to connect to the server
  client = new HTTPSRedirect(httpsPort);
  client->setInsecure();
  client->setPrintResponseBody(false);
  client->setContentTypeHeader("application/x-www-form-urlencoded"); // Adjust the content type if needed

  SPIFFS.begin();
   webSocket.begin();
  }


String urlEncode(const String &input) {
  String encodedString = "";
  
  for (char c : input) {
    if (isAlphaNumeric(c) || c == '-' || c == '_' || c == '.' || c == '~') {
      encodedString += c;
    } else {
      encodedString += String("%") + String(c, HEX);
    }
  }

  return encodedString;
}


void loop() {
  webSocket.loop();
  server.handleClient(); // handle client requests
  
   unsigned long currentMillis = millis();
  

  if (currentMillis - previousMillis >= interval) {
    
    readQPIGS(QPIGSdata);
    readBMS(BMSdata);
    byteArrayToString(BMSdata, 279, BMSdataString);
    webSocket.broadcastTXT(QPIGSdata);
    //delay(300);
    webSocket.broadcastBIN(BMSdata,279,true);
    if(send_to_sheets == true){
      digitalWrite(2, LOW);
    if (client->connect(host, httpsPort)) {
        
        String url = "/macros/s/"+key+"/exec?invertorData="+urlEncode(QPIGSdata)+"%20"+BMSdataString; // Replace with your script URL and parameters
        //For test purposes
        //String url = "/87770fa8-ea10-474d-a858-bd9238b11cf9/exec?invertorData="+urlEncode(QPIGSdata)+"%20"+BMSdataString; // Replace with your script URL and parameters
        //String url = "/f1a643b7-8127-46c3-ba97-78bf64508eb3?invertorData="+urlEncode(QPIGSdata)+"%20"+BMSdataString; // Replace with your script URL and parameters
        client->print(String("GET ") + url + " HTTP/1.1\r\n" +
                      "Host: " + host + "\r\n" +
                      "Connection: close\r\n\r\n");
      }
       client->stop();
       digitalWrite(2, HIGH);
    }
       
       
    
  previousMillis = currentMillis;
    
 
  }
}

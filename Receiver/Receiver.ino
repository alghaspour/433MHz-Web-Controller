#include <ESP8266WiFi.h>
#include <SPI.h>
#include <SD.h>
#include <RCSwitch.h>

RCSwitch mySwitch = RCSwitch();
const int receiverPin = D4;

const int sdPin = D8;

static const char* bin2tristate(const char* bin);
static char * dec2binWzerofill(unsigned long Dec, unsigned int bitLength);

const String pageHead = "<!doctype html><html lang='en'><head><meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1'><title>433MHz Remotes Controller</title></head><body><script>function run(){var e=new XMLHttpRequest,t='http://'+document.getElementById('tip').value+'/send'+document.getElementById('sendcode').value+'pl'+document.getElementById('pl').value+'proto'+document.getElementById('proto').value;e.open('GET',t,!0),e.onreadystatechange=function(){4==this.readyState&&200==this.status?(document.getElementById('httpres').style.background='green',document.getElementById('httpres').innerHTML='SUCCESS SENDING CODE : '+this.responseText):(document.getElementById('httpres').style.background='red',document.getElementById('httpres').innerHTML='ERROR SENDING CODE : '+this.status+' : '+this.responseText)},e.send()}function reloadPage(){window.location.href='/'}function erase(){var e=new XMLHttpRequest;e.open('GET','/trunc',!0),e.onreadystatechange=function(){4==this.readyState&&200==this.status?(document.getElementById('httpres').style.background='green',document.getElementById('httpres').innerHTML='SD ERASE SUCCESS : '+this.responseText,window.location.href='/?erased'):(document.getElementById('httpres').style.background='red',document.getElementById('httpres').innerHTML='SD ERASE ERROR :  : '+this.responseText)},e.send()}</script><div style='width: 100%;position: fixed;top: 0;background: #555;color: #f1f1f1;padding: 10px;height: 120px;'><h3>433MHz Remotes Controller</h3><hr>Code : <input type='text' name='sendcode' id='sendcode' placeholder='binary code' size='25'>&nbsp;&nbsp;&nbsp;Pulse Length : <input type='text' name='pl' id='pl' size='4'>&nbsp;&nbsp;&nbsp;Protocol : <input type='text' name='proto' id='proto' size='1'>&nbsp;&nbsp;&nbsp;Transmitter IP : <input type='text' name='tip' id='tip' placeholder='Transmitter IP Address'>&nbsp;&nbsp;&nbsp;<button type='button' onclick='run()'>Send Code</button>&nbsp;&nbsp;&nbsp;<span id='httpres' style='color: #f1f1f1;padding: 4px;'></span><br><br><button type='button' onclick='erase()' style='float: right;margin-right: 2rem;'>Erase SD</button><button type='button' onclick='reloadPage()' style='float: right;margin-right: 3rem;'>Reload Page</button></div><div style='padding-top: 150px;'></div></body></html>";

const String pageBottom = "</div></body></html>";

const char* ssid = "";
const char* password = "";

WiFiServer server(80);

void setup() {
  
    Serial.begin(9600);
    
    mySwitch.enableReceive(receiverPin);
  
    if (!SD.begin(sdPin)) {
      Serial.println("SD Initialization failed!");
    }

    // Connect to WiFi network
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
   
    WiFi.begin(ssid, password);
   
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
   
    // Start the server
    server.begin();
    Serial.println("Server started");
   
    // Print the IP address
    Serial.print("Use this URL : ");
    Serial.print("http://");
    Serial.print(WiFi.localIP());
    Serial.println("/");
}

void loop() {

    if(mySwitch.available()) {
    output(mySwitch.getReceivedValue(), mySwitch.getReceivedBitlength(), mySwitch.getReceivedDelay(), mySwitch.getReceivedRawdata(),mySwitch.getReceivedProtocol());    
    mySwitch.resetAvailable();
    }
    delay(100);
    
    // Check if a client has connected
    WiFiClient client = server.available();
    if (!client) {
      return;
    }
   
    // Wait until the client sends some data
    //Serial.println("new client");
    while(!client.available()){
      delay(1);
    }
   
    // Read the first line of the request
    String request = client.readStringUntil('\r');
    //Serial.println(request);
    client.flush();
   
    // Match the request
    if(request.indexOf("/send") != -1){      
      request.replace("GET /send","");
      request.replace(" HTTP/1.1","");
      request.trim();
      const char* charCode = request.c_str();
      mySwitch.setPulseLength(349);
      mySwitch.setProtocol(1);
      mySwitch.send(charCode);
      client.println(charCode);
      
    }else if(request.indexOf("/trunc") != -1){
      SD.remove("datalog.txt");
      File dataFile = SD.open("datalog.txt", FILE_WRITE);
      dataFile.close();
      client.println("Datalog Cleared Successfully!");
      
    }else{
    // Return the response
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Access-Control-Allow-Origin: *");
    client.println("");

    
    client.print(pageHead);
    
      File dataFile = SD.open("datalog.txt");
      if (dataFile) {
        while (dataFile.available()) {
          char c = dataFile.read();
          if (c == '\n'){
          client.print("<br>");
          }else{
          client.print(c);
          }
        }
        dataFile.close();
      } else {
        Serial.println("error opening test.txt");
      }
      
    client.println(pageBottom);  
    }
    
    delay(100);

}

void output(unsigned long decimal, unsigned int length, unsigned int delay, unsigned int* raw, unsigned int protocol) {

  const char* b = dec2binWzerofill(decimal, length);
  Serial.print("Decimal: ");
  Serial.print(decimal);
  Serial.print(" (");
  Serial.print( length );
  Serial.print("Bit) Binary: ");
  Serial.print( b );
  Serial.print(" Tri-State: ");
  Serial.print( bin2tristate( b) );
  Serial.print(" PulseLength: ");
  Serial.print(delay);
  Serial.print(" microseconds");
  Serial.print(" Protocol: ");
  Serial.println(protocol);
  
  Serial.print("Raw data: ");
  for (unsigned int i=0; i<= length*2; i++) {
    Serial.print(raw[i]);
    Serial.print(",");
  }
  Serial.println();
  Serial.println();
  
  File dataFile = SD.open("datalog.txt", FILE_WRITE);
    if (dataFile) {
      dataFile.print("Decimal: ");
      dataFile.print(decimal);
      dataFile.print(" (");
      dataFile.print( length );
      dataFile.println("Bit)");
      dataFile.print("Binary: ");
      dataFile.println( b );
      dataFile.print(" Tri-State: ");
      dataFile.println( bin2tristate( b) );
      dataFile.print(" PulseLength: ");
      dataFile.print(delay);
      dataFile.println(" microseconds");
      dataFile.print(" Protocol: ");
      dataFile.println(protocol);
      
      dataFile.print("Raw data: ");
      for (unsigned int i=0; i<= length*2; i++) {
        dataFile.print(raw[i]);
        dataFile.print(",");
      }
      dataFile.println();
      dataFile.println();      
      dataFile.close();
      Serial.println("Write OK!");
    }else{
      Serial.println("SD Write Error");
    }
}

static const char* bin2tristate(const char* bin) {
  static char returnValue[50];
  int pos = 0;
  int pos2 = 0;
  while (bin[pos]!='\0' && bin[pos+1]!='\0') {
    if (bin[pos]=='0' && bin[pos+1]=='0') {
      returnValue[pos2] = '0';
    } else if (bin[pos]=='1' && bin[pos+1]=='1') {
      returnValue[pos2] = '1';
    } else if (bin[pos]=='0' && bin[pos+1]=='1') {
      returnValue[pos2] = 'F';
    } else {
      return "not applicable";
    }
    pos = pos+2;
    pos2++;
  }
  returnValue[pos2] = '\0';
  return returnValue;
}

static char * dec2binWzerofill(unsigned long Dec, unsigned int bitLength) {
  static char bin[64]; 
  unsigned int i=0;

  while (Dec > 0) {
    bin[32+i++] = ((Dec & 1) > 0) ? '1' : '0';
    Dec = Dec >> 1;
  }

  for (unsigned int j = 0; j< bitLength; j++) {
    if (j >= bitLength - i) {
      bin[j] = bin[ 31 + i - (j - (bitLength - i)) ];
    } else {
      bin[j] = '0';
    }
  }
  bin[bitLength] = '\0';
  
  return bin;
}

#include <ESP8266WiFi.h>
#include <RCSwitch.h>

RCSwitch mySwitch = RCSwitch();

const char* ssid = "";
const char* password = "";

WiFiServer server(80);

void setup() {
  
    Serial.begin(9600);

    mySwitch.enableTransmit(D2);
    mySwitch.setRepeatTransmit(15);

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
      String charCode = request.c_str();
      
      int plIndex=charCode.indexOf("pl");
      int protoIndex=charCode.indexOf("proto");
      String sendCode=charCode.substring(0, plIndex+1);
      String pl=charCode.substring(plIndex+2, protoIndex);
      String proto=charCode.substring(protoIndex+5, charCode.length());

      mySwitch.setProtocol(proto.toInt(),pl.toInt());
      
      char char_array[sendCode.length()];
      sendCode.toCharArray(char_array, sendCode.length());
      mySwitch.send(char_array);
      delay(500);
      
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/html");
      client.println("Access-Control-Allow-Origin: *");
      client.println("");
      //client.println(charCode);
      //client.println(sendCode);
      client.println(pl.toInt());
      client.println(proto);
      client.println(char_array);
      
    }else{
    // Return the response
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Access-Control-Allow-Origin: *");
    client.println("");
    client.println("433MHz Transmitter Web Server");
    }
    
    delay(100);

}

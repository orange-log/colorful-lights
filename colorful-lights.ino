/*--------------------------------------------------
HTTP 1.1 Webserver for ESP8266
for ESP8266 adapted Arduino IDE

Stefan Thesen 04/2015

Running stable for days
(in difference to all samples I tried)

Does HTTP 1.1 with defined connection closing.
Reconnects in case of lost WiFi.
Handles empty requests in a defined manner.
Handle requests for non-exisiting pages correctly.

This demo allows to switch two functions:
Function 1 creates serial output and toggels GPIO2
Function 2 just creates serial output.

Serial output can e.g. be used to steer an attached
Arduino, Raspberry etc.
--------------------------------------------------*/

#include <ESP8266WiFi.h>
#include <NeoPixelBus.h>

const char* ssid = "abc";  // WLAN SSID
const char* password = "123";  // WLAN Password

const int pixelCount = 70;  //Anzahl LED
const int ledPort = 2;  // Ausgabeport
const int colorSaturation = 255;

unsigned long ulReqcount;
unsigned long ulReconncount;

NeoPixelBus strip = NeoPixelBus(pixelCount, ledPort);

RgbColor red = RgbColor(colorSaturation, 0, 0);
RgbColor green = RgbColor(0, colorSaturation, 0);
RgbColor blue = RgbColor(0, 0, colorSaturation);
RgbColor white = RgbColor(colorSaturation);
RgbColor yellow = RgbColor(colorSaturation, colorSaturation, 0);
RgbColor pink = RgbColor(colorSaturation, 0, colorSaturation);
RgbColor cyan = RgbColor(0, colorSaturation, colorSaturation);
RgbColor black = RgbColor(0);

String config = "";

// Create an instance of the server on Port 80
WiFiServer server(80);

void setup()
{
  // setup globals
  ulReqcount=0;
  ulReconncount=0;

  // prepare GPIO2
  pinMode(ledPort, OUTPUT);
  digitalWrite(ledPort, 0);

  // start serial
  Serial.begin(9600);
  delay(1);

  // inital connect
  WiFi.mode(WIFI_STA);
  // WiFiStart();

  // this resets all the neopixels to an off state
  strip.Begin();
  strip.Show();
}

void WiFiStart()
{
  ulReconncount++;

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
  Serial.println(WiFi.localIP());
}

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1  };
  int maxIndex = data.length()-1;
  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
      found++;
      strIndex[0] = strIndex[1]+1;
      strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }
  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}
void colorfulLights(String sCmd)
{
  RgbColor color = white;

  String sR = getValue(sCmd, ',', 1);
  String sG = getValue(sCmd, ',', 2);
  String sB = getValue(sCmd, ',', 3);
  int iR = sR.toInt();
  int iG = sG.toInt();
  int iB = sB.toInt();

  String sDelay = getValue(sCmd, ';', 1);
  int iDelay = 1000;
  if(sDelay != "") {
    iDelay = sDelay.toInt();
  }

  if(sCmd.indexOf("RGB")>=0) {
    color = RgbColor(iR,iG,iB);
  }
  else if(sCmd.indexOf("RED")>=0){
    color = red;
  }
  else if(sCmd.indexOf("GREEN")>=0){
    color = green;
  }
  else if(sCmd.indexOf("BLUE")>=0){
    color = blue;
  }
  else if(sCmd.indexOf("WHITE")>=0){
    color = white;
  }
  else if(sCmd.indexOf("YELLOW")>=0){
    color = yellow;
  }
  else if(sCmd.indexOf("PINK")>=0){
    color = pink;
  }
  else if(sCmd.indexOf("CYAN")>=0){
    color = cyan;
  }
  else if(sCmd.indexOf("BLACK")>=0){
    color = black;
  }

  for (int zaehler=0; zaehler<pixelCount; zaehler = zaehler+1){
    strip.SetPixelColor(zaehler, color);
    if(sCmd.indexOf("WAVE")>=0) {
      strip.Show();
      delay(iDelay);
    }
  }

  strip.Show();
  delay(iDelay);
}

void loop()
{

  colorfulLights(config);

  // check if WLAN is connected
  if (WiFi.status() != WL_CONNECTED)
  {
    WiFiStart();
  }

  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client)
  {
    return;
  }

  // Wait until the client sends some data
  Serial.println("new client");
  unsigned long ultimeout = millis()+250;
  while(!client.available() && (millis()<ultimeout) )
  {
    delay(1);
  }
  if(millis()>ultimeout)
  {
    Serial.println("client connection time-out!");
    return;
  }

  // Read the first line of the request
  String sRequest = client.readStringUntil('\r');
  //Serial.println(sRequest);
  client.flush();

  // stop client, if request is empty
  if(sRequest=="")
  {
    Serial.println("empty request! - stopping client");
    client.stop();
    return;
  }

  // get path; end of path is either space or ?
  // Syntax is e.g. GET /?pin=MOTOR1STOP HTTP/1.1
  String sPath="",sParam="", sCmd="";
  String sGetstart="GET ";
  int iStart,iEndSpace,iEndQuest;
  iStart = sRequest.indexOf(sGetstart);
  if (iStart>=0)
  {
    iStart+=+sGetstart.length();
    iEndSpace = sRequest.indexOf(" ",iStart);
    iEndQuest = sRequest.indexOf("?",iStart);

    // are there parameters?
    if(iEndSpace>0)
    {
      if(iEndQuest>0)
      {
        // there are parameters
        sPath  = sRequest.substring(iStart,iEndQuest);
        sParam = sRequest.substring(iEndQuest,iEndSpace);
      }
      else
      {
        // NO parameters
        sPath  = sRequest.substring(iStart,iEndSpace);
      }
    }
  }

  ///////////////////////////////////////////////////////////////////////////////
  // output parameters to serial, you may connect e.g. an Arduino and react on it
  ///////////////////////////////////////////////////////////////////////////////
  if(sParam.length()>0)
  {
    int iEqu=sParam.indexOf("=");
    if(iEqu>=0)
    {
      sCmd = sParam.substring(iEqu+1,sParam.length());
      Serial.println(sCmd);
    }
  }


  ///////////////////////////
  // format the html response
  ///////////////////////////
  String sResponse,sHeader;

  ////////////////////////////
  // 404 for non-matching path
  ////////////////////////////
  if(sPath!="/")
  {
    sResponse="<html><head><title>404 Not Found</title></head><body><h1>Not Found</h1><p>The requested URL was not found on this server.</p></body></html>";

    sHeader  = "HTTP/1.1 404 Not found\r\n";
    sHeader += "Content-Length: ";
    sHeader += sResponse.length();
    sHeader += "\r\n";
    sHeader += "Content-Type: text/html\r\n";
    sHeader += "Connection: close\r\n";
    sHeader += "\r\n";
  }
  ///////////////////////
  // format the html page
  ///////////////////////
  else
  {
    ulReqcount++;
    sResponse  = "<html><head><title>Demo f&uumlr ESP8266 Steuerung</title></head><body>";
    sResponse += "<font color=\"#000000\"><body bgcolor=\"#d0d0f0\">";
    sResponse += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=yes\">";
    sResponse += "<h1>Demo f&uumlr ESP8266 Steuerung</h1>";
    sResponse += "Funktion 1 schaltet GPIO2 und erzeugt eine serielle Ausgabe.<BR>";
    sResponse += "Funktion 2 erzeugt nur eine serielle Ausgabe.<BR>";
    sResponse += "<FONT SIZE=+1>";
    sResponse += "<p>Funktion 1 <a href=\"?pin=255,0,0\"><button>einschalten</button></a>&nbsp;<a href=\"?pin=0,0,0\"><button>ausschalten</button></a></p>";
    // sResponse += "<p>Funktion 2 <a href=\"?pin=FUNCTION2ON\"><button>einschalten</button></a>&nbsp;<a href=\"?pin=FUNCTION2OFF\"><button>ausschalten</button></a></p>";

    //////////////////////
    // react on parameters
    //////////////////////
    if (sCmd.length()>0)
    {
      // write received command to html page
      sResponse += "Kommando:" + sCmd + "<BR>";

      config = sCmd;

    }

    sResponse += "<FONT SIZE=-2>";
    sResponse += "<BR>Aufrufz&auml;hler=";
    sResponse += ulReqcount;
    sResponse += " - Verbindungsz&auml;hler=";
    sResponse += ulReconncount;
    sResponse += "<BR>";
    sResponse += "Stefan Thesen 04/2015<BR>";
    sResponse += "</body></html>";

    sHeader  = "HTTP/1.1 200 OK\r\n";
    sHeader += "Content-Length: ";
    sHeader += sResponse.length();
    sHeader += "\r\n";
    sHeader += "Content-Type: text/html\r\n";
    sHeader += "Connection: close\r\n";
    sHeader += "\r\n";
  }

  // Send the response to the client
  client.print(sHeader);
  client.print(sResponse);

  // and stop the client
  client.stop();
  Serial.println("Client disonnected");
}
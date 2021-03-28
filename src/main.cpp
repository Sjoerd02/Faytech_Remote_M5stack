#include <M5Stack.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <Preferences.h>

/*
* Multi slider:
*/

const IPAddress apIP(192, 168, 4, 1);
const char* apSSID = "M5STACK_SETUP";
boolean settingMode;
String ssidList;
String wifi_ssid;
String wifi_password;

// DNSServer dnsServer;
WebServer webServer(80);

// wifi config store
Preferences preferences;

// revieve serial data : https://forum.arduino.cc/index.php?topic=396450.0
const byte numChars = 32;
char receivedChars[numChars];   // an array to store the received data
boolean newData = false;

// -------------------
// slider config
//--------------------
const int output = 2;
String sliderValue = "0";
String param="";
const char *PARAM_INPUT = "value";


boolean restoreConfig() {
  wifi_ssid = preferences.getString("WIFI_SSID");
  wifi_password = preferences.getString("WIFI_PASSWD");
  Serial.print("WIFI-SSID: ");
  M5.Lcd.print("WIFI-SSID: ");
  Serial.println(wifi_ssid);
  M5.Lcd.println(wifi_ssid);
  Serial.print("WIFI-PASSWD: ");
  M5.Lcd.print("WIFI-PASSWD: ");
  Serial.println(wifi_password);
  M5.Lcd.println(wifi_password);
  WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());

  if(wifi_ssid.length() > 0) {
    return true;
} else {
    return false;
  }
}

boolean checkConnection() {
  int count = 0;
  Serial.print("Waiting for Wi-Fi connection");
  M5.Lcd.print("Waiting for Wi-Fi connection");
  while ( count < 30 ) {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println();
      M5.Lcd.println();
      Serial.println("Connected!");
      M5.Lcd.println("Connected!");
      return (true);
    }
    delay(500);
    Serial.print(".");
    M5.Lcd.print(".");
    count++;
  }
  Serial.println("Timed out.");
  M5.Lcd.println("Timed out.");
  return false;
}

String makePage(String title, String contents) {
  String s = "<!DOCTYPE html><html><head>";
  s += "<meta name=\"viewport\" content=\"width=device-width,user-scalable=0\">";
  s += "<title>";
  s += title;
  s += "</title></head><body>";
  s += contents;
  s += "</body></html>";
  return s;
}

String index_html(){

  String s ="<!DOCTYPE HTML>";
  s+="<html>";
  s+="";
  s+="<head>";
  s+="    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  s+="    <title>ESP Web Server</title>";
  s+="    <style>";
  s+="        html {";
  s+="            font-family: Arial;";
  s+="            display: inline-block;";
  s+="            text-align: center;";
  s+="        }";
  s+="";
  s+="        h2 {";
  s+="            font-size: 2.3rem;";
  s+="        }";
  s+="";
  s+="        p {";
  s+="            font-size: 1.9rem;";
  s+="        }";
  s+="";
  s+="        body {";
  s+="            max-width: 400px;";
  s+="            margin: 0px auto;";
  s+="            padding-bottom: 25px;";
  s+="        }";
  s+="";
  s+="        .slider {";
  s+="            -webkit-appearance: none;";
  s+="            margin: 14px;";
  s+="            width: 360px;";
  s+="            height: 25px;";
  s+="            background: #FFD65C;";
  s+="            outline: none;";
  s+="            -webkit-transition: .2s;";
  s+="            transition: opacity .2s;";
  s+="        }";
  s+="";
  s+="        .slider::-webkit-slider-thumb {";
  s+="            -webkit-appearance: none;";
  s+="            appearance: none;";
  s+="            width: 35px;";
  s+="            height: 35px;";
  s+="            background: #003249;";
  s+="            cursor: pointer;";
  s+="        }";
  s+="";
  s+="        .slider::-moz-range-thumb {";
  s+="            width: 35px;";
  s+="            height: 35px;";
  s+="            background: #003249;";
  s+="            cursor: pointer;";
  s+="        }";
  s+="    </style>";
  s+="</head>";
  s+="";
  s+="<body>";
  s+="    <h2>FayTech Remote</h2>";
  s+="    <p><span id=\"textSliderValue\">%SLIDERVALUE%</span></p>";
  s+="    <p>Brightness<input type=\"range\" onchange=\"updateSliderBr(this)\" id=\"sliderBr\" min=\"0\" max=\"10\" value=\"%SLIDERVALUE%\"";
  s+="            step=\"1\" class=\"slider\"></p>";
  s+="    <p>Volume<input type=\"range\" onchange=\"updateSliderVol(this)\" id=\"sliderVol\" min=\"0\" max=\"10\" value=\"%SLIDERVALUE%\"";
  s+="            step=\"1\" class=\"slider\"></p>";s+="";
  s+="    <script>";
  s+="        function updateSliderBr(element) {";
  s+="            var sliderValueBr = document.getElementById(\"sliderBr\").value;";
  s+="            document.getElementById(\"textSliderValue\").innerHTML = \"Brightness = \" +sliderValueBr;";
  s+="            console.log(\"Brightness = \" + sliderValueBr);";
  s+="            var xhr = new XMLHttpRequest();";
  s+="            xhr.open(\"GET\", \"/slider?value=\" + sliderValueBr + \"&param=bright\", true);";
  s+="            xhr.send();";
  s+="        }";

  s+="        function updateSliderVol(element) {";
  s+="            var sliderValueVol = document.getElementById(\"sliderVol\").value;";
  s+="            document.getElementById(\"textSliderValue\").innerHTML = \"Volume = \" + sliderValueVol;";
  s+="            console.log(\"Volume = \" + sliderValueVol);";
  s+="            var xhr = new XMLHttpRequest();";
  s+="            xhr.open(\"GET\", \"/slider?value=\" + sliderValueVol + \"&param=volume\", true);";
  s+="            xhr.send();";
  s+="        }";
  s+="    </script>";
  s+="</body>";
  s+="";
  s+="</html>";
  return s;
}

String urlDecode(String input) {
  String s = input;
  s.replace("%20", " ");
  s.replace("+", " ");
  s.replace("%21", "!");
  s.replace("%22", "\"");
  s.replace("%23", "#");
  s.replace("%24", "$");
  s.replace("%25", "%");
  s.replace("%26", "&");
  s.replace("%27", "\'");
  s.replace("%28", "(");
  s.replace("%29", ")");
  s.replace("%30", "*");
  s.replace("%31", "+");
  s.replace("%2C", ",");
  s.replace("%2E", ".");
  s.replace("%2F", "/");
  s.replace("%2C", ",");
  s.replace("%3A", ":");
  s.replace("%3A", ";");
  s.replace("%3C", "<");
  s.replace("%3D", "=");
  s.replace("%3E", ">");
  s.replace("%3F", "?");
  s.replace("%40", "@");
  s.replace("%5B", "[");
  s.replace("%5C", "\\");
  s.replace("%5D", "]");
  s.replace("%5E", "^");
  s.replace("%5F", "-");
  s.replace("%60", "`");
  return s;
}

void startWebServer() {
  if (settingMode) {
    Serial.print("Starting Web Server at ");
    M5.Lcd.print("1.Starting Web Server at ");
    Serial.println(WiFi.softAPIP());
    M5.Lcd.println(WiFi.softAPIP());
    webServer.on("/settings", []() {
      String s = "<h1>Wi-Fi Settings</h1><p>Please enter your password by selecting the SSID.</p>";
      s += "<form method=\"get\" action=\"setap\"><label>SSID: </label><select name=\"ssid\">";
      s += ssidList;
      s += "</select><br>Password: <input name=\"pass\" length=64 type=\"password\"><input type=\"submit\"></form>";
      webServer.send(200, "text/html", makePage("Wi-Fi Settings", s));
    });
    webServer.on("/setap", []() {
      String ssid = urlDecode(webServer.arg("ssid"));
      Serial.print("SSID: ");
      M5.Lcd.print("SSID: ");
      Serial.println(ssid);
      M5.Lcd.println(ssid);
      String pass = urlDecode(webServer.arg("pass"));
      Serial.print("Password: ");
      M5.Lcd.print("Password: ");
      Serial.println(pass);
      M5.Lcd.println(pass);
      Serial.println("Writing SSID to EEPROM...");
      M5.Lcd.println("Writing SSID to EEPROM...");

      // Store wifi config
      Serial.println("Writing Password to nvr...");
      M5.Lcd.println("Writing Password to nvr...");
      preferences.putString("WIFI_SSID", ssid);
      preferences.putString("WIFI_PASSWD", pass);

      Serial.println("Write nvr done!");
      M5.Lcd.println("Write nvr done!");
      String s = "<h1>Setup complete.</h1><p>device will be connected to \"";
      s += ssid;
      s += "\" after the restart.";
      webServer.send(200, "text/html", makePage("Wi-Fi Settings", s));
      delay(3000);
      ESP.restart();
    });
    webServer.onNotFound([]() {
      String s = "<h1>AP mode</h1><p><a href=\"/settings\">Wi-Fi Settings</a></p>";
      webServer.send(200, "text/html", makePage("AP mode", s));
    });
  }
  else {
    Serial.print("Starting Web Server at ");
    M5.Lcd.print("Starting Web Server at ");
    Serial.println(WiFi.localIP());
    M5.Lcd.println(WiFi.localIP());
    
    webServer.on("/", []() {
      M5.Lcd.println("web:root");
      String s = "<h1>STA mode</h1><p><a href=\"/reset\">Reset Wi-Fi Settings</a></p>";
      //webServer.send(200, "text/html", makePage("STA mode", s));
      webServer.send(200, "text/html", index_html());     
    });

    webServer.on("/slider", []() {
      M5.Lcd.println("web:on slider " + webServer.arg("param") + " : " + urlDecode(webServer.arg("value")));

      String inputMessage;
      // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
      if  (webServer.arg("value") != NULL)
      {
        inputMessage = urlDecode(webServer.arg("value"));
        param = urlDecode(webServer.arg("param"));
        sliderValue = inputMessage;
        //analogWrite(output, sliderValue.toInt());
        if (webServer.arg("param")=="bright"){
          byte message[] = {0x69, 0xA3, sliderValue.toInt()};
          Serial.write(message, sizeof(message));
        }else if(webServer.arg("param")=="volume"){
          byte message[] = {0x69, 0xB2, sliderValue.toInt()};
          Serial.write(message, sizeof(message));
        }
      }
      else
      {
        inputMessage = "No message sent";
      }
      webServer.send(200, "text/plain", "OK");
    });

    webServer.on("/reset", []() {
      // reset the wifi config
      preferences.remove("WIFI_SSID");
      preferences.remove("WIFI_PASSWD");
      String s = "<h1>Wi-Fi settings was reset.</h1><p>Please reset device.</p>";
      webServer.send(200, "text/html", makePage("Reset Wi-Fi Settings", s));
      delay(3000);
      ESP.restart();
    });
  }
  webServer.begin();
}

void setupMode() {
  WiFi.mode(WIFI_MODE_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  delay(100);
  Serial.println("");
  M5.Lcd.println("");
  for (int i = 0; i < n; ++i) {
    ssidList += "<option value=\"";
    ssidList += WiFi.SSID(i);
    ssidList += "\">";
    ssidList += WiFi.SSID(i);
    ssidList += "</option>";
  }
  delay(100);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(apSSID);
  WiFi.mode(WIFI_MODE_AP);
  // WiFi.softAPConfig(IPAddress local_ip, IPAddress gateway, IPAddress subnet);
  // WiFi.softAP(const char* ssid, const char* passphrase = NULL, int channel = 1, int ssid_hidden = 0);
  // dnsServer.start(53, "*", apIP);
  startWebServer();
  Serial.print("Starting Access Point at \"");
  M5.Lcd.print("Starting Access Point at \"");
  Serial.print(apSSID);
  M5.Lcd.print(apSSID);
  Serial.println("\"");
  M5.Lcd.println("\"");
}

void recvWithEndMarker() {
    static byte ndx = 0;
    char endMarker = 13;
    char rc;
   
    while (Serial2.available() > 0 && newData == false) {
        rc = Serial2.read();

        if (rc != endMarker) {
            receivedChars[ndx] = rc;
            ndx++;
            if (ndx >= numChars) {
                ndx = numChars - 1;
            }
        }
        else {
            receivedChars[ndx] = '\0'; // terminate the string
            ndx = 0;
            newData = true;
        }
    }
}
void showNewData() {
    if (newData == true) {
        M5.Lcd.print("Recieved:");
        M5.Lcd.println(receivedChars);
        newData = false;
    }
}

// the setup routine runs once when M5Stack starts up
void setup(){

      // init lcd, serial, but don't init sd card
  M5.begin(true, false, true);

  /*
    Power chip connected to gpio21, gpio22, I2C device
    Set battery charging voltage and current
    If used battery, please call this function in your project
  */
  M5.Power.begin();
  preferences.begin("wifi-config");

  Serial2.begin(38400);
  Serial.begin(38400);

  // LCD display
  M5.Lcd.setTextSize(1);
  M5.Lcd.println("Hello M5");

  //Initialize SPIFFS
  if(!SPIFFS.begin(true)){
    M5.Lcd.println("An Error has occurred while mounting SPIFFS");
  }

  delay(10);
  if (restoreConfig()) {
    if (checkConnection()) {
      settingMode = false;
      startWebServer();
      return;
    }
  }
  settingMode = true;
  setupMode();
}

// the loop routine runs over and over again forever
void loop() {

    M5.update();

    if (settingMode) {
    }
    webServer.handleClient();
    
    //read serial
    recvWithEndMarker();
    showNewData();
    
    // Handle Button
    if (M5.BtnA.wasReleased()) {
    M5.Lcd.print('-');
    
    byte message[] = {0x69, 0x72, 0x00};
    Serial.write(message, sizeof(message));
    Serial2.write(message, sizeof(message));
    Serial2.write(message, sizeof(message));
  } else if (M5.BtnB.wasReleased()) {
    M5.Lcd.print('B');
    Serial.println("0-ButtonB");
    Serial2.println("2-ButtonB");
  } else if (M5.BtnC.wasReleased()) {
    M5.Lcd.print('+');
    byte message[] = {0x69, 0x71, 0x00};
    Serial.write(message, sizeof(message));
    Serial2.write(message, sizeof(message));
    Serial2.write(message, sizeof(message));
    } else if (M5.BtnB.wasReleasefor(700)) {
    M5.Lcd.clear(BLACK);
    M5.Lcd.setCursor(0, 0);
  }

}
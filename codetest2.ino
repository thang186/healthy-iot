#include <WiFi.h>
#include <SPI.h>
#include <ThingerESP32.h>
#include <WebServer.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include "DHT.h" 
#include <HTTPClient.h>
#define DHTTYPE DHT22 
#define DHTPIN 18
#define DS18B20 5
#define REPORTING_PERIOD_MS     1000
#define USERNAME "thang186"
#define DEVICE_ID "thang"
#define DEVICE_CREDENTIAL "thang186"

ThingerESP32 thing(USERNAME,DEVICE_ID,DEVICE_CREDENTIAL);             
             
int pushPin = 4; //chân của nút nhấn
int ledPin = 2;
int val = 0; //giá trị nút nhấn ban đầu
 
float temperature, humidity, BPM, SpO2, bodytemperature;
 
/*Put your SSID & Password*/
const char* ssid = "POCO X3 Pro";  
const char* password = "thang186";  
 
DHT dht(DHTPIN, DHTTYPE);
PulseOximeter pox;
uint32_t tsLastReport = 0;
OneWire oneWire(DS18B20);
DallasTemperature sensors(&oneWire);
 
 
WebServer server(80);             
 
void onBeatDetected()
{
  Serial.println("Beat!");
}
 
void setup() {
  Serial.begin(115200);
  pinMode(19, OUTPUT);
  delay(100);   
  Serial.println(F("DHTxx test!"));
  dht.begin();
  Serial.println("Connecting to ");
  Serial.println(ssid);
  pinMode(ledPin, OUTPUT);  // 
  pinMode(pushPin, INPUT_PULLUP);  //
  Serial.begin(115200);
  //thinger kết nối
  thing.add_wifi("POCO X3 Pro", "thang186");                                     
  Serial.println("ấn nút");
  delay(1000);
//  kết nối với địa chỉ wifi
  WiFi.begin(ssid, password);
 
  //kiểm tra đã kết nối thành công hay chưa
  while (WiFi.status() != WL_CONNECTED) {
  delay(2000);
  Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected..!");
  Serial.print("Got IP: ");  
  Serial.println(WiFi.localIP());
 
 
 server.on("/", handle_OnConnect);
 server.onNotFound(handle_NotFound);
 
  server.begin();
  Serial.println("HTTP server started");
 
  Serial.print("Khởi động cảm biến oxy...");
 
  if (!pox.begin()) {
    Serial.println("FAILED");
    for (;;);
  } else {
    Serial.println("SUCCESS");
    pox.setOnBeatDetectedCallback(onBeatDetected);
  } 
 
   pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
  // gửi data lên thinger
   thing["dht"] >> [](pson& out){
      out["temperature"] = temperature;
      out["humidity"] = humidity;
  } ;delay(1000);

   thing["oneWire"] >> [](pson& out){
      out["temperature"] = bodytemperature;
      
  };delay(1000);
   thing["pox"] >> [](pson& out){
      out["BPM"] = BPM;
      out["Sp02"] = SpO2;
      
  };
}
void loop() {

  thing.handle();
  server.handleClient();
  
  sensors.requestTemperatures();
  float t = dht.readTemperature();
  String Temperature_Value = String(t);
  float h = dht.readHumidity();
  String Humidity_Value = String(h);
  temperature = t;
  humidity = h;
  pox.update();
  BPM = pox.getHeartRate();
  SpO2 = pox.getSpO2();
  bodytemperature = sensors.getTempCByIndex(0);
 
  
  if (millis() - tsLastReport > REPORTING_PERIOD_MS) 
  {
    Serial.print("Nhiệt độ phòng: ");
    Serial.print(t);
    Serial.println("°C");

    Serial.print("Độ ẩm phòng: ");
    Serial.print(h);
    Serial.println("%");

    Serial.print("Nhịp tim: ");
    Serial.println(BPM);

    Serial.print("SpO2: ");
    Serial.print(SpO2);
    Serial.println("%");

    Serial.print("Nhiệt độ cơ thể: ");
    Serial.print(bodytemperature);
    Serial.println("°C");

    Serial.println("____HOÀN THÀNH___");
    Serial.println();
 
    tsLastReport = millis();
  }
   if(WiFi.status() == WL_CONNECTED){
    
   HTTPClient http;
   String url = "http://bilodi.net/weather/index.php?data=1";
   url = url + "&moise=" + h +"&temp=" + t + "&BPM=" +BPM +"&body=" + bodytemperature +"&sp=" +SpO2;
   http.begin(url);
     int httpcode = http.GET();
    Serial.println("gui data thành công");
    Serial.println(httpcode);
    http.end();
    }
   val = digitalRead(pushPin);// đọc giá trị nút nhấn
  Serial.println(val); 
  if (val == LOW) {  
    digitalWrite(ledPin, HIGH);
    delay(1000);
    digitalWrite(ledPin, LOW);

    thing.handle();
    thing.call_endpoint("EmailService");// gọi đến thinger.io
    delay(5000);

  } else {
    digitalWrite(ledPin, LOW); 
  }
}
 
void handle_OnConnect() {
  
  server.send(200, "text/html", SendHTML(temperature, humidity, BPM, SpO2, bodytemperature)); 
}
 
void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}
 
  String SendHTML(float temperature, float humidity, float BPM, float SpO2, float bodytemperature) {
  String html = "<!DOCTYPE html>";
html += "<html>";
html += "<head>";
html += "<title>HETHONGTHEODOISUCKHOE</title>";
html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
html += "<link rel='stylesheet' href='https://cdnjs.cloudflare.com/ajax/libs/font-awesome/5.7.2/css/all.min.css'>";
html += "<link rel='stylesheet' type='text/css' href='styles.css'>";
html += "<style>";
html += "body { background-color: #fff; font-family: sans-serif; color: #333333; font: 14px Helvetica, sans-serif box-sizing: border-box;}";
html += "#page { margin: 20px; background-color: #fff;}";
html += ".container { height: inherit; padding-bottom: 20px;}";
html += ".header { padding: 20px;}";
html += ".header h1 { padding-bottom: 0.3em; color: #008080; font-size: 45px; font-weight: bold; font-family: Garmond, 'sans-serif'; text-align: center;}";
html += "h2 { padding-bottom: 0.2em; border-bottom: 1px solid #eee; margin: 2px; text-align: left;}";
html += ".header h3 { font-weight: bold; font-family: Arial, 'sans-serif'; font-size: 17px; color: #b6b6b6; text-align: center;}";
html += ".box-full { padding: 20px; border 1px solid #ddd; border-radius: 1em 1em 1em 1em; box-shadow: 1px 7px 7px 1px rgba(0,0,0,0.4); background: #fff; margin: 20px; width: 300px;}";
html += "@media (max-width: 494px) { #page { width: inherit; margin: 5px auto; } #content { padding: 1px;} .box-full { margin: 8px 8px 12px 8px; padding: 10px; width: inherit;; float: none; } }";
html += "@media (min-width: 494px) and (max-width: 980px) { #page { width: 465px; margin 0 auto; } .box-full { width: 380px; } }";
html += "@media (min-width: 980px) { #page { width: 930px; margin: auto; } }";
html += ".sensor { margin: 12px 0px; font-size: 2.5rem;}";
html += ".sensor-labels { font-size: 1rem; vertical-align: middle; padding-bottom: 15px;}";
html += ".units { font-size: 1.2rem;}";
html += "hr { height: 1px; color: #eee; background-color: #eee; border: none;}";
html += "</style>";

//Ajax Code START
  html += "<script>\n";
  html += "setInterval(loadDoc,1000);\n";
  html += "function loadDoc() {\n";
  html += "var xhttp = new XMLHttpRequest();\n";
  html += "xhttp.onreadystatechange = function() {\n";
  html += "if (this.readyState == 4 && this.status == 200) {\n";
  html += "document.body.innerHTML =this.responseText}\n";
  html += "};\n";
  html += "xhttp.open(\"GET\", \"/\", true);\n";
  html += "xhttp.send();\n";
  html += "}\n";
  html += "</script>\n";
  
  //Ajax Code END
  
html += "</head>";
html += "<body>";
html += "<div id='page'>";
html += "<div class='header'>";
html += "<h1>HỆ THỐNG THEO DÕI SỨC KHỎE</h1>";
html += "<h3><a href='quocthang_1814100'>quocthang</a></h3>";
html += "</div>";
html += "<div id='content' align='center'>";
html += "<div class='box-full' align='left'>";
html += "<h2>Chúc bạn khỏe mạnh</h2>";
html += "<div class='sensors-container'>";

//cảm biến nhiệt DHT22
html += "<div class='sensors'>";
html += "<p class='sensor'>";
html += "<i class='fas fa-thermometer-half' style='color:#0275d8'></i>";
html += "<span class='sensor-labels'> Nhiệt Độ Phòng </span>";
html += (int)temperature;
html += "<sup class='units'>°C</sup>";
html += "</p>";
html += "<hr>";
html += "</div>";

//cảm biến ẩm DHT22
html += "<div class='sensors'>";
html += "<p class='sensor'>";
html += "<i class='fas fa-tint' style='color:#5bc0de'></i>";
html += "<span class='sensor-labels'> Độ Ẩm Phòng </span>";
html += (int)humidity;
html += "<sup class='units'>%</sup>";
html += "</p>";
html += "<hr>";

//MAX30100
html += "<p class='sensor'>";
html += "<i class='fas fa-heartbeat' style='color:#cc3300'></i>";
html += "<span class='sensor-labels'> Nhịp Tim </span>";
html += (int)BPM;
html += "<sup class='units'>BPM</sup>";
html += "</p>";
html += "<hr>";

//MAX30100
html += "<p class='sensor'>";
html += "<i class='fas fa-burn' style='color:#f7347a'></i>";
html += "<span class='sensor-labels'> Sp02 </span>";
html += (int)SpO2;
html += "<sup class='units'>%</sup>";
html += "</p>";
html += "<hr>";

//Nhiệt độ cơ thể
html += "<p class='sensor'>";
html += "<i class='fas fa-thermometer-full' style='color:#d9534f'></i>";
html += "<span class='sensor-labels'> Nhiệt Độ Cơ Thể </span>";
html += (float)bodytemperature;
html += "<sup class='units'>°C</sup>";
html += "</p>";
html += "</div>";
html += "</div>";
html += "</div>";
html += "</div>";
html += "</div>";
html += "</body>";
html += "</html>";
return html;
}

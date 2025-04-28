#include <Firebase.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "UbidotsESPMQTT.h"
#include <MFRC522.h>
#include <WiFi.h>
#include <Wire.h>
#include <SPI.h>
#define ssid "Abo Selim"
#define password "12341234"
//-----------------------
MFRC522 rfid(2,5);
//-----------------------
#define fburl "https://door-lock-31536-default-rtdb.firebaseio.com/"
#define fbtoken "ucxHsRoUNy7BAevDzLhFN2X07ArFJQZcW16QVroL"
Firebase fb(fburl, fbtoken);
//------------------------
#define ubitoken "BBUS-KQYMLwfTdUNxPE2leENxkzaSYM5eyn"
Ubidots ubidots(ubitoken);
//------------------------
Adafruit_SSD1306 display(128,64,&Wire,-1);
//--------------------------
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}
void showMessage(String msg){
  display.clearDisplay();
  display.display();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,20);
  display.println(msg);
  display.display();
  Serial.println(msg);
}
String Getuid(){
  String uid ="";
  for(byte i =0;i < rfid.uid.size;i++){
    uid += String(rfid.uid.uidByte[i]);
  }
  uid.toUpperCase();
  return uid;
}
void AccessGranted(String uid){
  showMessage("Access Granted");
  digitalWrite(12, HIGH);
  digitalWrite(27, HIGH);
  delay(1000);
  digitalWrite(27, LOW);
  String timestamp = String(millis() / 1000);//2
  fb.pushString("logs","Access Granted uid: "+uid + " at "+timestamp);//2
  ubidots.add("access",1);
  ubidots.add("uid",uid.toInt());
  ubidots.add("time",timestamp.toInt());//2
  ubidots.ubidotsPublish("Control");
  delay(1000);
  digitalWrite(12, LOW);
}
void AccessDenied(String uid){
  showMessage("Access Denied");
  digitalWrite(14, HIGH);
  digitalWrite(27, HIGH);
  delay(2000);
  digitalWrite(27, LOW);
   String timestamp = String(millis() / 1000);//2
    fb.pushString("logs","Access Denied uid: "+uid + " at "+timestamp);//2
  ubidots.add("access",0);
  ubidots.add("uid",uid.toInt());
    ubidots.add("time",timestamp.toInt());//2
  ubidots.ubidotsPublish("control");
  delay(1000);
  digitalWrite(14, LOW);
}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  SPI.begin();         // Init SPI bus
  rfid.PCD_Init();
  pinMode(12, OUTPUT);
  pinMode(14, OUTPUT);
  pinMode(27, OUTPUT);
  display.begin(SSD1306_SWITCHCAPVCC,0x3C);
  display.clearDisplay();
  display.display();
  WiFi.begin(ssid,password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(500);
  }
  Serial.println("Connected");
  ubidots.setDebug(true);  // Pass a true or false bool value to activate debug messages
  ubidots.wifiConnection(ssid, password);
  ubidots.begin(callback);
}

void loop() {
  // put your main code here, to run repeatedly:
if (!ubidots.connected()) {
    ubidots.reconnect();
  }
  ubidots.loop();  // Important to keep connection alive

  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    String uid = Getuid();
    Serial.println("Scanned UID: " + uid);
    showMessage("Checking UID...");

    String path = "auth/" + uid;
    int result = fb.getInt(path);
    String name = fb.getString(path); 
    Serial.println("result: " + String(result));
//2
    if (name == "null") {
      AccessDenied(uid);
    } else {
      AccessGranted(uid);
      showMessage(name);
    }
    //2

    //1
  /*
  if(result==1){
    AccessGranted(uid);
  }
  else{
     AccessDenied(uid);
  }
  */
  //1

    delay(2000);
    rfid.PICC_HaltA();
  }
}

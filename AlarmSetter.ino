#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <ESP8266WebServer.h>

#define PORT 4824
#define debug Serial
WiFiClient client;
WiFiClient timeClient;
// 클라이언트 대리자를 선언합니다
WiFiServer server(PORT);
ESP8266WiFiClass Wifi8266;
ESP8266WebServer Web(80);
String dataClient;
String tempDataString;
String dataString[50];
String ssid = "107-408";
String pass = "72077207";
const int SET_TIME = 10;
const int SSID_ADDR = 1;
const int PASS_ADDR = 30;
const int SSID_SIZE_ADDR = 29;
const int PASS_SIZE_ADDR = 50;
const int STATE_ADDR = 0;
const int TIME_DATA_COUNT_ADDR = 51; //flag사용해서 저장 예정
const int FIRST_TIME_DATA_ADDR = 52; //12칸씩 사용예정
const int relayPin = 16;

int deviceActive;
int preDeviceActive = 0;
int i;
   
int bufferIndex = 0;
int deviceState = 0;
int dataStringIndex = 0;
int data;

char temp;
String buffer = "";
int tempIndex = 0;

void writeStringAtROM(char a, String s) {
  int i = 0;
  switch(a) {
    case 'w':
          i = SSID_ADDR;
    break;
    case 'p':
          i = PASS_ADDR;
    break;
  }
  // Serial.println("abc");
  while(1) {
    EEPROM.write(i,s.charAt(i++));
    Serial.println(s.charAt(i-1));
    if(s.charAt(i) == '\0') break;
  }
  EEPROM.write(++i,'\0');
  for (int j = 0; j < 5; j++) {
    Serial.println((char)EEPROM.read(i--));
  }
  /*
  if(a == 'w') i = WIFIADDR;
  if(a == 'p') i = PASSADDR;
  */
}
String readROM(char a) {
  String buffered = "";
  int readADDR = 0;
  int i = 0;
  int strLength;
  switch(a) {
    case 'w':
          strLength = EEPROM.read(SSID_SIZE_ADDR);
    readADDR = SSID_ADDR;
    break;
    case 'p':
          strLength = EEPROM.read(PASS_SIZE_ADDR);
    readADDR = PASS_ADDR;
    break;
  }
  for (i = 0; i < strLength; i++) {
    buffered += (char)EEPROM.read(i+readADDR);
    delay(100);
  }
  return buffered;
}
void indexpage() {
  Web.send(200, "text/html", "<p>Heater Setting<p></br><form method='post' action='save'>  SSID<input type='text' name='ssid' ></br>  PASSWORD<input type='password' name='pw' ></br>     <input type='submit' value='Save'></form>");
}
void savepage() {
  int count = 0;
   char temp[200];
  String rcvSSID=Web.arg("ssid");
  String rcvPW=Web.arg("pw");
  int lengthSSID=rcvSSID.length();
  int lengthPW=rcvPW.length();
  debug.print("ssid=");
  debug.println(rcvSSID);
  debug.print("password=");
  debug.println(rcvPW);
  const char* sid=rcvSSID.c_str();
  const char* pid=rcvPW.c_str();
  
  for (int i=0;i<lengthSSID;i++) {
    EEPROM.write(SSID_ADDR+i,sid[i]);
    debug.print((char)EEPROM.read(SSID_ADDR+i));
  }
  debug.println();
  for (int i=0;i<lengthPW;i++) {
    EEPROM.write(PASS_ADDR+i,pid[i]);
    debug.print(pid[i]);
  }
  debug.println();
  EEPROM.write(SSID_SIZE_ADDR,lengthSSID);
  EEPROM.write(PASS_SIZE_ADDR,lengthPW);
  EEPROM.commit();
  String temp1 = readROM('w');
  String temp2 = readROM('p');
  debug.print("readSSID = ");
  debug.println(temp1);
  debug.print("readPASS = ");
  debug.println(temp2);
  WiFi.begin(temp1.c_str(), temp2.c_str());
     while (WiFi.status() != WL_CONNECTED) {
      // 연결시까지 대기합니다
      Serial.print(".");
      delay(200);
      count++;
      if(count > 50){
        snprintf(temp, 200, "<p>Your AP information is not correct. pleas Again setting<p></br>");
       
         Web.send(200,"text/html",temp);
         delay(5000);
         EspClass esp;
         esp.restart(); 
      }
      // 20밀리초 지연을 진행합니다
    }
    // 무선네트워크 ID
    Serial.println("Alarm IP : ");
    Serial.print(WiFi.localIP());
    snprintf(temp, 200, "<p>Your AP information is saved successfully Your IP is %d.%d.%d.%d<p></br>", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
    EEPROM.write(STATE_ADDR,40);
    EEPROM.commit();
    delay(100);
    Web.send(200,"text/html",temp);
    delay(10000);
  EspClass esp;
  esp.restart(); 
  
  //debug.println(readROM()
}
void initialSetting() {
  Wifi8266.mode(WIFI_AP_STA);
  debug.println("ap");
  String AP_NameString = "HeaterSetter";
  char AP_NameChar[AP_NameString.length() + 1];
  memset(AP_NameChar, 0, AP_NameString.length() + 1);
  for (int i = 0; i < AP_NameString.length(); i++) {
    AP_NameChar[i] = AP_NameString.charAt(i);
  }
  Wifi8266.softAP(AP_NameChar);
  debug.println(Wifi8266.softAPIP());
  //Wifiserver.begin();
  Web.on("/", indexpage);
  Web.on("/save", savepage);
  Web.begin();
}

  String readData() {
    buffer = "";
    
    Serial.println("**************");
    if(client.available()){
      temp = client.read();
    }
    while(1){ // TCPIP 가 연결된 상태면?
      if(client.available()) {
            buffer += (char)client.read();
            
            if(buffer.charAt(buffer.length()-1) == 'a' || buffer.charAt(buffer.length()-1) == 'r'){
              Serial.println("read'a' or 'r'");
              Serial.println(buffer);
              //delay(200);
              //return buffer;
              break;// 수신값을 추가합니다
            }
            else if(buffer.equals("setting")){
              Serial.println("read setting");
              break;
            }
            
         }
      }
      return buffer;
      
   }   
/*
  tempDataString = "";
  for(int i = 0; i < buffer.length()/2;i++){
    tempDataString += (char)buffer.substring(i,i+1).toInt();
  }
  return tempDataString;
  
*/
   
    //Serial.print(tempDataString);
 String getTime() {
  String timeData;
  while (!timeClient.connect("google.com", 80)) {
  }
  timeClient.print("HEAD / HTTP/1.1\r\n\r\n");
  while(!timeClient.available()) {
  }
  while(timeClient.available()) {
    if (timeClient.read() == '\n') {
      if (timeClient.read() == 'D') {
        if (timeClient.read() == 'a') {
          if (timeClient.read() == 't') {
            if (timeClient.read() == 'e') {
              if (timeClient.read() == ':') {
                timeClient.read();
                timeData = timeClient.readStringUntil('\r');
                timeClient.stop();
                return timeData;
              }
            }
          }
        }
      }
    }
  }
  
}

  String getKoreaTime(){
    String timeData = getTime();
    String returnString = "";
    String day;
    int hour =  (timeData.substring(17,19).toInt()+9)%24;
    int minute = timeData.substring(20,22).toInt();
    hour <10 ? returnString +="0"+String(hour) : returnString += String(hour);
    minute < 10 ? returnString+="0"+String(minute):returnString += String(minute);
    if(timeData.equals(0,3) == "Mon"){
      day = "0";
    }
    else if(timeData.equals(0,3) == "Tue"){
      day = "1";
    }
    else if(timeData.equals(0,3) == "Wed"){
      day = "2";
    }
    else if(timeData.equals(0,3) == "Thu"){
      day = "3";
    }
    else if(timeData.equals(0,3) == "Fri"){
      day = "4";
    }
    else if(timeData.equals(0,3) == "Sat"){
      day = "5";
    }
    else if(timeData.equals(0,3) == "Sun"){
      day = "6";
    }
   
    return returnString+day;
  }

  int searchDataAndDelete(String s){
    int i;
    for(i = 0; i < dataStringIndex; i++){
      if(s.equals(dataString[i])){
        dataString[i] = dataString[dataStringIndex-1];
        dataStringIndex--;
        return 1;
      }
    }
    return 0;
  }

   int isAlarmRing(String nowTime, String targetTime){
    int time1 = nowTime.substring(0,4).toInt();
    time1 = time1/100*60 + time1%100;
    int time2 = targetTime.substring(0,4).toInt();
    time2 = time2/100*60 + time2%100;
    String targetDay = targetTime.substring(4);
    String nowDay = nowTime.substring(4);
    for(int i = 0; i <= targetDay.length();i++){
      if(0<(time2 - time1) && (time2 - time1) <= SET_TIME && nowDay.equals(targetDay.substring(i,i+1))){
         return 1;
       }
       else if(0<(time2 - time1) && (time2 - time1) <= SET_TIME && nowDay.substring(0).equals("")){
         return 1;
       }
    }
    return 0;
  }


  void deviceOn(){
    digitalWrite(relayPin,HIGH);
  }
  void deviceOff(){
    digitalWrite(relayPin,LOW);
  }
  void alarmThread(){
    deviceActive= 0;
    for(i = 0; i < dataStringIndex; i++){
      if(isAlarmRing(getKoreaTime(),dataString[i])){
        if(preDeviceActive == 0){
           deviceActive = 1;
           preDeviceActive = 1;
           deviceOn();
           Serial.println("deviceOn");
        }
        return;
       }
    }
    
    
    if (i == dataStringIndex && deviceActive == 0){
      if(preDeviceActive == 1){
        preDeviceActive = 0;
        deviceActive = 0;
        deviceOff();
        Serial.println("deviceOff");
      }
      
      
    }
    
    
  
  }

  
void setup() {
  // 처음, 한번만 실행되는 함수입니다
  Serial.begin(9600);
  // 통신속도를 설정합니다
  EEPROM.begin(512);
  if(EEPROM.read(0) != 40) {
    deviceState = 1;
    initialSetting();
  } else {
    pinMode(relayPin,OUTPUT);
    Wifi8266.mode(WIFI_STA);
    ssid = readROM('w');
    pass = readROM('p');
    Serial.println("test TCPIP ");
    // 제목을 송신합니다
    Serial.print("Port : ");
    Serial.println(PORT);
    Serial.println(ssid);
    Serial.println(pass);
    WiFi.begin(ssid.c_str(), pass.c_str());
    // TCPIP 를 시작합니다
    while (WiFi.status() != WL_CONNECTED) {
      // 연결시까지 대기합니다
      Serial.print(".");
      delay(200);
      // 20밀리초 지연을 진행합니다
    }
    Serial.print("Point : ");
    Serial.println(ssid);
    // 무선네트워크 ID
    Serial.println("Alarm IP : ");
    Serial.print(WiFi.localIP());
    // ex) 192.168.0.28
    delay(200);
    dataStringIndex = 0;
    server.begin();
  }
}

  void loop() {
    // 메인루프입니다
    if(EEPROM.read(STATE_ADDR) != 40) {
      Web.handleClient();
    } else {
      while(!client.connected()) {
        client = server.available();
        alarmThread();
      }
      Serial.println("client connected");
      while (client.connected()) {
        alarmThread();
        // TCPIP 가 연결된 상태면?
        if (client.available()) {
          data = client.read();
          // 수신버퍼에서 데이타를 읽어서 저장합니다
          if(data == 'w') {
            tempDataString = readData();
            Serial.println("DATA:"  +tempDataString);
          }
          
          if(tempDataString.equals("setting")) {
            EEPROM.write(0,STATE_ADDR);
             EEPROM.commit();
            Serial.println("reset");
             EspClass esp;
             esp.restart();
           }
          
          else if(tempDataString.charAt(tempDataString.length()-1) == 'r'){
            if(searchDataAndDelete(tempDataString.substring(0,2) + tempDataString.substring(3,5) + tempDataString.substring(6,tempDataString.indexOf("a")) == 1){
              Serial.println("sucess!! Delete:" + tempDataString);
              tempDataString = "";
            }
          }
         
         else if(tempDataString.charAt(tempDataString.length()-1) == 'a'){
           dataString[dataStringIndex++] = tempDataString.substring(0,2) + tempDataString.substring(3,5) + tempDataString.substring(6,tempDataString.indexOf("a"));
           tempDataString = "";
           Serial.println("add data:" + dataString[dataStringIndex-1]);
           Serial.println(getKoreaTime());
          
         }
          
        }
        
      }
      
      Serial.println("client disconnected");
    }
  
  }

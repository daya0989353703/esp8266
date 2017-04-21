/*專案名稱:ESP8266 + DS18B20 溫度上傳至GOOGLE EXECL（谷歌文檔）並產生圖表
 * 
 *教學網址: www.pdacontrolenglish.blogspot.com.co/2016/06/esp8266-direct-connection-to-google.html
 *          http://pdacontrol.blogspot.com.co/2016/06/conexion-esp8266-directa-google.html
 *          https://youtu.be/5f7wCeD4gB4  -- TUTORIAL
 *          https://youtu.be/YgMl30IDrxw
 *我的作品展示區https://docs.google.com/spreadsheets/d/14z5N5lohz9dkyl4n8AXi3m_SAMqCfWkI8k3IxbKhyS0/pubhtml
 */
 

#include <ESP8266WiFi.h>
#include "HTTPSRedirect.h"
#include <OneWire.h>

OneWire  ds(2);  // 在引腳2（需要4.7K電阻）


//const char* ssid = "Buffalo";//公司
//const char* password = "1a2b3c4d5e";//公司

const char* ssid = "45Y102686";//home
const char* password = "25693927";//home

const char* host = "script.google.com";
const char* googleRedirHost = "script.googleusercontent.com";
const char *GScriptId = "AKfycbzKFoaEjruE6TB9M-mHuNTwi_pxGDTx-yTzRw8pZOfKvpVRQ7Y";//GOOGLE EXECL的API KEY

const int httpsPort = 443;
//**************這是使用SSH加密的部分可以省略************************************************************
// http://askubuntu.com/questions/156620/how-to-verify-the-ssl-fingerprint-by-command-line-wget-curl/
// echo | openssl s_client -connect script.google.com:443 |& openssl x509 -fingerprint -noout
// www.grc.com doesn't seem to get the right fingerprint
// SHA1 fingerprint of the certificate
const char* fingerprint ="CA 13 6A C7 78 96 37 79 66 1A AE 43 A5 EE F0 67 59 6A E7 64";

// 準備寫入Google電子表格的字串格式
String url = String("/macros/s/") + GScriptId + "/exec?value=Conected_ESP8266";
// Fetch Google Calendar events for 1 week ahead
//String url2 = String("/macros/s/") + GScriptId + "/exec?cal";
// Read from Google Spreadsheet
String url3 = String("/macros/s/") + GScriptId + "/exec?read";

int   estado;  
int   porcentaje;

 float celsius, fahrenheit;  
 
///Connection One Wire
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];




void setup() {
  pinMode(2, OUTPUT);
  pinMode(0, OUTPUT);
  
  
  Serial.begin(115200);
  Serial.println();
  Serial.print("Connecting to wifi: ");
  Serial.println(ssid);
  // flush() is needed to print the above (connecting...) message reliably, 
  // in case the wireless connection doesn't go through
  Serial.flush();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Use HTTPSRedirect class to create TLS connection
  HTTPSRedirect client(httpsPort);
  Serial.print("Connecting to ");
  Serial.println(host);

  bool flag = false;
  for (int i=0; i<5; i++){
    int retval = client.connect(host, httpsPort);
    if (retval == 1) {
       flag = true;
       break;
    }
    else
      Serial.println("Connection failed. Retrying...");
  }
  
  Serial.flush();
  if (!flag){
    Serial.print("Could not connect to server: ");
    Serial.println(host);
    Serial.println("Exiting...");
    return;
  }
  
  Serial.flush();
  if (client.verify(fingerprint, host)) {
    Serial.println("Certificate match.");
  } else {
    Serial.println("Certificate mis-match");
  }

  // Note: setup() must finish within approx. 1s, or the the watchdog timer
  // will reset the chip. Hence don't put too many requests in setup()
  // ref: https://github.com/esp8266/Arduino/issues/34
  
 
  Serial.println("==============================================================================");
    url = String("/macros/s/") + GScriptId + "/exec?now=Connected ESP8266";
   client.printRedir(url, host, googleRedirHost);
//  client.printRedir(url2, host, googleRedirHost);
  Serial.println("==============================================================================");
  

}

void loop() { 
  
    
  if ( !ds.search(addr)) {
  //  Serial.println("No more addresses.");
   /// Serial.println();
    ds.reset_search();
    delay(250);
    return;
  }
  
  for( i = 0; i < 8; i++) {  
    addr[i];

  }

  if (OneWire::crc8(addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return;
  }
//  Serial.println();
 
  // the first ROM byte indicates which chip
  switch (addr[0]) {
    case 0x10:

      type_s = 1;
      break;
    case 0x28:

      type_s = 0;
      break;
    case 0x22:
    //  Serial.println("  Chip = DS1822");
      type_s = 0;
      break;
    default:
    //  Serial.println("Device is not a DS18x20 family device.");
      return;
  } 

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);      
  
  delay(1000);    

//  delay(1000);    
  
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         

  for ( i = 0; i < 9; i++) {       
    data[i] = ds.read();
  }
  OneWire::crc8(data, 8); 
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; 
    if (data[7] == 0x10) {      
      raw = (raw & 0xFFF0) + 12 - data[6];    }
  } else {
    byte cfg = (data[4] & 0x60);

    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    
  }

    celsius = (float)raw / 16.0;
 
   Serial.print("temperature float:");   
   Serial.println(celsius);       
    char outstr[15];
    dtostrf(celsius,4, 2, outstr);   //float to char  4 numero de caracteres  3 cifras sin espacio
    String valor= outstr;            // char to string

   Serial.print("temperature String 1=  ");
   Serial.println(valor);
   String  celsius_2 =  valor; 
   Serial.print("temperature String 2=  ");   
   celsius_2.replace(".", "."); //注意雙引號""內為小數點.非逗點,弄錯google圖表無法顯示,其原因是逗號為字串不能為圖表所用 
   Serial.println(celsius_2);
            
  HTTPSRedirect client(httpsPort);

  if (!client.connected())
   client.connect(host, httpsPort);
    url = String("/macros/s/") + GScriptId + "/exec?value="+celsius_2 ;
   client.printRedir(url, host, googleRedirHost); 
   
   Serial.println("================================================================");


  delay(300000);//5分鐘
  
   
    


  
}

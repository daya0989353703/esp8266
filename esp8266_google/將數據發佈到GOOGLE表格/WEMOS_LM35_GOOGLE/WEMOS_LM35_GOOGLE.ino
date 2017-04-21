// 專案:LM35+WEMOS上傳至GOOGLE EXCEL
// ON: 2017.04.06
// AT: http://embedded-lab.com/blog/post-data-google-sheets-using-esp8266/
//作者部落隔http://pdacontroles.com/page/3/

//google腳本在最後面
//圖表https://docs.google.com/spreadsheets/d/1IKJeIyeGbheMw63p_hT0rHzjymik9MqS-8P5_UlH63U/pubchart?oid=1392398359&format=interactive
#include <ESP8266WiFi.h>
#include "HTTPSRedirect.h"

//const char* ssid = "Buffalo";//公司
//const char* password = "1a2b3c4d5e";//公司

const char* ssid = "45Y102686";//home
const char* password = "25693927";//home

// 下面的ID來自Google表格。
const char *GScriptId = "AKfycbwRr7mv6-lJlQfL6nWj4UQEVvv53zcKf9uuRPn8wivyHKiWQaal";

//在此設定時間間隔推送數據
const int dataPostDelay = 15000;  // 15分鐘 = 15min * 60sec * 1000msec=900000msec微秒

const char* host = "script.google.com";
const char* googleRedirHost = "script.googleusercontent.com";

const int httpsPort =  443;
HTTPSRedirect client(httpsPort);

// 準備url網址（沒有不同的數據）
String url = String("/macros/s/") + GScriptId + "/exec?";
//ssl加密認證不用管它
const char* fingerprint = "F0 5C 74 77 3F 6B 25 D7 3B 66 4D 43 2F 7E BC 5B E9 28 86 AD";

// 我們將從A0引腳進行模擬輸入 
int outputpin = A0;
float millivolts = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("Connecting to wifi: ");
  Serial.println(ssid);
  Serial.flush();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" IP address: ");
  Serial.println(WiFi.localIP());

  
  Serial.print(String("Connecting to "));
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

  // 連接狀態，1 =已連接，0不連接。
  Serial.println("Connection Status: " + String(client.connected()));
  Serial.flush();
  
  if (!flag){
    Serial.print("Could not connect to server: ");
    Serial.println(host);
    Serial.println("Exiting...");
    Serial.flush();
    return;
  }

  //即使認證不符合，數據仍將被推送。
  if (client.verify(fingerprint, host)) {
    Serial.println("Certificate match.");
  } else {
    Serial.println("Certificate mis-match");
  }
}

// 這是將數據推送到Google工作表的主要方法
void postData(String tag, float value){
  if (!client.connected()){
    Serial.println("Connecting to client again..."); 
    client.connect(host, httpsPort);
  }
  String urlFinal = url+"tag="+tag+"&value="+String(value);
  client.printRedir(urlFinal, host, googleRedirHost);
}

// 繼續按給定間隔推送數據
void loop() {

int rawvoltage= analogRead(outputpin);
float millivolts = ((rawvoltage * 33.0f/1023.0f)*9);//LM35計算



  // 發布這些信息
  postData("SoilMoisture",millivolts);
  
  delay (dataPostDelay);
}



/*google腳本
 // BY: Akshaya Niraula
// ON: 2016 November 12th.
// AT: http://www.embedded-lab.com/.....
 
 
// Steps are valid as of 2016 November 12th.
// 0) From Google spreadsheet, Tools &gt; Scriipt Editor...
// 1) Write your code
// 2) Save and give a meaningful name
// 3) Run and make sure "doGet" is selected
//    You can set a method from Run menu
// 4) When you run for the first time, it will ask 
//    for the permission. You must allow it.
//    Make sure everything is working as it should.
// 5) From Publish menu &gt; Deploy as Web App...
//    Select a new version everytime it's published
//    Type comments next to the version
//    Execute as: "Me (your email address)"
//    MUST: Select "Anyone, even anonymous" on "Who has access to this script"
//    For the first time it will give you some prompt(s), accept it.
//    You will need the given information (url) later. This doesn't change, ever!
 
// Saving the published URL helps for later.
// https://script.google.com/macros/s/---Your-Script-ID--Goes-Here---/exec
// https://script.google.com/macros/s/---Your-Script-ID--Goes-Here---/exec?tag=test&amp;value=-1
 
// This method will be called first or hits first  
function doGet(e){
  Logger.log("--- doGet ---");
 
 var tag = "",
     value = "";
 
  try {
 
    // this helps during debuggin
    if (e == null){e={}; e.parameters = {tag:"test",value:"-1"};}
 
    tag = e.parameters.tag;
    value = e.parameters.value;
 
    // save the data to spreadsheet
    save_data(tag, value);
 
 
    return ContentService.createTextOutput("Wrote:\ntag:"+tag+"\nvalue:"+value);//不要有間隔
 
  } catch(error) { 
    Logger.log(error);    
    return ContentService.createTextOutput("oops...." + error.message 
                                            + "\n" + new Date() 
                                            + "\ntag: " + tag +
                                            + "\nvalue: " + value);
  }  
}
 
// Method to save given data to a sheet
function save_data(tag, value){
  Logger.log("--- save_data ---"); 
 
 
  try {
    var dateTime = new Date();
 
    // Paste the URL of the Google Sheets starting from https thru /edit
    // For e.g.: https://docs.google.com/..../edit 
    var ss = SpreadsheetApp.openByUrl("https://docs.google.com/spreadsheets/d/1IKJeIyeGbheMw63p_hT0rHzjymik9MqS-8P5_UlH63U/edit");
    var summarySheet = ss.getSheetByName("Summary");
    var dataLoggerSheet = ss.getSheetByName("DataLogger");
 
 
    // Get last edited row from DataLogger sheet
    var row = dataLoggerSheet.getLastRow() + 1;
 
 
    // Start Populating the data
    dataLoggerSheet.getRange("A" + row).setValue(row -1); // ID
    dataLoggerSheet.getRange("B" + row).setValue(dateTime); // dateTime
    dataLoggerSheet.getRange("C" + row).setValue(tag); // tag
    dataLoggerSheet.getRange("D" + row).setValue(value); // value
 
 
    // Update summary sheet
    summarySheet.getRange("B1").setValue(dateTime); // Last modified date
    // summarySheet.getRange("B2").setValue(row - 1); // Count 
  }
 
  catch(error) {
    Logger.log(JSON.stringify(error));
  }
 
  Logger.log("--- save_data end---"); 
}
 */
 

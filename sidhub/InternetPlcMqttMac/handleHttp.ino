/** Handle root or redirect to captive portal */

void handleRoot() {
  //Serial.println("Server Html");
  //Serial.println ( WiFi.localIP() );
  String s=""; 
  String s1= String(ssid);
  s="<meta name='viewport' content='width=device-width, initial-scale=1.0'/>";
  if(autoRead==1)
    s=s+"<meta http-equiv='refresh' content='5'/>";
  s=s+"<meta http-equiv='Content-Type' content='text/html;charset=utf-8' />";
  s=s+"<meta http-equiv='Content-Type' content='text/html;charset=utf-8' />";
  s=s+"<h1>에스아이디허브 웹서버</h1>";
  s=s+"<br>chip ID : "+sChipID+"<br>";
  s=s+"<br>출력버튼<br>";
  if(drive==0)
    s=s+"<a href='drive'><button style='background-color:DarkGreen; color:white;'>정  지</button></a>";
  else if(drive==1)
    s=s+"<a href='drive'><button style='background-color:Lime; color:blue;'>수동약</button></a>";
  else if(drive==2)
    s=s+"<a href='drive'><button style='background-color:Lime; color:blue;'>수동중</button></a>";
  else if(drive==3)
    s=s+"<a href='drive'><button style='background-color:Lime; color:blue;'>수동강</button></a>";
  else if(drive==4)
    s=s+"<a href='drive'><button style='background-color:Lime; color:blue;'>자  동</button></a>";
  s=s+"&emsp;";

  if(power==0)
    s=s+"<a href='power'><button style='background-color:DarkGreen; color:white;'>정상모드</button></a>";
  else if(power==1)
    s=s+"<a href='power'><button style='background-color:Lime; color:blue;'>파워모드</button></a>";
  s=s+"&emsp;";

  if(wind==0)
    s=s+"<a href='wind'><button style='background-color:DarkGreen; color:white;'>맞바람정지</button></a>";
  else if(wind==1)
    s=s+"<a href='wind'><button style='background-color:Lime; color:blue;'>맞바람보통</button></a>";
  else if(wind==2)
    s=s+"<a href='wind'><button style='background-color:Lime; color:blue;'>맞바람파워</button></a>";
  s=s+"&emsp;";

  s=s+"<br>아래 공유기 이름과 주소가 연결되었으면 주소를 선택한 후에 설정에서 사용 하시는 인터넷 공유기를 선택하세요. <br>";
  //s=s+"연결된 AP 이름과 IP : "+sAP_ssid+"  "+String(ssid)+"  "+"<p><a href='http://"+WiFi.localIP().toString()+"'/>"+WiFi.localIP().toString()+"</a></p>";
  s=s+"연결된 AP 이름과 IP : "+"  "+String(ssid)+"  "+"<p><a href='http://"+WiFi.localIP().toString()+"'/>"+WiFi.localIP().toString()+"</a></p>";
  s=s+"<p><a href='/wifi'>공유기를 바꾸려면 누르세요.</a></p>";

  server.send(200, "text/html", s);
}

void GoHome() {
  IPAddress ip = WiFi.localIP();
  String ipS;
  String s="";
  ipS=String(ip[0])+"."+String(ip[1])+"."+String(ip[2])+"."+String(ip[3]);
  IPAddress ipClient = server.client().remoteIP();
  if(ipClient[2]==4) 
    s="<meta http-equiv='refresh' content=\"0;url='http://192.168.4.1/'\">";
  else
    s="<meta http-equiv='refresh' content=\"0;url='http://"+ipS+"/'\">";
  server.send(200, "text/html", s);
}

void handleDrive() {
  drive += 1;
  if(drive >= 5)
    drive=0;
  plcOut();
  GoHome();
  delay(100);
}
void handlePower() {
 power += 1;
  if(power >= 2)
    power=0;
  plcOut();
  GoHome();
  delay(100);
}
void handleWind() {
 wind += 1;
  if(wind >= 3)
    wind=0;
  plcOut();
  GoHome();
  delay(100);
}

void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

void handleWifi() {
  String s; 
  String s1= String(ssid);
  //s="<meta name=\"viewport\" content=\"width=device-width, user-scalable=no\", meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\", meta http-equiv='refresh' content='5'/>";
  s="<meta name=\"viewport\" content=\"width=device-width, user-scalable=no\", meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\" />";
  //s=s+"<meta http-equiv='refresh' content='5'/>";
  s=s+"<h1>Wifi 사양</h1>";
  if (server.client().localIP() == apIP) {
    Serial.println(String(softAP_ssid));
    //s=s+String("<p>연결된 AP: ") + sAP_ssid + "</p>";
    s=s+String("<p>연결된 AP: 192.168.4.1") + "</p>";
  } else {
    s=s+"<p>연결된 와이파이 " + String(ssid) + "</p>";
  }
  s=s+"<table><tr><th align='left'>SoftAP 사양</th></tr>";
  s=s+"<tr><td>SSID " + String(softAP_ssid) + "</td></tr>";
  s=s+"<tr><td>IP   " + toStringIp(WiFi.softAPIP()) + "</td></tr>"+"</table>";
  s=s+"<br /><table><tr><th align='left'>WLAN 사양</th></tr>";
  s=s+"<tr><td>SSID " + String(ssid) + "</td></tr>";
  s=s+"<tr><td>IP   " + toStringIp(WiFi.localIP()) + "</td></tr>"+"</table>";
  
  s=s+"<br /><table><tr><th align='left'>검색된 와이파이 </th></tr>";
  Serial.println("scan start");
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n > 0) {
    for (int i = 0; i < n; i++) {
      s=s+"\r\n<tr><td>SSID " + WiFi.SSID(i) + String((WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":" *") + " (" + WiFi.RSSI(i) + ")</td></tr>";
    }
  } else {
    s=s+"<tr><td>No WLAN found</td></tr>";
  }
  s=s+"</table>";
  s=s+"<p><a href='/wifi'>와이파이가 없으면 다시 검색하세요.</a></p>";
  
  s=s+"<form method='POST' action='wifisave'><h4>연결하려는 와이파이 입력</h4>"
    +"와이파이 이름 <input type='text' value='"+ssid+"' name='n'/>"
    +"<br />비밀번호     <input type='password' value='"+password+"' name='p'/>"
    +"<br /><input type='submit' value='      저    장      '/></form>"
    +"<p><a href='/'>메인 홈페이지로 가기</a>.</p>";
  server.send(200, "text/html", s);
}

/** Handle the WLAN save form and redirect to WLAN config page again */
void handleWifiSave() {
  Serial.println("wifi save");
  server.arg("n").toCharArray(ssid, sizeof(ssid) - 1);
  server.arg("p").toCharArray(password, sizeof(password) - 1);
  server.sendHeader("Location", "wifi", true);
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send ( 302, "text/plain", "");  // Empty content inhibits Content-length header so we have to close the socket ourselves.
  server.client().stop(); // Stop is needed because we sent no content length
  saveCredentials();
  connect = strlen(ssid) > 0; // Request WLAN connect with new credentials if there is a SSID
  connectWifi();
}

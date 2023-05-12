#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

const char* ssid = "Esptest";
const char* password = "14052002";
const char* PARAM_INPUT_1 = "state";

#define output D6 //Đèn
#define PIR1_PIN D1  //Cảm biến phía trong
#define PIR2_PIN D2  //Cảm biến phía ngoài
#define SW_PIN D5 //Công tắc
#define SW_DELAY 100 //Delay check công tắc

typedef enum PIR_STATE {DETECTED = 0, N_DETECTED = 1} PIR_STATE; //Trạng thái của cảm biến
typedef enum DETECT_STATE {FREE, ENTERING1, ENTERING2, ENTERING3, EXITING1, EXITING2, EXITING3} DETECT_STATE; //Trạng thái của code

int ledState = LOW;

PIR_STATE statusPir1, statusPir2;
DETECT_STATE detectState = FREE;
unsigned int countPeople = 0;
unsigned long pastSw = 0; //Time check delay công tắc

void IRAM_ATTR PIR1_ISR();
void IRAM_ATTR PIR2_ISR();
void IRAM_ATTR SW_ISR();
void turnOnLed();
void turnOffLed();
void toggleLed();

AsyncWebServer server(80);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta charset="UTF-8">
  <title>ESP Web Server</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    form {
      display:flex;
      justify-content: center;
      align-items: center;
         }
    .hide {
            display: none;
          }
    html {font-family: Arial; display: inline-block; text-align: center;}
    h2 {font-size: 3.0rem;}
    p {font-size: 3.0rem;}
    body {max-width: 600px; margin:0px auto; padding-bottom: 25px;}
    .switch {position: relative; display: inline-block; width: 120px; height: 68px} 
    .switch input {display: none}
    .slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; border-radius: 34px}
    .slider:before {position: absolute; content: ""; height: 52px; width: 52px; left: 8px; bottom: 8px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 68px}
    input:checked+.slider {background-color: #2196F3}
    input:checked+.slider:before {-webkit-transform: translateX(52px); -ms-transform: translateX(52px); transform: translateX(52px)}
  </style>
</head>
<body>
  <h2>ESP Web Server</h2>
  %BUTTONPLACEHOLDER%
   <form>
        <table>
            <tr>
                <th colspan="2">Đăng nhập</th>
            </tr>
            <tr>
                <td>
                    Tài khoản:
                </td>
                <td>
                    <input id="username" type="text" placeholder="username">
                </td>
            </tr>
            <tr>
                <td>
                    Mật khẩu:
                </td>
                <td>
                    <input id="password" type="password" placeholder="********">
                </td>
            </tr>
            <tr>
                <th colspan="2"><input id="logBtn" type="button" value="Đăng nhập"></th>
            </tr>
        </table>
    </form>
    
    <di class="hide">
        <br><br>
        <h5>Tình trạng phòng: <span id = "count"></span></h5>
        <h5>Thời gian đèn đã được sử dụng: <span id = "time"></span> </h5>
        <h5>Tắt nguồn đèn: <input type="button" value="Tắt"> </h5> 
        <h5>Bật nguồn đèn: <input type="button" value="Bất"> </h5>
    </di>
    
<script>
  function toggleCheckbox(element) {
  var xhr = new XMLHttpRequest();
  if(element.checked){ 
    xhr.open("GET", "/update?state=1", true); 
  }
  else { 
    xhr.open("GET", "/update?state=0", true); 
  }
  xhr.send();
}

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var inputChecked;
      var outputStateM;
      var countM;
      if( this.responseText == 1){ 
        inputChecked = true;
        outputStateM = "On";
        countM = "Có người";
      }
      else { 
        inputChecked = false;
        outputStateM = "Off";
        countM = "Không có người";
      }
      document.getElementById("output").checked = inputChecked;
      document.getElementById("outputState").innerHTML = outputStateM;
      document.getElementById("count").innerHTML = countM;
    }
  };
  xhttp.open("GET", "/state", true);
  xhttp.send();
}, 1000 ) ;

        var user = "";
        user = document.getElementById("username").value;
        var pass = "";
        pass = document.getElementById("password").value;
        console.log(user,pass);


        const btn = document.getElementById('logBtn');
        const username = document.getElementById('username')
        const password = document.getElementById('password')
        
        btn.addEventListener('click', (e) => { 
          e.preventDefault();
        
          data = {
            username: username.value,
            password: password.value
          }
            
          console.log(data);
            
          (async () => {
            const res = await fetch('https://esp.f4koin.cyou/api/login', {
              method: 'POST',
              headers: {
                'Accept': 'application/json',
                'Content-Type': 'application/json'
              },
              body: JSON.stringify(data)
            });
            const content = await res.json();
            console.log(content);
        
            if(content.message == "Login success")
            {
                    let page = document.querySelector(".hide");
                    page.classList.remove("hide");
                    console.log("test");
                    document.getElementById("time").innerHTML = content.total_active_time;
            }
            else alert("Sai tài khoản / mật khẩu");
          })();
        })

var checkbox = document.querySelector("input[type=checkbox]");

dataOn = {
  status:"on"
}

dataOff = {
  status:"off"
}

checkbox.addEventListener('change', function() {
  if (this.checked) {
    (async () => {
            const resOn = await fetch('https://esp.f4koin.cyou/api/log', {
              method: 'POST',
              headers: {
                'Accept': 'application/json',
                'Content-Type': 'application/json'
              },
             body: JSON.stringify(dataOn)
            });
            const contentOn = await resOn.json();
            console.log(contentOn);
          })();
//    console.log("Checkbox is checked..");
  } else {
    (async () => {
            const resOff = await fetch('https://esp.f4koin.cyou/api/log', {
              method: 'POST',
              headers: {
                'Accept': 'application/json',
                'Content-Type': 'application/json'
              },
              body: JSON.stringify(dataOff)
            });
            const contentOff = await resOff.json();
            console.log(contentOff);
          })();
//    console.log("Checkbox is not checked..");
  }
});
</script>
</body>
</html>
)rawliteral";

String processor(const String& var){
  //Serial.println(var);
  if(var == "BUTTONPLACEHOLDER"){
    String buttons ="";
    String outputStateValue = outputState();
    buttons+= "<h4>Trạng thái: <span id=\"outputState\"></span></h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"output\" " + outputStateValue + "><span class=\"slider\"></span></label>";
    return buttons;
  }
  return String();
}

String outputState(){
  if(digitalRead(output)){
    return "checked";
  }
  else {
    return "";
  }
  return "";
}

void setup(){
  Serial.begin(115200);
  pinMode(output, OUTPUT);
  digitalWrite(output, LOW);
  pinMode(PIR1_PIN, INPUT_PULLUP);
  pinMode(PIR2_PIN, INPUT_PULLUP);
  pinMode(SW_PIN, INPUT_PULLUP);
  statusPir1 = (PIR_STATE)digitalRead(PIR1_PIN); //gán trạng thái ban đầu cho cảm biến
  statusPir2 = (PIR_STATE)digitalRead(PIR2_PIN); //gán trạng thái ban đầu cho cảm biến
  attachInterrupt(digitalPinToInterrupt(PIR1_PIN), PIR1_ISR, CHANGE); //Khởi tạo ngắt cho cảm biến 1
  attachInterrupt(digitalPinToInterrupt(PIR2_PIN), PIR2_ISR, CHANGE); //Khởi tạo ngắt cho cảm biến 2
  attachInterrupt(digitalPinToInterrupt(SW_PIN), SW_ISR, CHANGE); //Khởi tạo ngắt cho công tắc
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  // Send a GET request to <ESP_IP>/update?state=<inputMessage>
  server.on("/update", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    String inputParam;
    // GET input1 value on <ESP_IP>/update?state=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      inputParam = PARAM_INPUT_1;
      digitalWrite(output, inputMessage.toInt());
      ledState = !ledState;
    }
    else {
      inputMessage = "No message sent";
      inputParam = "none";
    }
    request->send(200, "text/plain", "OK");
  });

  // Send a GET request to <ESP_IP>/state
  server.on("/state", HTTP_GET, [] (AsyncWebServerRequest *request) {
    request->send(200, "text/plain", String(digitalRead(output)).c_str());
  });
  // Start server
  server.begin();
}
  
void loop() {}

void IRAM_ATTR PIR1_ISR() {
  statusPir1 = (PIR_STATE)digitalRead(PIR1_PIN);
  switch (detectState) {
    case FREE:
      if (statusPir1 == DETECTED) detectState = EXITING1;
      break;
    case ENTERING1:
      if (statusPir1 == DETECTED) {
        ++countPeople;
        if (countPeople == 1) turnOnLed();
        detectState = ENTERING2;
      }
      break;
    case ENTERING2:
      if (statusPir1 == N_DETECTED) detectState = EXITING3;
      break;
    case ENTERING3:
      if (statusPir1 == N_DETECTED) detectState = FREE;
      break;
    case EXITING1:
      if (statusPir1 == N_DETECTED) detectState = FREE;
      break;
    case EXITING2:
      if (statusPir1 == N_DETECTED) detectState = EXITING3;
      break;
    case EXITING3:
      if (statusPir1 == DETECTED) detectState = ENTERING2;
      break;
    }
    delayMicroseconds(60);
}
void IRAM_ATTR PIR2_ISR() {
  statusPir2 = (PIR_STATE)digitalRead(PIR2_PIN);
  switch (detectState) {
    case FREE:
      if (statusPir2 == DETECTED) detectState = ENTERING1;
      break;
    case ENTERING1:
      if (statusPir2 == N_DETECTED) detectState = FREE;
      break;
    case ENTERING2:
      if (statusPir2 == N_DETECTED) detectState = ENTERING3;
      break;
    case ENTERING3:
      if (statusPir2 == DETECTED) detectState = EXITING2;
      break;
    case EXITING1:
      if (statusPir2 == DETECTED) detectState = EXITING2;
      break;
    case EXITING2:
      if (statusPir2 == N_DETECTED) detectState = ENTERING3;
      break;
    case EXITING3:
      if (statusPir2 == N_DETECTED) {
        if (countPeople > 1) 
          --countPeople;
        else if (countPeople == 1) {
          turnOffLed();
          --countPeople;
        }
        detectState = FREE;
      }
      break;
  }
  delayMicroseconds(60);
}
void IRAM_ATTR SW_ISR() {
  if (((unsigned long)(millis() - pastSw) >= SW_DELAY)) {
    pastSw = millis();
    toggleLed();
  }
}
void turnOnLed() {
  if (!ledState) {
    digitalWrite(output, HIGH);
    ledState = 1;
  }
}
void turnOffLed() {
  if (ledState) {
    digitalWrite(output, LOW);
    ledState = 0;
  }
}
void toggleLed() {
  if (ledState) {
    digitalWrite(output, LOW);
    ledState = 0;
  } else {
    digitalWrite(output, HIGH);
    ledState = 1;
  }
}

#include <FastLED.h>

#include <Arduino.h>
// #include <Wire.h>
#include <VL53L0X.h>

#include <WiFi.h>
#include <AsyncTCP.h>             // https://github.com/me-no-dev/AsyncTCP/archive/master.zip                copiar para: C:\Users\jefer\AppData\Local\Arduino15\packages\esp32\hardware\esp32\1.0.6\libraries
#include <ESPAsyncWebServer.h>    // https://github.com/me-no-dev/ESPAsyncWebServer/archive/master.zip

// Para saber o IP alocado/reservado para o esp32-arduino é só entrar no Roteador de Internet, no
// meu caso é da Vivo (IP local 192.168.15.1) e lá consta a relação de Clientes conectados, ou,
// conectar o Esp32 pela USB do computador e rodar o programa com o "Serial" ativado que o programa
// exibe o IP após a conexão.


// #include "Sensor.h"
#include "Escada.h"

// -- Fita LED --
#define NUM_LEDS 210        // qtde leds na fita
#define DATA_PIN 33
CRGBArray<NUM_LEDS> leds;
String MASK = "1010101010";   // máscara - quais leds serão ativados por degrau
// --

// -- Sensor LDR - luminosidade
#define LDR_PIN 36      // WiFi ligado. Usar somente as saídas "ADC1" do Esp32
// ---

struct MyRGB {
  byte r;
  byte g;
  byte b;
};

// --- Sensores Ultrassonicos Laser ---
byte pins[] = {25,26,27};
int long lastCheckSensor = 0;
bool pausar_sensor = false;

// // byte pins[] = {25,26,27};
// // byte tam = sizeof(pins)/sizeof(pins[0]);
byte sensorAtivado;
// int intPin = 14;
// bool newData = false;
// uint32_t Now = 0;                         // used to calculate integration interval
// uint32_t lastUpdate = 0, firstUpdate = 0; // used to calculate integration interval
// --


// --- Configurações padrão global
int VELOCIDADE = 1;
int BRILHO_MAX = 200;
int PAUSA = 5000;
MyRGB COLOR = {0, 0, 255};   // blue
int DAY_LIGHT = 300;
// ---
int intervaloTentativaConexaoWiFi = 30000;
// ---

// --- Conexão WiFi
char *ssid = "Nolasco";
char *psw = "portugA2022";
long int ultimaTentativaConexaoWiFi = 0;
// ---

// ---  headers das funções ---
void toggleLED(byte idxEscada, int idxDegrau);
void message(String txt);
void fimProcesso(byte idxEsc);
// ---

// ---- Instância : ESCADA
// ----  myIndex - start idx - qtde degraus - e funções
Escada esc1(1 ,   0 , 14 , toggleLED, fimProcesso , message);
Escada esc2(2 , 140 ,  7 , toggleLED, fimProcesso , message);
// ---

// ---- Instância : SENSORES ULTRASSONICO
// Sensor sensores(pins, tam, message);
// ---

// WebServer server(80);
AsyncWebServer server(80);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<title>Escada Inteligente</title>
<script>
  function change(value){
    document.getElementById("sensor").value=value;
  }
</script>
<style>
  html { 
    font-family: Helvetica; 
    display: inline-block; 
    margin: 0px auto; 
    text-align: center;
  }
  body{
    margin-top: 50px;
  } 
  .title{ 
    color: rgb(0,86,179); 
  }
  .text{
    font-size: 120%;
  }

  .button {
    border: none;
    color: white;
    padding: 10px 25px;
    font-size: 16px;
    margin: 3px 1px;
    transition-duration: 0.4s;
    cursor: pointer;
    border-radius: 5px;
    text-decoration: none;
  }

  .button1 {
    background-color: rgb(197, 224, 253);
    color: black; 
    border: 1px solid rgb(130, 188, 250);
  }

  .button1:hover {
    background-color: rgb(0,86,179);
    color: white;
  }

  .buttonx {
    background-color: rgb(255, 255, 255);
    color: black; 
    border: 1px solid rgb(54, 86, 121);
  }

  .buttonx:hover {
    background-color: rgb(35, 141, 255);
    color: white;
  }

  .lbl1 {
    font-size: 60%;
    position: relative;
    bottom: 5px;
    right: -10px;
  }

  .lbl2 {
    font-size: 60%;
    position: relative;
    bottom: 5px;
    left: 360px;
  }

  .ipt {
    width: 100%;
  }

  .div-main {
    background-color: rgb(250, 250, 250);
    text-align: left;
    max-width: 420px;
    border-top: solid rgb(160, 160, 160) 1px;
    border-bottom: solid rgb(160, 160, 160) 1px;
    padding: 30px;
    margin: auto;
    margin-bottom: 10px;
  }

  .output {
    font-size: 80%;
  }

</style>
</head>
<body>
  <h1 class="title">Escada Inteligente</h1>
  <form action="/">
    <div class="div-main">
      <label class="text" for="cor">Selecione a cor da iluminação:</label>
      <input name="color" id="color" type="color" value="@@color_hex@@"><br><br> 

      <label class="text" for="velocity">Velocidade dos passos:</label>
      <input class="ipt" name="velocity" id="velocity" type="range" min="1" max="2000" value="@@velocity@@">
      <span class="lbl1">rápido</span>
      <span class="lbl2">lento</span><br><br>

      <label class="text" for="pause">Pausa para desligar:</label>
      <input class="ipt" name="pause" id="pause" type="range" min="3000" max="20000" value="@@pause@@">
      <span class="lbl1">menor</span>
      <span class="lbl2">maior</span><br><br>

      <label class="text" for="bright">Intensidade do brilho:</label>
      <input class="ipt" name="bright" id="bright" type="range" min="10" max="220" value="@@bright@@">
      <span class="lbl1">menor</span>
      <span class="lbl2">maior</span><br><br>

      <label class="text" for="daylight">Sensibilidade da luz natural:</label>
      <input class="ipt" name="daylight" id="daylight" type="range" min="1" max="1500" oninput="this.nextElementSibling.value = this.value" value="@@daylight@@">
      <output class="output"></output>
      <span class="lbl1">menor</span>
      <span class="lbl2">maior</span><br><br>

      <span>Simular ativação de sensores</span><br>
      <input type="hidden" id="sensor" name="sensor" value="">
      <button class="button buttonx" onclick="change(1)">Sensor 1</button>
      <button class="button buttonx" onclick="change(2)">Sensor 2</button>
      <button class="button buttonx" onclick="change(3)">Sensor 3</button>
      <br><br>
      
      <span>Luz atual: @@luzatual@@</span>
    </div>
    <button type="submit" class="button button1">Salvar</button>
  </form> 

</body>
</html>
)rawliteral";


void toggleLED(byte idxEscada, int idxDegrau)
{
  Escada escada = idxEscada==1 ? esc1 : esc2;
  if (escada.getModo() == eModo::OFF) {
    fadeOut(idxEscada, idxDegrau);
  } else {
    fadeIn(idxEscada,idxDegrau);
  }
};

void message(String txt)
{
  Serial.println(txt);
};

String rgbToHex() {
  String r = String(COLOR.r, HEX);
  r = r.length()==1?"0"+r:r;

  String g = String(COLOR.g, HEX);
  g = g.length()==1?"0"+g:g;

  String b = String(COLOR.b, HEX);
  b = b.length()==1?"0"+b:b;

  return "#"+r+g+b;
}

void effectAll() {
  esc1.setStandBy();
  esc2.setStandBy();

  for(int x=0; x<NUM_LEDS; x++) {
    leds[x] = CRGB::Black;
  }
  FastLED.show();

  for(int x=0; x<2; x++) {
    for(int x=0; x<NUM_LEDS; x+=2) {
      leds[x].setRGB(COLOR.r, COLOR.g, COLOR.b);
    }
    FastLED.delay(100);

    for(int x=0; x<NUM_LEDS; x+=2) {
      leds[x] = CRGB::Black;
    }
    FastLED.delay(100);
  }
}

void simularSensor(int sensor) {
 if (todasEmStandyBy()) {
   sensorAtivado = sensor;

    if (sensorAtivado==1) {
      esc1.prepararEvento(ON, eDirecao::DESCE);
    } 
    else
    if (sensorAtivado==2) {
      esc2.prepararEvento(ON, eDirecao::SOBE);
    } 
    if (sensorAtivado==3) {        
      esc1.prepararEvento(ON, eDirecao::SOBE);
      esc2.prepararEvento(ON, eDirecao::DESCE);
    } 
  }
}

void startServer() {
  server.on("/", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String html = index_html;
    int _simularSensor = 0;    

    int paramsCount = request->params();

    html.replace("@@luzatual@@", String(analogRead(LDR_PIN)) );

    if (paramsCount == 0) {
      html.replace("@@color_hex@@", rgbToHex());
      html.replace("@@velocity@@", String(VELOCIDADE));
      html.replace("@@pause@@", String(PAUSA));
      html.replace("@@bright@@", String(BRILHO_MAX));
      html.replace("@@daylight@@", String(DAY_LIGHT));
    }

    for(int i=0; i<paramsCount; i++) {
      AsyncWebParameter* p = request->getParam(i);

      Serial.println(p->name()+"  "+p->value());

      if (p->name() == "color") {
        setColor(p->value());
        html.replace("@@color_hex@@", p->value());
      }
      else
      if (p->name() == "velocity") {
        VELOCIDADE = p->value().toInt();
        html.replace("@@velocity@@", p->value());
      }
      else
      if (p->name() == "pause") {
        PAUSA = p->value().toInt();
        html.replace("@@pause@@", p->value());
      }
      else
      if (p->name() == "bright") {
        BRILHO_MAX = p->value().toInt();
        FastLED.setBrightness(BRILHO_MAX);
        html.replace("@@bright@@", p->value());
      }
      else
      if (p->name() == "daylight") {
        DAY_LIGHT = p->value().toInt();
        html.replace("@@daylight@@", p->value());
      }
      else
      if (p->name() == "sensor") {
        _simularSensor = p->value().toInt();
      }      
    }

    request->send(200, "text/html", html);

    if (_simularSensor >0) {
      simularSensor(_simularSensor);
    } else {
      effectAll();
    }
  });

  server.begin();
  Serial.println("HTTP server started");
}


void checkWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, LOW);
    return;
  }

  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));

  if (ultimaTentativaConexaoWiFi!=0 && (millis()-ultimaTentativaConexaoWiFi) < intervaloTentativaConexaoWiFi) {
    return;
  }

  Serial.println("conectando wifi ["+ String(ssid) +"]...");
  ultimaTentativaConexaoWiFi = millis();

  WiFi.begin(ssid, psw);

  for(int t=0; t<6; t++) {
    if (WiFi.status() == WL_CONNECTED) {
      break;
    }
    Serial.print("*");
    delay(500);
  }

  if (WiFi.status() == WL_CONNECTED) {
    server.begin();
    Serial.print("wifi conectado IP ");
    Serial.println(WiFi.localIP());

    startServer();
  }
}











// void myinthandler()
// {
//   newData = true; // set the new data ready flag to true on interrupt
// }

// void I2Cscan()
// {
// // scan for i2c devices
//   byte error, address;
//   int nDevices;

//   Serial.println("Scanning...");

//   nDevices = 0;
//   for(address = 1; address < 127; address++ ) 
//   {
//     // The i2c_scanner uses the return value of
//     // the Write.endTransmisstion to see if
//     // a device did acknowledge to the address.
//     Wire.beginTransmission(address);
//     error = Wire.endTransmission();

//     if (error == 0)
//     {
//       Serial.print("I2C device found at address 0x");
//       if (address<16) 
//         Serial.print("0");
//       Serial.print(address,HEX);

//       nDevices++;
//     }
//     else if (error==4) 
//     {
//       Serial.print("Unknown error at address 0x");
//       if (address<16) 
//         Serial.print("0");
//       Serial.println(address,HEX);
//     }    
//   }
//   if (nDevices == 0)
//     Serial.println("No I2C devices found\n");
//   else
//     Serial.println("done\n");
    
// }
// VL53L0X sensor;
// // Uncomment this line to use long range mode. This
// // increases the sensitivity of the sensor and extends its
// // potential range, but increases the likelihood of getting
// // an inaccurate reading because of reflections from objects
// // other than the intended target. It works best in dark
// // conditions.

// #define LONG_RANGE


// // Uncomment ONE of these two lines to get
// // - higher speed at the cost of lower accuracy OR
// // - higher accuracy at the cost of lower speed

// #define HIGH_SPEED
// //#define HIGH_ACCURACY



void setup() {
  Serial.begin(9600);

  pinMode(LED_BUILTIN, OUTPUT);  // sem WiFi, piscante

  for(int x=0; x<3; x++) {
    pinMode(pins[x], INPUT);
    digitalWrite(pins[x], LOW);
    delay(50);
  }

  
  // Wire.begin(21, 22, 400000); // SDA (21), SCL (22) on ESP32, 400 kHz rate
  //  I2Cscan();
  
  // delay(1000);
  
  // sensor.init();
  // sensor.setTimeout(500);
  // #if defined LONG_RANGE
  //   // lower the return signal rate limit (default is 0.25 MCPS)
  //   sensor.setSignalRateLimit(0.01);   // 0.1
  //   // increase laser pulse periods (defaults are 14 and 10 PCLKs)
  //   sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodPreRange, 10);       // 18   (10)
  //   sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodFinalRange, 14);      // 14
  // #endif

  // #if defined HIGH_SPEED
  //   // reduce timing budget to 20 ms (default is about 33 ms)
  //   sensor.setMeasurementTimingBudget(200000);  // minimum timing budget 20 ms
  // #elif defined HIGH_ACCURACY  
  //   // increase timing budget to 200 ms
  //   sensor.setMeasurementTimingBudget(200000);
  // #endif

  // // Start continuous back-to-back mode (take readings as
  // // fast as possible).  To use continuous timed mode
  // // instead, provide a desired inter-measurement period in
  // // ms (e.g. sensor.startContinuous(100)).
  // sensor.startContinuous();
  // pinMode(intPin, INPUT);
  // delay(10);
  // attachInterrupt(intPin, myinthandler, FALLING);  // define interrupt for GPI01 pin output of VL53L0X






  

  // sensores.init();  

  checkWiFi();
  
  // --- inicializar LEDS -----
  LEDS.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
  LEDS.setBrightness(BRILHO_MAX);
  effectAll();
  // ---  
}

void setColor(String cor) {
  int nr = (int) strtol(&cor[1], NULL, 16);

  int r = nr >> 16;
  int g = nr >> 8 & 0xFF;
  int b = nr & 0xFF;

  COLOR = {r,g,b};
}


bool detectouMovimento() {
  sensorAtivado = 0;

  if (pausar_sensor) {
    if (millis()-lastCheckSensor < 10000) {
Serial.println(".....nao verific.......");
      return false;
    }
    pausar_sensor = false;
  }
  
  // lastCheckSensor = millis();
//   Serial.println("detectou?");

//   int resp = sensores.checkSensor();
//   if (resp >= 0 && resp <= 2) {
//     sensorAtivado = resp+1;     

//     Serial.println("....sensor "+String(sensorAtivado));
//   }

  // long int pausa = 1000;

Serial.print("verificando.........") ;
  int read = 0;

  for (int x=0; x<3; x++) {
    delay(10);

    if (digitalRead(pins[x])) {
      Serial.println("S"+String(x+1));
      sensorAtivado = x+1;
      break;

      // delay(500);
      // sensorAtivado = x+1;
      // break;

      // long int inicio = millis();
      // while (millis() - inicio < pausa) {}

      // if (digitalRead(pins[x])) {
      //   sensorAtivado = x+1;
      //   break;
      // }
      // // int qtd = 0;

      // // while(digitalRead(pins[x])) {
      // //   qtd++;
      // // }
      // // Serial.print("S"+String(x+1)+"-qtd="+String(qtd));
    }
  }
  
  if (sensorAtivado > 0) {
    Serial.println("---sensor "+String(sensorAtivado)+"---");
  } else {
    Serial.print(".");
  }
 
  return sensorAtivado>0;
}


void fimProcesso(byte idxEsc) {
  // sensor 1 (processo 1) => DESCE : ON : esc1 ... DESCE : ON : esc2
  // sensor 2 (processo 2) => SOBE  : ON : esc2 ... SOBE  : ON : esc1  
  // sensor 3 (processo 3) => SOBE  : ON : esc1
  //                          DESCE : ON : esc2

  // aqui temos que tratar os processos 1 e 2  

  bool processo1 = 
    idxEsc==1 &&
    esc1.getStatus() == eStatus::INTERVALO &&
    esc1.getModo() == eModo::ON && 
    esc1.getDirecao() == eDirecao::DESCE;

  bool processo2 = 
    idxEsc==2 &&
    esc2.getStatus() == eStatus::INTERVALO &&
    esc2.getModo() == eModo::ON && 
    esc2.getDirecao() == eDirecao::SOBE;

  if (processo1) {
    // continuar o processo: acendeu a Esc1 agora acender a Esc2
    esc2.prepararEvento(ON, eDirecao::DESCE);
Serial.println("fim processo Nr 1");
pausar_sensor = true;
lastCheckSensor = millis();
  } else
  if (processo2) {
    // continuar o processo: acendeu a Esc2 agora acender a Esc1
    esc1.prepararEvento(ON, eDirecao::SOBE);
Serial.println("fim processo Nr 2");
pausar_sensor = true;
lastCheckSensor = millis();
  } else {
Serial.println("fim processo Nr 33333");
pausar_sensor = true;
lastCheckSensor = millis();

  }
}


bool todasEmStandyBy() {
  return
    (esc1.getStatus() == eStatus::STANDBY) &&
    (esc2.getStatus() == eStatus::STANDBY);
}


void fadeOut(byte idxEscada, int idxDegrau) {
  Escada escada = idxEscada==1 ? esc1 : esc2;
  
  for(int x = 255; x>=0; x-=8) {
    uint8_t idx = escada.getStartIndex() + (idxDegrau * 10);
    CHSV hsv = getHSV(idx);

    for (int s = 0; s < 10; s++)
    {
      bool flag = MASK.substring(s, s+1).toInt() == 1;
      if (flag) {
        leds[idx] = CHSV(hsv.h, hsv.s, x==7?0:x);
      }
      idx++;
    };
    FastLED.delay(7);
  }
}


void fadeIn(byte idxEscada, int idxDegrau) {
  Escada escada = idxEscada==1 ? esc1 : esc2;
  
  for(int x = 0; x<256; x+=8) {
    uint8_t idx = escada.getStartIndex() + (idxDegrau * 10);
    CHSV hsv = getHSV(idx);

    for (int s = 0; s < 10; s++)
    {
      bool flag = MASK.substring(s, s+1).toInt() == 1;
      if (flag) {
        leds[idx] = CHSV(hsv.h, hsv.s, x);
      }
      idx++;
    };
    FastLED.delay(7);
  }
}


CHSV getHSV(int idx) {
  CRGB volta = leds[1];
  leds[1] = CRGB(COLOR.r, COLOR.g, COLOR.b);
  CHSV corhsv = rgb2hsv_approximate( leds[1] );
  leds[1] = volta;
  return corhsv;
}

bool estaEscuro() {
  // int value = analogRead(LDR_PIN);
  // return  value <= DAY_LIGHT;
//jef
  return true;
}

void loop() {
  // checkWiFi();
  // detectouMovimento();


 
//  if (newData) // wait for data ready interrupt
//   {
//      newData = false; // reset data ready flag
//      Now = micros(); // capture interrupt time
//      // calculate time between last interrupt and current one, convert to sample data rate, and print to serial monitor
//      Serial.print("data rate = "); Serial.print(1000000./(Now - lastUpdate)); Serial.println(" Hz");

//     long int aa = sensor.readRangeContinuousMillimeters();

//     if (aa < 8000) {
//      Serial.print(aa); // prit range in mm to serial monitor
//      if (sensor.timeoutOccurred()) { Serial.print(" TIMEOUT"); }
//     }

//     Serial.println();
//   }
//   lastUpdate = Now;


  // delay(100);
  // Serial.print(".");


  if (todasEmStandyBy()) {
    if (estaEscuro()) {
      if (detectouMovimento()) {
        if (sensorAtivado==1) {
          esc1.prepararEvento(ON, eDirecao::DESCE);
        } 
        else
        if (sensorAtivado==2) {
          esc2.prepararEvento(ON, eDirecao::SOBE);
        } 
        if (sensorAtivado==3) {        
          esc1.prepararEvento(ON, eDirecao::SOBE);
          esc2.prepararEvento(ON, eDirecao::DESCE);
        } 
      } 
        
      delay(100);
    }
  } else {
    esc1.proximoPasso();
    esc2.proximoPasso();
    delay(VELOCIDADE);
  }

delay(200);
  Serial.println(".");
}
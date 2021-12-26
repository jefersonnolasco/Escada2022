
#include <FastLED.h>

blaaaa

#include <WiFi.h>
#include <AsyncTCP.h>             // https://github.com/me-no-dev/AsyncTCP/archive/master.zip                copiar para: C:\Users\jefer\AppData\Local\Arduino15\packages\esp32\hardware\esp32\1.0.6\libraries
#include <ESPAsyncWebServer.h>    // https://github.com/me-no-dev/ESPAsyncWebServer/archive/master.zip

// Para saber o IP alocado/reservado para o esp32-arduino é só entrar no Roteador de Internet, no
// meu caso é da Vivo (IP local 192.168.15.1) e lá consta a relação de Clientes conectados, ou,
// conectar o Esp32 pela USB do computador e rodar o programa com o "Serial" ativado que o programa
// exibe o IP após a conexão.


#include "Sensor.h"
#include "Escada.h"

// -- Fita LED --
#define NUM_LEDS 210        // qtde leds na fita
#define DATA_PIN 32
CRGBArray<NUM_LEDS> leds;
String MASK = "1010101010";   // máscara - quais leds serão ativados por degrau
// --

struct MyRGB {
  byte r;
  byte g;
  byte b;
};

// --- Sensores Ultrassonicos Laser ---
byte pins[] = {25,26,27};
byte tam = sizeof(pins)/sizeof(pins[0]);
byte sensorAtivado;
// --


// --- Configurações padrão global
int VELOCIDADE = 1;
int BRILHO_MAX = 200;
int PAUSA = 5000;
MyRGB COLOR = {0, 0, 255};   // blue
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
Sensor sensores(pins, tam, message);
// ---

// WebServer server(80);
AsyncWebServer server(80);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<title>Escada Inteligente</title>
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

  .fast {
    font-size: 60%;
    position: relative;
    bottom: -8px;
    left: -134px;
  }

  .slow {
    font-size: 60%;
    position: relative;
    bottom: -8px;
    left: -62px;
  }

  .div-main {
    background-color: rgb(250, 250, 250);
    text-align: left;
    max-width: 400px;
    border-top: solid rgb(160, 160, 160) 1px;
    border-bottom: solid rgb(160, 160, 160) 1px;
    padding: 30px;
    margin: auto;
    margin-bottom: 10px;
  }

</style>
</head>
<body>
  <h1 class="title">Escada Inteligente</h1>
  <form action="/">
    <div class="div-main">
      <!-- <hr> -->
      <label class="text" for="cor">Selecione a cor da iluminação:</label>
      <input name="color" id="color" type="color" value="@@color_hex@@"><br><br> 

      <div>
        <label class="text" for="velocity">Velocidade dos passos:</label>
        <input name="velocity" id="velocity" type="range" min="1" max="2000" value="@@velocity@@">
        <span class="fast">rápido</span>
        <span class="slow">lento</span><br><br> 
      </div>

      <div>
        <label class="text" for="pause">Pausa para apagar:</label>
        <input name="pause" id="pause" type="range" min="3000" max="20000" value="@@pause@@">
        <span class="fast">menor</span>
        <span class="slow">maior</span><br><br> 
      </div>

      <div>
        <label class="text" for="bright">Intensidade do brilho dos leds:</label>
        <input name="bright" id="bright" type="range" min="100" max="220" value="@@bright@@">
        <span class="fast">menor</span>
        <span class="slow">maior</span><br><br> 
      </div>

      luminosidade <br>
      brilho <br>

      simular sensor1 <br>
      simular sensor2 <br>
      simular sensor3 <br>

      <!-- <hr> -->
    </div>
    <button type="submit" class="button button1">Salvar</button>
  </form> 

</body>
</html>)rawliteral";


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
  // esc1.setStandBy();
  // esc2.setStandBy();

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


void startServer() {
  server.on("/", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String html = index_html;

    int paramsCount = request->params();

    if (paramsCount == 0) {
      html.replace("@@color_hex@@", rgbToHex());
      html.replace("@@velocity@@", String(VELOCIDADE));
      html.replace("@@pause@@", String(PAUSA));
      html.replace("@@bright@@", String(BRILHO_MAX));
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
        html.replace("@@bright@@", p->value());
      }
    }

    request->send(200, "text/html", html);
    effectAll();
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

void setup() {
  Serial.begin(9600);

  pinMode(LED_BUILTIN, OUTPUT);  // sem WiFi, piscante

  sensores.init();  

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

  int resp = sensores.checkSensor();
  if (resp >= 0 && resp <= 2) {
    sensorAtivado = resp+1;     

    Serial.println("....sensor "+String(sensorAtivado));
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
  }
  if (processo2) {
    // continuar o processo: acendeu a Esc2 agora acender a Esc1
    esc1.prepararEvento(ON, eDirecao::SOBE);
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


void loop() {
  checkWiFi();

  if (todasEmStandyBy()) {
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
  } else {
    esc1.proximoPasso();
    esc2.proximoPasso();

    delay(VELOCIDADE);
  }
}
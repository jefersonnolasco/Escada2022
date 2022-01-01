#include <Arduino.h>
#include <FastLED.h>
#include <WiFi.h>
#include <AsyncTCP.h>             // https://github.com/me-no-dev/AsyncTCP/archive/master.zip                copiar para: ...\AppData\Local\Arduino15\packages\esp32\hardware\esp32\1.0.6\libraries
#include <ESPAsyncWebServer.h>    // https://github.com/me-no-dev/ESPAsyncWebServer/archive/master.zip
#include <ESPmDNS.h>

#include "./libs/escada/Escada.h"
#include "./libs/sensor-mov/SensorMov.h"
#include "./constants/constants.h"
#include "./funcoes/funcoes.h"

// -- Fita LED --
#define NUM_LEDS 210        // qtde leds na fita
#define DATA_PIN 33
CRGBArray<NUM_LEDS> leds;
String MASK = "1010101010";   // máscara - quais leds serão ativados por degrau

// -- Sensor LDR - luminosidade
#define LDR_PIN 36      // Atenção! WiFi ligado! Usar somente as saídas "ADC1" do Esp32

// --- Sensores Movimento ---
SensorMov sensorMov(25,26,27);

// --- Configurações padrão global
int VELOCIDADE = 1;
int BRILHO_MAX = 100;
int PAUSA = 5000;
CRGB COLOR = {0,0,255};
int DAY_LIGHT = 300;

// ---
int intervaloTentativaConexaoWiFi = 30000;

// --- Conexão WiFi
const char* ssid = "Nolasco";
const char* psw = "portugA2022";
const char* host = "escada";
long int ultimaTentativaConexaoWiFi = 0;

// ---  headers das funções ---
void toggleLED(byte idxEscada, int idxDegrau);
void fimProcesso(byte idxEsc);
void fadeOut(byte idxEscada, int idxDegrau);
void fadeIn(byte idxEscada, int idxDegrau);
bool todasEmStandyBy();
void setColor(String cor);
CHSV getHSV(int idx);

// ---- Instância : ESCADA
// ----  myIndex - start idx - qtde degraus - e funções
Escada esc1(1 ,   0 , 14 , toggleLED, fimProcesso , message);
Escada esc2(2 , 140 ,  7 , toggleLED, fimProcesso , message);

AsyncWebServer server(80);

void toggleLED(byte idxEscada, int idxDegrau)
{
  Escada escada = idxEscada==1 ? esc1 : esc2;
  if (escada.getModo() == eModo::OFF) {
    fadeOut(idxEscada, idxDegrau);
  } else {
    fadeIn(idxEscada,idxDegrau);
  }
};

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

void simularSensor(eSensor sensor) {
  if (todasEmStandyBy()) {
    sensorMov.setAtivado(sensor);

    if (sensorMov.getAtivado() == eSensor::UM) {
      esc1.prepararEvento(ON, eDirecao::DESCE);
    } 
    else
    if (sensorMov.getAtivado() == eSensor::DOIS) {
      esc2.prepararEvento(ON, eDirecao::SOBE);
    } 
    if (sensorMov.getAtivado() == eSensor::TRES) {
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
      html.replace("@@color_hex@@", rgbToHex(COLOR));
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

    if (_simularSensor > 0) {
      simularSensor( (eSensor)(_simularSensor) );
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

    /* Usa MDNS para resolver o DNS */
    if (!MDNS.begin(host)) { 
        //http://escada.local
        Serial.println("Erro ao configurar mDNS. O ESP32 vai reiniciar em 1s...");
        delay(1000);
        ESP.restart();        
    }

    startServer();
  }
}

void setup() {
  Serial.begin(9600);

  pinMode(LED_BUILTIN, OUTPUT);  // sem WiFi, piscante
  checkWiFi();
  
  // --- inicializar LEDS -----
  LEDS.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
  LEDS.setBrightness(BRILHO_MAX);
  effectAll();
}

void setColor(String cor) {
  int nr = (int) strtol(&cor[1], NULL, 16);

  int r = nr >> 16;
  int g = nr >> 8 & 0xFF;
  int b = nr & 0xFF;

  COLOR.r = r;
  COLOR.g = g;
  COLOR.b = b;
}

void fimProcesso(byte idxEsc) {
  // sensor 1 (processo 1) => DESCE : ON : esc1 ... DESCE : ON : esc2
  // sensor 2 (processo 2) => SOBE  : ON : esc2 ... SOBE  : ON : esc1  
  // sensor 3 (processo 3) => SOBE  : ON : esc1
  //                          DESCE : ON : esc2

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
  } else
  if (processo2) {
    // continuar o processo: acendeu a Esc2 agora acender a Esc1
    esc1.prepararEvento(ON, eDirecao::SOBE);
  } else {
    sensorMov.pausarSensores();
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
  int value = analogRead(LDR_PIN);
  return  value <= DAY_LIGHT;
}

void loop() {
  checkWiFi();

  if (todasEmStandyBy()) {
    if (estaEscuro()) {
      if (sensorMov.detectouMovimento()) {
        if (sensorMov.getAtivado() == eSensor::UM) {
          esc1.prepararEvento(ON, eDirecao::DESCE);
        } 
        else
        if (sensorMov.getAtivado() == eSensor::DOIS) {
          esc2.prepararEvento(ON, eDirecao::SOBE);
        } 
        if (sensorMov.getAtivado() == eSensor::TRES) {
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
}
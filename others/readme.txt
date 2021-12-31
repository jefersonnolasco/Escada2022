a- fotos do ESP32 DevKit V1 ESP-WROOM-32
b- especificações
c- bibliotecas e tutorial de uso
d- datasheet
e- guia de pinagem

neste link: https://www.adrobotica.com/produto/modulo-wifi-esp32-bluetooth/
encontro as informações acima

github (item c):
https://github.com/espressif/arduino-esp32

vídeo explicando como instalar o framework do Arduino (bibliotecas do ESP32)
Resumindo o vídeo, pode começar a partir do: 7 minutos
URL...:   	https://dl.espressif.com/dl/package_esp32_index.json
vídeo.: 	https://www.youtube.com/watch?v=gLfVBOMJ2Nw&t=2s

---
Se ao plugar o ESP na USB e no gerenciador exibir falha, então devemos instalar o
driver USB (instalar para "CP210x")
link: https://s3-sa-east-1.amazonaws.com/robocore-tutoriais/163/CP210x_Windows_Drivers.zip
---
Teste básico:
Arduino-> arquivo, exemplos Basic - Blink (ctrl+U) para compilar/carregar o código no ESP32
=> o LED azul deve ficar piscando
=> Quando compilar e enviar ao ESP32, pressionar o botão BOOT no connectiong....


ESCADA:
------
1a) 4,58 metros - 14 degraus
2ª) 2,21 metros -  7 degraus
----------
total de 21 degraus   (3 leds por degrau = 63 leds)



SOFTWARES:
----------
biblioteca FastLED - Em Sketch - Incluir Biblioteca - Gerenciar bibliotecas "fastled"


MATERIAIS:
----------
FITA LED ENDEREÇÁVEL (WS2812)


projeto com:

a) Módulo WiFi ESP32 bluetooth
b) Fita LED RGB Endereçável WS2812
c) Sensor luminosidade - LDR 5mm  (1 unid)
d) Sensor de distância a laser Vl53l0x  (3 unid)


Escada Um com 14 degraus
Escada Dois com 7 degraus
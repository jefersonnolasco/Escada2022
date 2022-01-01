#ifndef FUNCOES_H
#define FUNCOES_H

#include <Arduino.h>
#include <FastLED.h>

void message(String txt)
{
  Serial.println(txt);
};

String rgbToHex(CRGB COLOR) {
  String r = String(COLOR.r, HEX);
  r = r.length()==1?"0"+r:r;

  String g = String(COLOR.g, HEX);
  g = g.length()==1?"0"+g:g;

  String b = String(COLOR.b, HEX);
  b = b.length()==1?"0"+b:b;

  return "#"+r+g+b;
}

#endif
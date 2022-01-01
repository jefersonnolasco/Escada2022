#ifndef ESCADA_H
#define ESCADA_H

#include <Arduino.h>

#include "./enums/direcao.enum.h"
#include "./enums/modos.enum.h"
#include "./enums/status.enum.h"

class Escada
{
private:
  eModo modo;
  eDirecao direcao;
  eStatus status = eStatus::STANDBY;

  byte startIndex;
  int qtdeDegraus;
  uint8_t intervaloMaxSegundos = 10;
  unsigned long inicioIntervalo;

  int degrauAtual; // este pode ficar (-)
  byte idxEscada;

  void (*toggle)(byte idxEscada, int idxDegrau);  // idxDegrau pode ficar (-)
  void (*msg)(String txt);
  void (*fimProcesso)(byte idxEsc);

  //---
  void checkIntervalo();
  void checkProximoPasso();

public:
  Escada(
      byte idxEscada,
      byte startIndex,
      int qtdeDegraus,  // deixar o mesmo type de 
      void (*toggle)(byte idxEscada, int idxDegrau), 
      void (*fimProcesso)(byte idxEsc), 
      void (*msg)(String txt)
  );

  void prepararEvento(eModo modo, eDirecao direcao);
  int getStartIndex();
  eModo getModo();
  eDirecao getDirecao();
  eStatus getStatus();
  bool proximoPasso();
  void setStandBy();
};
#endif
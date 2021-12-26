#include <Arduino.h>

#include "Escada.h"

Escada::Escada(
  byte idxEscada,
  byte startIndex,
  int qtdeDegraus, 
  void (*toggle)(byte idxEscada, int idxDegrau), 
  void (*fimProcesso)(byte idxEsc), 
  void (*msg)(String txt)
)
{   
  this->idxEscada = idxEscada;
  this->startIndex = startIndex;
  this->qtdeDegraus = qtdeDegraus;
  
  this->toggle = toggle;
  this->fimProcesso = fimProcesso;
  this->msg = msg;
};

void Escada::prepararEvento(eModo modo_, eDirecao direcao_){
  modo = modo_;
  direcao = direcao_;
  status = eStatus::EVOLUCAO;

  if (direcao == SOBE)
  {
      // Se estiver subindo significa que é para decrementar o idx
      degrauAtual = qtdeDegraus-1;
      // msg("== SOBE  degrau atual="+String(degrauAtual));
  } 
  else
  {
      degrauAtual = 0;
      // msg("== DESCE  degrau atual="+String(degrauAtual));
  };
};

void Escada::setStandBy(){
  status = eStatus.STANDBY;
}



eStatus Escada::getStatus() { 
  return status; 
}

int Escada::getStartIndex() {
  return this->startIndex;
};

eModo Escada::getModo() {
  return this->modo;
};
    
eDirecao Escada::getDirecao()
{
  return direcao;
};

bool Escada::proximoPasso(){  
  if (status == eStatus::STANDBY)
  {
    return false;
  };

  if (status == eStatus::INTERVALO)
  {
    checkIntervalo();
  }

  if (status == eStatus::STANDBY) {
    return false;
  }

  if (direcao == eDirecao::SOBE)
  {
    if (degrauAtual < 0) {
      return false;
    };
  } else {
    if (degrauAtual > qtdeDegraus-1) {
        return false;
    };
  }

  toggle(idxEscada, degrauAtual);
  checkProximoPasso();

  return true;
};

void Escada::checkProximoPasso() {
  bool _fimProcesso = false;

  if (direcao == eDirecao::SOBE)
  {   // Se estiver subindo significa que é para decrementar o idx
    degrauAtual--;
    _fimProcesso = degrauAtual < 0;
  }
  else  
  { // (this->_direcao == eDirecao::DESCE)
    this->degrauAtual++;
    _fimProcesso = degrauAtual > qtdeDegraus-1;
  };

  if (_fimProcesso) {
    // se o modo é apagando/OFF, siginifica que acabou o processo, colocar em STANDBY
    if (modo == eModo::OFF) {
      status = eStatus::STANDBY;
    }
    else 
    {
      // então o modo é ON, o processo de acender terminou, agora iniciar o 
      // timer de INTERVALO, para depois apagá-la
      status = eStatus::INTERVALO;
      inicioIntervalo = millis();
    }

    fimProcesso(idxEscada);
  };        
}



void Escada::checkIntervalo() {
  unsigned long atualMillis = millis();
  if (atualMillis - this->inicioIntervalo <= this->intervaloMaxSegundos*1000) {
    return;  // não deu o tempo máx ainda...
  };

  // já deu o tempo limite de espera, agora é INVERTER a evolução...
  if (modo == eModo::OFF) {
    status = eStatus::STANDBY;
  } else {
    prepararEvento(eModo::OFF, direcao);
  };
}


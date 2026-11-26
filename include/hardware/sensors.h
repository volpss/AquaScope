#pragma once

#include "models.h"

struct AtualizacaoSensores
{
  bool temperatura = false;
  bool ph = false;
  bool tds = false;
};

void iniciarSensores();
AtualizacaoSensores atualizarSensores(unsigned long agoraMs);
bool medicaoTdsEmAndamento();
unsigned long obterIdadeLeituraTdsMs(unsigned long agoraMs);
unsigned long obterTempoAteProximaLeituraTdsMs(unsigned long agoraMs);
const LeiturasSensores &obterLeituras();

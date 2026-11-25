#pragma once

#include "models.h"

void iniciarSensores();
// Retorna true quando temperatura/pH ou TDS foram atualizados.
bool atualizarSensores(unsigned long agoraMs);
bool medicaoTdsEmAndamento();
const LeiturasSensores &obterLeituras();
void publicarLeiturasSensores(bool wifiConectado, uint16_t quantidadeFilaOffline);

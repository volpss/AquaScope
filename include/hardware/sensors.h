#pragma once

#include "models.h"

// Leitura, conversão e classificação dos sensores; rede e fila entram apenas no log.
void lerEProcessarSensores(bool wifiConectado, uint16_t quantidadeFilaOffline);
const LeiturasSensores &obterLeituras();

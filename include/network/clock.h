#pragma once

#include <Arduino.h>

// Sincronização NTP e relógio mantido por millis() durante quedas de rede.
void sincronizarRelogioNtp();
uint32_t obterEpochAtual();
String converterEpochParaIso8601(uint32_t epoch);

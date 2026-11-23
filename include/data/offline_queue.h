#pragma once

#include "models.h"

// Fila circular em RAM: sobrescreve a leitura mais antiga quando cheia.
void salvarNaFilaOffline(const LeituraOffline &item);
uint16_t obterQuantidadeFilaOffline();
// O índice deve ser menor que obterQuantidadeFilaOffline().
const LeituraOffline &obterLeituraFilaOffline(uint16_t indice);
// Remove somente registros cujo envio foi confirmado pelo banco.
void confirmarEnvioFilaOffline(uint16_t quantidade);

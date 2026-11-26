#pragma once

#include "models.h"

// Fila circular em RAM: sobrescreve a medição mais antiga quando cheia.
void enfileirarMedicao(const MedicaoPendente &medicao);
uint16_t obterQuantidadeFilaOffline();
// O índice deve ser menor que obterQuantidadeFilaOffline().
const MedicaoPendente &obterMedicaoFilaOffline(uint16_t indice);
// Remove somente registros cujo envio foi confirmado pelo banco.
void confirmarEnvioFilaOffline(uint16_t quantidade);

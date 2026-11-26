#include "data/offline_queue.h"
#include "config.h"

namespace
{
MedicaoPendente filaOffline[CAPACIDADE_FILA_OFFLINE];
uint16_t inicioFilaOffline = 0;
uint16_t quantidadeFilaOffline = 0;
} // namespace

void enfileirarMedicao(const MedicaoPendente &medicao)
{
  uint16_t posicao;
  if (quantidadeFilaOffline >= CAPACIDADE_FILA_OFFLINE)
  {
    // Buffer cheio: descarta o registro mais antigo e grava o novo
    posicao = inicioFilaOffline;
    inicioFilaOffline = (inicioFilaOffline + 1) % CAPACIDADE_FILA_OFFLINE;
  }
  else
  {
    posicao = (inicioFilaOffline + quantidadeFilaOffline) % CAPACIDADE_FILA_OFFLINE;
    quantidadeFilaOffline++;
  }

  filaOffline[posicao] = medicao;
}

uint16_t obterQuantidadeFilaOffline()
{
  return quantidadeFilaOffline;
}

const MedicaoPendente &obterMedicaoFilaOffline(uint16_t indice)
{
  return filaOffline[(inicioFilaOffline + indice) % CAPACIDADE_FILA_OFFLINE];
}

void confirmarEnvioFilaOffline(uint16_t quantidade)
{
  if (quantidade > quantidadeFilaOffline)
    return;

  inicioFilaOffline = (inicioFilaOffline + quantidade) % CAPACIDADE_FILA_OFFLINE;
  quantidadeFilaOffline -= quantidade;
  if (quantidadeFilaOffline == 0)
    inicioFilaOffline = 0;
}

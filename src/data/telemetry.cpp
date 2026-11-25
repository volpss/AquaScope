#include "data/telemetry.h"
#include "network/clock.h"
#include "config.h"
#include "network/connectivity.h"
#include "data/database.h"
#include "data/offline_queue.h"
#include "hardware/sensors.h"

void enviarOuArmazenarLeitura()
{
  const LeiturasSensores &leituras = obterLeituras();
  if (leituras.estadoTemperatura == SENSOR_SEM_LEITURA ||
      leituras.estadoPh == SENSOR_SEM_LEITURA ||
      leituras.estadoTds == SENSOR_SEM_LEITURA)
  {
    return;
  }

  sincronizarRelogioNtp();

  LeituraOffline atual;
  atual.temperatura = leituras.temperatura;
  atual.ph = leituras.ph;
  atual.tds = leituras.tds;
  atual.epochSegundos = obterEpochAtual();
  atual.alertaAtivo = (leituras.estadoTemperatura != SENSOR_NORMAL || leituras.estadoPh != SENSOR_NORMAL || leituras.estadoTds != SENSOR_NORMAL) ? 1 : 0;
  atual.reservado[0] = atual.reservado[1] = atual.reservado[2] = 0;

  // Sem Wi-Fi, guarda na fila; uma falha de envio também será armazenada.
  if (!wifiEstaConectado())
  {
    salvarNaFilaOffline(atual);
    return;
  }

  // Online: envia a leitura ATUAL imediatamente para o dashboard, mesmo com fila antiga pendente
  String json = montarLeituraJson(atual);
  if (!enviarJsonSupabase(json))
  {
    salvarNaFilaOffline(atual);
  }
}

void sincronizarFilaOffline()
{
  const uint16_t quantidadeFilaOffline = obterQuantidadeFilaOffline();
  if (!wifiEstaConectado() || quantidadeFilaOffline == 0)
    return;

  int quantidadeLote = (quantidadeFilaOffline < TAMANHO_LOTE_ENVIO) ? quantidadeFilaOffline : TAMANHO_LOTE_ENVIO;
  String loteJson;
  loteJson.reserve(quantidadeLote * 90); // Evita fragmentação de memória ao montar lotes grandes
  loteJson = "[";

  for (int i = 0; i < quantidadeLote; i++)
  {
    if (i > 0)
      loteJson += ",";
    loteJson += montarLeituraJson(obterLeituraFilaOffline(i));
  }
  loteJson += "]";

  Serial.print(">> [SYNC] Enviando lote de ");
  Serial.print(quantidadeLote);
  Serial.println(" registros da RAM...");

  if (enviarJsonSupabase(loteJson))
  {
    confirmarEnvioFilaOffline(quantidadeLote);

    Serial.print(">> [SYNC] Lote confirmado! Restantes na RAM: ");
    Serial.println(obterQuantidadeFilaOffline());
  }
}

#include "data/telemetry.h"
#include "config.h"
#include "data/database.h"
#include "data/offline_queue.h"
#include "network/clock.h"
#include "network/connectivity.h"

namespace
{
unsigned long ultimaTentativaEnvioMs = 0;
bool tentouEnviar = false;

void registrarMedicao(TipoMedicao tipo, float valor, EstadoSensor estado, uint32_t epochSegundos)
{
  if (estado != SENSOR_SEM_LEITURA)
    enfileirarMedicao({valor, epochSegundos, tipo, estado != SENSOR_NORMAL});
}
} // namespace

void registrarMedicoes(AtualizacaoSensores atualizacao)
{
  const LeiturasSensores &leituras = obterLeituras();
  const uint32_t epochSegundos = obterEpochAtual();

  if (atualizacao.temperatura)
    registrarMedicao(TipoMedicao::Temperatura, leituras.temperatura, leituras.estadoTemperatura, epochSegundos);
  if (atualizacao.ph)
    registrarMedicao(TipoMedicao::Ph, leituras.ph, leituras.estadoPh, epochSegundos);
  if (atualizacao.tds)
    registrarMedicao(TipoMedicao::Tds, leituras.tds, leituras.estadoTds, epochSegundos);
}

void enviarMedicaoPendente()
{
  const unsigned long agoraMs = millis();
  if (!wifiEstaConectado() || obterQuantidadeFilaOffline() == 0 ||
      (tentouEnviar && agoraMs - ultimaTentativaEnvioMs < INTERVALO_SINCRONIZACAO_FILA_MS))
    return;

  const MedicaoPendente &medicao = obterMedicaoFilaOffline(0);
  if (enviarJsonSupabase(montarMedicaoJson(medicao)))
    confirmarEnvioFilaOffline(1);

  // Deixe o loop retomar a amostragem entre requisições, inclusive após falha.
  ultimaTentativaEnvioMs = millis();
  tentouEnviar = true;
}

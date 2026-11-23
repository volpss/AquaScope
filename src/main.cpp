#include <Arduino.h>

#include "config.h"
#include "network/connectivity.h"
#include "hardware/indicators.h"
#include "hardware/led_matrix.h"
#include "data/offline_queue.h"
#include "hardware/sensors.h"
#include "data/telemetry.h"
#include "network/web_server.h"

namespace
{
unsigned long ultimaLeituraSensoresMs = 0;
unsigned long ultimoEnvioNuvemMs = 0;
unsigned long ultimaSincronizacaoFilaMs = 0;
} // namespace

void setup()
{
  Serial.begin(VELOCIDADE_SERIAL);
  iniciarIndicadores();
  iniciarMatrizLed();

  Serial.println("=== Sistema IoT Aquario Iniciado (Buffer RAM) ===");
  iniciarConectividade();
  lerEProcessarSensores(wifiEstaConectado(), obterQuantidadeFilaOffline());
}

void loop()
{
  unsigned long agoraMs = millis();

  gerenciarWifi();

  processarOta();

  atenderNavegador();

  atualizarIndicadores(agoraMs);
  gerenciarAlarme(agoraMs);

  if (agoraMs - ultimaLeituraSensoresMs >= INTERVALO_LEITURA_SENSORES_MS)
  {
    ultimaLeituraSensoresMs = agoraMs;
    lerEProcessarSensores(wifiEstaConectado(), obterQuantidadeFilaOffline());
  }

  // A cada 15s: envia a leitura atual direto se online, ou guarda na RAM se offline
  if (agoraMs - ultimoEnvioNuvemMs >= INTERVALO_ENVIO_NUVEM_MS)
  {
    ultimoEnvioNuvemMs = agoraMs;
    enviarOuArmazenarLeitura();
  }

  // Se reconectou e há registros guardados na RAM, descarrega lotes de 50 a cada 1s
  if (wifiEstaConectado() && obterQuantidadeFilaOffline() > 0 && (agoraMs - ultimaSincronizacaoFilaMs >= INTERVALO_SINCRONIZACAO_FILA_MS))
  {
    ultimaSincronizacaoFilaMs = agoraMs;
    sincronizarFilaOffline();
  }
}

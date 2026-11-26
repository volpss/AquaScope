#include <Arduino.h>

#include "config.h"
#include "data/telemetry.h"
#include "hardware/indicators.h"
#include "hardware/led_matrix.h"
#include "hardware/sensors.h"
#include "network/connectivity.h"
#include "network/web_server.h"

namespace
{
  bool primeiraLeituraConcluida = false;
} // namespace

void setup()
{
  Serial.begin(VELOCIDADE_SERIAL);
  iniciarSensores();
  iniciarIndicadores();
  iniciarMatrizLed();

  Serial.println("=== Sistema IoT Aquario Iniciado (Buffer RAM) ===");
  iniciarConectividade();
}

void loop()
{
  const unsigned long agoraMs = millis();

  const AtualizacaoSensores atualizacao = atualizarSensores(agoraMs);
  const bool leiturasAtualizadas = atualizacao.temperatura || atualizacao.ph || atualizacao.tds;
  if (leiturasAtualizadas)
  {
    registrarMedicoes(atualizacao);
    primeiraLeituraConcluida = true;
  }

  // Wi-Fi, HTTP e telemetria podem bloquear por mais que o pulso de TDS.
  if (medicaoTdsEmAndamento())
    return;

  gerenciarWifi();
  processarOta();
  atenderNavegador();

  if (primeiraLeituraConcluida)
  {
    atualizarIndicadores(agoraMs);
    gerenciarAlarme(agoraMs);
  }

  enviarMedicaoPendente();
}

#include "hardware/indicators.h"
#include <Arduino.h>
#include "config.h"
#include "hardware/sensors.h"

namespace
{
unsigned long ultimaAlternanciaLentaMs = 0;
unsigned long ultimaAlternanciaRapidaMs = 0;
bool nivelPiscaLento = false;
bool nivelPiscaRapido = false;
unsigned long fimSilencioMs = 0;
unsigned long ultimoCliqueBotaoMs = 0;
bool buzzerSilenciado = false;
bool estadoBotaoAnterior = HIGH;

void atualizarLed(int pinoLed, EstadoSensor estado)
{
  if (estado == SENSOR_SEM_LEITURA)
    digitalWrite(pinoLed, LOW);
  else if (estado == SENSOR_NORMAL)
    digitalWrite(pinoLed, HIGH);
  else if (estado == SENSOR_ABAIXO_LIMITE)
    digitalWrite(pinoLed, nivelPiscaLento);
  else if (estado == SENSOR_ACIMA_LIMITE)
    digitalWrite(pinoLed, nivelPiscaRapido);
}
} // namespace

void iniciarIndicadores()
{
  pinMode(PINO_LED_TEMPERATURA, OUTPUT);
  pinMode(PINO_LED_PH, OUTPUT);
  pinMode(PINO_LED_TDS, OUTPUT);
  pinMode(PINO_BUZZER, OUTPUT);
  pinMode(PINO_BOTAO, INPUT_PULLUP);
}

void atualizarIndicadores(unsigned long agoraMs)
{
  const LeiturasSensores &leituras = obterLeituras();
  if (agoraMs - ultimaAlternanciaLentaMs >= INTERVALO_PISCA_LENTO_MS)
  {
    ultimaAlternanciaLentaMs = agoraMs;
    nivelPiscaLento = !nivelPiscaLento;
  }

  if (agoraMs - ultimaAlternanciaRapidaMs >= INTERVALO_PISCA_RAPIDO_MS)
  {
    ultimaAlternanciaRapidaMs = agoraMs;
    nivelPiscaRapido = !nivelPiscaRapido;
  }

  atualizarLed(PINO_LED_TEMPERATURA, leituras.estadoTemperatura);
  atualizarLed(PINO_LED_PH, leituras.estadoPh);
  atualizarLed(PINO_LED_TDS, leituras.estadoTds);
}

void gerenciarAlarme(unsigned long agoraMs)
{
  const LeiturasSensores &leituras = obterLeituras();
  bool algumAlerta = (leituras.estadoTemperatura != SENSOR_NORMAL || leituras.estadoPh != SENSOR_NORMAL || leituras.estadoTds != SENSOR_NORMAL);

  bool estadoBotaoAtual = digitalRead(PINO_BOTAO);
  if (estadoBotaoAtual == LOW && estadoBotaoAnterior == HIGH && (agoraMs - ultimoCliqueBotaoMs >= INTERVALO_DEBOUNCE_BOTAO_MS))
  {
    ultimoCliqueBotaoMs = agoraMs;
    if (algumAlerta)
    {
      if (!buzzerSilenciado)
      {
        fimSilencioMs = agoraMs + INCREMENTO_SILENCIO_MS;
        buzzerSilenciado = true;
      }
      else
      {
        fimSilencioMs += INCREMENTO_SILENCIO_MS;
      }
      Serial.print(">> +15s de silencio! Restante: ");
      Serial.print((fimSilencioMs - agoraMs + 500) / 1000);
      Serial.println("s.");
    }
  }
  estadoBotaoAnterior = estadoBotaoAtual;

  if (buzzerSilenciado && (long)(fimSilencioMs - agoraMs) <= 0)
  {
    buzzerSilenciado = false;
  }
  if (!algumAlerta)
  {
    buzzerSilenciado = false;
  }

  if (algumAlerta && !buzzerSilenciado)
  {
    digitalWrite(PINO_BUZZER, nivelPiscaLento);
  }
  else
  {
    digitalWrite(PINO_BUZZER, LOW);
  }
}

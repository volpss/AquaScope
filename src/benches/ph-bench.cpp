#include <Arduino.h>
#include <algorithm>

#include "config.h"

namespace
{
constexpr unsigned long ESPERA_INICIAL_MS = 1000UL;
constexpr unsigned long INTERVALO_EXIBICAO_MS = 1000UL;
} // namespace

void setup()
{
  Serial.begin(VELOCIDADE_SERIAL);
  analogReadResolution(RESOLUCAO_ADC);
  delay(ESPERA_INICIAL_MS);

  Serial.println("Leitura de tensao do modulo de pH em A1");
}

void loop()
{
  int amostras[TOTAL_AMOSTRAS];

  for (int &amostra : amostras)
  {
    amostra = analogRead(PINO_PH);
    delay(INTERVALO_AMOSTRA_MS);
  }

  std::sort(amostras, amostras + TOTAL_AMOSTRAS);
  long soma = 0;
  for (int i = DESCARTE_POR_EXTREMO; i < TOTAL_AMOSTRAS - DESCARTE_POR_EXTREMO; ++i)
    soma += amostras[i];

  const float mediaAdc = soma / static_cast<double>(AMOSTRAS_CENTRAIS);
  const float tensao = mediaAdc * (static_cast<double>(TENSAO_REFERENCIA_ADC) / ADC_MAXIMO);

  Serial.print("ADC medio: ");
  Serial.print(mediaAdc, 1);
  Serial.print(" | Tensao: ");
  Serial.print(tensao, 3);
  Serial.println(" V");

  delay(INTERVALO_EXIBICAO_MS);
}

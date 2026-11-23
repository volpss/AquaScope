#include "hardware/sensors.h"
#include <Arduino.h>
#include "config.h"

namespace
{
  LeiturasSensores leituras;

  float lerTermistor(int pino, float resistorFixo)
  {
    float somaLeituras = 0;
    for (int i = 0; i < 10; i++)
    {
      somaLeituras += analogRead(pino);
      delay(10);
    }
    float mediaAdc = somaLeituras / 10.0;
    if (mediaAdc <= 0 || mediaAdc >= 1023)
      return LEITURA_INVALIDA;

    float resistencia = resistorFixo * (mediaAdc / (1023.0 - mediaAdc));
    float steinhart = resistencia / RESISTENCIA_NOMINAL_TERMISTOR;
    steinhart = log(steinhart);
    steinhart /= COEFICIENTE_BETA;
    steinhart += 1.0 / (TEMPERATURA_NOMINAL + 273.15);
    steinhart = 1.0 / steinhart;
    return steinhart - 273.15;
  }
} // namespace

void lerEProcessarSensores(bool wifiConectado, uint16_t quantidadeFilaOffline)
{
  leituras.temperatura = lerTermistor(PINO_TEMPERATURA, RESISTOR_TEMPERATURA);
  // A1 e A2 usam temperaturas como base para simular pH e TDS.
  leituras.temperaturaBasePh = lerTermistor(PINO_PH, RESISTOR_PH);
  leituras.temperaturaBaseTds = lerTermistor(PINO_TDS, RESISTOR_TDS);

  if (wifiConectado)
  {
    Serial.print("[ONLINE] ");
  }
  else
  {
    Serial.print("[OFFLINE - RAM: ");
    Serial.print(quantidadeFilaOffline);
    Serial.print("] ");
  }

  Serial.print("Temp: ");
  if (leituras.temperatura == LEITURA_INVALIDA)
  {
    leituras.estadoTemperatura = SENSOR_SEM_LEITURA;
    Serial.print("ERRO");
  }
  else
  {
    Serial.print(leituras.temperatura, 1);
    Serial.print(" °C [");
    if (leituras.temperatura < TEMPERATURA_MINIMA)
    {
      leituras.estadoTemperatura = SENSOR_ABAIXO_LIMITE;
      Serial.print("BAIXA]");
    }
    else if (leituras.temperatura > TEMPERATURA_MAXIMA)
    {
      leituras.estadoTemperatura = SENSOR_ACIMA_LIMITE;
      Serial.print("ALTA]");
    }
    else
    {
      leituras.estadoTemperatura = SENSOR_NORMAL;
      Serial.print("OK]");
    }
  }

  Serial.print(" | pH: ");
  if (leituras.temperaturaBasePh == LEITURA_INVALIDA)
  {
    leituras.estadoPh = SENSOR_SEM_LEITURA;
    Serial.print("ERRO");
  }
  else
  {
    leituras.ph = constrain(6.0 + ((leituras.temperaturaBasePh - 20.0) * 2.5 / 15.0), 0.0, 14.0);
    Serial.print(leituras.ph, 2);
    Serial.print(" [");
    if (leituras.ph < PH_MINIMO)
    {
      leituras.estadoPh = SENSOR_ABAIXO_LIMITE;
      Serial.print("ACIDO]");
    }
    else if (leituras.ph > PH_MAXIMO)
    {
      leituras.estadoPh = SENSOR_ACIMA_LIMITE;
      Serial.print("ALCALINO]");
    }
    else
    {
      leituras.estadoPh = SENSOR_NORMAL;
      Serial.print("OK]");
    }
  }

  Serial.print(" | TDS: ");
  if (leituras.temperaturaBaseTds == LEITURA_INVALIDA)
  {
    leituras.estadoTds = SENSOR_SEM_LEITURA;
    Serial.println("ERRO");
  }
  else
  {
    leituras.tds = constrain(50.0 + ((leituras.temperaturaBaseTds - 20.0) * 450.0 / 15.0), 0.0, 1000.0);
    Serial.print(leituras.tds, 0);
    Serial.print(" ppm [");
    if (leituras.tds < TDS_MINIMO)
    {
      leituras.estadoTds = SENSOR_ABAIXO_LIMITE;
      Serial.println("BAIXO]");
    }
    else if (leituras.tds > TDS_MAXIMO)
    {
      leituras.estadoTds = SENSOR_ACIMA_LIMITE;
      Serial.println("ALTO]");
    }
    else
    {
      leituras.estadoTds = SENSOR_NORMAL;
      Serial.println("OK]");
    }
  }
}

const LeiturasSensores &obterLeituras()
{
  return leituras;
}

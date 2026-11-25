#include "hardware/sensors.h"
#include <algorithm>
#include <Arduino.h>
#include "config.h"

namespace
{
  constexpr int ADC_MAXIMO = (1 << RESOLUCAO_ADC) - 1;
  constexpr int TOTAL_AMOSTRAS = 30;
  constexpr int DESCARTE_POR_EXTREMO = 5;
  constexpr unsigned long INTERVALO_AMOSTRA_MS = 10;
  static_assert(TEMPO_ESTABILIZACAO_TDS_MS + (TOTAL_AMOSTRAS - 1) * INTERVALO_AMOSTRA_MS <
                    DURACAO_ALIMENTACAO_TDS_MS,
                "As amostras de TDS precisam caber no pulso de alimentacao");

  LeiturasSensores leituras;
  int amostrasPh[TOTAL_AMOSTRAS];
  int amostrasTds[TOTAL_AMOSTRAS];
  int somaAdcTemperatura = 0;
  int totalColetado = 0;
  unsigned long inicioJanelaMs = 0;
  unsigned long ultimaAmostraMs = 0;
  bool iniciouCicloTds = false;
  bool tdsEnergizado = false;
  int totalAmostrasTds = 0;
  unsigned long inicioCicloTdsMs = 0;
  unsigned long ultimaAmostraTdsMs = 0;

  float calcularTemperatura(float mediaAdc)
  {
    if (mediaAdc <= 0 || mediaAdc >= ADC_MAXIMO)
      return LEITURA_INVALIDA;

    const float resistencia = RESISTOR_TEMPERATURA * mediaAdc / (ADC_MAXIMO - mediaAdc);
    const float inversoKelvin = logf(resistencia / RESISTENCIA_NOMINAL_TERMISTOR) / COEFICIENTE_BETA +
                                1.0f / (TEMPERATURA_NOMINAL + 273.15f);
    return 1.0f / inversoKelvin - 273.15f;
  }

  float calcularTensaoFiltrada(int (&amostras)[TOTAL_AMOSTRAS])
  {
    std::sort(amostras, amostras + TOTAL_AMOSTRAS);
    int soma = 0;
    for (int i = DESCARTE_POR_EXTREMO; i < TOTAL_AMOSTRAS - DESCARTE_POR_EXTREMO; ++i)
      soma += amostras[i];

    constexpr int AMOSTRAS_CENTRAIS = TOTAL_AMOSTRAS - 2 * DESCARTE_POR_EXTREMO;
    const float mediaAdc = static_cast<float>(soma) / AMOSTRAS_CENTRAIS;
    return mediaAdc * TENSAO_REFERENCIA_ADC / ADC_MAXIMO;
  }

  float calcularPh(float tensao)
  {
    const float ph = 7.0f + INCLINACAO_PH * (tensao - TENSAO_PH_NEUTRO);
    if (ph <= 0.5f || ph >= 13.5f)
      return LEITURA_INVALIDA;

    return ph;
  }

  float calcularTds(float tensao, float temperatura)
  {
    if (temperatura == LEITURA_INVALIDA || tensao > TENSAO_MAXIMA_TDS)
      return LEITURA_INVALIDA;

    const float compensacao = 1.0f + 0.02f * (temperatura - 25.0f);
    if (compensacao <= 0.0f)
      return LEITURA_INVALIDA;

    const float v = tensao / compensacao;
    // Curva do módulo analógico Gravity TDS, em ppm a 25 °C.
    return FATOR_CALIBRACAO_TDS * 0.5f * (133.42f * v * v * v - 255.86f * v * v + 857.39f * v);
  }

  EstadoSensor classificarLeitura(float valor, float minimo, float maximo)
  {
    if (valor == LEITURA_INVALIDA)
      return SENSOR_SEM_LEITURA;
    if (valor < minimo)
      return SENSOR_ABAIXO_LIMITE;
    if (valor > maximo)
      return SENSOR_ACIMA_LIMITE;
    return SENSOR_NORMAL;
  }

  void desligarAlimentacaoTds()
  {
    digitalWrite(PINO_ALIMENTACAO_TDS, HIGH);
  }

  void iniciarCicloTds()
  {
    iniciouCicloTds = true;
    tdsEnergizado = true;
    totalAmostrasTds = 0;
    // Descarte a janela parcialmente coletada para ler pH com o TDS desligado.
    totalColetado = 0;
    somaAdcTemperatura = 0;
    digitalWrite(PINO_ALIMENTACAO_TDS, LOW);
    inicioCicloTdsMs = millis();
  }

  bool atualizarCicloTds(unsigned long agoraMs)
  {
    const unsigned long decorridoMs = agoraMs - inicioCicloTdsMs;
    if (decorridoMs >= DURACAO_ALIMENTACAO_TDS_MS)
    {
      desligarAlimentacaoTds();
      tdsEnergizado = false;
      inicioJanelaMs = agoraMs - INTERVALO_LEITURA_SENSORES_MS;
      leituras.tds = totalAmostrasTds == TOTAL_AMOSTRAS
                         ? calcularTds(calcularTensaoFiltrada(amostrasTds), leituras.temperatura)
                         : LEITURA_INVALIDA;
      leituras.estadoTds = classificarLeitura(leituras.tds, TDS_MINIMO, TDS_MAXIMO);
      Serial.println(">> TDS: ciclo concluido");
      return true;
    }

    if (decorridoMs >= TEMPO_ESTABILIZACAO_TDS_MS && totalAmostrasTds < TOTAL_AMOSTRAS &&
        (totalAmostrasTds == 0 || agoraMs - ultimaAmostraTdsMs >= INTERVALO_AMOSTRA_MS))
    {
      ultimaAmostraTdsMs = agoraMs;
      amostrasTds[totalAmostrasTds++] = analogRead(PINO_TDS);
    }

    return false;
  }

  void publicarLeitura(float valor, EstadoSensor estado, int casasDecimais,
                       const char *unidade, const char *abaixo, const char *acima)
  {
    if (estado == SENSOR_SEM_LEITURA)
    {
      Serial.print("ERRO");
      return;
    }

    Serial.print(valor, casasDecimais);
    Serial.print(unidade);
    Serial.print(" [");
    if (estado == SENSOR_ABAIXO_LIMITE)
      Serial.print(abaixo);
    else if (estado == SENSOR_ACIMA_LIMITE)
      Serial.print(acima);
    else
      Serial.print("OK");
    Serial.print("]");
  }
} // namespace

void iniciarSensores()
{
  // No Uno R4, configure direção e nível juntos para evitar um LOW na inicialização.
  R_IOPORT_PinCfg(nullptr, digitalPinToBspPin(PINO_ALIMENTACAO_TDS),
                 IOPORT_CFG_PORT_DIRECTION_OUTPUT | IOPORT_CFG_PORT_OUTPUT_HIGH);
  analogReadResolution(RESOLUCAO_ADC);
  // A primeira janela começa na primeira passagem pelo loop.
  inicioJanelaMs = millis() - INTERVALO_LEITURA_SENSORES_MS;
}

bool atualizarSensores(unsigned long agoraMs)
{
  if (tdsEnergizado)
    return atualizarCicloTds(agoraMs);

  if (iniciouCicloTds && agoraMs - inicioCicloTdsMs >= INTERVALO_MEDICAO_TDS_MS)
  {
    iniciarCicloTds();
    return false;
  }

  if (totalColetado == 0)
  {
    if (agoraMs - inicioJanelaMs < INTERVALO_LEITURA_SENSORES_MS)
      return false;
    inicioJanelaMs = agoraMs;
  }
  else if (agoraMs - ultimaAmostraMs < INTERVALO_AMOSTRA_MS)
  {
    return false;
  }

  ultimaAmostraMs = agoraMs;
  somaAdcTemperatura += analogRead(PINO_TEMPERATURA);
  amostrasPh[totalColetado] = analogRead(PINO_PH);
  ++totalColetado;
  if (totalColetado < TOTAL_AMOSTRAS)
    return false;

  leituras.temperatura = calcularTemperatura(static_cast<float>(somaAdcTemperatura) / TOTAL_AMOSTRAS);
  leituras.ph = calcularPh(calcularTensaoFiltrada(amostrasPh));
  leituras.estadoTemperatura = classificarLeitura(leituras.temperatura, TEMPERATURA_MINIMA, TEMPERATURA_MAXIMA);
  leituras.estadoPh = classificarLeitura(leituras.ph, PH_MINIMO, PH_MAXIMO);

  totalColetado = 0;
  somaAdcTemperatura = 0;
  if (!iniciouCicloTds)
    iniciarCicloTds();
  return true;
}

bool medicaoTdsEmAndamento()
{
  return tdsEnergizado;
}

const LeiturasSensores &obterLeituras()
{
  return leituras;
}

void publicarLeiturasSensores(bool wifiConectado, uint16_t quantidadeFilaOffline)
{
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
  publicarLeitura(leituras.temperatura, leituras.estadoTemperatura, 1, " °C", "BAIXA", "ALTA");
  Serial.print(" | pH: ");
  publicarLeitura(leituras.ph, leituras.estadoPh, 2, "", "ACIDO", "ALCALINO");
  Serial.print(" | TDS: ");
  publicarLeitura(leituras.tds, leituras.estadoTds, 0, " ppm", "BAIXO", "ALTO");
  Serial.println();
}

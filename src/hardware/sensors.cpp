#include "hardware/sensors.h"
#include <algorithm>
#include <Arduino.h>
#include "config.h"

namespace
{
  static_assert(TEMPO_ESTABILIZACAO_TDS_MS + (TOTAL_AMOSTRAS - 1) * INTERVALO_AMOSTRA_MS <
                    DURACAO_ALIMENTACAO_TDS_MS,
                "As amostras de TDS precisam caber no pulso de alimentacao");

  struct JanelaAmostragem
  {
    unsigned long intervaloMs;
    unsigned long inicioMs = 0;
    unsigned long ultimaAmostraMs = 0;
    int total = 0;
  };

  LeiturasSensores leituras;

  JanelaAmostragem janelaTemperatura{INTERVALO_MEDICAO_TEMPERATURA_MS};
  int somaAdcTemperatura = 0;

  JanelaAmostragem janelaPh{INTERVALO_MEDICAO_PH_MS};
  int amostrasPh[TOTAL_AMOSTRAS];

  int amostrasTds[TOTAL_AMOSTRAS];
  bool iniciouCicloTds = false;
  bool tdsEnergizado = false;
  int totalAmostrasTds = 0;
  unsigned long inicioCicloTdsMs = 0;
  unsigned long ultimaLeituraTdsMs = 0;
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

  void publicarMedicao(const char *sensor, float valor, EstadoSensor estado, int casasDecimais,
                       const char *unidade, const char *abaixo, const char *acima)
  {
    Serial.print("[MEDICAO] ");
    Serial.print(sensor);
    Serial.print(": ");
    if (estado == SENSOR_SEM_LEITURA)
    {
      Serial.println("ERRO");
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
    Serial.println("]");
  }

  void desligarAlimentacaoTds()
  {
    digitalWrite(PINO_ALIMENTACAO_TDS, HIGH);
  }

  void reiniciarJanelaInterrompida(JanelaAmostragem &janela, unsigned long agoraMs)
  {
    if (janela.total == 0)
      return;

    janela.total = 0;
    janela.inicioMs = agoraMs - janela.intervaloMs;
  }

  void iniciarCicloTds()
  {
    iniciouCicloTds = true;
    tdsEnergizado = true;
    totalAmostrasTds = 0;
    // Descarte as amostras parciais e retome as janelas depois do pulso.
    const unsigned long agoraMs = millis();
    reiniciarJanelaInterrompida(janelaTemperatura, agoraMs);
    reiniciarJanelaInterrompida(janelaPh, agoraMs);
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
      leituras.tds = totalAmostrasTds == TOTAL_AMOSTRAS
                         ? calcularTds(calcularTensaoFiltrada(amostrasTds), leituras.temperatura)
                         : LEITURA_INVALIDA;
      leituras.estadoTds = classificarLeitura(leituras.tds, TDS_MINIMO, TDS_MAXIMO);
      ultimaLeituraTdsMs = agoraMs;
      publicarMedicao("TDS", leituras.tds, leituras.estadoTds, 0, " ppm", "BAIXO", "ALTO");
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

  bool prepararAmostra(JanelaAmostragem &janela, unsigned long agoraMs)
  {
    if (janela.total == 0)
    {
      if (agoraMs - janela.inicioMs < janela.intervaloMs)
        return false;
      janela.inicioMs = agoraMs;
    }
    else if (agoraMs - janela.ultimaAmostraMs < INTERVALO_AMOSTRA_MS)
    {
      return false;
    }

    janela.ultimaAmostraMs = agoraMs;
    return true;
  }

  bool atualizarTemperatura(unsigned long agoraMs)
  {
    if (!prepararAmostra(janelaTemperatura, agoraMs))
      return false;

    somaAdcTemperatura += analogRead(PINO_TEMPERATURA);
    if (++janelaTemperatura.total < TOTAL_AMOSTRAS)
      return false;

    leituras.temperatura = calcularTemperatura(static_cast<float>(somaAdcTemperatura) / TOTAL_AMOSTRAS);
    leituras.estadoTemperatura = classificarLeitura(leituras.temperatura, TEMPERATURA_MINIMA, TEMPERATURA_MAXIMA);
    janelaTemperatura.total = 0;
    somaAdcTemperatura = 0;
    return true;
  }

  bool atualizarPh(unsigned long agoraMs)
  {
    if (!prepararAmostra(janelaPh, agoraMs))
      return false;

    amostrasPh[janelaPh.total++] = analogRead(PINO_PH);
    if (janelaPh.total < TOTAL_AMOSTRAS)
      return false;

    leituras.ph = calcularPh(calcularTensaoFiltrada(amostrasPh));
    leituras.estadoPh = classificarLeitura(leituras.ph, PH_MINIMO, PH_MAXIMO);
    janelaPh.total = 0;
    return true;
  }
} // namespace

void iniciarSensores()
{
  // No Uno R4, configure direção e nível juntos para evitar um LOW na inicialização.
  R_IOPORT_PinCfg(nullptr, digitalPinToBspPin(PINO_ALIMENTACAO_TDS),
                 IOPORT_CFG_PORT_DIRECTION_OUTPUT | IOPORT_CFG_PORT_OUTPUT_HIGH);
  analogReadResolution(RESOLUCAO_ADC);
  // As primeiras janelas começam na primeira passagem pelo loop.
  const unsigned long agoraMs = millis();
  janelaTemperatura.inicioMs = agoraMs - janelaTemperatura.intervaloMs;
  janelaPh.inicioMs = agoraMs - janelaPh.intervaloMs;
}

AtualizacaoSensores atualizarSensores(unsigned long agoraMs)
{
  if (tdsEnergizado)
    return {false, false, atualizarCicloTds(agoraMs)};

  if (iniciouCicloTds && agoraMs - inicioCicloTdsMs >= INTERVALO_MEDICAO_TDS_MS)
  {
    iniciarCicloTds();
    return {};
  }

  const bool temperaturaAtualizada = atualizarTemperatura(agoraMs);
  const bool phAtualizado = atualizarPh(agoraMs);
  if (temperaturaAtualizada)
    publicarMedicao("Temperatura", leituras.temperatura, leituras.estadoTemperatura, 1, " °C", "BAIXA", "ALTA");
  if (phAtualizado)
    publicarMedicao("pH", leituras.ph, leituras.estadoPh, 2, "", "ACIDO", "ALCALINO");

  if (!iniciouCicloTds && temperaturaAtualizada)
    iniciarCicloTds();
  return {temperaturaAtualizada, phAtualizado, false};
}

bool medicaoTdsEmAndamento()
{
  return tdsEnergizado;
}

unsigned long obterIdadeLeituraTdsMs(unsigned long agoraMs)
{
  return agoraMs - ultimaLeituraTdsMs;
}

unsigned long obterTempoAteProximaLeituraTdsMs(unsigned long agoraMs)
{
  if (!iniciouCicloTds)
    return DURACAO_ALIMENTACAO_TDS_MS;

  const unsigned long decorridoMs = agoraMs - inicioCicloTdsMs;
  if (tdsEnergizado)
    return decorridoMs < DURACAO_ALIMENTACAO_TDS_MS ? DURACAO_ALIMENTACAO_TDS_MS - decorridoMs : 0;

  const unsigned long ateInicioMs = decorridoMs < INTERVALO_MEDICAO_TDS_MS
                                        ? INTERVALO_MEDICAO_TDS_MS - decorridoMs : 0;
  return ateInicioMs + DURACAO_ALIMENTACAO_TDS_MS;
}

const LeiturasSensores &obterLeituras()
{
  return leituras;
}

#pragma once

#include <stdint.h>

enum EstadoSensor
{
  SENSOR_SEM_LEITURA = 0,
  SENSOR_NORMAL = 1,
  SENSOR_ABAIXO_LIMITE = 2,
  SENSOR_ACIMA_LIMITE = 3
};

struct LeiturasSensores
{
  float temperatura = 0.0;
  float ph = 0.0;
  float tds = 0.0;
  EstadoSensor estadoTemperatura = SENSOR_SEM_LEITURA;
  EstadoSensor estadoPh = SENSOR_SEM_LEITURA;
  EstadoSensor estadoTds = SENSOR_SEM_LEITURA;
};

enum class TipoMedicao : uint8_t
{
  Temperatura,
  Ph,
  Tds
};

struct MedicaoPendente
{
  float valor;
  uint32_t epochSegundos;
  TipoMedicao tipo;
  bool alertaAtivo;
};

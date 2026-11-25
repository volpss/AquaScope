#pragma once

#include <stdint.h>

enum EstadoSensor
{
  SENSOR_SEM_LEITURA = 0,
  SENSOR_NORMAL = 1,
  SENSOR_ABAIXO_LIMITE = 2,
  SENSOR_ACIMA_LIMITE = 3
};

struct LeituraOffline
{
  float temperatura;       // 4 bytes
  float ph;                // 4 bytes
  float tds;               // 4 bytes
  uint32_t epochSegundos;   // 4 bytes (Epoch UTC em segundos)
  uint8_t alertaAtivo;      // 1 byte
  uint8_t reservado[3];     // 3 bytes (alinhamento: total de 20 bytes)
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

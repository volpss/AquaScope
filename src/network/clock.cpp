#include "network/clock.h"
#include <WiFiS3.h>
#include "config.h"
#include "network/connectivity.h"

namespace
{
uint32_t epochBase = 0;
unsigned long relogioBaseMs = 0;
} // namespace

void sincronizarRelogioNtp()
{
  if (!wifiEstaConectado())
    return;
  unsigned long epochRede = WiFi.getTime();
  if (epochRede > EPOCH_MINIMO_VALIDO)
  {
    epochBase = epochRede;
    relogioBaseMs = millis();
  }
}

uint32_t obterEpochAtual()
{
  if (epochBase == 0)
    return 0;
  return epochBase + ((millis() - relogioBaseMs) / 1000UL);
}

String converterEpochParaIso8601(uint32_t epoch)
{
  unsigned long dias = epoch / 86400UL;
  unsigned long segundosDoDia = epoch % 86400UL;
  int hora = segundosDoDia / 3600;
  int minuto = (segundosDoDia % 3600) / 60;
  int segundo = segundosDoDia % 60;

  long diasAjustados = (long)dias + 719468L;
  long era = (diasAjustados >= 0 ? diasAjustados : diasAjustados - 146096L) / 146097L;
  unsigned long diasNaEra = (unsigned long)(diasAjustados - era * 146097L);
  unsigned long anoNaEra = (diasNaEra - diasNaEra / 1460 + diasNaEra / 36524 - diasNaEra / 146096) / 365;
  long ano = (long)anoNaEra + era * 400;
  unsigned long diaNoAno = diasNaEra - (365 * anoNaEra + anoNaEra / 4 - anoNaEra / 100);
  unsigned long mesAjustado = (5 * diaNoAno + 2) / 153;
  int dia = diaNoAno - (153 * mesAjustado + 2) / 5 + 1;
  int mes = mesAjustado + (mesAjustado < 10 ? 3 : -9);
  ano += (mes <= 2);

  char dataIso8601[25];
  snprintf(dataIso8601, sizeof(dataIso8601), "%04ld-%02d-%02dT%02d:%02d:%02dZ", ano, mes, dia, hora, minuto, segundo);
  return String(dataIso8601);
}

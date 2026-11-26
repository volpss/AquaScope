#include "data/database.h"
#include <WiFiS3.h>
#include "config.h"
#include "network/clock.h"
#include "network/connectivity.h"
#include "private_config.h"

namespace
{
WiFiSSLClient clienteSupabase;
} // namespace

String montarMedicaoJson(const MedicaoPendente &medicao)
{
  String medicaoJson;
  switch (medicao.tipo)
  {
    case TipoMedicao::Temperatura:
      medicaoJson = "{\"temperatura\":" + String(medicao.valor, 1);
      break;
    case TipoMedicao::Ph:
      medicaoJson = "{\"ph\":" + String(medicao.valor, 2);
      break;
    case TipoMedicao::Tds:
      medicaoJson = "{\"tds\":" + String(medicao.valor, 0);
      break;
  }
  medicaoJson += ",\"alerta_ativo\":" + String(medicao.alertaAtivo ? "true" : "false");
  if (medicao.epochSegundos > EPOCH_MINIMO_VALIDO)
  {
    medicaoJson += ",\"created_at\":\"" + converterEpochParaIso8601(medicao.epochSegundos) + "\"";
  }
  medicaoJson += "}";
  return medicaoJson;
}

bool enviarJsonSupabase(const String &conteudoJson)
{
  if (!wifiEstaConectado())
    return false;

  if (clienteSupabase.connect(SUPABASE_HOST, 443))
  {
    clienteSupabase.println("POST " + String(SUPABASE_CAMINHO) + " HTTP/1.1");
    clienteSupabase.println("Host: " + String(SUPABASE_HOST));
    clienteSupabase.println("apikey: " + String(SUPABASE_CHAVE));
    clienteSupabase.println("Authorization: Bearer " + String(SUPABASE_CHAVE));
    clienteSupabase.println("Content-Type: application/json");
    clienteSupabase.println("Prefer: return=minimal");
    clienteSupabase.print("Content-Length: ");
    clienteSupabase.println(conteudoJson.length());
    clienteSupabase.println("Connection: close");
    clienteSupabase.println();
    clienteSupabase.print(conteudoJson);

    int codigoHttp = 0;
    unsigned long inicioEsperaMs = millis();
    while (clienteSupabase.connected() && millis() - inicioEsperaMs < TEMPO_LIMITE_SUPABASE_MS)
    {
      if (clienteSupabase.available())
      {
        String resposta = clienteSupabase.readStringUntil('\n');
        resposta.trim();
        if (resposta.startsWith("HTTP/1."))
          codigoHttp = resposta.substring(9, 12).toInt();
        break;
      }
    }
    clienteSupabase.stop();

    Serial.print("[SUPABASE] POST ");
    Serial.print(conteudoJson);
    if (codigoHttp > 0)
    {
      Serial.print(" -> HTTP ");
      Serial.println(codigoHttp);
    }
    else
    {
      Serial.println(" -> sem resposta HTTP valida");
    }
    return codigoHttp >= 200 && codigoHttp < 300;
  }

  Serial.println("[SUPABASE] Falha TLS/rede antes do POST.");
  return false;
}

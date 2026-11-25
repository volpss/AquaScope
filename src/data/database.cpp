#include "data/database.h"
#include <WiFiS3.h>
#include "network/clock.h"
#include "config.h"
#include "network/connectivity.h"
#include "private_config.h"

namespace
{
WiFiSSLClient clienteSupabase;
} // namespace

String montarLeituraJson(const LeituraOffline &item)
{
  String leituraJson = "{";
  leituraJson += "\"temperatura\":" + String(item.temperatura, 1) + ",";
  leituraJson += "\"ph\":" + String(item.ph, 2) + ",";
  leituraJson += "\"tds\":" + String(item.tds, 0) + ",";
  leituraJson += "\"alerta_ativo\":" + String(item.alertaAtivo ? "true" : "false");
  if (item.epochSegundos > EPOCH_MINIMO_VALIDO)
  {
    leituraJson += ",\"created_at\":\"" + converterEpochParaIso8601(item.epochSegundos) + "\"";
  }
  leituraJson += "}";
  return leituraJson;
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
    clienteSupabase.println(conteudoJson);

    bool sucesso = false;
    unsigned long inicioEsperaMs = millis();
    while (clienteSupabase.connected() && millis() - inicioEsperaMs < TIMEOUT_SUPABASE_MS)
    {
      if (clienteSupabase.available())
      {
        String resposta = clienteSupabase.readStringUntil('\n');
        resposta.trim();
        Serial.println(">> Supabase: " + resposta);
        if (resposta.indexOf("201") >= 0 || resposta.indexOf("200") >= 0)
        {
          sucesso = true;
        }
        break;
      }
    }
    clienteSupabase.stop();
    return sucesso;
  }

  Serial.println(">> Supabase: falha TLS/rede.");
  return false;
}

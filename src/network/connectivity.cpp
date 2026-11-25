#include "network/connectivity.h"
#include <WiFiS3.h>
#include <ArduinoOTA.h>
#include "network/clock.h"
#include "config.h"
#include "network/web_server.h"
#include "private_config.h"

namespace
{
bool wifiConectado = false;
bool otaIniciado = false;
unsigned long ultimaTentativaWifiMs = 0;
unsigned int indiceRedeWifi = 0;

void tentarRedeAtual()
{
  const RedeWifi &rede = REDES_WIFI[indiceRedeWifi];
  Serial.print(">> Wi-Fi: tentando rede ");
  Serial.print(indiceRedeWifi + 1);
  Serial.print(" de ");
  Serial.println(QUANTIDADE_REDES_WIFI);

  WiFi.disconnect();
  if (rede.senha[0] == '\0')
    WiFi.begin(rede.nome);
  else
    WiFi.begin(rede.nome, rede.senha);
  ultimaTentativaWifiMs = millis();
}
} // namespace

void iniciarConectividade()
{
  indiceRedeWifi = 0;
  tentarRedeAtual();
}

void gerenciarWifi()
{
  IPAddress ipAtual = WiFi.localIP();
  bool temIpValido = (ipAtual[0] != 0);

  if (WiFi.status() == WL_CONNECTED && temIpValido)
  {
    if (!wifiConectado)
    {
      wifiConectado = true;
      iniciarServidorHttp();
      ArduinoOTA.begin(ipAtual, OTA_NOME, OTA_SENHA, InternalStorage);
      otaIniciado = true;
      sincronizarRelogioNtp();

      Serial.print(">> Wi-Fi CONECTADO! http://");
      Serial.println(ipAtual);
    }
  }
  else
  {
    if (wifiConectado)
    {
      wifiConectado = false;
      otaIniciado = false;
      Serial.println(">> Wi-Fi DESCONECTADO! Guardando leituras na RAM.");
      // Primeiro tenta recuperar a rede que estava funcionando.
      tentarRedeAtual();
    }
    if (millis() - ultimaTentativaWifiMs >= INTERVALO_RECONEXAO_WIFI_MS)
    {
      indiceRedeWifi = (indiceRedeWifi + 1) % QUANTIDADE_REDES_WIFI;
      tentarRedeAtual();
    }
  }
}

bool wifiEstaConectado()
{
  return wifiConectado;
}

void processarOta()
{
  if (otaIniciado)
  {
    ArduinoOTA.poll();
  }
}

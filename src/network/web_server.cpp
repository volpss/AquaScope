#include "network/web_server.h"
#include <WiFiS3.h>
#include "config.h"
#include "network/connectivity.h"
#include "data/offline_queue.h"
#include "hardware/sensors.h"

namespace
{
WiFiServer servidorHttp(80);

String obterTextoEstado(EstadoSensor estado, String baixo, String alto)
{
  if (estado == SENSOR_SEM_LEITURA)
    return "ERRO DE CONEXAO";
  if (estado == SENSOR_NORMAL)
    return "OK (NORMAL)";
  if (estado == SENSOR_ABAIXO_LIMITE)
    return baixo;
  return alto;
}
} // namespace

void iniciarServidorHttp()
{
  servidorHttp.begin();
}

void atenderNavegador()
{
  const LeiturasSensores &leituras = obterLeituras();
  if (!wifiEstaConectado())
    return;

  WiFiClient clienteHttp = servidorHttp.available();
  if (clienteHttp)
  {
    bool linhaEmBranco = true;
    unsigned long inicioAtendimentoMs = millis();

    while (clienteHttp.connected() && (millis() - inicioAtendimentoMs < TIMEOUT_HTTP_MS))
    {
      if (clienteHttp.available())
      {
        char caractere = clienteHttp.read();
        if (caractere == '\n' && linhaEmBranco)
        {
          clienteHttp.println("HTTP/1.1 200 OK");
          clienteHttp.println("Content-Type: text/html; charset=UTF-8");
          clienteHttp.println("Connection: close");
          clienteHttp.println();

          clienteHttp.println("<!DOCTYPE html><html><head>");
          clienteHttp.println("<meta http-equiv='refresh' content='3'>");
          clienteHttp.println("<title>Aquario IoT</title>");
          clienteHttp.println("<style>body{font-family:Arial;background:#0f172a;color:#f8fafc;text-align:center;padding:30px;}");
          clienteHttp.println(".card{background:#1e293b;border-radius:10px;padding:20px;margin:15px auto;max-width:400px;box-shadow:0 4px 6px rgba(0,0,0,0.3);}");
          clienteHttp.println(".ok{color:#4ade80;}.alerta{color:#f87171;font-weight:bold;}</style></head><body>");
          clienteHttp.println("<h1>Monitoramento do Aquario</h1>");

          clienteHttp.println("<div class='card'><h2>Temperatura (A0)</h2>");
          clienteHttp.print("<p style='font-size:24px;'>");
          clienteHttp.print(leituras.temperatura == LEITURA_INVALIDA ? "ERRO" : String(leituras.temperatura, 1) + " &deg;C");
          clienteHttp.println("</p>");
          clienteHttp.print("<p class='");
          clienteHttp.print(leituras.estadoTemperatura == SENSOR_NORMAL ? "ok" : "alerta");
          clienteHttp.print("'>");
          clienteHttp.print(obterTextoEstado(leituras.estadoTemperatura, "BAIXA", "ALTA"));
          clienteHttp.println("</p></div>");

          clienteHttp.println("<div class='card'><h2>pH (A1)</h2>");
          clienteHttp.print("<p style='font-size:24px;'>");
          clienteHttp.print(leituras.temperaturaBasePh == LEITURA_INVALIDA ? "ERRO" : String(leituras.ph, 2));
          clienteHttp.println("</p>");
          clienteHttp.print("<p class='");
          clienteHttp.print(leituras.estadoPh == SENSOR_NORMAL ? "ok" : "alerta");
          clienteHttp.print("'>");
          clienteHttp.print(obterTextoEstado(leituras.estadoPh, "ACIDO", "ALCALINO"));
          clienteHttp.println("</p></div>");

          clienteHttp.println("<div class='card'><h2>TDS (A2)</h2>");
          clienteHttp.print("<p style='font-size:24px;'>");
          clienteHttp.print(leituras.temperaturaBaseTds == LEITURA_INVALIDA ? "ERRO" : String(leituras.tds, 0) + " ppm");
          clienteHttp.println("</p>");
          clienteHttp.print("<p class='");
          clienteHttp.print(leituras.estadoTds == SENSOR_NORMAL ? "ok" : "alerta");
          clienteHttp.print("'>");
          clienteHttp.print(obterTextoEstado(leituras.estadoTds, "BAIXO", "ALTO"));
          clienteHttp.println("</p></div>");

          clienteHttp.print("<p style='color:#94a3b8;font-size:12px;'>Buffer RAM pendente: ");
          clienteHttp.print(obterQuantidadeFilaOffline());
          clienteHttp.println(" registros</p>");
          clienteHttp.println("</body></html>");
          break;
        }
        if (caractere == '\n')
          linhaEmBranco = true;
        else if (caractere != '\r')
          linhaEmBranco = false;
      }
    }
    delay(1);
    clienteHttp.stop();
  }
}

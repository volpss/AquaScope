#include "network/web_server.h"
#include <cstring>
#include <WiFiS3.h>
#include "config.h"
#include "network/connectivity.h"
#include "data/offline_queue.h"
#include "hardware/sensors.h"

namespace
{
WiFiServer servidorHttp(80);
constexpr size_t TAMANHO_LINHA_REQUISICAO = 128;

void enviarCabecalho(WiFiClient &cliente, const char *status, const char *tipo,
                     const char *extra = nullptr)
{
  cliente.print("HTTP/1.1 ");
  cliente.println(status);
  cliente.print("Content-Type: ");
  cliente.println(tipo);
  cliente.println("Cache-Control: no-store");
  cliente.println("Connection: close");
  if (extra)
    cliente.println(extra);
  cliente.println();
}

void responderErro(WiFiClient &cliente, const char *status, const char *erro,
                   const char *extra = nullptr)
{
  enviarCabecalho(cliente, status, "application/json", extra);
  cliente.print("{\"erro\":\"");
  cliente.print(erro);
  cliente.println("\"}");
}

bool lerRequisicao(WiFiClient &cliente, char (&linha)[TAMANHO_LINHA_REQUISICAO])
{
  size_t tamanho = 0;
  bool primeiraLinha = true;
  bool linhaVazia = true;
  const unsigned long inicioMs = millis();

  while (cliente.connected() && millis() - inicioMs < TIMEOUT_HTTP_MS)
  {
    if (!cliente.available())
      continue;

    const int caractere = cliente.read();
    if (caractere < 0 || caractere == '\r')
      continue;

    if (caractere == '\n')
    {
      if (primeiraLinha)
      {
        linha[tamanho] = '\0';
        primeiraLinha = false;
      }
      else if (linhaVazia)
      {
        return true;
      }

      linhaVazia = true;
      continue;
    }

    linhaVazia = false;
    if (!primeiraLinha)
      continue;

    if (caractere < ' ' || caractere == 127)
    {
      responderErro(cliente, "400 Bad Request", "requisicao_invalida");
      return false;
    }
    if (tamanho == sizeof(linha) - 1)
    {
      responderErro(cliente, "414 URI Too Long", "linha_de_requisicao_longa");
      return false;
    }
    linha[tamanho++] = static_cast<char>(caractere);
  }

  return false;
}

const char *obterEstadoJson(EstadoSensor estado)
{
  switch (estado)
  {
    case SENSOR_NORMAL: return "normal";
    case SENSOR_ABAIXO_LIMITE: return "abaixo_limite";
    case SENSOR_ACIMA_LIMITE: return "acima_limite";
    default: return "sem_leitura";
  }
}

void responderMedicao(WiFiClient &cliente, float valor, EstadoSensor estado,
                      const char *unidade, int casasDecimais)
{
  const bool disponivel = estado != SENSOR_SEM_LEITURA;
  enviarCabecalho(cliente, disponivel ? "200 OK" : "503 Service Unavailable", "application/json");
  cliente.print("{\"valor\":");
  if (disponivel)
    cliente.print(valor, casasDecimais);
  else
    cliente.print("null");

  cliente.print(",\"unidade\":\"");
  cliente.print(unidade);
  cliente.print("\",\"estado\":\"");
  cliente.print(obterEstadoJson(estado));
  cliente.println("\"}");
}

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
void responderPagina(WiFiClient &clienteHttp, const LeiturasSensores &leituras)
{
  enviarCabecalho(clienteHttp, "200 OK", "text/html; charset=UTF-8");

  clienteHttp.println("<!DOCTYPE html><html><head>");
  clienteHttp.println("<meta http-equiv='refresh' content='3'>");
  clienteHttp.println("<title>Aquario IoT</title>");
  clienteHttp.println("<style>body{font-family:Arial;background:#0f172a;color:#f8fafc;text-align:center;padding:30px;}");
  clienteHttp.println(".card{background:#1e293b;border-radius:10px;padding:20px;margin:15px auto;max-width:400px;box-shadow:0 4px 6px rgba(0,0,0,0.3);}");
  clienteHttp.println(".ok{color:#4ade80;}.alerta{color:#f87171;font-weight:bold;}</style></head><body>");
  clienteHttp.println("<h1>Monitoramento do Aquario</h1>");

  clienteHttp.println("<div class='card'><h2>Temperatura (A0)</h2>");
  clienteHttp.print("<p style='font-size:24px;'>");
  clienteHttp.print(leituras.estadoTemperatura == SENSOR_SEM_LEITURA ? "ERRO" : String(leituras.temperatura, 1) + " &deg;C");
  clienteHttp.println("</p>");
  clienteHttp.print("<p class='");
  clienteHttp.print(leituras.estadoTemperatura == SENSOR_NORMAL ? "ok" : "alerta");
  clienteHttp.print("'>");
  clienteHttp.print(obterTextoEstado(leituras.estadoTemperatura, "BAIXA", "ALTA"));
  clienteHttp.println("</p></div>");

  clienteHttp.println("<div class='card'><h2>pH (A1)</h2>");
  clienteHttp.print("<p style='font-size:24px;'>");
  clienteHttp.print(leituras.estadoPh == SENSOR_SEM_LEITURA ? "ERRO" : String(leituras.ph, 2));
  clienteHttp.println("</p>");
  clienteHttp.print("<p class='");
  clienteHttp.print(leituras.estadoPh == SENSOR_NORMAL ? "ok" : "alerta");
  clienteHttp.print("'>");
  clienteHttp.print(obterTextoEstado(leituras.estadoPh, "ACIDO", "ALCALINO"));
  clienteHttp.println("</p></div>");

  clienteHttp.println("<div class='card'><h2>TDS (A2)</h2>");
  clienteHttp.print("<p style='font-size:24px;'>");
  clienteHttp.print(leituras.estadoTds == SENSOR_SEM_LEITURA ? "ERRO" : String(leituras.tds, 0) + " ppm");
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
}

void atenderRota(WiFiClient &cliente, char *linha)
{
  char *caminho = std::strchr(linha, ' ');
  if (!caminho || caminho == linha)
  {
    responderErro(cliente, "400 Bad Request", "requisicao_invalida");
    return;
  }
  *caminho++ = '\0';

  char *versao = std::strchr(caminho, ' ');
  if (*caminho != '/' || !versao ||
      (std::strcmp(versao + 1, "HTTP/1.0") != 0 && std::strcmp(versao + 1, "HTTP/1.1") != 0))
  {
    responderErro(cliente, "400 Bad Request", "requisicao_invalida");
    return;
  }
  *versao = '\0';

  if (std::strcmp(linha, "GET") != 0)
  {
    responderErro(cliente, "405 Method Not Allowed", "metodo_nao_permitido", "Allow: GET");
    return;
  }

  if (char *consulta = std::strchr(caminho, '?'))
    *consulta = '\0';

  const LeiturasSensores &leituras = obterLeituras();
  if (std::strcmp(caminho, "/") == 0)
    responderPagina(cliente, leituras);
  else if (std::strcmp(caminho, "/api/temperatura") == 0)
    responderMedicao(cliente, leituras.temperatura, leituras.estadoTemperatura, "°C", 1);
  else if (std::strcmp(caminho, "/api/ph") == 0)
    responderMedicao(cliente, leituras.ph, leituras.estadoPh, "pH", 2);
  else if (std::strcmp(caminho, "/api/tds") == 0)
    responderMedicao(cliente, leituras.tds, leituras.estadoTds, "ppm", 0);
  else
    responderErro(cliente, "404 Not Found", "rota_nao_encontrada");
}
} // namespace

void iniciarServidorHttp()
{
  servidorHttp.begin();
}

void atenderNavegador()
{
  if (!wifiEstaConectado())
    return;

  WiFiClient clienteHttp = servidorHttp.available();
  if (!clienteHttp)
    return;

  char linha[TAMANHO_LINHA_REQUISICAO];
  if (lerRequisicao(clienteHttp, linha))
    atenderRota(clienteHttp, linha);

  delay(1);
  clienteHttp.stop();
}

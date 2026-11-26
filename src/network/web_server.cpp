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

  while (cliente.connected() && millis() - inicioMs < TEMPO_LIMITE_HTTP_MS)
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
                      const char *unidade, int casasDecimais, bool incluirTemposTds = false)
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
  cliente.print('"');

  if (incluirTemposTds)
  {
    const unsigned long agoraMs = millis();
    cliente.print(",\"idade_ms\":");
    if (disponivel)
      cliente.print(obterIdadeLeituraTdsMs(agoraMs));
    else
      cliente.print("null");
    cliente.print(",\"proxima_em_ms\":");
    cliente.print(obterTempoAteProximaLeituraTdsMs(agoraMs));
    cliente.print(",\"intervalo_ms\":");
    cliente.print(INTERVALO_MEDICAO_TDS_MS);
  }
  cliente.println('}');
}

const char *obterTextoEstado(EstadoSensor estado, const char *baixo, const char *alto)
{
  if (estado == SENSOR_SEM_LEITURA)
    return "Sem leitura";
  if (estado == SENSOR_NORMAL)
    return "OK (NORMAL)";
  if (estado == SENSOR_ABAIXO_LIMITE)
    return baixo;
  return alto;
}

void responderFila(WiFiClient &cliente)
{
  enviarCabecalho(cliente, "200 OK", "application/json");
  cliente.print("{\"pendentes\":");
  cliente.print(obterQuantidadeFilaOffline());
  cliente.println('}');
}

void escreverAtualizacaoPagina(WiFiClient &cliente)
{
  static const char script[] = R"HTML(
<script>
const sensores = [
  { id: 'temperatura', casas: 1, baixo: 'BAIXA', alto: 'ALTA' },
  { id: 'ph', casas: 2, baixo: 'ÁCIDO', alto: 'ALCALINO' },
  { id: 'tds', casas: 0, baixo: 'BAIXO', alto: 'ALTO' }
];
const ultimaTds = document.getElementById('ultima-tds');
const proximaTds = document.getElementById('proxima-tds');
let proximaLeituraTds = null;

async function consultar(rota) {
  const controle = new AbortController();
  const limite = setTimeout(() => controle.abort(), 8000);
  try {
    const resposta = await fetch('/api/' + rota, { signal: controle.signal });
    if (!resposta.ok && resposta.status !== 503) throw new Error('HTTP ' + resposta.status);
    return await resposta.json();
  } finally {
    clearTimeout(limite);
  }
}

function mostrarContagemTds() {
  const segundos = Math.ceil((proximaLeituraTds - performance.now()) / 1000);
  proximaTds.textContent = proximaLeituraTds === null ? '—' :
    segundos > 0 ? 'em cerca de ' + segundos + ' s' : 'aguardando leitura';
}

async function atualizarSensor(sensor) {
  const cartao = document.getElementById(sensor.id);
  const valor = cartao.querySelector('.valor');
  const estado = cartao.querySelector('.estado');
  try {
    const resposta = await consultar(sensor.id);
    valor.textContent = resposta.valor === null ? '—' :
      resposta.valor.toLocaleString('pt-BR', {
        minimumFractionDigits: sensor.casas,
        maximumFractionDigits: sensor.casas
      }) + (sensor.id === 'ph' ? '' : ' ' + resposta.unidade);
    const textos = {
      normal: 'OK (NORMAL)', abaixo_limite: sensor.baixo,
      acima_limite: sensor.alto, sem_leitura: 'Sem leitura'
    };
    estado.textContent = textos[resposta.estado];
    estado.className = resposta.estado === 'normal' ? 'estado ok' : 'estado alerta';

    if (sensor.id === 'tds') {
      ultimaTds.textContent = resposta.idade_ms === null ? '—' :
        new Date(Date.now() - resposta.idade_ms).toLocaleTimeString('pt-BR');
      proximaLeituraTds = performance.now() + resposta.proxima_em_ms;
      mostrarContagemTds();
    }
  } catch {
    estado.textContent = 'Falha na consulta';
    estado.className = 'estado alerta';
    if (sensor.id === 'tds') {
      proximaLeituraTds = null;
      mostrarContagemTds();
    }
  }
}

async function atualizarPagina() {
  for (const sensor of sensores) await atualizarSensor(sensor);
  const fila = document.getElementById('fila');
  try {
    const resposta = await consultar('fila');
    fila.textContent = resposta.pendentes;
  } catch {
    fila.textContent = '—';
  }
  setTimeout(atualizarPagina, 3000);
}

setInterval(mostrarContagemTds, 250);
atualizarPagina();
</script>
)HTML";

  // Envie o script em blocos para limitar o tamanho de cada escrita no modem.
  constexpr size_t TAMANHO_BLOCO = 512;
  for (size_t offset = 0; offset < sizeof(script) - 1; offset += TAMANHO_BLOCO)
  {
    const size_t restante = sizeof(script) - 1 - offset;
    const size_t tamanho = restante < TAMANHO_BLOCO ? restante : TAMANHO_BLOCO;
    if (cliente.write(reinterpret_cast<const uint8_t *>(script + offset), tamanho) != tamanho)
      return;
  }
}

void responderPagina(WiFiClient &clienteHttp, const LeiturasSensores &leituras)
{
  enviarCabecalho(clienteHttp, "200 OK", "text/html; charset=UTF-8");

  clienteHttp.println("<!DOCTYPE html><html lang='pt-BR'><head><meta charset='UTF-8'>");
  clienteHttp.println("<meta name='viewport' content='width=device-width, initial-scale=1'>");
  clienteHttp.println("<title>Aquario IoT</title>");
  clienteHttp.println("<style>body{font-family:Arial;background:#0f172a;color:#f8fafc;text-align:center;padding:30px;}");
  clienteHttp.println(".card{background:#1e293b;border-radius:10px;padding:20px;margin:15px auto;max-width:400px;box-shadow:0 4px 6px rgba(0,0,0,0.3);}");
  clienteHttp.println(".ok{color:#4ade80;}.alerta{color:#f87171;font-weight:bold;}</style></head><body>");
  clienteHttp.println("<h1>Monitoramento do Aquario</h1>");

  clienteHttp.println("<div class='card' id='temperatura'><h2>Temperatura (A0)</h2>");
  clienteHttp.print("<p class='valor' style='font-size:24px;'>");
  clienteHttp.print(leituras.estadoTemperatura == SENSOR_SEM_LEITURA ? "—" : String(leituras.temperatura, 1) + " &deg;C");
  clienteHttp.println("</p>");
  clienteHttp.print("<p class='estado ");
  clienteHttp.print(leituras.estadoTemperatura == SENSOR_NORMAL ? "ok" : "alerta");
  clienteHttp.print("'>");
  clienteHttp.print(obterTextoEstado(leituras.estadoTemperatura, "BAIXA", "ALTA"));
  clienteHttp.println("</p></div>");

  clienteHttp.println("<div class='card' id='ph'><h2>pH (A1)</h2>");
  clienteHttp.print("<p class='valor' style='font-size:24px;'>");
  clienteHttp.print(leituras.estadoPh == SENSOR_SEM_LEITURA ? "—" : String(leituras.ph, 2));
  clienteHttp.println("</p>");
  clienteHttp.print("<p class='estado ");
  clienteHttp.print(leituras.estadoPh == SENSOR_NORMAL ? "ok" : "alerta");
  clienteHttp.print("'>");
  clienteHttp.print(obterTextoEstado(leituras.estadoPh, "ACIDO", "ALCALINO"));
  clienteHttp.println("</p></div>");

  clienteHttp.println("<div class='card' id='tds'><h2>TDS (A2)</h2>");
  clienteHttp.print("<p class='valor' style='font-size:24px;'>");
  clienteHttp.print(leituras.estadoTds == SENSOR_SEM_LEITURA ? "—" : String(leituras.tds, 0) + " ppm");
  clienteHttp.println("</p>");
  clienteHttp.print("<p class='estado ");
  clienteHttp.print(leituras.estadoTds == SENSOR_NORMAL ? "ok" : "alerta");
  clienteHttp.print("'>");
  clienteHttp.print(obterTextoEstado(leituras.estadoTds, "BAIXO", "ALTO"));
  clienteHttp.println("</p>");
  clienteHttp.println("<p>Última leitura: <span id='ultima-tds'>—</span></p>");
  clienteHttp.println("<p>Próxima leitura: <span id='proxima-tds'>—</span></p></div>");

  clienteHttp.print("<p style='color:#94a3b8;font-size:12px;'>Buffer RAM pendente: <span id='fila'>");
  clienteHttp.print(obterQuantidadeFilaOffline());
  clienteHttp.println("</span> registros</p>");
  escreverAtualizacaoPagina(clienteHttp);
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
    responderMedicao(cliente, leituras.tds, leituras.estadoTds, "ppm", 0, true);
  else if (std::strcmp(caminho, "/api/fila") == 0)
    responderFila(cliente);
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

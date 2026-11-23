---
date: "2026-11-23"
type: "change"
topics: ["firmware","organização","PlatformIO"]
summary: "Firmware separado em módulos por responsabilidade, com compilação confirmada."
---

## Contexto e mudança

O usuário informou nesta conversa que o projeto estava funcionando e solicitou separar os arquivos por responsabilidade, usando uma organização comum em Arduino. O código antes concentrado em `src/main.cpp` foi dividido em interfaces `.h` em `include/` e implementações `.cpp` em `src/`. Os nomes de arquivos estão em inglês e os identificadores permanecem em português.

Foram separados sensores, indicadores, conectividade Wi-Fi/OTA, relógio NTP, banco Supabase, fila offline, coordenação da telemetria e página HTTP local. As configurações ficam em `include/config.h` e as estruturas em `include/models.h`. O estado interno fica restrito a cada módulo; o `main.cpp` mantém a inicialização e o agendamento das tarefas.

## Verificação e limites

A compilação de todos os módulos passou no ambiente `uno_r4_wifi`, com Renesas RA 1.9.0 e ArduinoOTA 1.1.1: 15.748 bytes de RAM e 72.072 bytes de Flash. A comparação com o arquivo anterior confirmou os valores de configuração, textos literais de comunicação e funções auxiliares de cálculo e data preservados.

Não houve upload nem teste na placa nesta reorganização. A fila continua exclusivamente em RAM, limitada a 400 registros, e as conversões de pH e TDS continuam simuladas. O serviço OTA foi mantido, sem configurar ou verificar upload por Wi-Fi no PlatformIO.

## Fontes e continuidade

Ver o [mapa dos módulos](../../README.md), o [ciclo principal](../../src/main.cpp) e a [configuração do PlatformIO](../../platformio.ini). A situação vigente está em [status.md](../../status.md). Os arquivos locais de orientação e acompanhamento foram inicializados nesta reorganização; este registro não documenta integralmente os acontecimentos anteriores.

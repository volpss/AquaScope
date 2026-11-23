# Aquario IoT

## Objetivo

Desenvolver um projeto acadêmico de monitoramento de aquário com Arduino Uno R4 WiFi e PlatformIO.

## Escopo

Firmware para Arduino Uno R4 WiFi com leituras analógicas, indicadores locais, conexão Wi-Fi, página HTTP, envio ao Supabase e fila offline em RAM. As conversões atuais de pH e TDS são simuladas a partir das temperaturas dos canais A1 e A2.

A organização usa módulos `.h`/`.cpp`, com interfaces em `include/` e implementações em `src/`, agrupadas nas pastas correspondentes `hardware/`, `network/` e `data/`. A configuração de compilação fica no `platformio.ini`. O mapa dos módulos e os comandos de uso ficam no [README.md](README.md).

Os dados privados de Wi-Fi, OTA e Supabase ficam no `.env` local, ignorado pelo Git. O [.env.example](.env.example) contém somente valores fictícios; o cabeçalho privado é gerado em `.pio/` antes da compilação.

A configuração `WIFI_REDES` contém uma lista JSON de objetos com `nome` e `senha`, na ordem das tentativas de conexão. A lista é convertida em array C++ na compilação, sem carregar JSON na placa.

## Registros

- [Instruções](AGENTS.md)
- [Situação atual](status.md)
- [Histórico](history/README.md)

## Fontes, chats e projetos relacionados

- Fontes locais: [ciclo principal](src/main.cpp), [configuração do firmware](include/config.h) e [configuração do PlatformIO](platformio.ini).
- O usuário informou nesta conversa, em 23/11/2026, que o projeto estava funcionando antes da separação em módulos.
- Não há vínculo externo de chat ou de outro projeto registrado nestes arquivos.

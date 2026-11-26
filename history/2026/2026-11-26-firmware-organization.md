---
date: "2026-11-26"
type: "change"
topics: ["firmware","organização","PlatformIO","pH"]
summary: "Organizadas as constantes do firmware, a nomenclatura da telemetria e a pasta de bancada."
---

O [sketch de pH](../../src/benches/ph-bench.cpp) foi movido para `src/benches/`, com referência atualizada no [PlatformIO](../../platformio.ini). O ambiente `uno_r4_wifi_ph` compila o sketch e o ambiente padrão continua usando [src/main.cpp](../../src/main.cpp). Os testes automatizados permanecem em `test/`.

As constantes de [config.h](../../include/config.h) foram agrupadas por sensor ou serviço. O sketch e o firmware compartilham pino de pH, parâmetros de ADC, amostragem e descarte dos extremos. As declarações de estado da amostragem foram agrupadas por canal. A fila e a serialização passaram a usar a terminologia de medição pendente, com os mesmos campos JSON e parâmetros de operação.

Os ambientes `uno_r4_wifi` e `uno_r4_wifi_ph` compilaram e os nove testes do carregador de configuração passaram. O build padrão usou 13.060 bytes de RAM e 78.840 bytes de Flash. A bancada usou 2.480 bytes de RAM e 36.328 bytes de Flash. A validação na placa segue pendente.

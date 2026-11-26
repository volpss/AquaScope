---
date: "2026-11-25"
type: "change"
topics: ["firmware", "hardware", "TDS"]
summary: "Configurado pulso de 3 segundos no controle de alimentação do TDS a cada 5 minutos."
---

O [módulo de sensores](../../src/hardware/sensors.cpp) usa `millis()` para programar o acionamento do `D7`. A configuração prevê um MOSFET de canal P chaveando o positivo de 5 V e um resistor externo que mantém o módulo desligado durante o reset. O [circuito e a calibração](../../README.md#alimentação-e-calibração-do-tds) estão descritos no README.

O primeiro pulso começa após a primeira leitura de temperatura e pH. A cada pulso, o firmware aguarda 2,5 segundos, coleta 30 amostras de `A2` e corta a alimentação aos 3 segundos. O [ciclo principal](../../src/main.cpp) adia chamadas de rede durante o pulso. Página, API e telemetria reutilizam o último resultado entre ciclos.

`pio run -e uno_r4_wifi` compilou com 16.240 bytes de RAM e 75.760 bytes de Flash. Ainda faltam montagem, upload, medição física da duração do pulso e verificação da estabilidade da leitura no módulo.

A alternativa de alimentar o módulo diretamente por `D7` foi avaliada. O [Uno R4 WiFi admite até 8 mA por GPIO](https://docs.arduino.cc/hardware/uno-r4-wifi/) e a [DFRobot especifica 3 a 6 mA em funcionamento](https://wiki.dfrobot.com/sen0244). O esquema do módulo mostra capacitores na alimentação e não há medição do pico de partida. A ligação direta ficou sem validação elétrica e o firmware conserva `D7` como comando da chave de alimentação.

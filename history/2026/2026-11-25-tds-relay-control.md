---
date: "2026-11-25"
type: "change"
topics: ["firmware", "hardware", "TDS"]
summary: "Adaptado o D7 para acionar o relé de alimentação do TDS."
---

O usuário informou que montou o circuito proposto com um módulo relé ativo em nível baixo. O `5V` alimenta o `COM` e o contato `NO` alimenta o `+` do TDS. O sinal `A` continua em `A2` e o `-` vai ao `GND` comum. A [pinagem atual](../../README.md#alimentação-e-calibração-do-tds) substitui a montagem com MOSFET descrita no registro anterior.

O [módulo de sensores](../../src/hardware/sensors.cpp) inicializa `D7` diretamente como saída `HIGH`. O relé recebe `LOW` durante a janela de 3 segundos e `HIGH` em repouso. O intervalo de 5 minutos e o filtro da leitura foram mantidos.

`pio run -e uno_r4_wifi` compilou com 16.240 bytes de RAM e 75.760 bytes de Flash. O upload e o funcionamento do relé na placa ainda não foram observados.

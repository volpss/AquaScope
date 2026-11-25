---
date: "2026-11-25"
type: "change"
topics: ["firmware", "hardware", "temperatura", "pH", "TDS"]
summary: "Integradas as leituras reais de temperatura, pH e TDS no firmware completo."
---

O usuário confirmou o ajuste de 2,50 V do PH-4502C com o BNC em curto e pediu o firmware completo antes da calibração em solução tampão. O [PlatformIO](../../platformio.ini) seleciona [mainOriginal.cpp](../../src/mainOriginal.cpp) no ambiente padrão `uno_r4_wifi` e o [sketch de bancada](../../src/main.cpp) no ambiente `uno_r4_wifi_ph`.

O [módulo de sensores](../../src/hardware/sensors.cpp) faz a leitura com ADC de 10 bits, em janelas de 30 amostras espaçadas por pelo menos 10 ms. O cálculo do NTC em `A0` mantém o divisor de 10 kΩ. No pH (`A1`) e no TDS (`A2`), o filtro usa a média das 20 amostras centrais. O pH usa 2,500 V como ponto neutro e a inclinação provisória de −5,70 pH/V. O TDS usa a [curva do Gravity TDS](https://wiki.dfrobot.com/sen0244/docs/20305) e compensa a temperatura medida no NTC.

O monitor serial, a página local, os indicadores e a telemetria usam as mesmas leituras e estados. A telemetria mantém a regra de enviar apenas conjuntos com os três valores válidos e conserva os campos existentes do Supabase.

`pio run -e uno_r4_wifi -e uno_r4_wifi_ph` compilou os dois programas. O firmware completo usou 16.220 bytes de RAM e 74.416 bytes de Flash. O sketch de bancada usou 2.480 bytes de RAM e 35.884 bytes de Flash. O upload do firmware completo e a leitura conjunta na placa estão pendentes.

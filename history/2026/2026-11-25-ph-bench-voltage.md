---
date: "2026-11-25"
type: "change"
topics: ["firmware", "hardware", "pH"]
summary: "Adicionado ambiente de bancada para medir a tensão da saída PO do PH-4502C."
---

O [PlatformIO](../../platformio.ini) oferece o ambiente `uno_r4_wifi_ph`, que seleciona o teste em [main.cpp](../../src/main.cpp). O ambiente `uno_r4_wifi` continua sendo o padrão. A cópia local `mainOriginal.cpp` fica fora da compilação para evitar definições duplicadas de `setup()` e `loop()`.

O [módulo de sensores](../../src/hardware/sensors.cpp) coleta 30 amostras de `A1` em intervalos de 10 ms, descarta cinco amostras de cada extremo e calcula a média das 20 centrais. A saída serial a 9600 baud mostra o ADC médio e a tensão estimada com resolução de 10 bits e referência nominal de 5 V.

A compilação com `pio run -e uno_r4_wifi -e uno_r4_wifi_ph` passou. O firmware padrão usou 15.972 bytes de RAM e 73.984 bytes de Flash. O ambiente de bancada usou 2.608 bytes de RAM e 36.364 bytes de Flash. O upload e a medição na placa ainda estão pendentes.

Na revisão do `main.cpp`, a seleção do modo passou para dentro de um único `setup()` e um único `loop()`. O cabeçalho privado passou a ser incluído diretamente pelos módulos de conectividade e banco, que usam suas definições. Os dois ambientes compilaram novamente com o mesmo uso de memória. O PlatformIO também gerou a configuração local do IntelliSense do VS Code.

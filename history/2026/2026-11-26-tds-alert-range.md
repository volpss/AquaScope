---
date: "2026-11-26"
type: "change"
topics: ["firmware","TDS","alertas"]
summary: "A faixa normal do TDS passou a abranger leituras válidas de 0 a 350 ppm."
---

A lista de [pendências](../../PENDENCIES.md) relatava que leituras abaixo de 150 ppm apareciam em alerta e acionavam o buzzer. Para atender ao comportamento pedido, o limite inferior em [include/config.h](../../include/config.h) passou de 150 para 0 ppm. O teto continua em 350 ppm. Leituras válidas nessa faixa, com os extremos incluídos, recebem o estado normal usado pelo LED, pelo buzzer, pela página local e pela telemetria.

`pio run -e uno_r4_wifi` compilou com 16.240 bytes de RAM e 75.760 bytes de Flash. A confirmação do LED e do buzzer com uma leitura abaixo de 150 ppm depende de upload e teste na placa com os demais sensores em condições normais.

---
date: "2026-11-26"
type: "change"
topics: ["firmware","temperatura","alertas"]
summary: "Reduzido para 23 °C o limite inferior de alerta de temperatura."
---

A pedido do usuário, o limite inferior de temperatura em [config.h](../../include/config.h) passou de 26 °C para 23 °C. Com o limite superior de 28 °C, o firmware classifica como normais as leituras válidas entre 23 °C e 28 °C, inclusive.

`pio run -e uno_r4_wifi` compilou. A verificação na placa segue pendente.

---
date: "2026-11-25"
type: "change"
topics: ["firmware", "HTTP", "temperatura", "pH", "TDS"]
summary: "Adicionadas três rotas HTTP locais para consultar as medições separadamente."
---

O [servidor HTTP local](../../src/network/web_server.cpp) oferece `GET /api/temperatura`, `GET /api/ph` e `GET /api/tds`. Cada rota retorna o valor, a unidade e o estado do sensor em JSON. Se o sensor estiver sem leitura, a rota responde com HTTP 503 e valor nulo. A página HTML continua acessível em `GET /`.

A compilação `pio run -e uno_r4_wifi` passou com 16.220 bytes de RAM e 75.464 bytes de Flash. O upload e a consulta das rotas pela rede local ainda estão pendentes.

---
date: "2026-11-25"
type: "change"
topics: ["firmware", "TDS"]
summary: "Adicionado intervalo temporário de 30 segundos para testar o TDS."
---

A opção `USAR_INTERVALO_TESTE_TDS` em [include/config.h](../../include/config.h) está em `true` para os testes. Ela seleciona um ciclo a cada 30 segundos. Com `false`, o intervalo volta a 5 minutos. O relé continua ligado por 3 segundos a cada ciclo.

`pio run -e uno_r4_wifi` compilou com o intervalo de teste ativo. O upload e a confirmação do intervalo na placa estão pendentes.

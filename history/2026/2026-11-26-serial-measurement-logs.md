---
date: "2026-11-26"
type: "change"
topics: ["firmware","Monitor Serial","telemetria","Supabase"]
summary: "O Monitor Serial passou a registrar cada nova medição e o JSON de cada POST ao Supabase."
---

O [módulo de sensores](../../src/hardware/sensors.cpp) registra uma linha `[MEDICAO]` somente quando o canal correspondente conclui uma janela. Temperatura e pH são registrados antes de um eventual pulso inicial de TDS. O TDS é registrado após o desligamento do relé. O valor, a unidade e o estado da leitura são exibidos na mesma linha, ou `ERRO` quando a medição está indisponível.

O [módulo de banco](../../src/data/database.cpp) registra o mesmo JSON usado no corpo de cada POST, acompanhado do código HTTP da resposta ou da indicação de falta de resposta válida. Falhas de conexão antes do POST têm mensagem própria. O registro usa apenas o corpo da requisição e o resultado HTTP.

`pio run -e uno_r4_wifi` compilou com 13.060 bytes de RAM e 78.792 bytes de Flash. A conferência dos logs na placa segue pendente.

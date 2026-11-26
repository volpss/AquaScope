---
date: "2026-11-26"
type: "change"
topics: ["firmware","telemetria","Supabase","TDS"]
summary: "A telemetria passou a enviar uma medição por requisição ao Supabase."
---

O usuário pediu um envio por medição para evitar repetir o mesmo TDS em cada envio periódico. Em [telemetria](../../src/data/telemetry.cpp), temperatura e pH têm intervalos independentes de 15 segundos e só registram uma leitura válida recém-concluída. Cada ciclo válido de TDS gera um item. O modo de teste mantém a medição a cada 30 segundos e o modo normal usa 5 minutos.

A fila circular comporta 400 medições individuais em RAM. O firmware envia a mais antiga em um POST por vez, com pelo menos um segundo entre o término de uma tentativa e a próxima. Cada item sai da fila após resposta HTTP de sucesso. Os POSTs ocorrem fora do pulso de TDS. A [documentação](../../README.md#envio-ao-supabase) orienta a conferir se a tabela aceita linhas com apenas uma coluna de medição preenchida, preservando a compatibilidade de leitura com o histórico.

`pio run -e uno_r4_wifi` compilou com 13.060 bytes de RAM e 78.632 bytes de Flash. A validação da tabela, o upload e o teste de envio permanecem pendentes. O [checklist](../../PENDENCIES.md) também registra os testes de interferência entre pH e TDS pedidos pelo usuário.

---
date: "2026-11-26"
type: "change"
topics: ["firmware","HTTP","temperatura","pH","TDS"]
summary: "A página local passou a consultar cada medição pela sua rota JSON."
---

A [página local](../../src/network/web_server.cpp) conserva os três cartões exibidos ao abrir o monitor. As atualizações seguintes consultam `/api/temperatura`, `/api/ph` e `/api/tds` em sequência, sem recarregar a página. Cada cartão trata sua própria consulta. `/api/fila` atualiza a quantidade de registros pendentes na RAM.

A rota de TDS expõe a idade da leitura, o intervalo entre inícios de ciclo e o tempo estimado até o fim do próximo pulso. A página calcula o horário da última leitura pelo relógio do navegador e mostra a contagem regressiva. O firmware está configurado para um ciclo de teste a cada 30 segundos. O [README](../../README.md#api-http-local) descreve o contrato. A [lista de pendências](../../PENDENCIES.md) inclui a revisão dos limites de alarme de TDS após identificação das espécies e calibração da sonda.

`pio run -e uno_r4_wifi` compilou com 16.244 bytes de RAM e 79.040 bytes de Flash. O script da página foi exercitado com respostas simuladas, incluindo HTTP 503, falha isolada e recuperação da consulta de pH, atualização da fila e contagem do TDS. Falta verificar a página e as respostas HTTP na rede local após gravar a placa.

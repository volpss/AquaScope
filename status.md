# Situação atual — Aquario IoT

Atualização: 2026-11-26 (America/Sao_Paulo).

## Situação confirmada

O usuário informou que o firmware estava funcionando antes da modularização. Em 23/11/2026, o código foi separado em módulos `.h`/`.cpp`, preservando os cálculos, configurações, textos de comunicação e a ordem das tarefas. O `main.cpp` coordena a inicialização e o ciclo principal.

Os módulos agora estão agrupados em `hardware/`, `network/` e `data/`, com a mesma divisão em `src/` e `include/`. Os includes e as referências da documentação foram atualizados; a compilação após a movimentação passou, com o mesmo uso de memória.

O módulo `hardware/led_matrix` inicializa a matriz integrada e exibe um peixe estático no `setup()`, usando a biblioteca do framework `Arduino_LED_Matrix`. O mapa de responsabilidades está no [README.md](README.md).

As configurações privadas de Wi-Fi, OTA e Supabase ficam no `.env`, mantendo os valores locais. As redes são configuradas na lista JSON `WIFI_REDES`, que substitui o par `WIFI_NOME`/`WIFI_SENHA`. O `.env.example` contém duas redes fictícias. O hook de pré-compilação valida a lista e gera o array de redes no cabeçalho privado em `.pio/`; os nove testes do carregador e a compilação passaram.

O firmware percorre as redes em ordem, com intervalo de 15 segundos após cada tentativa. Ao perder uma conexão estabelecida, tenta recuperar a mesma rede antes de alternar. A retomada circular e a conexão em redes diferentes ainda precisam de validação na placa.

O ambiente `uno_r4_wifi_ph` usa `src/benches/ph-bench.cpp` para conferir a tensão estimada de `PO` em `A1`. O sketch compartilha as constantes de aquisição com o firmware em `include/config.h`, agora organizado por sensor e serviço. O usuário confirmou o ajuste de 2,50 V com o BNC em curto e optou por adiar a calibração da sonda em soluções tampão.

O ambiente padrão `uno_r4_wifi` usa `src/main.cpp` como ponto de entrada. O firmware lê a temperatura em `A0`, o pH em `A1` e o TDS em `A2`. As leituras de pH usam a inclinação provisória de −5,70 pH/V. O TDS usa compensação pela temperatura lida em `A0`. Os dois ambientes compilaram após a troca dos arquivos.

O servidor HTTP local oferece as rotas JSON `GET /api/temperatura`, `GET /api/ph` e `GET /api/tds`. A página consulta cada medição separadamente e usa os campos de tempo da rota de TDS para mostrar a última leitura e a previsão da próxima. A rota `GET /api/fila` atualiza o contador de registros pendentes. A compilação passou no ambiente `uno_r4_wifi`.

O firmware mede temperatura a cada 3 segundos, pH a cada 45 segundos e TDS a cada 15 minutos no modo normal. Cada nova leitura válida entra na fila em RAM para envio em um POST individual ao Supabase. A fila comporta 400 medições e remove cada item após confirmar o envio. A compilação passou no ambiente `uno_r4_wifi`.

O Monitor Serial registra somente os sensores que acabaram de concluir uma medição, além dos eventos de operação. Cada POST registra o corpo JSON e o resultado HTTP. A compilação passou e a conferência dos logs na placa está pendente.

O usuário informou que montou a alimentação do TDS com relé. O firmware configura `D7` como saída em `HIGH` e aciona o relé em `LOW` por 3 segundos. `USAR_INTERVALO_TESTE_TDS` está em `false`. A entrada `A2` é amostrada após 2,5 segundos de estabilização. Temperatura e pH pausam durante o pulso. Entre ciclos, o firmware mantém o último resultado de TDS.

O firmware usa a faixa provisória de 0 a 350 ppm, inclusive, para o alarme de TDS. O limite inferior anterior era 150 ppm. A alteração compilou no ambiente `uno_r4_wifi`.

A faixa de temperatura considerada normal pelo firmware passou a ser de 23 °C a 28 °C, inclusive. A alteração compilou no ambiente `uno_r4_wifi`.

## Pendências

- Validar a versão modularizada na placa: leituras, LEDs, alarme, página local, envio ao Supabase e retomada da fila após uma queda de Wi-Fi. Não foi feito upload nesta reorganização.
- Conferir o desenho do peixe na matriz integrada após o upload; a exibição física ainda não foi observada.
- Validar a alternância Wi-Fi com duas redes configuradas, a primeira indisponível e posterior queda da rede conectada.
- A fila permanece em RAM e perde os registros após reinicialização.
- Validar os três sensores juntos na placa e calibrar a sonda de pH em solução tampão quando possível.
- Comparar o pH com o TDS desligado e logo após cada pulso. Comparar também o TDS com o módulo de pH ligado e desligado.
- Após gravar o firmware, conferir a comutação do relé, medir o pulso no `+` do módulo, verificar a estabilidade do TDS e confirmar o LED e o buzzer com leituras abaixo de 150 ppm e os outros sensores em condições normais.
- Calibrar a sonda de TDS, identificar as espécies do aquário e revisar os limites de alerta conforme os parâmetros de água dessas espécies.
- Após o upload, consultar as três rotas HTTP, conferir a atualização dos cartões e verificar a contagem do TDS na rede local.
- Conferir se a tabela do Supabase aceita as medições individuais antes do upload. Validar na placa as cadências de 3 s, 45 s e 15 min, a recuperação da fila e as consultas de eventual painel externo.
- O serviço OTA foi preservado, mas o upload via Wi-Fi pelo PlatformIO não foi configurado ou verificado nesta reorganização.

## Próximo passo

Conferir a tabela do Supabase conforme o [README](README.md#envio-ao-supabase). Depois, gravar `uno_r4_wifi` por USB e conferir o pulso do relé, as leituras no Monitor Serial a 9600 baud e os POSTs individuais.

## Evidências e cobertura do histórico

- [Registro da modularização](history/2026/2026-11-23-firmware-modularization.md).
- [Agrupamento por área](history/2026/2026-11-23-module-folders.md).
- [Peixe na matriz integrada](history/2026/2026-11-23-led-matrix-fish.md).
- [Configuração privada por .env](history/2026/2026-11-23-private-env-config.md).
- [Lista de redes Wi-Fi](history/2026/2026-11-23-wifi-network-list.md).
- [Teste de bancada do módulo de pH](history/2026/2026-11-25-ph-bench-voltage.md).
- [Integração dos três sensores](history/2026/2026-11-25-analog-sensor-integration.md).
- [Rotas HTTP das medições](history/2026/2026-11-25-measurement-http-routes.md).
- [Alimentação temporizada do TDS](history/2026/2026-11-25-tds-power-cycle.md).
- [Acionamento do relé do TDS](history/2026/2026-11-25-tds-relay-control.md).
- [Intervalo de teste do TDS](history/2026/2026-11-25-tds-test-interval.md).
- [Faixa de alerta do TDS](history/2026/2026-11-26-tds-alert-range.md).
- [Consultas individuais na página local](history/2026/2026-11-26-measurement-dashboard.md).
- [Envio individual ao Supabase](history/2026/2026-11-26-individual-telemetry.md).
- [Intervalos das medições](history/2026/2026-11-26-sensor-intervals.md).
- [Ponto de entrada do firmware](history/2026/2026-11-26-firmware-entry-point.md).
- [Organização das configurações e da bancada](history/2026/2026-11-26-firmware-organization.md).
- [Logs de medições e envios](history/2026/2026-11-26-serial-measurement-logs.md).
- [Limite inferior de temperatura](history/2026/2026-11-26-temperature-alert-minimum.md).
- Código em `src/` e interfaces em `include/`.
- O histórico local começa com esta reorganização; não representa uma migração completa dos acontecimentos anteriores.

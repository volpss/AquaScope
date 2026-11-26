# Roteiro por tópicos — Aquário IoT

## Integrante 1 — Abertura e objetivo

- Apresentar o Aquário IoT e a proposta de acompanhar as condições da água ao longo do tempo.
- Explicar a situação que motivou o trabalho. Uma medição manual mostra como a água está naquele momento. Com leituras periódicas, dá para acompanhar as mudanças e perceber quando algum valor sai da faixa configurada.
- Dar uma visão rápida do protótipo. A placa recebe as leituras, sinaliza alertas e disponibiliza os dados numa página.

## Integrante 2 — O que é medido

- Apresentar os três parâmetros. Temperatura em graus Celsius, pH como indicação de acidez ou alcalinidade e TDS como estimativa de sólidos dissolvidos em partes por milhão.
- Dar um exemplo de leitura e alerta. Se a temperatura estiver em 22 °C e o mínimo configurado for 23 °C, aparece um aviso de temperatura baixa.
- Passar para o integrante 3 explicar como os sensores chegam a esses valores.

## Integrante 3 — Hardware, sensores e montagem

- Apresentar o Arduino Uno R4 WiFi e as ligações principais. O termistor NTC de temperatura usa a entrada A0, o módulo de pH usa A1 e o módulo de TDS usa A2.
- Explicar a leitura analógica de forma simples. O Arduino lê uma tensão e o firmware converte esse sinal em graus Celsius, pH ou ppm.
- Explicar o que muda de um sensor para o outro.
  - **Temperatura** — A resistência do NTC varia com a temperatura e, com isso, muda a tensão lida pelo Arduino.
  - **pH** — O módulo recebe o sinal da sonda. O ajuste inicial de bancada foi feito em 2,5 V. A calibração com soluções de pH conhecido ainda é uma etapa a concluir.
  - **TDS** — A estimativa usa o sinal do módulo e uma compensação pela temperatura.
- Detalhar o relé do TDS. A placa liga o módulo por 3 segundos, espera 2,5 segundos antes de começar a coletar amostras e pausa a coleta de pH durante o pulso. Explicar que a separação reduz a chance de interferência e que a comparação entre as leituras ainda precisa ser feita na placa.
- Mostrar como o usuário é avisado. Tem um LED por sensor, um buzzer para os alertas e um botão para silenciar temporariamente o som.

## Integrante 4 — Código, página e caminho dos dados

- Explicar o papel do `main.cpp`. Ele coordena os módulos de sensores, indicadores, rede e telemetria.
- Acompanhar uma leitura de temperatura como exemplo. O firmware coleta as amostras, calcula o valor, compara com os limites e registra a nova medição. Cada sensor tem seu ritmo. Temperatura a cada 3 segundos, pH a cada 45 segundos e TDS a cada 15 minutos no modo normal.
- Explicar a **página local**. Cada cartão consulta a rota do seu sensor. O cartão de TDS mostra a última leitura e uma estimativa de quanto falta para a próxima.
- Explicar o **Supabase**. O firmware prepara um POST por medição válida e usa o mesmo endereço da tabela para os três sensores. Cada envio leva só o sensor que foi medido e seu estado de alerta. Por exemplo, uma nova leitura de temperatura segue sem repetir o último valor de TDS.
- Explicar a **fila offline** com um exemplo. Se o Wi-Fi cair, as novas medições ficam na memória da placa. Quando a conexão volta, o firmware tenta enviar primeiro os registros mais antigos e retira cada um depois da confirmação do servidor. Se a placa for reiniciada, os registros pendentes na RAM se perdem.
- Comentar como acompanhar os envios. O Monitor Serial mostra o sensor que acabou de medir e, nos envios, o JSON e o resultado HTTP.

## Integrante 5 — Testes realizados e pendentes

- Mencionar que o firmware completo e o programa de bancada de pH compilaram. Os testes automatizados do carregamento de configurações também passaram.
- Explicar o que ainda precisa ser validado na montagem. Conferir os sensores funcionando juntos, os avisos e a atualização da página.
- Citar um teste de ponta a ponta. Derrubar o Wi-Fi, aguardar novas medições, reconectar e conferir se os registros chegam ao Supabase. O ajuste da tabela para receber medições individuais está previsto para depois.

## Integrante 6 — Fechamento

- Retomar o objetivo em uma frase. O projeto reúne leituras da água, avisos locais e consulta pelo navegador.
- Destacar o que ainda falta para usar o monitoramento com mais confiança. Calibrar as sondas, conferir as faixas de alerta e validar o envio ao banco.
- Agradecer e abrir para perguntas.

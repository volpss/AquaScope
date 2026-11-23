---
date: "2026-11-23"
type: "change"
topics: ["firmware","hardware","matriz LED"]
summary: "Adicionado peixe estático na matriz integrada da Uno R4 WiFi."
---

A pedido do usuário, foi criado o módulo [hardware/led_matrix.cpp](../../src/hardware/led_matrix.cpp), com interface em [led_matrix.h](../../include/hardware/led_matrix.h). O módulo usa a biblioteca Arduino_LED_Matrix já incluída no framework da placa e desenha um peixe voltado para a direita, com bitmap de 8 linhas por 12 colunas.

A função iniciarMatrizLed é chamada no setup, antes da conexão Wi-Fi. O desenho é estático e não adiciona chamadas ao loop nem pausas com delay. O README foi atualizado com o módulo e a forma de editar os pixels.

A compilação passou no ambiente uno_r4_wifi: 15.968 bytes de RAM e 73.872 bytes de Flash. Não houve upload nem observação da matriz física; falta conferir a aparência na placa. A API utilizada foi conferida no cabeçalho instalado e na documentação oficial da matriz da Uno R4 WiFi.

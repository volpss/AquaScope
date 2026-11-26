# Aquario IoT

Projeto acadêmico de monitoramento de aquário com Arduino Uno R4 WiFi, framework Arduino e PlatformIO.

## Organização do código

Os arquivos `.h` em `include/` declaram as interfaces dos módulos. Os arquivos `.cpp` em `src/` implementam essas interfaces. O PlatformIO compila os fontes dessa pasta automaticamente; não é necessário incluir um `.cpp` em outro nem alterar o `platformio.ini` ao adicionar um módulo.

Os módulos de `hardware/`, `network/` e `data/` têm interfaces nas pastas correspondentes em `include/`.

```text
src/
├── main.cpp
├── benches/
│   └── ph-bench.cpp
├── hardware/
│   ├── sensors.cpp
│   ├── indicators.cpp
│   └── led_matrix.cpp
├── network/
│   ├── connectivity.cpp
│   ├── web_server.cpp
│   └── clock.cpp
└── data/
    ├── database.cpp
    ├── offline_queue.cpp
    └── telemetry.cpp
```

Os sketches de bancada ficam em `src/benches/`, com ambientes próprios no PlatformIO. Os testes automatizados ficam em `test/`. As configurações compartilhadas ficam agrupadas em `include/config.h` por sensor ou responsabilidade, e os modelos de leitura e de telemetria ficam em `include/models.h`.

| Arquivo | Responsabilidade |
| --- | --- |
| [src/main.cpp](src/main.cpp) | Inicialização e ciclo do firmware completo no ambiente padrão `uno_r4_wifi`. |
| [src/benches/ph-bench.cpp](src/benches/ph-bench.cpp) | Sketch de bancada de pH no ambiente `uno_r4_wifi_ph`. |
| [include/config.h](include/config.h) | Pinos, limites e intervalos. |
| [.env.example](.env.example) | Exemplo das variáveis privadas para configurar no `.env` local. |
| [scripts/load_env.py](scripts/load_env.py) | Hook de compilação que carrega o `.env` e disponibiliza o cabeçalho privado. |
| [include/models.h](include/models.h) | Estados dos sensores, leituras atuais e medições pendentes. |
| [src/hardware/sensors.cpp](src/hardware/sensors.cpp) | Amostragem, filtragem e conversão dos sensores, com controle de alimentação do TDS. |
| [src/hardware/indicators.cpp](src/hardware/indicators.cpp) | LEDs, buzzer, botão e silêncio temporário do alarme. |
| [src/hardware/led_matrix.cpp](src/hardware/led_matrix.cpp) | Peixe na matriz de LEDs integrada de 12 × 8. |
| [src/network/connectivity.cpp](src/network/connectivity.cpp) | Conexão e reconexão Wi-Fi e atendimento OTA. |
| [src/network/clock.cpp](src/network/clock.cpp) | NTP, manutenção do horário durante quedas e formatação de datas. |
| [src/data/database.cpp](src/data/database.cpp) | JSON e envio HTTPS à API REST do Supabase. |
| [src/data/offline_queue.cpp](src/data/offline_queue.cpp) | Fila circular de leituras em RAM. |
| [src/data/telemetry.cpp](src/data/telemetry.cpp) | Registro de novas medições na fila e envio individual ao Supabase. |
| [src/network/web_server.cpp](src/network/web_server.cpp) | Página HTTP local e rotas JSON das medições. |

Cada módulo tem seu cabeçalho de mesmo nome na subpasta correspondente de `include/`. Por exemplo, `src/hardware/sensors.cpp` usa `include/hardware/sensors.h`, referenciado com `#include "hardware/sensors.h"`. As variáveis internas ficam restritas ao respectivo `.cpp`; os outros módulos acessam funções declaradas no `.h`. As leituras dos sensores são expostas por uma referência somente para leitura.

`lib/` fica disponível para bibliotecas próprias reutilizáveis. Os módulos específicos deste programa ficam em `src/`. `.pio/` contém dependências e resultados de compilação gerenciados pelo PlatformIO.

## Funcionamento atual

O ciclo principal atende Wi-Fi, OTA, página local e alarmes. Temperatura e pH usam janelas independentes de 30 amostras, com pelo menos 10 ms entre amostras do mesmo sensor. O firmware inicia uma janela de temperatura a cada 3 segundos e uma de pH a cada 45 segundos. As duas coletas pausam durante o pulso do TDS e reiniciam as janelas interrompidas depois do pulso.

Na inicialização, a matriz integrada exibe um peixe estático. O desenho fica em `desenhoPeixe`, no módulo `hardware/led_matrix.cpp`: cada `1` acende um LED e cada `0` o apaga. A biblioteca `Arduino_LED_Matrix` já acompanha o framework da placa; não há dependência adicional no `platformio.ini` nem chamadas extras no `loop()`.

A fila comporta 400 medições individuais e sobrescreve a mais antiga quando cheia. Com conexão, o programa envia da mais antiga para a mais recente e aguarda pelo menos um segundo após cada tentativa. Uma medição permanece na fila até o Supabase confirmar o envio.

A fila fica apenas na RAM: desligar ou reiniciar a placa perde os registros pendentes.

| Entrada | Ligação | Conversão |
| --- | --- | --- |
| `A0` | `5V` → resistor de `10 kΩ` → `A0` → NTC → `GND` | Temperatura em °C. |
| `A1` | `PO` do PH-4502C, com GND comum e a sonda no BNC | pH estimado com ponto neutro de 2,500 V e inclinação provisória de −5,70 pH/V. |
| `A2` | Saída analógica do módulo de TDS, com GND comum | TDS estimado em ppm, compensado pela temperatura de `A0` com a [curva do Gravity TDS](https://wiki.dfrobot.com/sen0244/docs/20305). |

O ajuste de 2,500 V foi feito com o BNC em curto. Retire o jumper e conecte a sonda para medir a água. O pH tem precisão pendente de calibração com soluções tampão. Se algum sensor estiver sem leitura válida, a página local mostra erro nesse canal e o conjunto fica fora da telemetria.

### Alimentação e calibração do TDS

Com o Arduino desligado, faça as ligações abaixo com um módulo relé de 5 V acionado em nível baixo. Se o relé tiver seletor `H/L`, use `L`. A corrente do TDS vem do pino `5V` do Uno e passa pelos contatos `COM` e `NO`.

A ligação direta entre `D7` e `VCC` do TDS pode ultrapassar o [limite de 8 mA do GPIO](https://docs.arduino.cc/hardware/uno-r4-wifi/) quando os [capacitores de alimentação do módulo](https://dfimg.dfrobot.com/wiki/20205/SEN0244_analog-tds-sensor_schematics_V1.0.pdf) carregam. O valor especificado de 3 a 6 mA refere-se à corrente de funcionamento.

| Ponto | Ligação |
| --- | --- |
| `IN` do relé | `D7` do Arduino |
| `VCC` e `COM` do relé | `5V` do Arduino |
| `NO` do relé | `+` do módulo TDS |
| `NC` do relé | Sem ligação |
| `GND` do relé e `-` do TDS | `GND` do Arduino |
| `A` do TDS | `A2` do Arduino |

O firmware configura `D7` como saída em `HIGH` para manter o relé desligado. Durante a medição, comanda o relé com `LOW`. O primeiro pulso ocorre após a primeira leitura de temperatura e pH. Os pulsos seguintes são agendados por `millis()`, inclusive sem Wi-Fi. Em cada pulso, o firmware aguarda 2,5 segundos, coleta 30 amostras em `A2` e desliga aos 3 segundos. O atendimento de rede e as leituras de pH aguardam o fim do pulso.

O modo normal está ativo com `USAR_INTERVALO_TESTE_TDS` em `false` em [include/config.h](include/config.h). O primeiro pulso começa após a primeira leitura de temperatura e os seguintes são agendados a cada 15 minutos. Para testar a comutação a cada 30 segundos, use `true` temporariamente e grave o firmware.

O Monitor Serial mostra uma linha `[MEDICAO] TDS` ao concluir cada pulso. Entre pulsos, a página e a API mantêm o último resultado de TDS. Confira com um multímetro se o `VCC` do módulo sobe para perto de `5V` durante o pulso e cai depois.

Para conferir a precisão, coloque a sonda e o NTC na mesma solução de referência de [707 ppm a 25 °C](https://wiki.dfrobot.com/sen0244/docs/20304) e aguarde um ciclo. Se a leitura estiver estável e precisar de correção, ajuste `FATOR_CALIBRACAO_TDS` em [include/config.h](include/config.h) para `707 / valor exibido` a partir do fator inicial `1.0`. Grave novamente e confira o resultado na mesma solução.

### API HTTP local

Use o IP da placa mostrado no Monitor Serial. A rota `GET /` abre a página HTML. A página consulta as rotas abaixo em sequência, com uma pausa de três segundos entre rodadas, e atualiza cada cartão na própria página.

| Rota | Medição | Unidade |
| --- | --- | --- |
| `GET /api/temperatura` | Temperatura do NTC em `A0` | `°C` |
| `GET /api/ph` | pH em `A1` | `pH` |
| `GET /api/tds` | TDS em `A2` | `ppm` |

Exemplo de resposta de `GET /api/temperatura`:

```json
{"valor":27.1,"unidade":"°C","estado":"normal"}
```

O campo `estado` pode ser `normal`, `abaixo_limite`, `acima_limite` ou `sem_leitura`. A rota retorna HTTP `503` e `"valor":null` quando a medição não está disponível. Os limites de alerta não impedem a consulta de valores válidos.

`GET /api/tds` também retorna `idade_ms` (idade da leitura em milissegundos, ou `null` quando indisponível), `intervalo_ms` (intervalo configurado entre inícios de ciclo) e `proxima_em_ms` (tempo estimado até o fim do próximo pulso). A página usa o relógio do navegador e a idade da leitura para exibir o horário da última atualização. A contagem regressiva pode chegar a zero enquanto a placa conclui o ciclo ou atende outra tarefa.

`GET /api/fila` retorna a quantidade atual de registros pendentes na RAM no campo `pendentes`.

### Envio ao Supabase

Cada leitura válida entra na fila assim que é concluída, na cadência de 3 segundos para temperatura, 45 segundos para pH e 15 minutos para TDS no modo normal. Os intervalos ficam em [include/config.h](include/config.h). O ciclo do TDS e o atendimento de rede podem atrasar a amostragem. Uma fila acumulada ou uma falha de conexão pode adiar a entrega ao Supabase.

O Monitor Serial registra uma linha `[MEDICAO]` por leitura concluída, com o valor ou erro do sensor. A linha `[SUPABASE] POST` mostra o JSON usado como corpo da requisição e o código HTTP retornado. Uma falha de conexão antes do POST ou a falta de uma resposta HTTP aparece como mensagem de falha.

Cada POST para `SUPABASE_CAMINHO` contém um objeto com o campo do sensor, seu `alerta_ativo` e `created_at` quando o horário da placa é conhecido. Antes do upload, confira se a tabela aceita linhas com apenas uma das três colunas de medição preenchida. Se for preciso ajustar a tabela, substitua `sua_tabela` pelo nome usado em `SUPABASE_CAMINHO` e execute no SQL Editor do Supabase.

```sql
alter table public.sua_tabela
  alter column temperatura drop not null,
  alter column temperatura drop default,
  alter column ph drop not null,
  alter column ph drop default,
  alter column tds drop not null,
  alter column tds drop default;
```

O ajuste preserva as linhas antigas. Se houver outras restrições de inserção ou políticas RLS na tabela, confira se permitem o mesmo formato. Para consultar o último valor de cada sensor em um painel externo, filtre a coluna correspondente por valores preenchidos antes de ordenar por `created_at`. Assim, as consultas incluem tanto as linhas antigas com três valores quanto as novas linhas individuais.

## Compilar e enviar

Abra esta pasta no VS Code com a extensão PlatformIO IDE. O ambiente `uno_r4_wifi` está configurado em [platformio.ini](platformio.ini), com as dependências declaradas para instalação pelo PlatformIO.

Na primeira configuração de uma cópia do projeto, copie `.env.example` para `.env` na raiz e preencha os valores:

```powershell
Copy-Item .env.example .env
```

O `.env` local guarda `WIFI_REDES`, `OTA_NOME`, `OTA_SENHA`, `SUPABASE_HOST`, `SUPABASE_CAMINHO` e `SUPABASE_CHAVE`. O host do Supabase deve ser informado sem `https://` e sem barra final. Use a chave pública destinada ao dispositivo.

Configure as redes como uma lista JSON em uma única linha, na ordem desejada das tentativas:

```dotenv
WIFI_REDES='[{"nome":"RedeCasa","senha":"SENHA_CASA"},{"nome":"RedeFaculdade","senha":"SENHA_FACULDADE"}]'
```

Cada rede contém `nome` e `senha`; use `"senha":""` para uma rede aberta. Para adicionar outra rede, acrescente outro objeto à lista, separado por vírgula. É obrigatório configurar pelo menos uma rede. Para um apóstrofo dentro desse JSON entre aspas simples, use `\u0027`.

A placa começa pela primeira rede. Se ainda não tiver conexão e IP após o intervalo configurado (15 segundos), tenta a próxima; depois da última, volta à primeira. O intervalo é contado após o retorno de `WiFi.begin()`, que também pode levar algum tempo. Enquanto estiver conectada, permanece na rede atual. Se a conexão cair, tenta recuperar essa rede primeiro e depois retoma a alternância. O Monitor informa somente o número da rede tentada, sem imprimir nome ou senha.

O carregador aceita linhas `CHAVE=valor`, comentários com `#` e valores entre aspas simples ou duplas. Nas aspas duplas, use `\"` para aspas e `\\` para barras invertidas. Não há expansão de variáveis como `$VAR`. Todas as seis variáveis são obrigatórias e precisam estar preenchidas; senhas vazias são permitidas dentro da lista de redes abertas.

O PlatformIO lê o arquivo antes de compilar e gera `.pio/build/<ambiente>/generated/private_config.h`. Se faltar o `.env` ou houver valores ausentes ou inválidos, a compilação informa o problema sem imprimir as credenciais. Depois de mudar o `.env`, faça Build/Upload novamente para aplicar os novos dados à placa.

O `.env` e os arquivos em `.pio/` são ignorados pelo Git; `.env.example` pode ser versionado. Os valores continuam incorporados ao firmware gravado na placa: o `.env` separa os dados privados dos fontes e do versionamento, sem criptografar o firmware.

- **Build:** compila sem precisar da placa conectada.
- **Upload:** compila e envia pela USB para a placa conectada.
- **Monitor:** acompanha a saída serial em 9600 baud.

No terminal do PlatformIO, os comandos equivalentes são:

```powershell
pio run
pio run --target upload
pio device monitor
```

O serviço OTA foi mantido no firmware; o envio pelo PlatformIO está preparado para USB. A compilação não comprova que uma atualização por Wi-Fi foi realizada.

Para conferir a organização na placa, faça Upload e confira as leituras, a página local e o envio ao Supabase. Depois interrompa o Wi-Fi sem reiniciar a placa, aguarde novas medições e restaure a conexão para conferir os POSTs dos registros pendentes no Monitor Serial.

Os testes do carregador podem ser executados sem conectar a placa:

```powershell
python -m unittest discover -s test -p test_env_config.py
```

## Bancada de pH

O ambiente `uno_r4_wifi_ph` lê a saída `PO` do módulo em `A1` e mostra o ADC médio e a tensão estimada no Monitor Serial. Ele compila sem `.env`.

```sh
pio run -e uno_r4_wifi_ph -t upload
pio device monitor -b 9600
```

Para retomar o monitoramento do aquário, grave o ambiente padrão `uno_r4_wifi`.

## Registros do projeto

- [Identidade e escopo](project.md)
- [Situação atual](status.md)
- [Histórico](history/README.md)

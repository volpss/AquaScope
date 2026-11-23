# Aquario IoT

Projeto acadêmico de monitoramento de aquário com Arduino Uno R4 WiFi, framework Arduino e PlatformIO.

## Organização do código

Os arquivos `.h` em `include/` declaram as interfaces dos módulos. Os arquivos `.cpp` em `src/` implementam essas interfaces. O PlatformIO compila os fontes dessa pasta automaticamente; não é necessário incluir um `.cpp` em outro nem alterar o `platformio.ini` ao adicionar um módulo.

As subpastas agrupam os módulos por área e são espelhadas em `include/`:

```text
src/
├── main.cpp
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

`hardware/` reúne a interação com os componentes físicos; `network/`, os serviços de rede e o relógio sincronizado por NTP; `data/`, o armazenamento e o envio das leituras. As configurações e os modelos compartilhados permanecem em `include/config.h` e `include/models.h`.

| Arquivo | Responsabilidade |
| --- | --- |
| [src/main.cpp](src/main.cpp) | `setup()`, `loop()` e agendamento das tarefas. |
| [include/config.h](include/config.h) | Pinos, limites, intervalos e inclusão da configuração privada gerada. |
| [.env.example](.env.example) | Exemplo das variáveis privadas para configurar no `.env` local. |
| [scripts/load_env.py](scripts/load_env.py) | Hook de compilação que carrega o `.env` e disponibiliza o cabeçalho privado. |
| [include/models.h](include/models.h) | Estruturas de leitura e estados dos sensores. |
| [src/hardware/sensors.cpp](src/hardware/sensors.cpp) | Leitura analógica, conversões e classificação das leituras. |
| [src/hardware/indicators.cpp](src/hardware/indicators.cpp) | LEDs, buzzer, botão e silêncio temporário do alarme. |
| [src/hardware/led_matrix.cpp](src/hardware/led_matrix.cpp) | Peixe na matriz de LEDs integrada de 12 × 8. |
| [src/network/connectivity.cpp](src/network/connectivity.cpp) | Conexão e reconexão Wi-Fi e atendimento OTA. |
| [src/network/clock.cpp](src/network/clock.cpp) | NTP, manutenção do horário durante quedas e formatação de datas. |
| [src/data/database.cpp](src/data/database.cpp) | JSON e envio HTTPS à API REST do Supabase. |
| [src/data/offline_queue.cpp](src/data/offline_queue.cpp) | Fila circular de leituras em RAM. |
| [src/data/telemetry.cpp](src/data/telemetry.cpp) | Envio da leitura atual, armazenamento em caso de falha e sincronização da fila. |
| [src/network/web_server.cpp](src/network/web_server.cpp) | Página HTTP local de monitoramento. |

Cada módulo tem seu cabeçalho de mesmo nome na subpasta correspondente de `include/`. Por exemplo, `src/hardware/sensors.cpp` usa `include/hardware/sensors.h`, referenciado com `#include "hardware/sensors.h"`. As variáveis internas ficam restritas ao respectivo `.cpp`; os outros módulos acessam funções declaradas no `.h`. As leituras dos sensores são expostas por uma referência somente para leitura.

`lib/` fica disponível para bibliotecas próprias reutilizáveis. Os módulos específicos deste programa ficam em `src/`. `.pio/` contém dependências e resultados de compilação gerenciados pelo PlatformIO.

## Funcionamento atual

O ciclo principal atende Wi-Fi, OTA, página local e alarmes. Os sensores são lidos a cada 2 segundos. A cada 15 segundos, o programa tenta enviar a leitura atual; sem conexão ou se o envio falhar, ela entra na fila offline.

Na inicialização, a matriz integrada exibe um peixe estático. O desenho fica em `desenhoPeixe`, no módulo `hardware/led_matrix.cpp`: cada `1` acende um LED e cada `0` o apaga. A biblioteca `Arduino_LED_Matrix` já acompanha o framework da placa; não há dependência adicional no `platformio.ini` nem chamadas extras no `loop()`.

A fila comporta 400 registros e sobrescreve o mais antigo quando cheia. Com conexão, o programa tenta sincronizar até 50 registros por lote, em intervalos de pelo menos 1 segundo, removendo-os somente após a confirmação do envio. Requisições de rede podem prolongar esses intervalos.

A fila fica apenas na RAM: desligar ou reiniciar a placa perde os registros pendentes. As conversões atuais de pH e TDS usam temperaturas dos canais A1 e A2 como simulação; a separação em módulos preserva esse comportamento.

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

Para conferir a organização na placa, faça Upload e confira as leituras, a página local e o envio ao Supabase. Depois interrompa o Wi-Fi sem reiniciar a placa, observe o aumento da fila no Monitor e restaure a conexão para conferir o envio dos registros pendentes.

Os testes do carregador podem ser executados sem conectar a placa:

```powershell
python -m unittest discover -s test -p test_env_config.py
```

## Registros do projeto

- [Identidade e escopo](project.md)
- [Situação atual](status.md)
- [Histórico](history/README.md)

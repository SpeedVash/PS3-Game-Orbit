# PS3 Game Orbit 1.6 — Teste 10

Corrige um erro concreto na gravação dos arquivos do módulo e encurta as
mensagens, colocando os códigos no início. A validação local reproduziu a
falta de registro com as flags antigas e completou três consultas e STOP
com a correção. A execução no PS3 ainda precisa ser confirmada.

## Teste no console

1. Reinicie o PS3 antes de instalar e executar o teste. No HEN, ative-o.
2. Instale o PKG **Teste 10** e abra o Orbit. O ID continua `PGORBT301`.
3. Aguarde a biblioteca aparecer.
4. Abra **START → Teste do serviço → Testar serviço do Orbit**.
5. Aguarde a mensagem final, até cerca de 15 segundos.
6. Sucesso esperado: **“D10-OK: 3 respostas; teste encerrado.”**
7. Saia normalmente e informe se voltou ao XMB.

Comece pelo CFW. Se funcionar, repita no HEN após reiniciar e ativá-lo.
Se falhar, envie a mensagem completa. Agora o código aparece primeiro.
Não precisa repetir o Teste 09. Reinicie antes de repetir o Teste 10, pois
o módulo permanece no slot até o reinício.

O log do app fica em `/dev_hdd0/tmp/PS3_GAME_ORBIT_TESTE10.log`, com alternativa
no `USRDIR`. Para D10-E1, D10-W0 ou D10-W1, esse log ajuda a analisar os
retornos e os endereços do descritor da tarefa.

## Erro encontrado

Os Testes 08 e 09 abriam arquivos nativos com o valor `0x601`. No SDK
PSL1GHT fixado, esse valor combina WRONLY, TRUNC e APPEND e não inclui
`SYS_O_CREAT`. Um arquivo ausente não era criado. Isso afetava tanto o
READY quanto os novos registros de entrada/worker do Teste 09.

O plugin mínimo do Teste 07 usava as constantes corretas do SDK. O Teste 10
passa a usar `SYS_O_WRONLY | SYS_O_CREAT | SYS_O_TRUNC` nos dois caminhos
de gravação do módulo: o registro simples e o transporte atômico compartilhado
com o serviço. No SDK fixado, a combinação é `0x241`.

A validação host anterior também tinha um erro: tratava qualquer abertura
de escrita como CREATE/TRUNC. Isso mascarava a ausência da flag CREATE.
Agora a tradução de cada flag LV2 para a operação POSIX é feita separadamente,
e as constantes do modelo são comparadas com o header real do SDK antes
da compilação nativa. O mock do dispatcher completo também foi corrigido.

O teste de regressão usa o módulo C e o cliente C++ reais: força o valor
antigo, confirma que não há arquivo de entrada e observa D10-E0; depois
usa as constantes corrigidas e confirma três STATUS, ACK de STOP e DONE.
Também verifica criação de arquivo ausente, preservação de tamanho sem TRUNC
e truncamento quando solicitado.

O resultado relatado no CFW foi a frase de D09-E0, cortada no final.
A frase tinha 509 pixels na fonte real do HUD, cujo limite era 492.
As 19 novas mensagens de diagnóstico têm o código no início e até 311 pixels
nos valores verificados, incluindo códigos de erro completos.

Isso identifica e corrige um defeito de código que explica a ausência dos
arquivos; ainda não confirma que todas as etapas passarão nos consoles.

## Mensagens curtas

| Código | Significado |
| --- | --- |
| D10-OK | Três respostas recebidas e fim do diagnóstico confirmado. |
| D10-E0 | Sem registro válido da entrada do módulo. |
| D10-E1 | A entrada confirmou execução, mas não registrou o resultado da criação. |
| D10-C | Erro retornado pela criação da tarefa, com hexadecimal. |
| D10-S | Erro retornado pela partida da tarefa, com hexadecimal. |
| D10-W0 | Criação e partida retornaram zero; sem confirmação do worker. |
| D10-W1 | Worker confirmou execução; comunicação não foi confirmada. |
| D10-F | Falha de arquivo; mostra a operação FS e o hexadecimal. |
| D10-Q1/Q2/Q3 | Falha na consulta indicada. |
| D10-T | Sem confirmação de término. |

Para D10-F: FS2 abertura, FS3 gravação, FS4 sincronização, FS5 fechamento,
FS6 primeira tentativa de renomear e FS8 commit após backup. FS1 indica
caminho longo. `0xFFFFFFFE` indica gravação curta detectada pelo módulo.
Ausência de registro, isoladamente, não prova que a função não executou.

## Escopo preservado

Montagem, catálogo NTFS e controle da ventoinha continuam suspensos.
O STATUS é fixo, sem leitura de temperatura ou alteração de configurações.
CONFIGURE, MOUNT_NATIVE, MOUNT_NTFS e RESTORE_SYSCON recebem `0x160A`.
O módulo não contém adaptadores de Cobra, storage, fan, sensores, peek/poke.

O carregamento continua manual, depois da biblioteca. Há uma única tarefa
com stack de 64 KiB, prioridade 1700, helper 52/53, OPD32 e TLS zero.
O prazo da tarefa sem cliente é de 60 segundos, sujeito ao retorno das
operações do sistema. O cliente usa esperas de até 3 segundos por etapa.
Em falha/cancelamento, tenta enviar STOP.

O único SPRX incluído é `orbit_service10.sprx` (`OrbitService10`). O aplicativo
procura slot livre entre 1 e 6, preserva os ocupados e não descarrega outros
plugins. Não altera `boot_plugins.txt`.

O canal é `/dev_hdd0/tmp/PS3_GAME_ORBIT_SERVICE10` com `.entry`, `.worker`,
`.req`, `.reply`, `.ready`, `.done` e temporários. Os registros simples
usam open/write/close e validam sessão e checksum. Não usam o canal ou as
configurações do serviço completo.

DONE é escrito antes da saída da tarefa; não é um join feito pelo app.
O stop exportado do módulo faz o join. O reinício libera o módulo de teste.
O serviço completo continua disponível como fonte, mas seu SPRX não vai no PKG.

## Validação e compilação

Foram executados os casos de flags e regressão acima, três consultas/STOP,
erros de criação/partida, falhas de open/write/fsync/close/rename, tokens
antigos/corrompidos, comandos bloqueados, deadline e join. Os testes do
dispatcher usam APIs de hardware simuladas; não comprovam montagem ou fan
no console. Também foram conferidos os 37 cenários de acionamento manual
no loop do app e os bloqueios do serviço antigo/NTFS.

A compilação verifica flags contra o header real do SDK, largura das
mensagens na fonte real, todos os OPD64/OPD32 incluindo o worker, relocations,
exports, assinatura decodificada e HMAC. O PKG final é extraído e comparado
com os arquivos compilados. Esses testes locais não emulam o PS3.

Use o GCC/G++ 7.5.0 e o SDK do checkpoint anterior. Defina `PS3DEV`,
`ORBIT_SCETOOL` e `ORBIT_SCETOOL_DATA`. SCETool e seus dados externos não
são redistribuídos. O antigo signer SPRX não é usado para este módulo.

Prepare o modelo JFX original separado com
`python3 scripts/prepare-jfx.py --source /caminho/JFX_DVDBlu.zip`, depois
execute `bash scripts/build-test10.sh`. O modelo editável e o include gerado
não são redistribuídos, como nas entregas anteriores.

Testes: `bash tests/run_service10.sh`, `bash tests/run_service_dispatch_fix48.sh`,
`bash tests/run_diagnostic03.sh`, `python3 scripts/check-library-loop-probe04.py`,
`python3 tests/check_service10_messages.py` e
`python3 tests/check_service10_flags.py "$PSL1GHT/ppu/include/lv2/sysfs.h"`.
Documentação e scripts de versões anteriores são históricos; o ponto de
entrada desta entrega é `build-test10.sh`.

**Teste 10 no hardware: pendente do resultado do usuário.**

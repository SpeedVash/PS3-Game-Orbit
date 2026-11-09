# PS3 Game Orbit 1.6 — Teste 12

Corrige o relógio nativo do serviço e acrescenta rolagem lateral às mensagens longas do menu.

O log do Teste 11 no CFW mostra criação/partida com retorno zero, worker C no estágio 3, r2 recebido e esperado iguais a `008E8000`, e pilha inicial `D00FFF90`. Portanto o C executou. A mensagem D11-B1 foi imprecisa porque o aplicativo não encontrou um registro C válido para a sessão, embora tenha conseguido ler seus campos.

Foi encontrado um defeito concreto: `orbit48_time()` chamava a syscall 145 sem argumentos. Essa chamada recebe dois ponteiros de saída para segundos/nanossegundos e retorna um código de resultado; não retorna um relógio em microssegundos. O SDK fixado confirma a assinatura em `ppu/include/sys/systime.h`.

A execução do código PPC64 com essa ABI simulada reproduziu READY corrompido, escrita em `session+15`, nonce do registro C incorreto e worker no estágio 3. É uma reprodução local compatível com o log; os bytes corrompidos do console não estavam registrados no Teste 11.

## Testar no CFW

1. Reinicie o PS3 para liberar o módulo anterior.
2. Instale o PKG **Teste 12** e abra o Orbit (`PGORBT301`).
3. Aguarde a biblioteca e abra **START → Teste do serviço → Testar serviço do Orbit**.
4. Aguarde a mensagem final. Resultado esperado: **“D12-OK: 3 respostas; teste encerrado.”**
5. Informe a mensagem e se conseguiu sair normalmente para o XMB.

Se falhar, envie `/dev_hdd0/tmp/PS3_GAME_ORBIT_TESTE12.log`. Há uma alternativa no `USRDIR` do aplicativo. O novo log inclui o nonce esperado, tokens READY/DONE e bytes/checksums dos registros C. Priorize o CFW antes de repetir no HEN.

## Alterações desta revisão

- O relógio lê o timebase monotônico do PPU (`mftb`) e converte ticks para microssegundos com a frequência em Hz informada por `sysGetTimebaseFrequency()` do SDK (syscall 147). A frequência, sozinha, não é um timestamp. A fórmula usa quociente/resto para evitar multiplicar o contador inteiro por um milhão.
- A implementação é compartilhada com o adaptador nativo do serviço completo, que continua desativado neste PKG. A ponte assembly, TOC explícito, stack de 64 KiB, prioridade de diagnóstico 0 e helper 52/53 do Teste 11 permanecem.
- Um registro C presente com nonce/checksum inválidos recebe **D12-WV**. Um registro C válido no estágio 3 sem READY confirmado recebe **D12-R0**. Assim o diagnóstico diferencia essas situações da ausência de registro C.
- Mensagens de status do menu com mais de 492 pixels deslizam para a esquerda a 36 pixels por segundo. Há pausas de 1,2 segundo no começo e no final; depois a leitura recomeça. Mensagens curtas ficam paradas. Alterar a mensagem, trocar uma opção ou reabrir o menu reinicia a leitura.
- A rolagem respeita UTF-8 e recorta os dois lados da faixa. Atualiza somente a região de 492×26 pixels do HUD quando muda um pixel, mantendo o restante da textura em cache. O tempo vem do relógio que o laço principal já utiliza.
- As 23 mensagens de diagnóstico continuam começando pelo código e cabem inteiras na largura normal do menu. A rolagem atende também outros status longos do menu.

## Códigos principais

| Código | Significado |
| --- | --- |
| D12-OK | Três STATUS, ACK do STOP e DONE da sessão recebidos. |
| D12-WV | Arquivo de registro C com nonce ou checksum inválido. |
| D12-R0 | Registro C válido no estágio 3; READY não confirmado. |
| D12-W1 | Registro C válido anterior à comunicação. |
| D12-B0 / D12-B1 | Sem marca assembly válida / marca assembly válida sem registro C válido disponível. |
| D12-BS | Marca assembly com pilha inicial nula ou desalinhada. |
| D12-C / D12-S | Erro na criação/partida da tarefa. |
| D12-E0 / D12-E1 | Sem registro de entrada / criação sem resultado completo. |
| D12-F | Falha de arquivo registrada pelo C, com etapa FS e erro hexadecimal. |
| D12-Q1/Q2/Q3 / D12-T | Consulta indicada falhou / término não confirmou. |

Ausência de registro continua sendo ausência de confirmação, sem provar que o código nunca executou.

## Escopo e ciclo de vida

O único módulo distribuído é `orbit_service12.sprx` (`OrbitService12`). O carregamento continua manual após a biblioteca, procurando um slot livre de 1 a 6. Os slots ocupados são preservados e `boot_plugins.txt` não é alterado.

O canal é `/dev_hdd0/tmp/PS3_GAME_ORBIT_SERVICE12`, com `.bridge`, `.entry`, `.worker`, `.req`, `.reply`, `.ready`, `.done` e temporários. O cliente aguarda até 3 segundos por etapa, tenta STOP em falha/cancelamento e o worker tem prazo de 60 segundos sem cliente, sujeito ao retorno das chamadas do sistema. DONE precede a saída; o stop exportado do módulo faz join. Reinicie antes de repetir.

Montagem, catálogo NTFS e controle da ventoinha continuam suspensos. STATUS contém valores fixos de diagnóstico. Configurar fan, montar ou restaurar SYSCON recebem `0x160A`; o módulo não contém adaptadores de fan, sensores, storage ou Cobra.

## Verificação local

- Código PPC64 linkado executado com Unicorn 2.1.4, TLB virtual e chamadas LV2 simuladas: reprodução do erro antigo com a ABI real da syscall 145; versão corrigida com r2 inicial zero/incorreto/correto, três respostas STATUS exatas, STOP, READY/worker/DONE válidos e expiração de 60 segundos. Os registradores voláteis declarados pelo SDK são alterados pelo simulador.
- O cliente C++ real também lê os arquivos produzidos pelo binário PPC64 com o erro antigo e escolhe D12-WV.
- Cliente C++ e worker C reais com threads POSIX/arquivos: 17 casos de cliente, 13 decisões de diagnóstico, falhas de criação/partida e de open/write/fsync/close/rename, argumentos copiados, mensagens inválidas, deadline/join e comandos suspensos.
- Teste de pixels do HUD: pausas, deslocamento, final da mensagem, reinício, UTF-8, texto curto e mudança/reabertura do menu. A imagem fora da faixa de status permanece idêntica; uma atualização de rolagem altera 51.168 bytes.
- Laço principal extraído: 37 cenários, incluindo acionamento manual, cancelamento e apresentação da biblioteca. Serviço antigo e NTFS continuam bloqueados.
- Auditoria de syscalls/ponte, nove descritores OPD64/OPD32 e relocations em bases independentes, exports por parser independente, assinatura SCETool decodificada e HMACs conferidos. PKG extraído e comparado aos arquivos compilados.

A execução PPC64 usa um modelo de chamadas do sistema; não executa firmware nem escalonador PS3. **O Teste 12 no console ainda depende da confirmação do usuário.** O serviço completo da versão 1.6 ainda não está validado.

## Compilar e reproduzir

Use GCC/G++ 7.5.0 e o SDK do checkpoint fixado anterior. Defina `PS3DEV`, `ORBIT_SCETOOL` e `ORBIT_SCETOOL_DATA`. SCETool, chaves/curvas e plugins de terceiros não são redistribuídos.

Prepare o modelo original separado com `python3 scripts/prepare-jfx.py --source /caminho/JFX_DVDBlu.zip`, depois rode `bash scripts/build-test12.sh`. O modelo editável e o include gerado continuam fora do ZIP distribuído.

Testes: `bash tests/run_service12.sh`, `bash tests/run_status_scroll12.sh`, `bash tests/run_diagnostic03.sh`, `python3 scripts/check-library-loop-probe04.py`. Após compilar o módulo, instale `unicorn==2.1.4` no Python de teste e execute `bash tests/run_native_service12.sh`. Esse script gera uma variante temporária com o relógio antigo exclusivamente para regressão; ela não é assinada nem incluída no PKG. Os demais verificadores são chamados por `scripts/build-service12.sh`.

Documentos e scripts de revisões anteriores são históricos. Os arquivos `src/jfx_case_data_fix29.inc` e `assets/jfx_bluray/Bluray_Tris.obj` são necessários para recompilar, preparados a partir do arquivo original separado.

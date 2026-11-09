# PS3 Game Orbit 1.6 — Teste 09

O Teste 08 carregou o módulo, mas não recebeu a confirmação da tarefa tanto
no CFW 4.91 Evilnat CEX/Cobra 8.5 quanto no HEN 3.6.0, conforme informado
pelo usuário. Esse resultado não distingue uma falha de thread de uma falha
ao gravar o arquivo de confirmação. A causa ainda não está confirmada.

O Teste 09 registra essas etapas separadamente e mostra o código de retorno.
Mantém o mesmo caminho de criação 52/53, prioridade 1700, stack de 64 KiB e
TLS zero do Teste 08. Não troca o mecanismo antes de medir onde ele falhou.
O Teste 07 mínimo já foi confirmado nos dois consoles.

## Como testar

1. Reinicie o PS3. Se estiver no HEN, ative-o.
2. Instale o PKG **Teste 09** e abra o Orbit. O ID continua `PGORBT301`.
3. Espere a biblioteca aparecer.
4. Abra **START → Teste do serviço → Testar serviço do Orbit**.
5. Aguarde a mensagem final. Copie a mensagem inteira ou tire uma foto.
6. Saia do aplicativo e informe se voltou normalmente ao XMB.

Comece pelo CFW. Não precisa repetir o Teste 08. Reinicie antes de repetir
o teste, pois o módulo de diagnóstico permanece no slot até o reinício.
O aplicativo preserva plugins existentes e não modifica `boot_plugins.txt`.

## Mensagens de diagnóstico

| Código | O que o aplicativo conseguiu observar |
| --- | --- |
| D09-E0 | O carregamento retornou sucesso, mas não há registro válido da entrada do módulo. |
| D09-E1 | A entrada confirmou execução, mas não registrou a conclusão da chamada de criação. |
| D09-C | A criação devolveu erro; a mensagem inclui o código hexadecimal. |
| D09-S | A partida devolveu erro; a mensagem inclui o código hexadecimal. |
| D09-W0 | Criação e partida retornaram zero, mas não chegou um registro válido do worker. |
| D09-W1 | O worker confirmou execução, mas não confirmou a comunicação. |
| D09-F | O worker confirmou execução e registrou uma falha de arquivo. |

Um registro ausente significa ausência de confirmação; não prova, sozinho,
que a entrada ou o worker nunca executaram. Os registros também usam arquivos.

Para D09-F, **FS2** é abertura, **FS3** gravação, **FS4** sincronização,
**FS5** fechamento, **FS6** primeira tentativa de renomear e **FS8** conclusão
após criar o backup. FS1 é caminho longo. O código hexadecimal vem da operação
que falhou; `0xFFFFFFFE` indica gravação curta detectada pelo diagnóstico.

O resultado de sucesso continua sendo **“Serviço respondeu 3 vezes e encerrou.”**
Isso valida somente este diagnóstico de comunicação.

Se aparecer D09-W0, D09-W1 ou D09-E1, envie também, se possível,
`/dev_hdd0/tmp/PS3_GAME_ORBIT_TESTE09.log`. Esse log registra os códigos,
o ID e os endereços do descritor da tarefa para analisar essa etapa.
Há uma alternativa de log no `USRDIR` do app.

## O que mudou no código

- A entrada grava um registro simples antes da criação; depois grava os
  resultados separados de 52 e 53, o ID e o descritor OPD32 do worker.
- O worker grava outro registro simples antes de consultar o relógio e de
  tentar criar o arquivo READY com fsync/rename.
- Esses registros usam apenas open/write/close, como o plugin mínimo que
  funcionou. Cada registro contém sessão, estágio e checksum para rejeitar
  dados antigos, parciais ou corrompidos.
- O helper de criação preserva os parâmetros do Teste 08. Sua versão com
  rastreamento registra os retornos mesmo quando a criação/partida falha.
- `plugin/file_ipc.h`, compartilhado com o serviço, passa a oferecer uma
  versão com resultado de cada etapa. A variante sem rastreamento continua
  disponível para o código anterior. A lógica de commit/backup permanece.
- O canal passa a ser `PS3_GAME_ORBIT_SERVICE09` em `/dev_hdd0/tmp`, com
  `.entry`, `.worker`, `.req`, `.reply`, `.ready`, `.done` e temporários.
  Não toca o canal ou as configurações do serviço completo.
- O único SPRX no PKG é `orbit_service09.sprx`, módulo `OrbitService09`.
  O cliente verifica slots de 1 a 6 e só usa um slot livre.
- Continuam três STATUS, STOP específico do diagnóstico, tokens por sessão,
  limites de espera e prazo de 60 segundos para a tarefa sem cliente.
  O limite depende do retorno das chamadas do sistema.
- READY/DONE usam o mesmo transporte atômico testado anteriormente. DONE é
  escrito antes da saída da tarefa; não representa um join feito pelo app.
  O stop exportado do módulo faz o join. O aplicativo não descarrega plugins.

## Recursos ainda suspensos

Montagem de jogos, catálogo NTFS e controle da ventoinha continuam desativados.
O STATUS é fixo e não lê sensores nem representa a configuração real do PS3.
Os comandos CONFIGURE, MOUNT_NATIVE, MOUNT_NTFS e RESTORE_SYSCON recebem
`0x160A`. O módulo não contém APIs de Cobra, storage, fan, sensores, peek/poke.
O serviço completo permanece no código-fonte, mas seu SPRX não vai no PKG.

## Validação local e compilação

Foram exercitados o cliente C++ e o worker C reais com arquivos e threads POSIX,
incluindo três consultas e STOP; erros de criação/partida e falhas de
abertura/gravação/fsync/fechamento/rename foram identificados nas mensagens.
Também são verificados tokens antigos/corrompidos, comandos bloqueados,
deadline, join, acionamento manual e os bloqueios do serviço antigo.
Esses testes não emulam o PS3 nem confirmam execução no hardware.

O ABI foi confrontado com o SDK PSL1GHT fixado e os fontes públicos do RPCS3:
`rpcs3/Emu/Cell/lv2/sys_ppu_thread.h`, `sys_ppu_thread.cpp` e
`rpcs3/Emu/Cell/Modules/sys_ppu_thread_.cpp`. O teste binário confere todos
os descritores OPD64/OPD32, inclusive o worker, após relocar os segmentos
em bases independentes. A tabela de exports também é analisada por um parser
independente. Não foi encontrada, por isso apenas, uma causa confirmada.

Use GCC/G++ 7.5.0 e o SDK do checkpoint anterior. Defina `PS3DEV`,
`ORBIT_SCETOOL` e `ORBIT_SCETOOL_DATA`. SCETool e seus dados externos não
são redistribuídos. A assinatura é decodificada e comparada ao PRX e também
verificada com OpenSSL/HMAC. O signer antigo não é usado para este SPRX.

O modelo JFX editável e o include gerado ficam separados, como nas entregas
anteriores. Prepare a cópia original com
`python3 scripts/prepare-jfx.py --source /caminho/JFX_DVDBlu.zip` e execute
`bash scripts/build-test09.sh`.

Testes: `bash tests/run_service09.sh`, `bash tests/run_diagnostic03.sh` e
`python3 scripts/check-library-loop-probe04.py`. Scripts e documentação de
versões anteriores são históricos; o ponto de entrada desta entrega é
`build-test09.sh`.

**Teste 09 no hardware: pendente. O problema do Teste 08 não foi declarado resolvido.**

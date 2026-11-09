# PS3 Game Orbit 1.6 — Teste 08

Este pacote testa uma única tarefa nativa e a comunicação PGOR com o aplicativo.
Não é a versão 1.6 funcional completa. A montagem de jogos, o catálogo NTFS e a
ventoinha continuam suspensos. A biblioteca usa a base do Teste 03.

## Resultado anterior informado pelo usuário

O Teste 07 mostrou “Plugin mínimo carregado e respondeu.” em:

| Ambiente | Resultado do Teste 07 |
| --- | --- |
| CFW 4.91 Evilnat CEX / Cobra 8.5 | Carregou e respondeu |
| HEN 3.6.0 | Carregou e respondeu |

Isso confirma o plugin mínimo. Não confirma a montagem, o controle da ventoinha,
o serviço completo ou a ausência de travamento da versão 1.6 original.

## Teste no console

1. Reinicie o PS3 antes de instalar e executar este teste. No HEN, ative-o.
2. Instale o PKG do Teste 08 e abra o Orbit. O ID permanece `PGORBT301`.
3. Aguarde a biblioteca aparecer.
4. Abra **START → Teste do serviço → Testar serviço do Orbit**.
5. Aguarde até cerca de 15 segundos e anote a mensagem completa.
6. O resultado esperado é **“Serviço respondeu 3 vezes e encerrou.”**
7. Saia normalmente e informe se voltou ao XMB. Reinicie antes de repetir.

Valide primeiro no CFW e depois no HEN. Não é necessário apagar configurações
ou remover outros plugins. O teste procura um slot livre entre 1 e 6; preserva
os slots ocupados. Se o mesmo módulo já estiver carregado, pede reinício.
O app não descarrega plugins e não altera `boot_plugins.txt`.

Se falhar, informe a mensagem exata e o ambiente. O log do app fica em
`/dev_hdd0/tmp/PS3_GAME_ORBIT_TESTE08.log` (alternativa no `USRDIR` do app).

## Escopo técnico

- O único módulo incluído no PKG é `orbit_service08.sprx` (`OrbitService08`).
  Ele é carregado manualmente depois da biblioteca e cria uma tarefa de 64 KiB.
- Usa o helper nativo 52/53 com OPD32, sem TLS de libc, e o protocolo big endian
  `PGOR` versão 1.6. A leitura limitada e a gravação com fsync/rename são
  compartilhadas com `plugin/service.c` por `plugin/file_ipc.h`.
- Um token de 16 bytes identifica cada execução. O módulo copia o argumento
  antes de a entrada terminar; o carregador pode liberar esse argumento.
- O cliente espera a tarefa iniciar, faz três consultas STATUS, envia um STOP
  exclusivo deste diagnóstico e confere o token final. Cada etapa tem espera
  limitada a 3 segundos. Respostas antigas não confirmam uma consulta nova.
- O STATUS é fixo, com sensores zerados e estado de ventoinha indisponível.
  Não representa temperaturas ou configuração real do console.
- CONFIGURE, MOUNT_NATIVE, MOUNT_NTFS, RESTORE_SYSCON e comandos desconhecidos
  recebem `0x160A`. O módulo não contém adaptações de fan, sensores, storage,
  Cobra, peek ou poke. Os syscalls nativos são auditados na compilação final.
- O canal isolado é `/dev_hdd0/tmp/PS3_GAME_ORBIT_SERVICE08` mais as extensões
  `.req`, `.reply`, `.ready`, `.done` e temporários. Não usa o canal ou as
  configurações do serviço completo.
- Ao cancelar/falhar, o cliente tenta enviar STOP. Mesmo sem cliente, a tarefa
  tem prazo de 60 segundos, sujeito ao retorno das operações do sistema.
- A confirmação final é escrita imediatamente antes da saída da tarefa. Ela
  confirma o fim do diagnóstico, não um join feito pelo app. O módulo permanece
  no slot; seu stop exportado faz o join. Reiniciar libera o módulo de teste.
- Mantém as correções do Teste 07: tabela de exports no formato correto,
  descritores OPD32, relocations ADDR32, flags PRX `0x01000000` e SCETool 0.2.9.
  A assinatura é decodificada e comparada ao PRX, com verificação independente
  de HMAC e tamanho do SELF.

## Compilação e testes

Use GCC/G++ 7.5.0 e o SDK fixado no checkpoint anterior. Defina `PS3DEV`,
`ORBIT_SCETOOL` e `ORBIT_SCETOOL_DATA`. O SCETool e seus dados externos não
fazem parte deste ZIP. O antigo `make_self_sprx` não é usado para este módulo.

O modelo JFX editável e o include gerado não são redistribuídos. Prepare a
cópia original separada com
`python3 scripts/prepare-jfx.py --source /caminho/JFX_DVDBlu.zip`.
Depois execute `bash scripts/build-test08.sh`. O script recusa GCC diferente.

Testes direcionados: `bash tests/run_service08.sh`,
`bash tests/run_diagnostic03.sh`, `python3 scripts/check-library-loop-probe04.py`.
O primeiro executa o código C real do módulo e o cliente C++ real em arquivos
locais com threads POSIX, além dos casos de falha do cliente. O segundo
confere o bloqueio do serviço antigo. O terceiro confere o acionamento manual
no loop extraído do aplicativo. Esses testes locais não emulam o PS3.

Arquivos e scripts de testes anteriores permanecem no histórico de fontes;
o ponto de entrada desta entrega é `build-test08.sh`, não os scripts antigos.
O serviço completo `plugin/service.c` continua presente apenas como fonte;
seu binário não é incluído nem carregado por este diagnóstico.

A revisão do ABI do worker usou o SDK PSL1GHT fixado e a implementação pública
do RPCS3 em `rpcs3/Emu/Cell/lv2/sys_ppu_thread.cpp` (parâmetro entry aponta
para OPD, TLS separado). A conversão PRX preenche esses descritores.

**Teste 08 no hardware: pendente de resultado do usuário.**

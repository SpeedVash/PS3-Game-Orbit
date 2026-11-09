# PS3 Game Orbit 1.6 — Teste 13

Etapa de montagem própria para jogos em pasta e ISOs no HD interno ou USB FAT32.

O usuário confirmou **D12-OK: 3 respostas; teste encerrado** tanto no CFW 4.91 Evilnat CEX/Cobra 8.5 quanto no HEN 3.6.0. Isso confirma o carregamento, o worker C, as três respostas e o término do diagnóstico do Teste 12 nos dois consoles. O relógio monotônico e a ponte assembly dessa revisão são mantidos.

**A montagem do Teste 13 ainda precisa ser confirmada no console.** Os testes locais usam interfaces LV2/Cobra simuladas; não substituem a montagem nem a execução real de um jogo.

## Testar primeiro no CFW

1. Reinicie o PS3 e instale o PKG do Teste 13 (`PGORBT301`).
2. Abra o Orbit e aguarde a biblioteca. O serviço começa em segundo plano somente após dois frames da biblioteca terem sido apresentados e confirmados.
3. Para conferir a comunicação: **START → Teste do serviço → Testar serviço do Orbit**. Esperado: **D13-OK: serviço pronto para montagem.** O diagnóstico não encerra mais o worker, pois ele atenderá às montagens.
4. Selecione um jogo em pasta ou uma ISO do HD interno e pressione **X**.
5. O Orbit envia a montagem, aguarda a ID esperada em `/dev_bdvd/PS3_GAME/PARAM.SFO` e só então volta ao XMB. Confirme se o ícone do disco mostra o jogo correto e se o jogo inicia.
6. Depois, repita com um jogo em USB FAT32, se disponível. ISOs divididas em `.iso.0`, `.iso.1` etc. também são aceitas quando as partes estão completas.

Informe o formato/local testado, se voltou ao XMB e se o jogo iniciou. Se falhar, envie a mensagem e `/dev_hdd0/tmp/PS3_GAME_ORBIT_TESTE13.log` (há fallback no `USRDIR` do Orbit). Após confirmar a montagem no CFW, repetiremos no HEN.

Pastas devem estar em `/dev_hdd0/GAMES`, `/dev_hdd0/GAMEZ` ou nos equivalentes de `/dev_usbNNN`. ISOs devem estar em `PS3ISO`. Se a ISO não tiver ID conhecida, o aplicativo exige conferência manual em vez de afirmar que confirmou o jogo.

## Implementação desta etapa

- Módulo `OrbitService13`, arquivo `orbit_service13.sprx`, com um único worker IPC. Mantém a ponte/TOC, stack de 64 KiB, prioridade diagnóstica 0, export table correta, relocations e assinatura SCETool já usados no Teste 12.
- Serviço residente até seu módulo ser encerrado ou o console reiniciar. O app não escreve `boot_plugins.txt`. Os slots ocupados são preservados; ao reabrir o Orbit, o serviço já carregado é consultado e reutilizado sem novo load e sem apagar seu canal.
- Canal isolado `/dev_hdd0/tmp/PS3_GAME_ORBIT_SERVICE13`: READY/entrada/worker/ponte, request/reply, cancelamento e DONE. O nonce do READY é lido ao reutilizar um módulo; cada consulta usa uma sequência nova. Um mutex serializa a verificação e a montagem feitas por threads do aplicativo.
- Parser de montagem limitado a caminhos nativos, até 64 partes e payload de 128 KiB. Os buffers grandes ficam fora da stack do worker. Caminhos, tipos de arquivo, tamanho das partes e arquivos necessários são verificados antes dos eventos de disco.
- A pasta usa a ISO vazia privada do Orbit, com a ID do jogo, e mapeamento para `/dev_bdvd`. O diretório privado usado para bloquear `PS3_UPDATE` é incluído no pacote.
- Foi corrigida outra incompatibilidade do adaptador: os arrays de ponteiros passados ao Cobra usam entradas explícitas de **32 bits**, mesmo dentro do PRX compilado com `-m64`. A implementação antiga usava arrays de ponteiros de 64 bits. O código público do HEN confirma a leitura de `uint32_t` tanto para ISO quanto para mapeamento.
- Cancelamento verificado antes dos eventos e antes/depois da inserção. Falhas após o início da montagem tentam desfazer a emulação e os mapeamentos usados e restaurar a inserção física. Isso não promete restaurar uma ISO anterior que já tenha sido desmontada.
- O aplicativo continua sem depender de webMAN para enviar a montagem. STATUS ainda contém dados fixos de diagnóstico; não representa temperatura ou velocidade de ventoinha.
- A rolagem das mensagens longas do Teste 12 é preservada: 36 px/s, pausas de 1,2s e atualização somente da faixa de status.

## Recursos que continuam suspensos

Controle da ventoinha e leitura de NTFS permanecem suspensos. A configuração antiga do fan não inicia seu worker, o catálogo NTFS não é pesquisado e uma montagem NTFS é recusada antes de criar a tarefa do aplicativo ou carregar o serviço. O novo módulo contém apenas as operações Cobra de tipo/evento de disco, montagem de ISO, desmontagem e mapeamento; não contém sensor, fan, peek/poke, leitura de setores ou worker NTFS.

## Validação local

- Cliente C++ e worker C reais, com arquivos/threads POSIX: 16 cenários do cliente, 13 decisões de diagnóstico, reutilização de módulo ativo com novo cliente/nonce de consulta e sem reload/limpeza, erros de criação/partida e falhas dos arquivos READY.
- Montagem com interfaces simuladas: pasta, ISO interna e duas partes no USB; caminhos ausentes/inválidos/traversal recusados antes de ejetar; cancelamento antes e depois da ejeção; falhas de ejeção, ISO, mapeamento e inserção com limpeza; transporte C++ real lendo as respostas do dispatcher C.
- Execução do código PPC64 linkado em Unicorn 2.1.4 com TLB virtual: mesmos três formatos, tabelas de ponteiros de 32 bits consumidas pelo modelo do kernel, STATUS antes/depois da montagem, STOP/DONE, r2 inicial zero/incorreto/correto e nenhuma operação de fan/storage. Uma variante temporária com tabelas de 64 bits falha no mesmo modelo, como controle da regressão.
- Laço principal real extraído: 37 cenários, incluindo início automático somente após dois frames apresentados/confirmados, nova verificação manual, cancelamento, prefetch, disco físico, menus e gerenciador.
- Flags reais do Teste 13: fan antigo inerte e NTFS bloqueado. Teste de pixels confirma que a rolagem segue recortada e altera apenas a faixa de 492×26 pixels.
- Descritores OPD64/OPD32 e relocations em bases independentes, auditoria da ponte, PRX sem imports de VSH, SCETool decodificado e OpenSSL/HMACs conferidos. PKG final extraído e comparado ao staging.

O teste PPC64 simula a fronteira de chamadas do sistema. O comportamento real do firmware, dos dispositivos USB, do XMB e dos jogos ainda depende dos testes físicos. Sucesso no Teste 12 não valida automaticamente todas as funções da versão 1.6.

## Fontes e compilação

Use o toolchain GCC/G++ 7.5.0 e o SDK fixado no checkpoint anterior. Prepare o modelo JFX original separado com `python3 scripts/prepare-jfx.py --source /caminho/JFX_DVDBlu.zip`, depois `bash scripts/build-test13.sh`. O modelo editável e o include gerado permanecem fora do ZIP distribuído. Chaves/curvas do signer e plugins de terceiros também não são redistribuídos.

Testes: `bash tests/run_service13.sh`, `bash tests/run_stage13_guards.sh`, `python3 scripts/check-library-loop-service13.py`, `bash tests/run_status_scroll12.sh`. Após compilar o módulo, com `unicorn==2.1.4` e `PS3DEV` definidos: `bash tests/run_native_service13.sh`. A variante de ABI antiga é gerada apenas no diretório temporário de testes, sem assinatura e fora do PKG.

Referências primárias consultadas:
- https://github.com/PS3Xploit/PS3HEN/blob/master/payload/storage_ext.c (`copy_user_pointer_array`)
- https://github.com/PS3Xploit/PS3HEN/blob/master/payload/mappath.c (`sys_map_paths`)
- https://github.com/aldostools/webMAN-MOD/blob/ed8c3bfa449b5e48edcd6d10351cc0ea8f4357c0/cobra/storage_ext.h

Documentos de testes anteriores são históricos. **Teste 13 no CFW/HEN: pendente.**

# PS3 Game Orbit 1.6 — Teste 16

Revisão atual: [Teste 16 R1 — temperaturas durante suspensão por outro controlador](TESTE_16_R1.md).

A opção **Teste do serviço** foi substituída por **Controle da ventoinha**. O menu e o título do aplicativo exibem PS3 Game Orbit 1.6. As verificações internas do serviço continuam automáticas e são registradas no log, sem uma opção de diagnóstico no menu.

Esta etapa acrescenta o controle efetivo da ventoinha ao serviço residente já usado para montar os jogos. Preserva a montagem de pastas e ISOs, o leitor de ISOs NTFS/exFAT, a preparação de permissões e os recursos visuais aprovados.

## Estado físico confirmado

Em 17/10/2026, o usuário informou que o **Teste 15 funcionou no HEN**. Não especificou nessa confirmação o jogo, o formato ou se o USB estava em NTFS ou exFAT; portanto ela não é registrada como comprovação individual dos dois sistemas. O CFW 4.91 Evilnat CEX/Cobra 8.5 tem a montagem anterior confirmada, incluindo DiRT 3 em pasta no HD interno. O HEN 3.6.0 também tem os resultados anteriores de pasta/USB e a correção de permissões confirmados.

O controle de ventoinha do Teste 16 passou nas verificações locais descritas abaixo e ainda precisa ser testado nos consoles.

## Instalar e testar

1. Reinicie o console para liberar o serviço da versão anterior. No HEN, ative-o novamente.
2. Instale `PS3_GAME_ORBIT_TESTE16.pkg` sobre o Orbit anterior. A ID permanece `PGORBT301`, versão de instalação 01.60.
3. Abra a biblioteca e entre em **START → Controle da ventoinha**. A opção de teste não deve aparecer.
4. Sem configuração anterior salva, o modo inicial é **Padrão do PS3 (SYSCON)**. CPU e RSX mostram as leituras reais; velocidade não consultada no modo padrão aparece como `--%`.
5. Para a primeira verificação, selecione **Manual**, ajuste para **45%** e confira a leitura da velocidade e a resposta da ventoinha. Direcional esquerdo/direito ajusta os valores; L1/R1 altera valores numéricos de cinco em cinco.
6. Confira **Automático**, com alvo inicial de 68°C, mínimo 35% e máximo 85%. Depois selecione **Padrão do PS3** para conferir o retorno ao controle do console.
7. Verifique que o último ajuste aceito é restaurado ao reabrir o Orbit. Monte também uma ISO/jogo que já funcionava, inicie pelo ícone do disco no XMB e confira a segunda montagem. O serviço permanece residente durante o jogo.

No HEN, alterações de ventoinha ficam suspensas durante os primeiros 15 segundos detectados de um novo jogo e após a montagem; a velocidade anterior é mantida nesse intervalo. Uma alteração solicitada durante a pausa pode ser recusada como serviço ocupado. Aguarde e tente novamente.

Se houver falha, informe CFW/HEN, modo escolhido, temperaturas, velocidade mostrada, mensagem e o resultado da montagem. Envie `/dev_hdd0/tmp/PS3_GAME_ORBIT_TESTE16.log` (fallback no `USRDIR` do aplicativo). As linhas `FAN16` incluem estado, temperaturas, velocidade, resultado e código nativo.

## Comportamento

- **Padrão do PS3:** na primeira abertura, sem uma configuração salva, o Orbit consulta temperaturas e mantém o controle existente. Uma seleção explícita desse modo restaura SYSCON quando nenhum controlador externo é identificado.
- **Manual:** 35% a 100%, com proteção térmica. A velocidade aplicada é consultada novamente antes de confirmar o ajuste.
- **Automático:** usa o maior valor entre CPU e RSX, alvo de 60°C a 75°C e histerese para reduzir a velocidade gradualmente. O mínimo pode variar de 35% a 70%, respeitando o máximo configurado.
- **Temperatura alta:** a proteção pode ultrapassar o máximo automático ou a velocidade manual; a partir de 80°C pede pelo menos 85%, e a partir de 83°C pede 100%, quando as chamadas de controle estão disponíveis.
- **Leitura inválida:** três falhas consecutivas de sensores durante um modo próprio levam a uma tentativa de retorno ao SYSCON e interrompem a reaplicação automática. Um novo ajuste explícito é necessário para retomar.
- **Outro controlador:** webMAN/sMAN e alguns plugins de controle conhecidos são identificados. Se outro programa alterar a política/velocidade enquanto o Orbit controla a ventoinha, o Orbit também suspende suas escritas. Não descarrega plugins alheios nem tenta disputar o controle.
- **Persistência:** grava somente ajustes confirmados pelo serviço. Uma recusa preserva a configuração anterior. Ao fechar a interface, uma alteração já solicitada termina sua confirmação antes de a tarefa do aplicativo encerrar.
- **Serviço indisponível:** o menu informa a falha e recusa novos ajustes, em vez de deixá-los indefinidamente pendentes.

## Integração e compatibilidade

O novo módulo é `orbit_service16.sprx`, com canal `SERVICE16`/tokens `ORBIT16`, isolado dos testes anteriores. O aplicativo só inicia seu cliente depois de dois quadros da biblioteca efetivamente apresentados e reconhecidos. O controlador executa uma etapa por segundo no worker de IPC existente; não acrescenta outra tarefa nativa. A tarefa raw do USB continua independente.

O HEN usa diretamente as interfaces de temperatura e política de ventoinha 383/389/409, presentes no código do PS3HEN 3.6.0 consultado. Nesse caminho não há peek/poke do kernel. A pausa de inicialização usa a lista de processos do PS3MAPI, excluindo o PID informado pelo aplicativo, além do intervalo após uma montagem. Se não puder consultar os processos, mantém a pausa de controle.

No CFW, as mesmas chamadas diretas são tentadas primeiro. O fallback de permissão está limitado ao **4.91 CEX**, com os endereços conhecidos de leitura/escrita de política. Só modifica temporariamente a instrução de verificação se seu padrão for reconhecido; preserva a instrução seguinte e confere a restauração dos bytes originais após a chamada. Firmware/padrão desconhecido ou uma alteração concorrente são recusados. Outros firmwares dependem de as interfaces diretas estarem disponíveis; não são declarados validados por esta etapa.

A configuração no RPC pode ter 20, 24 ou 28 bytes; o cliente atual usa 28: cinco campos de configuração, PID do aplicativo e indicador de alteração explícita. Os limites são validados no serviço antes de qualquer mudança. STATUS mantém os 44 bytes, com telemetria real e o estado do leitor raw.

## Verificações locais

- `tests/run_fan16.sh`: controlador C real e adaptador nativo real com a fronteira LV2 simulada; padrão sem escrita de política, modos, maior temperatura, histerese, limites, proteção térmica, falhas, retorno a SYSCON, conflito e pausa HEN. O adaptador verifica ABI, ausência de poke no HEN, padrão de instruções CFW e restauração exata.
- `tests/run_fan_client16.sh`: cliente C++ assíncrono real; pacote de 28 bytes, leitura real do arquivo de configurações temporário, persistência após ACK, recusa sem sobrescrever o arquivo, reabertura, conclusão da última alteração ao sair e recusa após falha do bootstrap.
- `tests/run_service16.sh`: C real do serviço e C++ real do transporte em threads; fan Manual/recusa/SYSCON, pasta/ISO/split, NTFS/exFAT raw, três timeouts reais de ociosidade seguidos de leitura, bytes/EOF e limpeza de falhas/cancelamento.
- `tests/execute_native_service16.py`: código PPC64 do módulo efetivamente compilado, executado por Unicorn 2.1.4 com LV2/Cobra simulados. Executa caminhos CFW e HEN com r2 inicial zerado, incorreto e correto; fan Manual 45%, proteção 100%, retorno a SYSCON, montagem nativa e raw, descritores/TOC e restauração dos bytes temporários. O leitor raw também é executado separadamente com falhas e recuperação.
- `scripts/check-library-loop-service16.py`: 37 cenários do bloco real de `main`, incluindo início somente após dois quadros, menu funcional, falha de criação do cliente, pré-carga, cancelamento, disco físico e fluxos anteriores.
- `tests/run_permissions16.sh`: regressão da preparação de permissões na tarefa de montagem, com arquivos reais temporários; ordem anterior ao bootstrap, preservação dos bytes, falha, cancelamento e TITLE_ID.
- `tests/run_status_scroll16.sh`: pixels reais da rolagem de mensagens com o menu funcional. Geometria, shaders, imagens e recursos visuais são comparados ao pacote 15.
- Compilação GCC 7.5.0 com `-Werror`; PRX/relocações/OPDs; decodificação SCETool comparada ao PRX e verificação independente OpenSSL/HMAC; extração e comparação do PKG final. O PRX final é comparado ao executado no verificador PPC64.

Esses testes simulam a fronteira do kernel e não substituem os testes físicos de ruído, velocidade, temperatura, inicialização e jogo em CFW/HEN.

## Compilar

Use `scripts/build-test16.sh`, o SDK GCC 7.5.0 fixado e as variáveis `PS3DEV`, `PSL1GHT`, `ORBIT_SCETOOL` e `ORBIT_SCETOOL_DATA` descritas nos testes anteriores. Prepare o modelo aprovado com `scripts/prepare-jfx.py --source /caminho/JFX_DVDBlu.zip`. O modelo editável e o include gerado não são redistribuídos, como nas entregas anteriores. Chaves e curvas do assinador são dependências externas.

Fontes técnicas consultadas: PS3HEN revisão `6c529455261db97a0f9028fe96db522a97fd5ccc`, `payload/main.c` (wrappers 389/409 e PS3MAPI); webMAN-MOD revisão `ed8c3bfa449b5e48edcd6d10351cc0ea8f4357c0`, `include/poll/fancontrol.h` e `include/init/firmware.h` (ABI, pausa HEN e endereços CEX). São referências da implementação; nenhum plugin binário externo é incluído.

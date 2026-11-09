# PS3 Game Orbit 1.6 — Teste 16 R1: temperaturas e SYSCON

Atualização em 17/10/2026: o usuário confirmou funcionamento no HEN e temperaturas com suspensão pelo webMAN na CFW. Esta base foi promovida à versão funcional [1.6.1](docs/ORBIT_1.6.1.md). O restante deste documento registra o estado anterior à confirmação.

## Problema corrigido

Na CFW 4.91 Evilnat CEX/Cobra 8.5 com webMAN carregado, o usuário viu “Outro controlador ativo: Orbit suspenso.” e `CPU: --°C`. O ciclo do serviço verificava o controlador externo e retornava antes de consultar os sensores.

A leitura de CPU e RSX agora acontece antes da suspensão. O Orbit continua sem consultar ou alterar a política da ventoinha enquanto outro controlador estiver detectado. A leitura também continua na pausa de inicialização do HEN. Uma falha dos sensores limpa as temperaturas, em vez de apresentar valores antigos; a leitura volta na próxima consulta válida.

O adaptador das syscalls, a detecção dos plugins, a política térmica, a interface e a montagem dos jogos não foram alterados. O bloqueio considera a presença do plugin carregado, sem consultar sua configuração de ventoinha. Apenas ter o multiMAN instalado não causa esse bloqueio.

## Retorno ao SYSCON no HEN

No HEN 3.6.0, o usuário confirmou que as temperaturas aparecem. Após Manual ou Automático, selecionar Padrão do PS3 volta ao modo anterior nas primeiras tentativas; depois de insistir, o modo Padrão fica selecionado.

O código original fazia uma única tentativa e exigia leitura imediata da política. Após uma confirmação falhar, podia continuar o ciclo anterior. A R1 trata recusas transitórias e leituras atrasadas com até oito tentativas, separadas por 100 ms. Mantém a propriedade do controle até confirmar o retorno e suspende o ciclo Manual/Automático durante a entrega. Não anuncia sucesso nem salva Padrão se a confirmação falhar.

Se atingir o limite, o modo anterior permanece suspenso e o serviço só tenta liberar sua própria política. A configuração continua exigindo uma nova confirmação para ser salva. Se outro controlador tomar o controle ou começar a pausa de jogo do HEN, as tentativas param. A causa específica no console ainda precisa ser confirmada no teste físico da R1; as regressões cobrem recusa temporária, confirmação atrasada e falha persistente.

## Instalação e verificação

1. Termine o teste em andamento se desejar e instale `PS3_GAME_ORBIT_TESTE16_R1.pkg` sobre o Orbit anterior.
2. Reinicie o PS3 antes de abrir a revisão R1. O serviço é residente e mantém o nome `orbit_service16.sprx`; a reinicialização é necessária para executar o binário corrigido. Ative novamente o HEN quando aplicável.
3. Na CFW com webMAN carregado, abra START → Controle da ventoinha. CPU e RSX devem apresentar e atualizar temperaturas. O aviso de outro controlador e a recusa de alterações permanecem.
4. No HEN sem outro controlador, aplique Manual em 45%. Selecione Padrão do PS3 uma vez e aguarde a resposta, sem ficar alternando os modos. Repita Automático → Padrão. Confira se Padrão permanece selecionado e se a velocidade responde. Confirme um jogo já conhecido.

O log permanece `/dev_hdd0/tmp/PS3_GAME_ORBIT_TESTE16.log`, com fallback no USRDIR do aplicativo. A versão pública continua 1.6 e a identidade é PGORBT301.

## Verificações locais

- `tests/run_fan16.sh`: a nova regressão falha no código original e passa na correção. Cobre sensores ao iniciar com controlador externo, atualização, falha/recuperação, conflito persistente e pausa HEN, mantendo a ausência de chamadas get/set.
- `tests/run_service16.sh`: cliente C++ e serviço C reais, transporte por arquivos, modos da ventoinha, montagem nativa, proxy NTFS/exFAT e encerramento.
- `tests/execute_temperature_external16r1.py`: executa o PRX PPC64 real com webMAN simulado. Exige temperaturas crescentes em STATUS, recusa de Manual, manutenção do bloqueio em Padrão e nenhuma chamada get/set/peek/poke, com r2 inicial zero, incorreto e correto. Falha também contra o PRX original.
- `tests/execute_native_service16.py`: execução do módulo compilado nos modelos CFW e HEN, montagem nativa, NTFS/exFAT, Manual, proteção térmica e restauração SYSCON.
- `tests/execute_syscon_transition16r1.py`: executa o PRX PPC64 real no modelo HEN, recusando as primeiras três chamadas e atrasando a leitura da política, a partir de Manual, Automático e RESTORE. Verifica ainda o limite de oito tentativas, ausência de reaplicação do modo anterior, manutenção da propriedade para recuperação e próxima confirmação. Todos os casos usam r2 zero, incorreto e correto; não permitem peek/poke. O PRX original falha na nova regressão.
- Compilação GCC 7.5.0 com `-Werror`, verificações de PRX/assinatura e extração do PKG final. SFO e recursos são idênticos ao Teste 16. O ELF do aplicativo só difere no horário de compilação registrado no log; o EBOOT é novamente assinado. A alteração funcional fica no SPRX.

Os modelos de syscalls não substituem o teste físico. O usuário confirmou a detecção do conflito na CFW e as temperaturas no HEN do Teste 16 original, além de retorno ao modo Padrão após várias tentativas. As correções da R1 e as velocidades reais ainda precisam de confirmação.

## Compilação

Use o mesmo SDK e dependências externas do Teste 16 e execute `bash scripts/build-test16r1.sh`. O script produz `PS3_GAME_ORBIT_TESTE16_R1.pkg` a partir do pacote finalizado. O modelo JFX editável, seu include gerado, SDK e material externo do assinador permanecem fora da entrega de código, conforme as revisões anteriores.

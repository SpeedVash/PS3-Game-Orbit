# PS3 Game Orbit 1.6 — Teste 14

O Orbit agora prepara as permissões do jogo em pasta no HD interno antes de carregar/consultar o serviço para montar. A operação acontece na tarefa de montagem, permitindo que a interface continue respondendo e que Círculo cancele a preparação.

## O que foi confirmado pelo usuário no Teste 13

| Ambiente | Formato e local | Resultado físico |
| --- | --- | --- |
| CFW 4.91 Evilnat CEX / Cobra 8.5 | DiRT 3 em pasta no HD interno | Montou e o jogo iniciou normalmente |
| HEN 3.6.0 | Odin Sphere Leifthrasir em pasta no HD interno | Não iniciou na primeira tentativa; iniciou pelo Orbit após reaplicar permissões e selecionar HD interno no multiMAN |
| HEN 3.6.0 | ISO no USB | Montou e iniciou; jogo e divisão da ISO não informados |

O diagnóstico de comunicação D12-OK havia sido confirmado nos dois consoles. Estes resultados não validam automaticamente todos os formatos, a correção nova do Teste 14, NTFS ou a ventoinha.

## Teste no console

1. Instale PS3_GAME_ORBIT_TESTE14.pkg por cima da versão anterior. Se reiniciar o console com HEN, ative-o antes de abrir o Orbit.
2. Selecione um jogo **em pasta no HD interno que ainda não tenha recebido os ajustes de permissões de outro gerenciador**. O Odin Sphere já corrigido verifica regressão, mas sozinho não comprova a correção nova.
3. Pressione X e aguarde a preparação e a montagem. O aplicativo confere a ID do disco montado antes de voltar ao XMB. Inicie pelo ícone do disco.
4. Informe jogo, formato, local, se voltou ao XMB e se o jogo iniciou. Se falhar, envie a mensagem e `/dev_hdd0/tmp/PS3_GAME_ORBIT_TESTE14.log` (há fallback no USRDIR do Orbit).
5. Uma segunda montagem do mesmo jogo deve funcionar também. Se possível, confira novamente a ISO do USB como regressão.

Não é preciso usar o multiMAN como etapa de preparação deste teste. A seleção de HD interno é obtida pelo caminho real da pasta, sem ler a configuração do multiMAN. Se houver falha em preparar uma permissão, o Orbit informa o erro e não envia o pedido de montagem.

## Implementação

- A preparação só percorre a pasta selecionada dentro de `/dev_hdd0/GAMES/` ou `/dev_hdd0/GAMEZ/`. Caminhos que apontem para a raiz compartilhada, contenham componentes vazios, `.`/`..`, caracteres de controle ou excedam o limite são recusados.
- Diretórios são preparados antes dos seus filhos. As permissões de leitura, escrita e execução são reaplicadas com modo 0777 somente quando faltam bits; uma nova consulta confere o resultado. Os dados dos arquivos não são abertos para escrita.
- USB FAT32, ISO, NTFS e disco físico não recebem essa alteração de modos. O procedimento não atua no firmware, em saves ou nas configurações de outros gerenciadores.
- Falhas de consulta, chmod ou listagem são registradas com operação, caminho e código. O resumo registra quantidade de arquivos, diretórios e entradas alteradas. Cancelar interrompe novos ajustes e fecha os diretórios; alterações já concluídas permanecem aplicadas.
- A montagem e a conferência de TITLE_ID continuam nas rotinas existentes. A preparação roda antes do bootstrap e antes de enviar MOUNT_NATIVE, sem consumir o timeout da transação com o serviço. Há limite de caminho de 1024 bytes e de 64 níveis internos.
- O SPRX é **exatamente o arquivo do Teste 13**, SHA-256 `3b23af99669540d4d2c165f3fd4f97e0bd5e428b6aac9c0a55c993b712b753ed`. Por isso o teste manual de comunicação ainda mostra **D13-OK: serviço pronto para montagem**. O aplicativo, o menu e o log identificam Teste 14.
- A rolagem das mensagens, a biblioteca, o modelo e os shaders são preservados. NTFS e controle da ventoinha continuam bloqueados.

## Verificações locais

`bash tests/run_permissions14.sh` verifica preparação de cima para baixo, repetição sem novas escritas, escopo, limites, tipos inesperados, falhas de filesystem e cancelamento. Também executa a tarefa real de montagem C++ com arquivos reais temporários, traduzindo os caminhos PS3 na fronteira do filesystem e simulando apenas as chamadas do transporte ao serviço. Confere os modos antes do bootstrap/RPC e a preservação de todos os bytes, além de falha, cancelamento, nova tentativa e ID incompatível.

`bash tests/run_stage14_guards.sh` verifica os bloqueios de fan e NTFS com as flags efetivas do Teste 14. A compilação nativa usa GCC 7.5.0. O pacote é extraído e comparado ao staging; os arquivos congelados e o SPRX são comparados com o pacote anterior. Esses testes locais não substituem a validação do Teste 14 no PS3.

## Compilar

Prepare separadamente o modelo JFX original: `python3 scripts/prepare-jfx.py --source /caminho/JFX_DVDBlu.zip`. Depois defina PS3DEV para o checkpoint GCC 7.5.0 e execute `bash scripts/build-test14.sh`. Esse script verifica e reutiliza o módulo assinado incluído; não exige reassiná-lo.

Os fontes nativos do serviço e seu script de recompilação continuam incluídos. Uma recompilação opcional desse módulo usa o processo SCETool do Teste 13 e requer as dependências externas já documentadas em TESTE_13.md. O modelo editável, o include gerado do modelo e chaves/curvas externas não são redistribuídos.

O SDK fixado fornece sysLv2FsStat (808), sysLv2FsChmod (834) e as funções de diretório (805/806/807) em `ppu/include/sys/file.h`, commit PSL1GHT `af9d3d964c8faa69abce4961a269f9582e05a33f`.

# Validação da v1.4

## O que os testes no computador verificam

- Ações dos dois menus, favoritos, montagem com caixa aberta e opções de fundo/último jogo.
- Gravação/restauração de configurações, checksum, rejeição de arquivo inválido e escrita atômica.
- USB/PS3COVERS, importação de uma arte, ID e ISO, conversão para dimensões exatas e preservação de alpha no PNG.
- Limites reais de caches, mudança de layout, filtros vazios, três vizinhos Spine por lado e pré-carga estabilizada.
- Geometria com índices válidos, coordenadas finitas, arte interna do tamanho completo, emendas unidas de 0° a 180° e borda do disco de 0,4 mm.
- Loop principal efetivo com atualização USB em lote, deduplicação de ID, cancelamento e restauração do último jogo ligada/desligada.
- Corpo nativo de desenho simulado: materiais, UVs, texturas por jogo, articulações, fundo/interface sem caixa e limites de submissão/FIFO.
- Identidade/formato dos arquivos ELF, SELF, PARAM.SFO e PKG, referências das APIs nativas e shaders arquivados.

A suíte pública também é executada com AddressSanitizer e UndefinedBehaviorSanitizer. A verificação de vazamentos do LeakSanitizer é desativada porque o ambiente não permite inspecionar processos; isso não equivale a um teste de vazamentos.

Relatórios e hashes estão na pasta `validacao` do ZIP. A simulação de APIs não reproduz o RSX real nem mede fluidez no PS3.

## Teste físico pendente

A v1.3.3 foi aprovada pelo usuário; isso não valida automaticamente a v1.4. Antes de publicar como estável:

1. Instale por cima da versão anterior e confirme título/ícone/splash da v1.4, sem tela rosa.
2. Altere layout, fundo, zoom, posição e filtro; saia normalmente e abra novamente. Confira nomes e favoritos antigos.
3. Ative “Salvar último jogo visto”, escolha um jogo e reinicie o app. Desative e confira que começa no primeiro jogo do filtro salvo.
4. Navegue em Clássico, Spine e Lista com a biblioteca de 57 jogos. Observe pausas iniciais e ao retornar a jogos recentes; confirme apenas três caixas por lado no Spine.
5. Teste Favoritos/USB/HD sem jogos: nenhum modelo deve aparecer, e START deve continuar disponível.
6. Importe só a capa, só o inside e só o disco de um jogo em pasta e de um ISO sem ID reconhecida. Confira que os tipos ausentes ficam intactos.
7. Use arquivos maiores e menores que o padrão. Confira no HDD: 1000×550 em JPG para capa/inside e 500×500 em PNG para disco. Depois retire o pendrive e abra o jogo no app.
8. No menu START, atualize todas as capas; confira progresso e cancele com Círculo durante a operação. Imagens já concluídas devem permanecer.
9. Abra/feche com L3, gire a caixa e confira presilhas, bandeja, topo transparente, arte interna completa, lombada unida e borda estreita do disco. Monte o jogo com X também nessa visualização.
10. Teste webMAN disponível e indisponível. Uma montagem confirmada deve voltar ao XMB; falha deve permitir continuar usando a biblioteca.

Se houver erro, envie o log `/dev_hdd0/tmp/PS3_GAME_ORBIT_FIX37.log` (ou a alternativa em `USRDIR`), modelo do console, firmware, HEN/CFW, nome/ID/formato do jogo e passos para reproduzir. Os tempos de leitura, decodificação, upload e interface continuam no log.

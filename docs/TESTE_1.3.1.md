# Roteiro de teste no PS3 — 1.3.1 / FIX34

Compilação e verificações feitas no computador. Teste físico desta versão pendente.

1. Instale por cima da 1.3 e confira 1.3.1 na interface, biblioteca, favoritos e layout salvo.
2. Navegue nos três layouts com os 57 jogos e volte a títulos recentes. O log mostra `CACHE 1.3.1` com os três contadores, hits, misses e bytes; cada tipo deve ficar em até 15 texturas válidas.
3. Para `Bayonetta.iso`, sem ID no nome, copie `Bayonetta.jpg`, `Bayonetta_INSIDE.jpg` e `Bayonetta_DISC.png` para `/dev_hdd0/PS3COVERS`. Use o nome exato, sem `.iso`. Para ID, ela precisa ser reconhecida: em jogos ISO, estar no nome como `BLES01287`, sem hífen. A ID interna do ISO não é lida. Feche a caixa e pressione START; confira a orientação também em SELECT.
4. Abra 15 jogos diferentes com inside e disc; feche e volte aos recentes. As imagens devem reaparecer corretas, sem nova decodificação enquanto residentes. Abra o 16º com artes válidas; os caches de inside e disc descartam a imagem menos recente de cada tipo. Um jogo sem arte não deve ocupar uma textura desses caches.
5. Verifique interior esquerdo/direito, topo e centro/lombada, sem espelhamento. Confira o centro opaco e prateado do disco, mantendo o furo e a arte circular.
6. Feche com Círculo e L3, inclusive invertendo a animação no meio. X deve montar o jogo, com caixa aberta ou fechada. Com webMAN ativo, aguarde confirmação e retorno ao XMB; abra o jogo pelo ícone do disco. Um segundo X durante a operação não cria outra montagem.
7. Abra um jogo sem artes opcionais: interior neutro e rótulo padrão, sem reaproveitar imagem de outro título.
8. Substitua uma arte no HDD, feche e pressione START. Confirme a nova imagem na próxima abertura e os caches reiniciados.
9. Teste arquivo inválido e indisponibilidade do webMAN: a biblioteca deve permanecer utilizável. Círculo durante a montagem cancela a operação e sai ao XMB.

Log: `/dev_hdd0/tmp/PS3_GAME_ORBIT_FIX34.log`, ou `/dev_hdd0/game/PGORBT301/USRDIR/PS3_GAME_ORBIT_FIX34.log`. Informe layout, nome exato do ISO/TITLE_ID, dimensões e formato das imagens e o botão usado se houver falha.

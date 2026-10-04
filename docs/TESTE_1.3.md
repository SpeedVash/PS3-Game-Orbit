# Roteiro de teste no PS3 — 1.3 / FIX33

A versão 1.2 foi testada e aprovada pelo usuário. A nova 1.3 precisa destes testes no console.

1. Instale por cima da 1.2 e confira a identificação 1.3 na interface, biblioteca, favoritos e layout salvo.
2. Navegue em Clássico, Spine e Lista com os 57 jogos. Volte a jogos recentes para observar o reuso. O log mostra as capas residentes; o Spine mantém 11 visíveis e transições podem manter até 14, reduzindo novamente ao voltar à Lista.
3. Para um jogo com TITLE_ID conhecido, copie `TITLE_ID.jpg`, `TITLE_ID_INSIDE.jpg` e `TITLE_ID_DISC.png` para `/dev_hdd0/PS3COVERS`. Feche a caixa e pressione START.
4. Abra com L3: verifique interior esquerdo e direito, parte superior, centro/lombada e texto sem reflexo ou inversão; confira a arte do disco, o furo e o retorno animado.
5. Feche com Círculo e L3. Abra e feche também no meio da animação. Confirme que X monta em vez de fechar.
6. Com webMAN ativo, aperte X com a caixa aberta e depois com ela fechada. Verifique retorno ao XMB após a confirmação e abra o jogo pelo ícone do disco. Um segundo X durante a montagem não deve criar outra solicitação.
7. Verifique um jogo sem artes opcionais: interior neutro e disco padrão. Depois de fechar, selecione esse jogo e abra; não deve reutilizar a imagem de outro jogo.
8. Troque uma arte no HDD, feche e aperte START: a nova imagem deve aparecer na próxima abertura.
9. Experimente uma arte inválida e falta de conexão com webMAN: a biblioteca deve permanecer utilizável. Na montagem, Círculo cancela a operação e sai ao XMB.

Para diagnóstico, recolha `/dev_hdd0/tmp/PS3_GAME_ORBIT_FIX33.log` ou o fallback em `/dev_hdd0/game/PGORBT301/USRDIR/`. Informe layout, TITLE_ID, formato, dimensões das três imagens e o botão usado se houver falha.

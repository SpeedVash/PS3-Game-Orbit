# Teste físico da v1.3.2

Compilação e testes no computador concluídos; o teste físico desta versão no PS3 está pendente.

1. Instale sobre a 1.3.1 e confira a primeira tela: v1.3.2 / Criado por SpeedVash. Verifique que favoritos e layout foram mantidos.
2. Navegue pelos três layouts, incluindo trocas rápidas, uma biblioteca de 57 jogos, retornos a capas recentes e jogos sem arte. Observe pausas e depois confira PERF no log.
3. SELECT deve conter somente controles. Feche ajuda e abra START, em caixa fechada e aberta. Círculo deve fechar apenas o menu.
4. Altere o nome de um jogo, confirme e reabra o aplicativo. Confira persistência, montagem, ID/capas originais preservadas. Repita cancelando no teclado.
5. Substitua uma capa manualmente. START → Recarregar capas deve mostrar a nova imagem sem descartar as dos outros jogos.
6. Em um pendrive FAT32, coloque na raiz ID.jpg, ID_INSIDE.jpg e ID_DISC.png do jogo selecionado. Importe pelo menu e abra a caixa para conferir as três imagens.
7. Tente importar com imagem faltando e com arquivo corrompido. A atualização deve ser recusada e as imagens anteriores continuar disponíveis.
8. Confira um disco PNG com arte colorida no centro: a imagem deve chegar até o furo físico; a caixa continua com plástico neutro transparente.
9. X, fora do menu, deve montar com caixa aberta ou fechada. Confira retorno ao XMB após confirmar a ID e abra pelo ícone de disco.
10. Atualizar biblioteca deve reconhecer novos jogos, limpar os três caches e manter nomes/favoritos.

Log: /dev_hdd0/tmp/PS3_GAME_ORBIT_FIX35.log, com fallback no USRDIR do aplicativo. Feche normalmente para forçar a gravação do último lote. Compare read/decode/upload/interface com capas iguais às usadas na 1.3.1; médias e máximos são acumulados desta sessão.

Arquivos e nomes de capas, comportamento para ISOs e limites de memória estão no README do GitHub.

# Gerenciador HD/USB da v1.5.1

Abra **START → Gerenciador de arquivos HD/USB**. A tela de entrada avisa que o fundo animado será pausado. X continua; Círculo retorna. Ao sair, o fundo volta à preferência anterior.

## Exemplo: copiar vários arquivos do HD para o USB

1. No painel esquerdo, abra a pasta de origem no HD com X.
2. Pressione Quadrado nos arquivos desejados. Eles ficam azuis. SELECT marca/desmarca todos os arquivos daquela pasta.
3. Troque para o painel direito com direita ou R2. Abra a pasta de destino no USB. Para escolher um dispositivo, pressione Círculo até chegar à lista de HD/USB; R3 atualiza os dispositivos conectados.
4. Volte ao painel esquerdo. Pressione Triângulo para copiar os arquivos marcados para o diretório exibido à direita. Sem marcações, será copiado somente o arquivo destacado.
5. Acompanhe o progresso. Se já houver um arquivo com o mesmo nome, escolha sobrescrever um, ignorar um, sobrescrever todos, ignorar todos ou cancelar o lote.
6. Arquivos concluídos ficam verdes nos dois painéis. A marcação azul é removida da origem concluída. Os destaques verdes duram até sair do gerenciador.

Para USB → HD, use o USB como origem e o HD como destino. O mesmo procedimento serve entre pastas do HD ou entre USBs disponíveis. Cada painel preserva pasta e posição ao alternar.

## Controles

| Botão | Ação |
| --- | --- |
| Cima / baixo | Navegar pelos itens |
| Esquerda / direita ou L2 / R2 | Escolher painel |
| L1 / R1 | Navegar oito itens por vez |
| X | Entrar em pasta ou dispositivo; confirmar diálogo |
| Quadrado | Marcar/desmarcar arquivo |
| SELECT | Marcar/desmarcar todos os arquivos na pasta ativa |
| Triângulo | Copiar para a pasta do outro painel |
| R3 | Atualizar pastas e dispositivos |
| Círculo | Voltar; durante cópia, cancelar o arquivo atual e o restante do lote |
| START | Sair; durante cópia, cancelar e sair |

## Comportamento da cópia

A cópia conserva os bytes da origem. Não move/apaga arquivos nem copia pastas recursivamente. A seleção pode reunir até 4096 arquivos, inclusive em pastas diferentes; nomes repetidos no mesmo lote precisam ser copiados separadamente. A opção de sobrescrever/ignorar todos é reiniciada em cada lote.

O cancelamento preserva os arquivos já concluídos e o destino anterior do arquivo em andamento. Cada arquivo usa um temporário, verificação de leitura/gravação e troca com backup. Uma origem alterada durante a leitura é rejeitada. O espaço disponível é consultado no volume de destino.

O HD aceita tamanhos de 64 bits. FAT32 permite no máximo 4 GiB menos 1 byte por arquivo; o gerenciador informa quando esse limite seria excedido. Esta versão usa os dispositivos montados pelo sistema e não acrescenta suporte a NTFS/exFAT. R3 permite detectar USB conectado após a entrada.

Se houver falha, anote a etapa e o código da mensagem. O diagnóstico fica em `/dev_hdd0/tmp/PS3_GAME_ORBIT_FIX41.log`, com alternativa no USRDIR do aplicativo. Informe também caminho de origem, destino, HEN/CFW e modelo do PS3.

Para redimensionar artes, use **Triângulo → Copiar capas do USB** ou **START → Atualizar todas as capas via USB** na biblioteca. O gerenciador copia arquivos sem redimensionar.

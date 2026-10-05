# Teste físico da v1.3.3

A compilação e os testes no computador foram concluídos. O teste desta versão no PS3 ainda está pendente.

1. Instale o PKG por cima da versão anterior. Confira **v1.3.3 / Criado por SpeedVash**, favoritos, nomes salvos e layout.
2. Navegue pelos três layouts com as mesmas capas de aproximadamente 1000 pixels usadas na 1.3.2. Compare trocas rápidas, retornos a jogos recentes e jogos sem arte.
3. Pare no mesmo jogo por cerca de dois segundos e pressione L3. Confira que inside e disco aparecem. Compare com abrir imediatamente após uma seleção nova; as artes ainda não preparadas podem aparecer em quadros sucessivos.
4. Abra START e alterne **Fundo animado**. Ligado deve mover as ondas suavemente; desligado deve mostrar a imagem estática. Feche normalmente, abra o aplicativo e confira a preferência salva. Verifique os três layouts e a caixa aberta nos dois modos.
5. No pendrive FAT32, crie **PS3COVERS na raiz**. Coloque somente `ID.jpg` e importe. Apenas a capa exterior deve mudar; inside e disco anteriores devem permanecer. Repita com apenas `ID_INSIDE.jpg`, apenas `ID_DISC.png` e com dois arquivos.
6. Coloque uma arte válida e outra corrompida. A válida deve ser copiada; a corrompida deve manter o tipo anterior, com erro informado no menu. Arquivos soltos na raiz ou em subpastas de PS3COVERS não devem ser importados.
7. Para `Bayonetta.iso` sem ID reconhecida, use `PS3COVERS/Bayonetta.jpg`, `Bayonetta_INSIDE.jpg` e/ou `Bayonetta_DISC.png`. Importe somente uma arte e depois o conjunto. Confira também nome com espaços e `.ISO` em maiúsculas.
8. Para ISO com ID reconhecida no nome, teste arquivos pela ID e pelo nome completo sem extensão. A importação deve funcionar nas duas formas; a cópia no HDD usa a ID quando conhecida.
9. Recarregar capas deve reler as três artes apenas do jogo escolhido. Atualizar biblioteca deve reconhecer novos títulos e manter nomes/favoritos/layout/fundo. Confira teclado e cancelamento ao renomear.
10. Pressione X com a caixa fechada e aberta, fora do menu. Confira montagem pelo webMAN e retorno ao XMB após confirmação. Abra pelo ícone de disco.
11. Confira plástico neutro/transparente, capa inteira na orientação correta e arte do disco até o furo físico. SELECT deve mostrar somente controles.
12. Com biblioteca vazia, START deve permitir configurar o fundo e atualizar a biblioteca.

Saia normalmente para gravar o último lote. Log principal: **`/dev_hdd0/tmp/PS3_GAME_ORBIT_FIX36.log`**, com fallback no USRDIR do aplicativo.

Compare `PERF 1.3.3` (`read`, `decode`, `upload`, `interface`) com a 1.3.2 usando as mesmas imagens e uma sequência de seleção equivalente. Quantidade, total, média e máximo são acumulados por sessão. `OPT 1.3.3` informa reutilizações, alocações, bytes livres no pool e desenhos das ondas.

Os tempos de upload medem preparação/cópia, não conclusão da GPU. Testes no computador não substituem medições de HD, decode nativo, RSX e saída de TV no PS3.

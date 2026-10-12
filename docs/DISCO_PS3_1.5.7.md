# Disco de PS3 na biblioteca 3D

A v1.5.7 inclui o jogo exposto pelo leitor na biblioteca. Ele aparece em **Todos**, identificado como **Disco**, e usa a caixa PS3 dos três layouts. L3 abre a caixa e mostra o disco com as mesmas animações dos jogos do HD e USB.

## Artes pela ID

A ID e o nome são lidos do `PS3_GAME/PARAM.SFO` do disco. Por exemplo, para `BLES01234`, coloque em `/dev_hdd0/PS3COVERS`:

- `BLES01234.jpg` — capa completa, incluindo frente, lombada e verso.
- `BLES01234_INSIDE.jpg` — arte do interior.
- `BLES01234_DISC.png` — arte do disco.

Os arquivos são opcionais. JPEG, JPG e PNG são aceitos, incluindo extensões maiúsculas. Se faltar a capa completa, o ícone do próprio disco pode ser usado como imagem frontal; sem ele, a caixa permanece neutra. Interiores e discos sem arte usam a aparência padrão. As opções de importar pelo USB e recarregar capas continuam disponíveis.

## Inserção, troca e retirada

O app lê o disco ao iniciar e monitora mudanças em segundo plano. Durante navegação, menus, inspeção da caixa ou uso do gerenciador, a atualização visual aguarda um momento sem interação. Os jogos do HD e USB conservam seus índices e texturas quando a entrada do disco é adicionada, trocada ou retirada.

Cada ID tem sua própria identidade para favoritos, nomes e estilo da caixa. Trocar o disco não transfere essas configurações ao novo título. Um disco pode ser marcado como favorito; ele não pertence aos filtros HD e USB.

X confirma que a ID ainda corresponde ao disco mostrado e volta ao XMB, sem enviar montagem ao webMAN. Inicie o jogo pelo ícone do disco no XMB. O leitor e seus arquivos são apenas consultados; esta função não ejeta nem desmonta conteúdo.

## Conteúdo montado

Uma ISO ou pasta montada pode ocupar `/dev_bdvd`. Quando as consultas Cobra/HEN identificam emulação ou redirecionamento do leitor, essa entrada não é apresentada como disco físico. Para visualizar o original, primeiro disponibilize o leitor usando o seu gerenciador de jogos. O Orbit não desmonta uma ISO automaticamente.

Se a consulta do tipo de disco não estiver disponível, a entrada **Disco** representa o jogo PS3 atualmente acessível em `/dev_bdvd`; a origem física não é confirmada nesse caso. DVDs de vídeo, discos de áudio, PS1 e PS2 não são adicionados por esta função. PS1 permanece em hiato.

## Teste no PS3

1. Insira um disco de PS3 e abra o Orbit em Todos. Confira nome, ID e indicação Disco.
2. Com as artes da ID em PS3COVERS, confira frente, lombada, verso, interior e disco ao abrir com L3.
3. Alterne Clássico, Spine e Lista; confirme a resposta e a indicação [Disco] na lista.
4. Insira, retire e troque discos com a biblioteca aberta. Aguarde parar a navegação para a entrada atualizar.
5. Marque um disco como favorito, troque-o e confira que o novo título não herda o favorito ou nome do anterior.
6. Use X; confira o retorno ao XMB e inicie pelo ícone do disco.
7. Repita sem artes e com o leitor vazio; a biblioteca deve continuar utilizável.

A detecção e o retorno ao XMB precisam ser confirmados no PS3 com HEN/CFW. Os testes no computador validam metadados, cache, concorrência, seleção e as transições do programa; não simulam o leitor físico.

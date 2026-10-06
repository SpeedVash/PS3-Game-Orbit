# PS3 Game Orbit

![PS3 Game Orbit](pkgfiles/ICON0.PNG)

**v1.4.2 — criado por SpeedVash.** Biblioteca de jogos PS3 com capas completas, caixas 3D articuladas, layouts Clássico, Spine e Lista, favoritos e montagem pelo webMAN MOD.

## Novidades da v1.4.2

- **Acabamento transparente e sem o tom cinza da lombada:** os relevos usam plástico de cor neutra, com normais suavizadas nas curvas. A faixa reta sobreposta foi substituída pela espessura do próprio invólucro.
- **Extremidades arredondadas e contínuas:** a lombada acompanha as curvas superiores e inferiores da caixa. As duas peças usam os mesmos pontos de união; a articulação transporta esses pontos durante a abertura.
- Mesmo modelo fechado e aberto, limite de **160°**, disco mais para fora/frente, bordas transparentes interna e externa de **0,5 mm** e verso prateado.
- **v1.4.2 em todas as telas:** início, cabeçalho da biblioteca e opções do homebrew usam a versão central do projeto. A palavra TESTE foi retirada da tela inicial.
- Configurações, favoritos, nomes, último jogo, USB parcial/em lote e montagem webMAN mantidos.
- Caches mantidos: Clássico 7/7/7, Lista 5/5/5 e Spine 15/15/15, com três vizinhos visíveis de cada lado.

Compilação nativa e testes no computador concluídos. O teste físico desta versão no PS3 está pendente; consulte [validação](docs/TESTE_1.4.2.md). A iluminação continua sendo a do renderizador aprovado: varia com o ângulo e não simula reflexão óptica real do plástico. Leitura/decodificação inicial continuam síncronas.

## Instalação

1. Coloque `PS3_GAME_ORBIT_v1.4.2.gnpdrm.pkg` em um pendrive FAT32.
2. Instale pelo gerenciador de pacotes do XMB, por cima da versão anterior. A identidade continua **PGORBT301**, com `APP_VER 01.09`.
3. No Super Slim, habilite HEN antes de abrir. Para montar jogos, mantenha o webMAN MOD ativo.
4. Use X para montar o jogo, com a caixa aberta ou fechada. Após a confirmação, o app volta ao XMB; abra o jogo no ícone do disco.

## Modelo 3D

Prévias geradas da geometria compilada no computador: [caixa fechada](assets/previews/CAIXA_FECHADA_1_4_2.png), [acabamento superior](assets/previews/LOMBADA_TOPO_1_4_2.png), [acabamento inferior](assets/previews/LOMBADA_BASE_1_4_2.png) e [caixa aberta](assets/previews/CAIXA_ABERTA_1_4_2.png). Usam arte de demonstração e iluminação difusa equivalente; não são capturas de um PS3.

## Menus

Prévias da interface geradas no computador: [tela inicial v1.4.2](assets/previews/TELA_INICIAL_1_4_2.png), [homebrew](assets/previews/MENU_HOME_1_4_2.png) e [jogo](assets/previews/MENU_JOGO_1_4_2.png).

| Botão | Opções |
| --- | --- |
| **Triângulo — jogo** | Alterar nome · Copiar capas do USB · Recarregar capas · Adicionar/Remover dos favoritos |
| **START — homebrew** | Atualizar biblioteca · Fundo animado · Atualizar todas as capas via USB · Salvar último jogo visto |

Cima/baixo selecionam, X confirma, Círculo fecha. Triângulo fecha o menu do jogo; START fecha o menu do homebrew. É possível trocar de menu com esses botões. START também funciona sem jogos no filtro atual. Durante uma cópia em lote, Círculo cancela antes do próximo jogo; as imagens já concluídas ficam salvas.

Alterar nome modifica apenas o título exibido, sem renomear ISO/pasta/ID/PARAM.SFO ou arquivos de arte. Recarregar capas relê as três artes daquele jogo e permite tentar novamente arquivos que falharam. Atualizar biblioteca refaz a busca pelos jogos e limpa os caches, preservando as preferências.

## Imagens no pendrive e no HDD

Crie **PS3COVERS na raiz do pendrive**. As imagens devem ficar diretamente nessa pasta, sem subpastas.

| Arte | Exemplo com ID | ISO sem ID reconhecida | Saída no HDD |
| --- | --- | --- | --- |
| Capa completa | `PS3COVERS/BLES01287.jpg` | `PS3COVERS/Bayonetta.jpg` | 1000 × 550, JPG |
| Inside Cover Full | `PS3COVERS/BLES01287_INSIDE.jpg` | `PS3COVERS/Bayonetta_INSIDE.jpg` | 1000 × 550, JPG |
| Disco | `PS3COVERS/BLES01287_DISC.png` | `PS3COVERS/Bayonetta_DISC.png` | 500 × 500, PNG |

Use **Triângulo → Copiar capas do USB** para o jogo atual, ou **START → Atualizar todas as capas via USB** para a biblioteca inteira, incluindo jogos fora do filtro atual.

**Uma imagem já basta.** Tipos ausentes ou inválidos preservam a arte anterior; os outros tipos válidos ainda são copiados. A busca percorre `/dev_usb000` até `/dev_usb007`, somente dentro de `PS3COVERS`. JPG, JPEG e PNG são aceitos como origem, inclusive extensões maiúsculas. `_INSIDE` e `_DISC` devem permanecer em maiúsculas. A origem no USB fica intacta.

O destino é **`/dev_hdd0/PS3COVERS`**. A cópia usa dimensões exatas, sem manter proporções diferentes: prepare a arte no aspecto 1000:550 para evitar distorção. A transparência do PNG do disco é preservada; capa/inside são convertidos para JPG. Arquivos antigos concorrentes são removidos apenas para o tipo atualizado, após gravação e troca com backup.

Para ISO, use o nome exato sem `.iso`/`.ISO`, preservando espaços e maiúsculas/minúsculas. Exemplo: `My ISO Game.ISO` → `My ISO Game_DISC.png`. Uma ID reconhecida tem prioridade; a origem pelo nome da ISO também é aceita e gravada no HDD com a ID. Sem ID, o destino mantém o nome da ISO. A ID interna do ISO ainda não é lida; o scanner reconhece uma ID de quatro letras e cinco números juntos no nome do arquivo.

Jogos em pasta usam a TITLE_ID. Renomear um jogo pelo menu não muda os nomes esperados das imagens. Cópias manuais no HDD não são redimensionadas: use Triângulo → Recarregar capas depois de copiá-las.

Modelos de arte: [Inside Cover Full 1000×550](assets/examples/INSIDE_COVER_FULL_MODELO_1000x550.png) e [disco 500×500](assets/examples/DISC_EXEMPLO_500.png).

A capa completa segue **verso · lombada · frente**. O inside segue **interior esquerdo · lombada · interior direito**, usando a mesma área e o mesmo recorte central. O disco é recortado pela geometria circular com furo de 15 mm. A arte cobre a região opaca de raio 8,0 até 59,5 mm. A borda de 7,5 a 8,0 mm e a de 59,5 a 60,0 mm ficam transparentes. Sem imagens opcionais, o app usa o interior neutro e o disco padrão.

## Controles

| Controle | Função |
| --- | --- |
| Esquerda/direita ou analógico esquerdo | Trocar de jogo |
| Cima/baixo no layout Lista | Navegar na lista |
| Analógico direito | Girar/inclinar a caixa |
| Quadrado | Alternar Clássico → Spine → Lista |
| L1 / R1 | Alternar Todos / Favoritos / USB / HD |
| L3 | Abrir/fechar a caixa e retirar/recolocar o disco |
| X | Montar o jogo pelo webMAN MOD |
| Triângulo | Opções do jogo |
| START | Opções do homebrew |
| L2 / R2 | Zoom |
| R3 | Restaurar a visualização |
| Cima em Clássico/Spine | Rotação automática |
| SELECT | Ajuda dos controles |
| Círculo | Fechar menu/caixa, cancelar operação ou sair |

## Caches e pré-carga

| Layout | Jogos anteriores | Atual | Jogos seguintes | Limite de cada cache |
| --- | ---: | ---: | ---: | ---: |
| Clássico | 3 | 1 | 3 | 7 capas, 7 inside, 7 discos |
| Lista | 2 | 1 | 2 | 5 capas, 5 inside, 5 discos |
| Spine | Recentes | Atual | Recentes | 15 capas, 15 inside, 15 discos |

Janelas circulares seguem o filtro ativo e não repetem jogos em bibliotecas curtas. A pré-carga considera a direção da navegação; Após 900 ms de espera também prepara inside/disco. Ao abrir a caixa, ambas as imagens do jogo atual são preparadas antes do desenho. Uma imagem é preparada por etapa, alternando trabalho de capa e interior/disco. Spine mantém o cache de artes recentes e prepara inside/disco do jogo atual. Caixas que estão terminando a transição mantêm sua capa até o último quadro reconhecido pela GPU; esse período pode reter uma capa adicional ao limite de repouso.

Buffers de decodificação e de texturas liberadas continuam sendo reaproveitados. Os logs seguem agrupados e incluem tempo de leitura, decodificação, upload e interface, em milissegundos.

## Configurações e compatibilidade

Preferências unificadas: `/dev_hdd0/game/PGORBT301/USRDIR/PS3_GAME_ORBIT_SETTINGS.dat`. Arquivos de nomes, favoritos/seleção, layout e fundo usam o mesmo diretório. Os dados antigos de `/dev_hdd0/tmp` continuam sendo lidos para migração. Cada gravação usa um temporário sincronizado e uma cópia `.bak` da versão anterior; o PS3 não precisa sobrescrever um arquivo via rename. Se o arquivo principal estiver ausente, a leitura usa o backup. Para restaurar padrões manualmente, remova o arquivo de configuração e sua cópia `.bak`. Arquivos de favoritos/seleção, nomes, layout e fundo das versões anteriores continuam compatíveis. As preferências antigas são migradas para USRDIR quando ainda não existe uma configuração atual válida.

“Salvar último jogo visto” vem ligado para preservar o comportamento anterior. Desligado, o app começa no primeiro jogo do filtro salvo. Se o último jogo não estiver disponível, começa no primeiro disponível. Alterações são gravadas após um breve intervalo e ao sair normalmente.

Diagnóstico: `/dev_hdd0/tmp/PS3_GAME_ORBIT_FIX39.log`; alternativa em `USRDIR`. Consulte [o roteiro de teste](docs/TESTE_1.4.2.md) ao reportar um problema.

## Fontes, compilação e GitHub

- [Compilar a v1.4.2](docs/BUILD.md)
- [Passo a passo para publicar](docs/GITHUB_1.4.2.md)
- [Notas prontas para o release](docs/RELEASE_NOTES_1.4.2.md)
- [Histórico de mudanças](CHANGELOG.md)
- [Créditos e licenças](docs/CREDITS.md)

A malha exterior JFX licenciada não é redistribuída em formato editável. Para compilar localmente, use sua própria cópia obtida na fonte indicada em `assets/jfx_bluray/README.md` e execute `scripts/prepare-jfx.py --source SEU_ARQUIVO.zip`. O ZIP público contém o código das melhorias procedurais, os shaders arquivados e as instruções; o PKG já está compilado. A renderização PS3 usa os shaders RSX preservados e transparência por alpha, portanto a iluminação difere do preview PBR aprovado.

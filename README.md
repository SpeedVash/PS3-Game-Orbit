# PS3 Game Orbit 2.1.9

A **v2.1.9** permite escolher qual USB liberar no Padrão e no PLUS. O Orbit pausa apenas a unidade selecionada e identifica qual USB pode ser retirado. Veja [NOTAS_2.1.9.md](docs/NOTAS_2.1.9.md).

A **v2.1.8** passou a solicitar a desmontagem ao PS3 e a exibir **“Não retire o USB”** quando a liberação não pôde ser confirmada.

A **v2.1.7** passou a mostrar a versão ao lado do logotipo do PLUS na abertura e na biblioteca.

A **v2.1.6** salva texturas de PS3 e PS1 no HD interno. Na primeira abertura, o Orbit prepara as capas da tela e, enquanto a biblioteca fica parada, prepara as capas restantes em segundo plano. Nas próximas aberturas, reutiliza as imagens já preparadas. No Spine, as caixas ao lado podem mostrar uma versão pequena da capa enquanto a imagem completa entra na memória gráfica. Funciona tanto no Padrão quanto no PLUS.

A **v2.1.5** inclui a capa padrão aprovada do Orbit. Frente, lombada e verso aparecem nas caixas de PS3 que estão sem capa, incluindo **Disco físico no leitor**. A capa própria do jogo tem prioridade. Se houver apenas uma imagem frontal, a arte padrão preenche o verso e a lombada.

A **v2.1.4** mantém o disco do leitor visível quando uma ISO ou pasta está montada. Na aba **PS3**, origem **Todos**, aparece **Disco físico no leitor** enquanto o jogo virtual ocupa o leitor. Atualizar a biblioteca preserva a montagem. Selecione essa entrada e pressione **X — Restaurar** para desmontar o jogo virtual e ler o título do disco físico sem sair do Orbit.

O fluxo de restauração funciona nas duas interfaces e inclui as melhorias de navegação de PKGs da v2.1.3. Veja `docs/NOTAS_2.1.4.md`.

Desde a **v2.1.0**, o aplicativo reúne o **PS3 Game Orbit padrão** e o **PS3 ORBIT PLUS** no mesmo aplicativo. O PLUS traz a interface aprovada, com navegação por plataforma e origem, painéis em azul, tipografia maior e opções redesenhadas.

## Alternar a interface

1. Abra o Orbit e pressione **START**.
2. Entre em **Interface**.
3. Escolha **PS3 Game Orbit padrão** ou **PS3 ORBIT PLUS** com o direcional e confirme com **X**. **Círculo** volta sem aplicar.

A troca é imediata e a escolha fica salva para a próxima abertura. Na primeira instalação sem uma escolha salva, o visual inicial é o **Padrão**. Os dois visuais compartilham os mesmos jogos, favoritos, configurações, catálogo e covers. Alternar a interface não faz uma nova leitura da biblioteca nem descarta as imagens já carregadas.

O identificador continua **PGORBT301**, com versão de instalação **02.19**. O log desta versão é `/dev_hdd0/tmp/PS3_GAME_ORBIT_2_1_9.log`.

O visual padrão conserva o desenho da v2.0.4. As caixas PS3 e PS1, materiais, transparência aprovada de **96%** nas áreas FRONT/BACK de PS1 e animações permanecem iguais à base. Os dois visuais oferecem Clássico, Spine e Lista; PS3/PS1 separados; Todos/HD/USB/Favoritos; seleção de CD de PS1; importação de covers por USB; fundo personalizado; ventoinha; cópia de disco e instalação de PKGs.

A compilação nativa e os testes no PC passaram. O tempo de carregamento no console depende do HD e precisa ser medido em hardware. As imagens em `assets/previews/2.1.0/` são prévias no PC com o código da interface e a geometria original, usando artes ilustrativas.

Os recursos abaixo são herdados da v2.0.4.

## Spine de PS1

O modo mostra o jogo selecionado e **três jogos de cada lado**, quando a biblioteca tem pelo menos sete jogos. O limite antigo de pacotes gráficos descartava algumas caixas PS1; agora as sete montagens completas permanecem no desenho. O controlador RSX continua dividindo os comandos em segmentos confirmados e faz uma apresentação final por quadro.

As BACK fornecem as lombadas PS1. Ao entrar no Spine, o Orbit prepara a caixa selecionada em tamanho completo e a fila visível de até sete jogos. As capas dos vizinhos podem aparecer como prévias até que as texturas completas fiquem prontas. As caixas próximas e as capas internas continuam a ser preparadas em segundo plano.

## Catálogo salvo para HD e USB

A primeira abertura desta versão faz uma leitura completa e salva um índice por armazenamento em `/dev_hdd0/game/PGORBT301/USRDIR/catalog17/`. O índice inclui jogos PS3/PS1, títulos, IDs, caminhos das covers e todos os CDs de cada jogo. Nas próximas aberturas, o Orbit reutiliza esses dados e confere apenas os armazenamentos e as informações das pastas principais, sem varrer os arquivos de cada jogo novamente.

Mudanças nas pastas principais podem renovar o índice automaticamente. **Depois de adicionar, remover ou alterar jogos, discos ou covers, use START → Atualizar biblioteca**, especialmente para mudanças dentro de uma pasta já existente. Essa opção faz uma leitura completa e atualiza o índice. Catálogos antigos ou corrompidos são reconstruídos. Um USB desconectado deixa de contribuir com jogos; ao reconectar o mesmo volume, os caminhos são associados ao ponto de montagem atual.

Quando a identificação nativa de um USB não está disponível, o Orbit cria nele o pequeno arquivo `PS3_GAME_ORBIT_VOLUME.ID` para reconhecer esse volume nas próximas aberturas e em outra porta. O catálogo com os dados dos jogos permanece no HD interno. A leitura do disco físico continua atualizada.

O índice guarda os caminhos. O cache de imagens em `/dev_hdd0/game/PGORBT301/USRDIR/artwork216/` guarda as texturas prontas, inclusive de jogos USB. O Orbit confere se a imagem original mudou antes de reaproveitar a textura. Para forçar a leitura de uma arte de um jogo, use **Opções do Jogo → Recarregar capas**.



## Instalar

1. Instale `PS3_GAME_ORBIT_v2.1.9.pkg` pelo XMB, sobre a versão atual.
2. Reinicie o PS3 para carregar o serviço desta versão. Ative o HEN, quando aplicável, e abra o Orbit.
3. Organize os jogos em `PSXISO` no HD interno ou em um USB FAT32. Use uma pasta por jogo e uma subpasta por CD.
4. Conecte o USB antes de abrir o Orbit. Selecione a plataforma com **L1/R1** e confira **Origem: Todos**. Use **Atualizar biblioteca** se necessário.
5. Pressione **X** no jogo. Jogos com mais de um CD abrem **Selecionar disco de PS1**. Escolha com o direcional e confirme com **X**; **Círculo** cancela. A mesma escolha está disponível nas **Opções do Jogo**.

Exemplo de caminhos:

```text
/dev_usb000/PSXISO/Final Fantasy IX/Disco 1/FFIX.cue
/dev_usb000/PSXISO/Final Fantasy IX/Disco 1/FFIX.bin
/dev_usb000/PSXISO/Final Fantasy IX/Disco 2/FFIX.cue
/dev_usb000/PSXISO/Final Fantasy IX/Disco 2/FFIX.bin
/dev_usb000/PSXISO/Final Fantasy IX/Disco 3/FFIX.cue
/dev_usb000/PSXISO/Final Fantasy IX/Disco 3/FFIX.bin
/dev_usb000/PSXISO/Final Fantasy IX/Disco 4/FFIX.cue
/dev_usb000/PSXISO/Final Fantasy IX/Disco 4/FFIX.bin
```

Para um CD, também é aceito `PSXISO/Nome do jogo/Jogo.cue` com seus BINs na mesma pasta, ou uma imagem diretamente em `PSXISO`. Os nomes `Disco`, `Disc` e `CD` seguidos de número são reconhecidos, inclusive quando uma pasta está vazia; nesse caso o seletor mostra o erro.

Na Lista, use Cima/Baixo para navegar; no Clássico/Spine, Cima controla o autogiro.

A caixa é escolhida automaticamente, sem opção manual adicional. **L3** abre/fecha a inspeção e exibe todos os CDs do jogo. Cada disco mantém sua própria arte. O seletor define o disco para iniciar no XMB; não troca o CD durante a execução do jogo.

## Capas da caixa simples (um ou dois CDs)

Aceita PNG, JPG e JPEG. Coloque as capas na pasta principal do jogo. Para vários CDs, coloque um `DISC.png` na subpasta de cada CD:

| Arquivo | Medida recomendada (largura × altura) | Uso |
| --- | --- | --- |
| `FRONT.png` | 1000 × 1000 | Somente a frente do livreto, presa à tampa. |
| `BACK.png` | 1000 × 780 | Traseira externa, com as duas lombadas. |
| `INSIDE1.png` | 1000 × 780 | Mesmo espaço da BACK, no lado interno atrás do CD. |
| `INSIDE2.png` | 1000 × 1000 | Parte interna do livreto, presa à tampa. |
| `DISC.png` | 1000 × 1000 | Arte do CD, centralizada. |

Em `BACK` e `INSIDE1`, use **50 pixels de lombada esquerda + 900 pixels de painel central + 50 pixels de lombada direita**, com 780 pixels de altura. Monte cada imagem como ela deve ser vista do respectivo lado; não espelhe manualmente a `INSIDE1`. As duas têm o mesmo recorte e dimensões. Outras resoluções são aceitas com a mesma proporção.

Exemplo: `PSXISO/Nome do jogo/FRONT.png`, `BACK.png`, `INSIDE1.png`, `INSIDE2.png`, `DISC.png`, `Jogo.cue` e os BINs correspondentes. Não é preciso juntar as quatro capas numa imagem. `FRONT` tem prioridade sobre o antigo `COVER`; o antigo `INSIDE` é aceito como alternativa para `INSIDE2` quando esta não existe. Se uma capa não existir ou estiver inválida, somente aquela face usa o material neutro.

Para uma imagem solta diretamente em `PSXISO`, prefira `Nome do jogo_FRONT.png`, `Nome do jogo_BACK.png`, `Nome do jogo_INSIDE1.png`, `Nome do jogo_INSIDE2.png` e `Nome do jogo_DISC.png`; os mesmos sufixos são aceitos em `/dev_hdd0/PS3COVERS`.

Os arquivos em `assets/models/ps1/templates191/` são exemplos coloridos para conferir cada posição e as lombadas. As prévias usam esses exemplos e não são capturas do PS3.

## Caixa com três ou quatro CDs

Usa o modelo fornecido em `boite-de-jeux-ps120240703-1-f1tep2.zip`, pasta `Crafitys/boite-de-jeux-ps1/`, com a adaptação de quatro suportes e a animação aprovadas na prévia. A caixa provisória anterior foi substituída.

Os três suportes têm fundos e encaixes centrais fechados, presentes com a caixa fechada e aberta. O suporte da base e o da tampa são transparentes; a bandeja articulada central e a faixa lateral da dobradiça são pretas e opacas. As capas internas ficam atrás dos suportes; nenhum furo de impressão foi reaberto. Os encaixes são: CD 1 na base; CD 2 na frente da bandeja central; CD 3 no verso da mesma bandeja; CD 4 na tampa, quando existir. Os CDs da bandeja e da tampa acompanham essas peças até a retirada. Fora da caixa, todos ficam retos.

Coloque um `DISC.png` em cada subpasta: `Disco 1/DISC.png`, `Disco 2/DISC.png` etc. Também são aceitos `DISC1.png` a `DISC4.png` na pasta principal. A arte de um disco ausente não é substituída pela de outro: usa a imagem padrão. A escolha de qual CD montar continua no seletor aberto por **X** e não muda a disposição das artes.

`COVER.png` continua aceito para a capa completa (traseira 48%, lombada 4%, frente 48%). `FRONT.png` é aceito quando não houver COVER. `BACK.png` fornece a traseira e as duas lombadas. **Na caixa de 3/4 CDs, use 1000 × 728 pixels**, com o seguinte recorte exato:

| Parte da BACK de 3/4 CDs | Largura × altura | Colunas de pixels |
| --- | --- | --- |
| Lombada esquerda | **99 × 728 px** | 0 a 98 |
| Traseira | **802 × 728 px** | 99 a 900 |
| Lombada direita | **99 × 728 px** | 901 a 999 |

As dobras ficam em **x = 99** e **x = 901**. Esquerda e direita são vistas pela traseira externa, sem espelhar. Em outra resolução, mantenha **9,9% / 80,2% / 9,9%** e a proporção **1000:728**. O gabarito PNG e o SVG editável estão em `assets/models/ps1/templates196/`; substitua a arte e remova as guias antes de salvar como `BACK.png`. A caixa simples de 1/2 CDs continua usando 1000 × 780 e 50/900/50; não use esse recorte na caixa maior.

O recorte anterior de 5/90/5 reservava pouca largura para a espessura da caixa maior e esticava a arte da lombada. Na 2.0.1, o papel foi ajustado sob a parede transparente: traseira de 140,18 × 127 mm e lombadas de 16,54 mm de profundidade. O recorte 99/802/99 das imagens existentes foi mantido; o encaixe interno exige uma pequena compressão horizontal das lombadas. O gabarito é destinado ao Orbit, não à impressão física.

`DISC.png` continua quadrada, por exemplo 1000 × 1000 px, em todas as caixas. Cada subpasta de CD usa sua própria arte.

 As duas lombadas acompanham a base em todas as fases. O fallback de COVER completo usa sua faixa central nas lombadas e o painel traseiro no verso. A organização simples continua aceitando as quatro capas independentes.

A forma base permanece em `assets/models/ps1/stl201/`, com os manifestos `NATIVE_IMPORT_201.json` e `NATIVE_COVER_203.json`. O segundo registra as áreas locais de 92% deste teste, aplicadas à geometria nativa por `src/ps1_cover202.inc`. Os modelos atuais exportados diretamente do código estão em `assets/models/ps1/native203/`; as imagens atuais estão em `assets/previews/2.0.4/`. As artes dessas prévias são demonstrativas.

## Carregamento da biblioteca de PS1 e PS3

As duas plataformas usam a mesma preparação de biblioteca e imagens. Na abertura, a tela de carregamento permanece até as imagens necessárias à primeira exibição estarem prontas. Ao trocar de plataforma ou origem, essa tela reaparece se houver imagens pendentes; se elas já estiverem em cache, a troca é imediata. **Atualizar biblioteca** apresenta a mesma tela durante a varredura e a preparação das imagens.

A conclusão depende da prontidão do carregador, sem espera artificial por tempo. A preparação inclui as capas internas e as artes dos CDs necessárias ao jogo selecionado. Uma imagem ausente ou inválida é resolvida com o material padrão, sem prender a biblioteca no carregamento. Círculo encerra durante a preparação, como na abertura. A montagem do disco pelo X permanece no fluxo existente.

## Copiar capas do USB

A importação pelo menu ainda usa o padrão antigo de capa, interior e disco único. FRONT/BACK/INSIDE1/INSIDE2 e artes por CD podem ser lidas diretamente das pastas do jogo no HD ou USB FAT32; a importação completa dessas artes pelo menu não foi adicionada nesta revisão.

## Listagem do USB

A varredura procura volumes nativos de `/dev_usb000` a `/dev_usb127`. O catálogo persistente do HDD continua ativo; um USB nativo é sempre relido para incorporar mudanças e não ficar preso a um resultado vazio. A identidade do volume continua independente da porta. Os leitores próprios de NTFS/exFAT para PS3 mantêm o caminho anterior de cache e leitura.

Se o USB ainda não aparecer após **Atualizar biblioteca**, envie `/dev_hdd0/tmp/PS3_GAME_ORBIT_2_1_9.log`. As linhas `SCAN192` separam os totais de PS3/PS1 e HDD/USB; as contagens mostradas nas prévias são apenas exemplos.

## Liberar o USB

1. Em **START → Liberar USB para retirada**, escolha a unidade pela identificação e capacidade. Com uma única unidade reconhecida e nenhuma pausada, o Orbit vai direto à verificação.
2. Aguarde **USB 000 liberado pelo Orbit para retirada** (o número mostrado depende da unidade). Remova somente o USB indicado. Se aparecer **Não retire o USB**, mantenha a unidade conectada e siga a indicação na tela. **Círculo** cancela a espera.
3. Para voltar a usar a unidade, reconecte-a e selecione **Reativar** na linha correspondente em **START → Gerenciar unidades USB**.

O Orbit deixa os jogos, capas e PKGs das outras unidades disponíveis. A unidade liberada fica fora da biblioteca e dos seletores até ser reativada ou até o Orbit ser reaberto. Atualizar a biblioteca também respeita a pausa.

## Formatos e limites desta versão

- PS1 no HD interno e USB FAT32: CUE/BIN BINARY com setores de 2352 bytes, faixas de dados/áudio e INDEX 01; ISO de dados com setores de 2048 bytes; BIN/IMG raw. Prefira CUE quando houver faixas de áudio.
- Um CUE com vários BINs continua sendo **um CD**. Na primeira montagem, o Orbit reúne suas faixas em uma imagem separada no HD e mostra a porcentagem. A fonte não é alterada; a imagem pronta é reutilizada nas montagens seguintes.
- O cache fica em `/dev_hdd0/game/PGORBT301/USRDIR/ps1_cache`. Reserve no HD o tamanho do CD escolhido. Para liberar espaço, apague os arquivos desse cache somente após desmontar o jogo. Não apague o cache do CD que está montado.
- CUEs com WAVE, gaps sintéticos não nulos, modos mistos de setor, imagens incompletas, CHD/PBP/CCD/MDF e mais de quatro CDs são recusados com mensagem. SBI/LSD não são copiados ao cache de vários BINs; para títulos protegidos que precisem desses dados, use BIN único com o arquivo auxiliar compatível com o payload.
- PS1 em NTFS/exFAT não integra esta versão. Se uma ISO de PS3 montada pelo leitor NTFS/exFAT do Orbit ainda estiver ativa, o Orbit pede reinicialização antes de montar PS1, para preservar o serviço validado.
- Antes da montagem de PS1, o controle de ventoinha do Orbit volta ao padrão do sistema. Sua preferência salva é mantida para a próxima abertura do Orbit.

## PKGs e validação

A fila de PKGs e a opção **Voltar ao Orbit ao terminar** mantêm exatamente o auxiliar da 1.8.9, cujo retorno foi confirmado no HEN. As preferências existentes e a geometria de PS3 são preservadas. O residente foi atualizado para fechar o leitor de ISOs NTFS/exFAT ao liberar o USB.

As versões anteriores registram a compilação PPC64, os testes de catálogo/seletor/montagem e a simulação de 14.400 quadros de desenho nativo. A 2.0.3 executou a compilação atual, a verificação dos limites de transparência, os dados PPC, a prévia, os GLBs e a integridade do pacote em `docs/validation203/`; a lógica de execução dessas bases foi preservada. A animação C++ foi comparada com a prévia aprovada em 2.613 matrizes. A geometria PS3, o residente e o auxiliar de PKGs permanecem iguais às bases validadas. A revisão 1.9.6 verificou os quatro CDs idênticos ao CD simples, fechamento dos três suportes, recorte 99/802/99 da BACK, transmissão nas janelas e lombadas com o mesmo alfa, desenho de produção e regressão PS3. O carregamento foi verificado com a rotina de produção e o worker real alternando PS1/PS3 sem desenhar a biblioteca antes da prontidão. As evidências estão em `docs/validation196/`.

A 1.9.7 acrescenta o teste com escritores de comandos do SDK, confirmações intermediárias e a reprodução do limite do corpo de desenho antigo; os logs ficam em `docs/validation197/`. A validação anterior fica em `docs/validation199/`: restauração exata da simples à base 1.9.4, exterior original da múltipla e quatro vazados na articulação, INSIDE1/INSIDE2 dos jogos vizinhos prontos antes da seleção, alternância PS1/PS3 com imagens grandes e limite conjunto de memória.

O teste visual e de desempenho em HEN/CFW ainda depende do console. Instale o PKG e confira jogos de dois, três e quatro CDs com **L3**. O seletor de montagem continua independente da inspeção.

Log do aplicativo: `/dev_hdd0/tmp/PS3_GAME_ORBIT_1_9_9.log`. O log do instalador continua em `/dev_hdd0/tmp/PS3_GAME_ORBIT_PKG_1_8_9.log`.

O ZIP completo inclui o PKG, fontes, documentação, ativos e prévias. SDK, dados de assinatura e o ativo JFX privado não são redistribuídos. GitHub não atualizado.

[Compilação](docs/BUILD.md) · [Validação](docs/VALIDACAO_2.0.4.md) · [Prévia do Spine PS1](assets/previews/2.0.4/PS3_GAME_ORBIT_v2.0.4_SPINE_PS1.png)

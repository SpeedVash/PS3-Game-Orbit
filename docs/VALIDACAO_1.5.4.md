# Validação da v1.5.4 — 2026-10-11

A política de cache preserva imagens recentes até precisar de espaço. Ao reduzir a capacidade de um layout, prioriza o jogo selecionado, os vizinhos da nova janela e as caixas que ainda estão desaparecendo na animação. Capa, interior e disco permanecem em suas respectivas memórias. Os arquivos JPEG/PNG já lidos também priorizam a janela do destino.

Os limites estáveis são Clássico 7/7/7, Spine 15/15/15 e Lista 5/5/5. As caixas que desaparecem conservam a proteção temporária já existente até seu último quadro. Os limites em bytes e a ordem de preparação entre quadros RSX confirmados foram preservados.

O teste usa as classes reais de cache e renderer, com a implementação do computador para decodificar, alocar e liberar texturas. No cenário Clássico → Lista em outro jogo → Clássico no jogo inicial, a política anterior precisou de quatro novas preparações de imagem. FIX44 reutilizou as quatro imagens, com zero preparações adicionais. Na redução do cache de arquivos de 15 para 5, a nova política manteve 5/5 arquivos comuns; a anterior manteve 0/5 naquele cenário. A remoção temporária dos arquivos de origem confirma que os cinco foram reutilizados da memória.

Também passaram: redução de caches preenchidos, preservação das alocações comuns, proteção durante a transição, atualização explícita de imagens, 96 trocas de layout/seleção, retorno circular da lista, limpeza em filtro vazio e encerramento. A suíte geral de menus, configurações, USB, seleção de arte e gerenciador passou. A compilação nativa passou nas oito verificações de arquitetura, versão, SFO, identidade/tamanho do PKG, ativos e referências de API.

O usuário confirmou que a tela do gerenciador funcionou bem na v1.5.3. O desempenho da v1.5.4 ainda precisa de teste físico no PS3. Os números acima contam preparações e reutilizações em cenários de teste; não são uma medição de FPS ou tempo de travamento do console.

PKG: `PS3_GAME_ORBIT_v1.5.4.gnpdrm.pkg`, 2.560.432 bytes.

SHA-256: `4b85af34a1b0487d82664732a6bf12a5f8f661cdfad6de384053779a936dcc22`.

O modelo PS1 deriva do anexo `Compact disc and case`, de effe, com licença CC0 incluída. A caixa tem tampa e livreto articulados; disco móvel com furo circular, bordas transparentes e verso escuro. Abertura de 160°, deslocamento de 96 mm para o lado e 27 mm para a frente. A prévia usa Blender e texturas demonstrativas próprias. Não representa uma captura nem uma implementação PS1 já ativa no PS3.

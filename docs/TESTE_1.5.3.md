# Testar a correção 1.5.3 no PS3

Instale `PS3_GAME_ORBIT_v1.5.3.gnpdrm.pkg` sobre a instalação anterior (PGORBT301, APP_VER 01.13). Confirme v1.5.3 no início/cabeçalho.

1. Abra START → Gerenciador de arquivos HD/USB. Confira se o aviso aparece inteiro, centralizado e legível.
2. Confirme com X. Cabeçalho, status, painéis e controles devem aparecer em ordem, sem metade deslocada para cima ou para baixo.
3. Use L1/R1, navegue nas pastas e abra Triângulo, R3 e SELECT. As janelas devem continuar dentro do mesmo retângulo.
4. Saia com START. Confira biblioteca, menus e lista; abra e feche o gerenciador novamente.
5. Após confirmar a imagem, teste uma cópia pequena entre HD e USB. Confira preparação, confirmação, progresso e resumo. Use o guia de cópia `GERENCIADOR_1.5.2.md`, cujos controles continuam válidos.

Teste, se possível, a saída de vídeo em 720p e 1080p. A correção mantém as proporções nos dois tamanhos; a prévia computacional não confirma a saída real da TV.

Se persistir a divisão, registre a resolução e tire uma foto da tela do gerenciador. O diagnóstico continua em `/dev_hdd0/tmp/PS3_GAME_ORBIT_FIX42.log` ou no USRDIR da instalação; o nome interno do log é herdado da versão anterior.

## O que foi verificado no computador

- Funções reais de preparação, upload, desenho e encerramento do HUD nativo, com API RSX observável.
- Geometria contínua do gerenciador e retorno à geometria em três faixas da biblioteca.
- Onze telas, atualização parcial dos pixels, cache de revisão e transições repetidas.
- Falhas de alocação, mapeamento e upload, limite de comandos e liberação da geometria.
- Compilação PPU, ELF/SELF, SFO e identidade/tamanho do PKG.

**A validação física desta correção no PS3 está pendente.** Jogos PS1 ficam para a etapa seguinte.

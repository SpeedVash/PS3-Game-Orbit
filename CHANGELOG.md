# Histórico

## 1.1.0-rc.1 — 2026-10-03

Build de teste **FIX31**, com SFO `01.01` e o mesmo TITLE_ID `PGORBT301`.

- Caixas do Clássico reduzidas a `0,55`, o mínimo anterior; R3 também retorna a esse tamanho.
- Segunda caixa reposicionada para caber inteira na tela.
- Saída e entrada animadas, mantendo a textura anterior até o fim do desaparecimento; inversão da navegação preserva as poses já visíveis.
- Novo layout Spine, adaptado do Aurora/Phoenix, com até cinco lombadas de cada lado e a capa selecionada à frente.
- Quadrado alterna os layouts; a escolha é persistida em arquivo separado do estado compatível com a 1.0.
- Barra de controles e ajuda atualizadas.
- Limites de cache/texturas e de quadros ajustados para a fila de lombadas. Geometria aprovada, UVs, shaders, segmentos FIFO, branding e integração webMAN preservados.
- Compilação nativa, testes de fluxo/persistência/renderização e prévias concluídos no computador. **Teste físico da 1.1 pendente.**

## 1.0.0 — 2026-10-03

Primeira release pública do **PS3 Game Orbit**, baseada no FIX30 aprovado pelo usuário.

- Biblioteca de jogos do HDD e USB, com filtros e favoritos persistentes.
- Caixas Blu-ray 3D com plástico transparente sem pigmento azul.
- Capa completa com frente, lombada central e verso; giro, inclinação e zoom.
- Jogo selecionado e caixa vizinha com capas independentes.
- Montagem assíncrona pelo webMAN MOD e confirmação do TITLE_ID do disco quando conhecido.
- Novo ícone do XMB e tela de início com a identidade PS3 Game Orbit.
- Fundo com ondas inspirado no XMB, Noto Sans Light suavizada e barra de botões compacta.
- Ajuda em SELECT e orientação normal de capa como padrão.
- Inicialização e calibração sem exibir a antiga tela rosa.
- Compilação nativa e verificações de geometria, controles, persistência, montagem e fluxo de comandos concluídas no computador.

O PKG é byte a byte o mesmo instalador FIX30 aprovado, com versão SFO `01.00` e TITLE_ID `PGORBT301`. Os identificadores FIX presentes nos fontes registram a evolução interna do projeto.

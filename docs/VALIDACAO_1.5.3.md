# Validação da v1.5.3 — 2026-10-11

A imagem contínua do gerenciador na v1.5.2 era desenhada pela geometria da biblioteca: as linhas 0–64 iam ao cabeçalho, 64–176 ao rodapé e 176–512 ao centro. Isso reordenava e cortava os painéis. FIX43 usa uma geometria própria para todo o bitmap, centrada entre y=104 e y=616 no quadro de referência 1280×720. A biblioteca conserva as três faixas originais.

O teste `scripts/check-manager-hud-fix43.py` pré-processa e executa os corpos reais dos métodos de criação, upload, desenho e encerramento nativo. A API RSX é simulada de forma observável; apenas a barreira PowerPC é substituída por um marcador no computador. Passaram as onze telas do gerenciador, pixels de atualizações parciais, cache de revisão, retorno à biblioteca, transições repetidas, falhas de alocação/mapeamento/upload, limite de comandos e liberação de memória. As linhas mantêm a ordem nas projeções 720p e 1080p.

A suíte pública FIX43 passou sem o modelo licenciado. As regressões de cópia, o ramo nativo sysFs simulado e 22 cenários do loop principal também passaram. Alguns relatórios herdados ainda identificam FIX42/v1.5.2; esses testes verificam os recursos de cópia e o loop preservados.

O instalador foi compilado com GCC PPU 7.5.0 e o SDK fixado do projeto. As oito verificações de ELF/SELF, SFO, identidade/tamanho do PKG, ativos e referências nativas passaram. Os 151 arquivos de código, shaders, ativos do PKG, SFO e Makefile permanecem idênticos aos usados na compilação. A versão compilada é 1.5.3; PGORBT301, APP_VER/VERSION 01.13.

SHA-256 do instalador:

```
e94f209c003698841ea1a315b045bbafae6b1dc984cfa2c500afbd140d85cfd2  PS3_GAME_ORBIT_v1.5.3.gnpdrm.pkg
```

A prévia é uma projeção computacional dos pixels, vértices e UVs reais. **O teste físico no PS3 permanece pendente**: os testes no computador não reproduzem a televisão, o driver RSX nem a fluidez do console. Siga `TESTE_1.5.3.md` antes da próxima etapa de visualização de PS1.

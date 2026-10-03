# Validação da release 1.0

A versão 1.0 preserva o instalador FIX30 que o usuário aprovou após testar a interface no PS3 Slim CFW. Não houve alteração de renderização ou reconstrução do modelo para esta publicação.

Verificações concluídas no computador na preparação do FIX30:

- Compilação PowerPC64 big endian com GCC 7.5.0 e SDK ps3aqua fixado.
- Cabeçalhos ELF/SELF, identidade do PKG finalizado e PARAM.SFO `PGORBT301 / 01.00`.
- Comparação dos 5.597 triângulos, normais e UVs com o OBJ original.
- Duas caixas e texturas independentes, plástico transparente e ordem de desenho.
- Controles, favoritos, preferências, filtros e normalização de capas completas.
- Montagem HTTP assíncrona com testes TCP locais e verificação da lógica de confirmação de disco.
- Texto suavizado, UTF-8 limitado, títulos longos, ajuda e composição do HUD.
- Início com splash, calibração fora da tela e apresentação com esperas limitadas.
- Limites do FIFO, alternância dos segmentos de comandos e rejeição de regressões.

A aprovação visual no console não é uma certificação para todos os modelos, firmwares e jogos. HEN/Super Slim permanece sem validação nesta release. Não foi recebido um log de montagem física do FIX30; a integração foi verificada com simulação e TCP local.

As imagens em `docs/images` foram produzidas no computador usando dados nativos. São referências de aparência com capas de teste, não capturas da saída de vídeo de um PS3.

Para repetir os testes completos, siga `BUILD.md` e execute `bash scripts/test-release.sh`. Não são publicados logs pessoais do console nem arquivos do SDK.

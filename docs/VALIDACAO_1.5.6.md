# Validação da v1.5.6

## Correção

A v1.5.5 desenhava quadros sem jogos durante a pré-carga, substituindo a imagem de início pelo fundo da biblioteca. Em FIX46, a pré-carga conserva a imagem que já está no buffer de exibição. A calibração continua fora da tela; o carregamento de artes apenas prepara as texturas e reconhece os ciclos da fila RSX, sem limpar ou trocar os framebuffers. O primeiro quadro normal da biblioteca substitui a imagem quando a janela inicial está pronta.

A rotina continua respeitando o reconhecimento das operações anteriores antes de alterar as texturas. Entre ciclos incompletos, uma espera de 1 ms cede CPU ao worker; essa espera não define a duração total. Arquivos ausentes não geram pedidos e imagens inválidas concluem com a aparência neutra.

## Testes

`python3 scripts/check-library-loop-fix46.py` executa o bloco real do main nativo em 33 cenários. Verifica que HUD, fundo e biblioteca não são apresentados durante a preparação, que cada ciclo de upload é reconhecido e que o primeiro desenho acontece após a conclusão. Inclui preparação demorada, catálogo vazio, jogos sem artes, cancelamento antes e durante a pré-carga, falhas no worker e FIFO, falha no primeiro HUD e as rotinas anteriores de layout, menu, montagem e gerenciador.

Os testes da v1.5.5 permanecem para o worker, cache, equivalência de pixels e sua rotina anterior. A CI pública também executa a nova verificação de FIX46.

## Compilação e pacote

Use PS3DEV e PSL1GHT apontando para a mesma instalação aprovada, GCC 7.5.0, e execute `scripts/build-fix46.sh`. O script confere o modelo aprovado e os shaders congelados, registra os inputs, compila o ELF PowerPC64, gera SELF e PKG finalizado e verifica PGORBT301/01.16.

O modelo PS3 privado e o SDK não são redistribuídos entre os fontes públicos. O instalador contém o programa compilado. O ZIP utiliza Deflate e cabeçalhos ZIP32 e tem sua extração completa verificada.

## Teste físico

O usuário confirmou que a v1.5.5 eliminou a pausa ao mudar de layout. Essa confirmação se refere à versão anterior. A permanência da imagem de início na v1.5.6 ainda precisa do teste no PS3, conforme `TESTE_1.5.6.md`. PS1 permanece em hiato.

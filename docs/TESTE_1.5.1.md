# Validação e teste da v1.5.1

## Verificado no computador

- Cópia real em lote nos dois sentidos HD/USB simulados, com identidade dos bytes, arquivo vazio, seleção múltipla, painéis independentes, escolha de dispositivo e destaques.
- Sobrescrever/ignorar um ou todos, cancelamento, política reiniciada por lote, rollback após falha na troca final e origem alterada durante leitura.
- Cancelamento com arquivos já concluídos preservados, tamanhos de 64 bits, buffer limitado a 256 KiB e recusa de arquivo acima do limite FAT32.
- Compilação e execução do ramo nativo sysFs com APIs simuladas: descritores consistentes, timestamps de caminho/descritor separados, consulta de espaço no destino e códigos exatos de erro de abertura.
- Preferências antigas migradas, cores salvas por jogo e registros de jogos USB ausentes preservados. A ação real de Triângulo alterna o estilo.
- Plano de desenho e corpo nativo simulados com três cores, oito fases de abertura e sete ângulos: apenas plástico externo colorido, interior transparente, texturas de capa/inside/disco preservadas.
- Loop principal efetivo em 22 cenários: menus, ordenação, gerenciador, montagem, salvamento, pausa/restauração do fundo e pré-carga adiada até cessar a rotação manual e transcorrer o intervalo de repouso.
- Regressões de imagens por ID/nome de ISO, USB parcial/em lote, PNG/ARGB, normalização 1000×550/500×500, filtros vazios e caches 7/7/7, 15/15/15 e 5/5/5.
- Mesmo modelo fechado/aberto, abertura de 160°, junções da lombada, inside completo, disco com bordas transparentes de 0,5 mm e verso prateado.
- ELF PowerPC64 big endian, SELF, SFO PGORBT301/01.11, PKG finalizado, APIs nativas e shaders arquivados verificados.
- AddressSanitizer e UndefinedBehaviorSanitizer nas suítes de recursos e geometria/interface, com detecção de vazamentos desativada.

Os relatórios e hashes estão em `validacao` no ZIP. As prévias PNG são geradas do código da interface no computador. Simulação de APIs não reproduz o driver RSX, a saída de televisão ou a velocidade do PS3; os cenários de HD/USB usam diretórios de teste.

## Teste no PS3 pendente

1. Instale por cima da versão anterior. Confira **1.5.1** na tela inicial, cabeçalho e START, com **SpeedVash** e sem TESTE. No HEN, habilite-o antes de abrir.
2. Verifique os nomes, favoritos e ordenação existentes. Altere layout, fundo, zoom, filtro e último jogo; saia normalmente e abra novamente.
3. Em Triângulo → Tipo de caixa, teste padrão, Platinum e Greatest Hits nos três layouts. Gire a caixa fechada e abra com L3. Confira cor externa, interior transparente e as três artes. Reinicie e confirme a escolha salva em jogos diferentes.
4. Troque para um jogo cuja arte ainda não esteja em memória e gire imediatamente. Compare com esperar um segundo antes de girar. Repita em Clássico, Spine e Lista; observe se o pequeno travamento inicial melhorou. O carregamento necessário ainda pode causar uma pausa.
5. Entre no gerenciador com fundo ligado. Confira os dois painéis, a pausa do fundo, posições independentes e retorno à biblioteca. Repita com fundo desligado: deve continuar desligado ao sair.
6. Usando arquivos de teste, copie um do HD para outra pasta do HD. Depois copie vários do HD para USB e do USB para HD; se houver dois USBs, copie entre eles. Confira tamanho, conteúdo e destaques azul/verde. Teste SELECT e pastas sem arquivos.
7. Crie conflitos no destino. Confira sobrescrever um, ignorar um, sobrescrever todos, ignorar todos e cancelar. Inicie outro lote e confirme que a decisão anterior não foi reaplicada.
8. Cancele uma cópia em andamento: os arquivos concluídos devem permanecer; o arquivo anterior no destino deve ficar intacto. Teste arquivo vazio e espaço insuficiente. Conecte um USB após abrir o gerenciador e use R3.
9. Teste A-Z/Z-A/Custom, atualização de artes por ID e nome de ISO em `USB/PS3COVERS`, uma arte isolada, importação em lote e filtros vazios.
10. Teste montagem pelo webMAN com caixa aberta e fechada. A confirmação volta ao XMB; falha deve permitir continuar na biblioteca.

Se ocorrer erro, informe modelo do PS3, HEN/CFW, passos e o texto completo com código. Envie o log `/dev_hdd0/tmp/PS3_GAME_ORBIT_FIX41.log` ou a alternativa no USRDIR. Para o gerenciador, inclua origem, destino e tamanho do arquivo; para pausas, inclua layout, dimensões das artes e se o giro foi imediato ou após esperar.

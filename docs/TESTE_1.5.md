# Validação da v1.5

## Verificado no computador

- Ordenação alfabética e inversa com acentos latinos, índices do catálogo preservados, deslocamento Custom no filtro ativo e retenção de jogos USB ausentes.
- Ordem salva com checksum, rejeição de dados inválidos e arquivo temporário/backup.
- Menus START/Triângulo efetivos, seleção preservada e início no primeiro jogo quando salvar último jogo está desligado.
- Cópias reais por blocos, identidade dos bytes, arquivo vazio, tamanho acima de 4 GB, confirmação, cancelamento, mudanças na origem/destino e recuperação do destino após falha de troca final.
- Loop principal efetivo: gerenciador, pausa/restauração do fundo, ausência de caixas e pré-carga nesse modo, salvamento e operações entre quadros já reconhecidos pela GPU.
- Atualização real dos pixels da textura de interface, reutilização por revisão e restauração da biblioteca ao voltar.
- Preparação de inside/disco na entrada; caches 7/7/7, 15/15/15 e 5/5/5, layouts e filtros vazios.
- Geometria aprovada, junções da lombada, arte interna completa, disco com bordas transparentes de 0,5 mm e verso prateado.
- Corpo nativo de desenho simulado em oito fases fechadas/abertas e sete ângulos; texturas por jogo, materiais, UVs e limites de submissão.
- ELF PowerPC64 big endian, SELF, SFO PGORBT301/01.10, PKG finalizado, APIs nativas e shaders arquivados.
- AddressSanitizer/UndefinedBehaviorSanitizer nas suítes de recursos e de geometria/interface. LeakSanitizer desativado; não constitui uma verificação de vazamentos.

Os relatórios e hashes acompanham a pasta `validacao` do ZIP. Simulação de APIs não reproduz o driver, saída de televisão ou desempenho do PS3.

## Teste no console pendente

1. Instale por cima da versão anterior. Confira 1.5 no início, no cabeçalho e em START, com crédito SpeedVash e sem TESTE.
2. Altere layout, fundo, zoom, rotação e filtro. Saia normalmente e abra novamente três vezes. Confira também nomes, favoritos e último jogo ligado/desligado.
3. Em START, alterne A-Z/Z-A/Custom. Mova jogos para o início, meio e fim. Reinicie; teste filtros e retirada/reconexão de USB.
4. Gire a caixa fechada: confira encarte, bandeja e disco. Abra/feche com L3 sem troca brusca de modelo; confira 160°, lombada contínua, bordas do disco e artes personalizadas. Teste X com caixa aberta.
5. Abra o gerenciador com fundo ligado, confirme o aviso, navegue e saia. Repita com fundo desligado: cada preferência deve voltar como estava.
6. Copie um arquivo de teste entre pastas. Confira progresso, tamanho e conteúdo. Cancele uma cópia parcial e uma confirmação de substituição; o arquivo anterior deve continuar intacto. Teste substituição concluída e arquivo vazio.
7. Em Clássico, Spine e Lista, navegue pela biblioteca. Teste filtros vazios, USB parcial por ID/nome de ISO, dimensões das artes e atualização em lote.
8. Teste webMAN disponível e indisponível. Montagem confirmada volta ao XMB; falha mantém a biblioteca acessível.

Se houver erro, informe modelo do PS3, HEN/CFW, jogo/formato, passos e o log `/dev_hdd0/tmp/PS3_GAME_ORBIT_FIX40.log` ou a alternativa em `USRDIR`. As medições de leitura, decodificação, upload e interface continuam no log.

# Validação da v1.6 / FIX48

## Verificações automatizadas

- Política da ventoinha: CPU/RSX, limites, histerese, emergência, falhas dos sensores, controlador concorrente e configuração inválida.
- Leitor de setores: mapas fragmentados, endereços acima de 2 TiB, leituras desalinhadas, transição entre extensões, final da ISO, falha de USB e overflow.
- NTFS: runlists com deltas negativos, USA/fixups, recusa de atributos inválidos/esparsos e 5000 entradas aleatórias malformadas.
- AddressSanitizer/UndefinedBehaviorSanitizer no parser e leitor de setores. A detecção de vazamentos foi desativada por incompatibilidade do ambiente de teste com LeakSanitizer.
- Volume NTFS real criado com NTFS-3G: 121 entradas, índice alocado, nome e ID da ISO e comparação de todos os bytes pelo mapa do leitor residente. NTFS-3G é usado apenas para produzir a fixture.
- Cliente real: partes de ISO, lacuna na sequência, troca de ID, backup de configuração, respostas atrasadas/malformadas, cancelamento, tamanho máximo, timeout e 25 ajustes rápidos com revisão.
- Dispatcher residente real com APIs simuladas: validação antes de ejetar, USB trocado, limpeza na falha, conflito de configuração e recusa de descarregar durante leitura.
- Adaptador da ventoinha com APIs simuladas: restauração exata da instrução, código desconhecido recusado, leitura de volta e pausa de processo HEN.
- Bloco real do main FIX48: 37 cenários, incluindo menu da ventoinha, revisão antiga, falha de início do serviço, pré-carga lenta, saída, disco original, gerenciador, layouts e montagem.
- PRX: relocação de dois segmentos em endereços separados, módulo, exports, descritores OPD, TOC, ausência de imports VSH e identidade SELF do plugin.
- Compilação nativa PowerPC64, SELF/EBOOT, PARAM.SFO 01.60, conteúdo e tamanho do PKG; modelo PS3 e shaders aprovados preservados.

Os logs acompanham o ZIP completo. Testes com APIs simuladas verificam decisões do programa, sem executar chamadas reais de LV2/Cobra. A validação em hardware está pendente; TV, firmware, montagem real e velocidade física precisam do teste no PS3.

## Reproduzir

Execute `tests/run_service_core_fix48.sh`, `tests/run_ntfs_volume_fix48.sh`, `tests/run_service_dispatch_fix48.sh`, `tests/run_service_client_fix48.sh`, `scripts/check-library-loop-fix48.py` e `tests/run_fix48_artwork.sh`. Os testes públicos de arte usam `FIX48_PUBLIC_ONLY=1`; não redistribuem o modelo JFX privado. Para criar o PKG, use `scripts/build-fix48.sh` com a stack ps3aqua e o modelo autorizado, conforme os requisitos existentes do projeto.

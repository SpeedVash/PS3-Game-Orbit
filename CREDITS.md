# Créditos e componentes

Criador do homebrew **PS3 Game Orbit: SpeedVash**.

## Modelo 3D

**JFX_DVDBlu**, de **JooseFX Digital / jayokay951**, publicado no TurboSquid:

https://www.turbosquid.com/3d-models/free-dvd-bluray-case-3d-model/724523

Licença do ativo: **TurboSquid Standard / 3D Model License**:

https://www.turbosquid.com/licensing

O PKG incorpora a geometria compilada ao aplicativo interativo. Os arquivos editáveis OBJ/PSD e a representação da malha em código-fonte não acompanham o repositório nem o ZIP público. Quem compila obtém sua cópia diretamente da página original e usa `scripts/prepare-jfx.py`. O modelo mantém os termos do autor; não recebe uma licença de código aberto por integrar este projeto.

## Referência do layout Spine

**“Spine by Phoenix”**, incluído no Aurora 0.7b.2, equipe Phoenix:

https://phoenix.xboxunity.net/

O modo Normal serviu de referência para a fila de lombadas, ângulos e afastamento da capa selecionada. O projeto contém uma implementação própria adaptada ao PS3; não distribui o Aurora, seu código, o arquivo `.cfljson` ou seus ativos.

## Fontes

- **Noto Sans**, Google Fonts / autores Noto. Licença SIL Open Font License 1.1 em `assets/fonts/OFL.txt`, também incluída no PKG. Origem: https://github.com/google/fonts/tree/main/ofl/notosans
- O bitmap de fonte histórico permanece no código para compatibilidade com testes anteriores. Avisos correspondentes em `docs/FONT_LICENSE_FIX28.txt`.

## Ferramentas e integração

- **PSL1GHT / ps3aqua**: SDK e ferramentas de compilação. Não são redistribuídos neste repositório. https://github.com/ps3aqua/PSL1GHT e https://github.com/ps3aqua/ps3toolchain
- **webMAN MOD**, aldostools e colaboradores: montagem local dos jogos. Não acompanha o instalador. https://github.com/aldostools/webMAN-MOD
- **libpng / libjpeg**: decodificação usada pelos testes no computador. No PS3, o aplicativo usa os decodificadores do SDK.

## Identidade visual

Ícone e arte de início criados com geração de imagens para o PS3 Game Orbit. Arquivo de origem e registro do prompt em `assets/branding`. O fundo com ondas é produzido pelo código do aplicativo, inspirado no XMB.

PS3, PlayStation, XMB e Blu-ray são nomes e marcas de seus respectivos titulares. Este é um projeto homebrew independente.

## Licença do código do projeto

Nenhuma licença geral de reutilização do código foi definida nesta preparação. Publicar os fontes no GitHub não altera as licenças dos componentes acima. Uma licença escolhida pelo mantenedor poderá ser acrescentada em uma atualização.

## Disco 3D da 1.2

Geometria original criada para PS3 Game Orbit, aprovada pelo usuário; variante leve de 48 segmentos/1.056 triângulos. O JSON das malhas e a representação nativa acompanham os fontes. Texturas de demonstração próprias; nenhuma arte de disco de jogos foi baixada. A licença geral do projeto continua pendente de escolha do mantenedor.

## Componentes da v1.4

Interior, disco e duas articulações são construídos proceduralmente em `src/case_model_fix37.cpp`, a partir do exterior preparado localmente. A lombada flexiona sem recortar sua arte; o disco tem borda transparente externa de 0,4 mm. Os shaders arquivados são preservados.

A importação usa **stb_image_write v1.16**, Sean Barrett e colaboradores, do commit upstream `2c980bb59875b0d32144a71867fbdebb2f77cd20`. Inclui um ajuste local no acumulador de bits JPEG, documentado em `include/third_party/README.md`. Licença MIT/domínio público em `assets/licenses/STB_LICENSE.txt`, também incluída em `USRDIR` do PKG.

## v1.4.1

O fechamento usa a mesma malha aprovada em repouso, agrupada para o RSX. A borda dupla do disco, a correção da sobreposição e o armazenamento com backup são código do projeto. ORBIT_DISC_BACK.png é uma textura procedural reproduzível por scripts/bake-disc-back-fix38.py; a iluminação continua nos shaders originais e não é reflexão de ambiente em tempo real. A arte da marca e a fonte mantêm os créditos anteriores.

## Acabamento da v1.4.2

A lombada usa as bordas curvas do mesmo exterior JFX, preparadas localmente, junto ao interior procedural aprovado. A união é recortada no mesmo plano das laterais e transportada pelas duas articulações. Os relevos usam plástico neutro; as normais suavizadas reutilizam buffers. Os termos do modelo de terceiros continuam os mesmos.

## v1.5

A ordenação persistente, gerenciador de arquivos e integração do conjunto articulado fechado/aberto são código do projeto. O gerenciador reutiliza a fonte Noto rasterizada e o renderizador existente. Mantêm-se os termos e créditos de todos os ativos anteriores.

## v1.5.1

O gerenciador de dois painéis, a seleção múltipla, o tratamento dos lotes, os materiais coloridos por jogo e o controle de pré-carga durante a interação são código do projeto. A aparência de dois painéis usa o Aurora como referência funcional, sem incorporar seus fontes ou ativos. A geometria licenciada mantém os termos anteriores; as cores não adicionam modelos de terceiros.

## Modelo PS1 de prévia

Compact disc and case, por effe (Blend Swap), licença CC0 fornecida no anexo. Caixa adaptada para articulação; disco regular e arte demonstrativa preparados para PS3 Game Orbit. Licença em assets/models/ps1/BLENDSWAP_LICENSE.txt. O modelo de PS3 aprovado conserva suas condições de distribuição.

## Serviço próprio da v1.6

Implementação do serviço residente, protocolo local, política de ventoinha, parser NTFS e leitor de setores do Orbit. Interfaces LV2/Cobra/PS3MAPI verificadas nos headers públicos do PSL1GHT e webMAN MOD; não inclui a implementação de webMAN, multiMAN, rawseciso ou NTFS-3G. O NTFS-3G foi usado somente para criar o volume sintético de teste. A assinatura SELF usa a ferramenta geohot da stack PSL1GHT.

## Instalador interno da v1.8.0

O parser PKG, planejamento, transação com recuperação, conferência de arquivos, thread e telas são implementação própria do Orbit. AES usa os dois fontes mínimos de BearSSL, Copyright 2016 Thomas Pornin, licença MIT; SHA-1 usa o cabeçalho ps3-dev de Mohammed Asif, Copyright 2026, licença Apache 2.0. Licenças e aviso acompanham os fontes e o PKG. O formato PS3 foi conferido com os fontes públicos do RPCS3; nenhum código GPL do emulador ou do IRISMAN foi incorporado. As chamadas de conteúdo usam os headers e stubs públicos do PSL1GHT.

## Correção de entradas na v1.8.1

Atributo NPDRM 0x100 reproduzido em PKG de tradução fornecido pelo autor e comparado ao tratamento de arquivos no RPCS3. Tipos 5/6 e diretório 0x12 conferidos em unpkg.cpp. Diagnóstico, normalização limitada e testes de regressão são código do Orbit. Pacotes de terceiros/tradução usados na validação não são redistribuídos neste ZIP.

## Fila nativa da v1.8.2

Seleção múltipla, plano de conferência, protocolo de sessão, auxiliar VSH, ponte de callback com pilha própria e testes são implementação do Orbit. Interfaces e sequência LoadPlugin3/LoadPage/installPKG conferidas nos fontes públicos do PS3HEN e webMAN MOD, com revisões fixadas em `docs/VALIDACAO_1.8.2.md`. Não foi copiada a implementação GPL de seus instaladores. AES/SHA-1 e respectivos avisos mantêm as licenças já distribuídas. O auxiliar usa o instalador do sistema, sem redistribuir módulos proprietários do XMB.

## Reconstrução da v1.8.3

Novo envio de sentido único, auxiliar mínimo e seletor de pacotes são código do Orbit. A extração/instalação interna, leitor/criptografia de PKG e planejador por arquivos das revisões anteriores foram removidos do novo build. A sequência de interface nativa mantém as referências públicas do PS3HEN; adaptadores de ABI e inicialização de thread reutilizam utilitários próprios verificados. Não inclui módulos proprietários ou implementação GPL do instalador. Os avisos instalados de componentes anteriores são preservados por compatibilidade.

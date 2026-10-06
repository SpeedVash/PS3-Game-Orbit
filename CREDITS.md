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

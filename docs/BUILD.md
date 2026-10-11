# Compilar PS3 Game Orbit 1.5.4

O PKG da release já está pronto para instalação. Este procedimento é para desenvolvedores, em Linux x86_64.

## 1. Preparar o modelo licenciado

Obtenha gratuitamente **JFX_DVDBlu.zip** pela página do autor no TurboSquid e aceite os termos do download:

https://www.turbosquid.com/3d-models/free-dvd-bluray-case-3d-model/724523

Na raiz do repositório, execute:

```bash
python3 scripts/prepare-jfx.py --source /caminho/JFX_DVDBlu.zip
```

Também é aceito o arquivo `Bluray_Tris.obj` extraído do pacote. O script confere o SHA-256 do original, copia apenas o OBJ necessário, gera a malha nativa e verifica todos os triângulos e UVs. Nenhum arquivo é baixado automaticamente.

O OBJ e `src/jfx_case_data_fix29.inc` são locais e ignorados pelo Git. Não os acrescente ao repositório público. O modelo continua sob os termos do TurboSquid. Para apenas rodar o aplicativo, essa etapa não é necessária.

## 2. Dependências no computador

```bash
sudo apt-get update
sudo apt-get install -y build-essential git curl wget patch autoconf automake \
  libtool pkg-config bison flex texinfo python3 python3-pil ripgrep libgmp-dev libmpfr-dev libmpc-dev \
  libelf-dev libssl-dev libncurses5-dev zlib1g-dev libpng-dev libjpeg-dev
```

## 3. SDK PPU fixado

Se já tiver a instalação usada pelo projeto, `PS3DEV` e `PSL1GHT` devem apontar para a mesma pasta. A compilação verifica GCC 7.5.0 e os commits do SDK. Para preparar uma instalação nova:

```bash
bash scripts/setup-stack-fix25.sh B "$PWD/.stack"
export PS3DEV="$PWD/.stack/ps3dev"
export PSL1GHT="$PS3DEV"
export PATH="$PS3DEV/bin:$PS3DEV/ppu/bin:$PATH"
```

Commits usados:

- ps3toolchain: `961fddac01337f18da08f4471d558a7a5b0d9af2`
- PSL1GHT: `af9d3d964c8faa69abce4961a269f9582e05a33f`

O script compila a pilha PPU e as ferramentas necessárias; reserve tempo para essa instalação. SPU, Cg e portlibs não são necessários para o caminho de build atual. Os shaders VPO/FPO arquivados acompanham os fontes e têm hashes verificados.

## 4. Testes e compilação

```bash
bash scripts/test-release.sh
bash scripts/build-release.sh
```

O pacote com nome público fica em `dist/release/PS3_GAME_ORBIT_v1.5.4.gnpdrm.pkg`. Os produtos nativos e relatórios ficam em `dist/B`.

Os fontes implementam FIX44: reaproveitamento de capa, interior, disco e arquivos já lidos entre layouts, com prioridade para a janela de destino e proteção durante as transições. Preservam o desenho contínuo do gerenciador, a cópia recursiva HD/USB, as cores por jogo e a pré-carga adiada durante comandos manuais. A caixa selecionada usa o conjunto completo articulado desde a fase fechada. As caixas vizinhas preservam o exterior leve do modelo aprovado. Os shaders e o controlador FIFO arquivados permanecem intactos. O SFO usa APP_VER 01.14; a versão pública é 1.5.4. Data/hora e metadados podem variar entre builds; confira o hash da entrega em `docs/RELEASE_1.5.4.json`.

Esta entrega foi compilada com o GCC/newlib/binutils e bibliotecas dos commits fixados acima. As ferramentas host de empacotamento vieram da distribuição oficial PS3DEV `nightly-2026-07-26`, com os mesmos argumentos de linha de comando. Em uma pilha híbrida, use um diretório de headers limpo para o SDK fixado, evitando headers antigos de outro PSL1GHT. A configuração pelo script `setup-stack-fix25.sh` mantém a rota de compilação integral das ferramentas.

Se os arquivos foram enviados ao GitHub pela interface web, os bits de execução podem ter sido perdidos. O script de build restaura a permissão dos scripts necessários; também é possível executar `chmod +x scripts/*.sh tests/*.sh` na cópia local.

Para executar os testes públicos de menus, configurações, importação, caches e layout sem obter o modelo licenciado:

```bash
FIX44_PUBLIC_ONLY=1 bash tests/run_fix44_tests.sh
bash tests/run_manager_fix42.sh
python3 scripts/check-file-manager-native-fix42.py
python3 scripts/check-library-loop-fix42.py
python3 scripts/check-manager-hud-fix43.py
```

Para regenerar as prévias da interface usando o código real e a fonte rasterizada:

```bash
python3 scripts/render-manager-preview-fix43.py
```

## Ativos visuais

A fonte rasterizada, ícone, splash e fundo necessários ao aplicativo já acompanham o código. Para regenerar os ativos de fonte e branding, instale Pillow e execute:

```bash
python3 scripts/bake-orbit-assets-fix30.py
python3 scripts/bake-disc-back-fix38.py
```

Nenhuma biblioteca de fontes precisa ser instalada no PS3.

## GitHub Actions

O workflow `checks.yml` executa testes públicos FIX42/FIX43/FIX44 e simula as funções nativas de desenho do gerenciador, cópia em lote e simulação da API sysFs, além das regressões herdadas de USB parcial/ISO, buffers, caches, menu, fundo e layout, simulações do loop, testes herdados de fluxo/interface e verificações de apresentação/FIFO sem o modelo de terceiros. Ele não compila o PKG nem publica releases automaticamente. Os testes completos e a compilação nativa exigem a preparação local do modelo acima.

## Modelo de PS1

`assets/models/ps1/` contém a preparação editável e animada do modelo CC0 enviado para o estudo de PS1. Os arquivos Blender/GLB não são entradas da compilação nativa desta versão. Consulte `docs/PS1_MODELO_PREVIEW.md`; a adaptação RSX e a descoberta de jogos PS1 serão etapas posteriores.

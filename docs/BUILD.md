# Compilar PS3 Game Orbit 1.0

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
  libtool bison flex texinfo python3 ripgrep libgmp-dev libmpfr-dev libmpc-dev \
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

O pacote com nome público fica em `dist/release/PS3_GAME_ORBIT_v1.0.gnpdrm.pkg`. Os produtos nativos e relatórios ficam em `dist/B`.

Os fontes de execução são os mesmos da base FIX30 aprovada. A data/hora de compilação e os metadados de empacotamento podem variar entre builds; compare a geometria, fontes e verificações em vez de esperar o mesmo hash de um PKG recompilado. A release original tem o hash publicado em `docs/RELEASE_1.0.json`.

## Ativos visuais

A fonte rasterizada, ícone, splash e fundo necessários ao aplicativo já acompanham o código. Para regenerar os ativos de fonte e branding, instale Pillow e execute:

```bash
python3 scripts/bake-orbit-assets-fix30.py
```

Nenhuma biblioteca de fontes precisa ser instalada no PS3.

## GitHub Actions

O workflow `checks.yml` executa os testes da interface FIX30 e as verificações da apresentação/FIFO sem o modelo de terceiros. Ele não compila o PKG nem publica releases automaticamente. Os testes completos e a compilação nativa exigem a preparação local do modelo acima.

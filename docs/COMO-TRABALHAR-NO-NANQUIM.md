# Como trabalhar no Nanquim (base estável 5.3.5)

Guia para qualquer pessoa ou IA que for mexer no código do Nanquim nesta arquitetura.

## Estrutura do repositório

- **`KRITA_REF`**: commit estável do Krita no servidor oficial (`https://invent.kde.org/graphics/krita.git`). Hoje: `1706bc8e0469b330017001fe9ddf89ddbfe15950` (Krita 5.3.5 final).
- **`nanquim-src/`**: arquivos 100% próprios do Nanquim (espelhando o caminho dentro do Krita).
  - C++: `libs/ui/NanquimTabletUi.cpp`, `NanquimTabletUi.h`, `NanquimSutImport.cpp`, `NanquimSutImport.h`, `NanquimWheel.h`, `libs/global/NanquimStepTimer.h`.
  - Android/Java e recursos: `packaging/android/apk/...`, `krita/pics/nanquim/...`, `krita/data/themes/Nanquim.colors`, `krita/data/splash/...`.
  - **Edite esses arquivos diretamente em `nanquim-src/`** — sem gerar `.patch`!
- **`patches/`**: apenas 4 patches temáticos e disjuntos (nenhum arquivo do Krita aparece em dois patches):
  1. `0001-Nanquim-pacote-android-enxuto.patch` — remove plugins/dados de desktop no Android.
  2. `0002-Nanquim-desempenho-abertura-e-liquidificar.patch` — otimizações de abertura, memória, PNG e liquidificar multithread.
  3. `0003-Nanquim-ganchos-interface-tablet.patch` — ganchos no Krita para carregar a UI do Nanquim, tema, fontes, ícones e importação.
  4. `0004-Nanquim-identidade-e-abertura-android.patch` — nome, ícone, splash nativa, intents de importação e `versionCode` monotônico.

## Como alterar arquivos próprios do Nanquim (`nanquim-src/`)

1. Atualize o ramo (`git pull --rebase`).
2. Edite o arquivo diretamente em `nanquim-src/` (ex.: `nanquim-src/libs/ui/NanquimTabletUi.cpp`).
3. Confira todas as APIs do Krita contra os cabeçalhos do commit em `KRITA_REF` (no 5.3.5, use `KoToolManager::instance()` — não existe `viewManager()->toolManager()`).
4. Faça commit normal no Git. O workflow `Verificar patches` valida automaticamente se não há colisões nem marcas de conflito.

## Como alterar um arquivo do Krita (`patches/`)

Só mexa em `patches/` se precisar de um gancho novo no código do Krita:

1. Monte a árvore do Krita no commit de `KRITA_REF`:
   ```bash
   git clone --filter=blob:none https://invent.kde.org/graphics/krita.git krita
   git -C krita checkout $(cat KRITA_REF)
   for p in $(ls patches/*.patch | LC_ALL=C sort -V); do git -C krita am --3way "$PWD/$p" || break; done
   ```
2. Faça a alteração mínima no arquivo do Krita e gere o patch com `git format-patch -1` (nunca à mão).
3. Teste a fila inteira do zero numa árvore limpa do `KRITA_REF` e verifique que `grep -rn '^<<<<<<<\|^>>>>>>>' patches/ nanquim-src/` volta vazio.

## Como atualizar para uma nova versão `5.3.x` do Krita

1. Rode o workflow **Verificar patches** passando a tag nova em `base_comparacao` (ex.: `v6.0.5.1` para Krita 5.3.5.1). Leva ~2 minutos e não compila.
2. Se os 4 patches aplicarem limpos e sem colisões em `nanquim-src/`, atualize `KRITA_REF` e rode uma build de teste.
3. Se algum patch falhar, ajuste apenas aquele patch específico em cima da nova base.

## Regras de ouro (lições aprendidas)

- **Uma mudança arriscada por build:** misturar mudanças esconde a causa de bugs.
- **Medir antes de otimizar:** validar no `KritaUsageLog.txt` e em gravação de tela antes e depois.
- **Só usar APIs confirmadas no `KRITA_REF`:** nunca usar método do Krita "de memória".
- **Mudança visual vai para protótipo antes do código.**
- **Armadilhas do Qt no Android:**
  - Nunca criar `QPixmap` global antes do `QGuiApplication` (use `static` dentro de função).
  - Evitar `QMessageBox` nativo; usar folhas do Nanquim (`makeSheet`, `nqConfirm`) ou `showToast`.
  - Sempre deixar `QScrollArea` transparente (`background: transparent; border: none;` e `viewport()->setAutoFillBackground(false)`).
  - Não depender de animações durante `openDocument` (bloqueia a thread principal).

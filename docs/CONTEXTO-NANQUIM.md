# Nanquim: contexto do projeto

Atualizado em 27/11/2026, depois da build 100 (patches até o 0146).

Fork do Krita para Android, feito para quadrinistas. Roda no Galaxy Tab S10 Lite (SM-X400, Android 16, paisagem, com S Pen). A interface é híbrida, no estilo Procreate e Clip Studio, com tela inicial própria, capítulos, modo foco, requadros e balões. Os desenhos abrem como `.kra`.

## Situação agora (leia primeiro)

- **Projeto em pausa.** O usuário está em semana de provas e cansado do crescimento do escopo. Não propor recursos novos até ele voltar e escolher um caminho (veja "Decisão em aberto").
- **Versão em uso:** build 100, que compilou e funciona. Não há build rodando nem patch pendente.
- **Problema principal sem solução:** o traço com estabilizador chega aos trancos no Nanquim, e no Krita 5.3.5 estável é fluido. A causa mais provável é a base do Nanquim, o Krita 5.4 em desenvolvimento. Veja "Diagnóstico do estabilizador".

## Decisão em aberto

Três caminhos, todos válidos. Recomendação dada: o 1 para desenhar agora e o 2 se houver vontade depois.

1. **Parar.** Usar o Krita 5.3.5 estável para desenhar e acompanhar a interface de tablet oficial, que a equipe do Krita está fazendo (roteiro de 2026).
2. **Versão pequena.** Um pacote do Nanquim para o Krita normal: pincéis, modelos de página de quadrinho e área de trabalho para tablet. Sem builds e sem fork.
3. **Continuar com escopo fechado.** Uma única meta: migrar o Nanquim para o Krita 5.3.5 estável (git `1706bc8`). Nenhum recurso novo até isso funcionar.

## Como trabalhar com o usuário

- Responder em português, curto e direto (skill `i-have-adhd`): começar pela próxima ação, passos numerados, listas curtas e estimativa de tempo.
- Ele não tem computador agora; o diagnóstico depende dos arquivos que o app grava em Downloads (`KritaUsageLog.txt`, `nanquim-erro-*.txt`), de prints e de gravações de tela.
- Cada build leva de 45 a 60 minutos. Um push novo no `main` cancela a build em andamento.
- Patches de outras IAs (Gemini, OpenCode) são revisados antes de aplicar. Ele só sobe ao `main` depois de dizer "manda".
- Mudança de design vai para protótipo (canvas) antes do código.
- Questionar ideias com problema de usabilidade, sem só concordar.
- Leia `docs/COMO-TRABALHAR-NO-NANQUIM.md` antes de escrever qualquer patch.

## Repositório e build

- App: `isaiasdaniel290308-bot/Nanquim`, ramo `main`. Patches em `patches/`, aplicados em ordem natural (`sort -V`) com `git am --3way` sobre o Krita no commit de `KRITA_REF` (`2088c59`, Krita 5.4.0-prealpha de 05/11/2026).
- **Android arm64** (`.github/workflows/android.yml`): compila e publica uma release `build-N`. A lista de releases do GitHub ordena por nome, então a build 100 aparece abaixo da 99.
- **Verificar patches** (`.github/workflows/verificar-patches.yml`, novo): em uns 5 minutos e sem compilar, aplica a fila inteira na base atual e numa base de comparação (padrão `v5.3.4`, ou o que for passado em `base_estavel`). Roda sozinho a cada push em `patches/`. A versão corrigida (baixa a base estável do invent.kde.org e falha com clareza se a versão não existir) está no ramo `ccr-63f736b6-vlut7n`; o `main` ainda tem a versão anterior, que testa o ramo padrão quando não acha a base.
- O espelho `github.com/KDE/krita` está desatualizado (só tem versões até 5.2.15). Para o 5.3, use `invent.kde.org/graphics/krita.git`.
- Daqui não dá para baixar o Krita (invent.kde.org e o espelho do GitHub bloqueados), nem abrir krita.org. Para revisar código, reconstrua os arquivos do Nanquim aplicando os patches só com `--include='libs/ui/Nanquim*'` (eles são arquivos novos).

## Arquivos principais

- `libs/ui/NanquimTabletUi.cpp` (cerca de 17 mil linhas): toda a interface Tablet.
- `libs/ui/NanquimSutImport.cpp`: importador `.sut` do Clip Studio.
- `packaging/android/apk/src/org/krita/android/NanquimSplash.java`: animação de abertura.
- `scripts/atualizar_catalogo.py` e `catalogo-comunidade.json`: catálogo da comunidade (Krita-Artists), atualizado por `atualizar-catalogo.yml`.
- `docs/COMO-TRABALHAR-NO-NANQUIM.md`: regras para escrever patches sem conflito.

## Linha do tempo recente (0125 a 0146)

- **0125 a 0127 (Gemini, revisados):** remover pincéis importados, importador `.sut` v3 e catálogo v2.
- **0128:** corrige a compilação do 0125, importa todas as subferramentas de um `.sut` e grava o que foi lido no log.
- **0129:** Central de Quadrinhos (mangá, quadrinhos, webtoon, pincéis).
- **0130:** revisão de interface (alvos de toque, ícones próprios).
- **0131:** catálogo instala de verdade (redirecionamentos, Dropbox, Drive) e mostra prévias; o script do catálogo testa cada link.
- **0132:** aba Comunidade na tela inicial.
- **0133:** pincéis baixados vão para Importados.
- **0134:** instalar sem a caixa de mensagem quebrada.
- **0135:** abertura mais curta da animação inicial.
- **0136:** corrige a compilação do 0133 a 0135 (`data()` exige o papel; `synchronizeStorage` é privado).
- **0137 e 0138 (revisados pelo Gemini):** caixas do Krita viram folhas do Nanquim; textura de 256 px e time-lapse mais calmo.
- **0139:** a folha da caixa saía bege e travava a abertura de desenhos (build 95); corrigido.
- **0140:** transição tela inicial ↔ área de desenho (protótipo aprovado).
- **0141 e 0142 (Gemini):** volta a textura para 512 px; pincéis importados carregados sob demanda (o 0133 travava a abertura com 480 pincéis).
- **0143 a 0145 (Gemini, revisados):** importador `.sut` sem avisos repetidos e com sensores certos (`ascension`/`declination`); busca na aba Pincéis e `.sut` no topo; transição com a proporção real da página.
- **0146:** Configurações → Desempenho ganha "Qualidade da página na tela" (Alta qualidade, Trilinear, Bilinear) e "Blocos de textura" (256 ou 512 px); padrão igual ao Krita estável (256 px, Alta qualidade).

## Diagnóstico do estabilizador

Medido pelas gravações de tela (se a área de desenho muda a cada 1/20 s):
- **Krita 5.3.5 estável (git `1706bc8`):** o traço cresce sem parar. Fluido mesmo numa página de 9920 × 14032.
- **Nanquim:** o traço aparece aos trancos, muitas vezes de uma vez no fim, com os mesmos valores no estabilizador.

Descartados: tamanho da página, controles por cima da área de desenho (tela cheia), Antevisão Instantânea (não age com pincel de 20 px), blocos de textura e modo de escala (build 100 e Bilinear), e a interface Tablet (modo Clássico também trava).

Causa mais provável: o Krita 5.4 em desenvolvimento. Confirmação possível: uma build do 5.4 sem nenhum patch (cuidado: usa o mesmo nome de app e substituiria o Nanquim).

Migração para o 5.3.5: a verificação no git `1706bc8` aplicou 17 e falhou em 130 patches, quase tudo em cascata. O trabalho real está em uns 25 a 30 arquivos do Krita; os 31 arquivos do Nanquim passam inteiros. Abordagem proposta: uma "fotografia" do Nanquim de hoje (arquivos do Nanquim + um patch por arquivo do Krita adaptado), num ramo separado, sem mexer no `main`.

## Pendências e ideias (só se o projeto continuar)

- Abertura da "he's coming": o Krita leu o arquivo em 3 s, mas o app ficou mais 8 s travado depois de a página aparecer.
- "Abrir com" de `.bundle` ainda mostra a caixa nativa durante a abertura (fica nativa de propósito desde o 0139).
- Organizar a pasta de pincéis logo depois de instalar (evita ~1 min na abertura seguinte) exige alterar `KisResourceCacheDb.h`, que é privado.
- Molde do capítulo, página dupla e backup `.nqzip`: protótipo, não implementado.

## Lições

- Uma build por mudança arriscada; misturar patches esconde a causa.
- Medir antes de mudar desempenho. Duas mudanças "otimizadas" pioraram as coisas (textura de 512 px no 0075 e a leitura de todos os pincéis no 0133).
- Só usar funções do Krita vistas no código ou em outro patch que compilou.
- Patch que entrou no `main` não se reescreve; a correção vira patch novo.
- Uma verificação que falha em silêncio engana: confira se ela testou mesmo o que diz.
- Cuidado com o crescimento do escopo: cada correção abriu uma frente nova, e três IAs no mesmo arquivo multiplicaram os conflitos.

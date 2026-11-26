# Como mexer no Nanquim sem patches conflituosos

Guia para qualquer pessoa ou IA (Claude, Gemini, OpenCode) que for escrever um patch.
Ele junta o que deu errado de verdade nos patches 0112 a 0140. Leia antes de começar.

## A regra que resolve quase tudo

**Todo patch nasce em cima da fila inteira, no estado atual do `main`.**

Um patch escrito em cima de um arquivo velho, de memória ou de um resumo vai conflitar ou não compilar. Foi o que aconteceu com o 0121 (reintroduziu um erro que o 0028 já tinha corrigido) e com o "0124" do Gemini (número repetido).

## Passo a passo de um patch

1. **Atualize antes de tudo.**
   `git pull --rebase` no repositório e confira se há build rodando (aba Actions). Outra IA pode ter subido algo há minutos.
2. **Monte a árvore de trabalho com a fila inteira.**
   Baixe o Krita no commit de `KRITA_REF` e aplique **todos** os patches em ordem:
   ```bash
   git -C krita checkout $(cat KRITA_REF)
   for p in $(ls patches/*.patch | sort -V); do git -C krita am --3way "$PWD/$p" || break; done
   ```
   Se algum falhar, pare: a fila já está quebrada e precisa ser corrigida primeiro.
3. **Escolha o número.**
   `ls patches | sort -V | tail -3` e use o **próximo inteiro livre**. Nunca reaproveite um número, nem para "substituir" um patch.
4. **Faça a mudança nessa árvore e gere o patch com o git**, nunca à mão:
   ```bash
   git -C krita commit -am "Nanquim: <o que muda>"
   git -C krita format-patch -1 --stdout > patches/NNNN-Nanquim-<assunto>.patch
   ```
5. **Teste a fila inteira de novo**, do zero, numa árvore limpa (passo 2). Aplicar só o último patch esconde conflitos.
6. **Procure marcas de conflito** antes de subir:
   `grep -rn '^<<<<<<<\|^>>>>>>>' patches/` tem que voltar vazio.
7. **Suba um patch por build** quando ele mexe em algo arriscado (abertura, desenho, recursos). Um push novo no `main` **cancela** a build anterior que ainda está rodando.

## Patch que já está no `main` não se reescreve

Depois que um patch entrou no `main`, qualquer correção vira um **patch novo** (como o 0124 corrigiu o 0121 e o 0136 corrigiu o 0133 a 0135).

Reescrever um patch antigo no lugar (como aconteceu com o 0137 e o 0138) deixa os ramos divergentes e gera conflitos de `add/add` em quem tinha a versão anterior.

A única exceção é um patch que **nunca compilou** e ainda não foi usado por ninguém.

## Só use funções do Krita que você viu no código

Os erros de compilação mais caros vieram de APIs "lembradas":

| Patch | O que se supôs | O que era de verdade |
|---|---|---|
| 0133 | `KisStorageModel::data(index)` | `data()` exige o papel; use `index(r, coluna).data()` |
| 0135 | `KisResourceCacheDb::synchronizeStorage` público | é privado |
| 0121 | `?:` misturando `KisImageWSP` e `KisImageSP` | não compila; atribua em duas linhas |

Antes de usar uma função do Krita, confirme de um destes jeitos:
- ela já aparece em outro patch do Nanquim (`grep -rn 'nomeDaFuncao' patches/`); ou
- você leu o cabeçalho dela no Krita, no commit de `KRITA_REF`.

Se não der para confirmar, escreva no patch: "API não verificada".

## Armadilhas do Qt no Android que já custaram builds

- **`QPixmap` global** criado antes do `QGuiApplication`: o app fecha ao abrir (0106). Use `static` dentro de função.
- **Caixas nativas (`QMessageBox`)** saem com faixa branca e texto cortado. Use as folhas do Nanquim (`makeSheet`, `nqConfirm`) ou um aviso (`showToast`).
- **`QScrollArea` sem fundo próprio** fica bege (cor de papel do tema) e esconde texto claro. Sempre pinte a área: `scroll->setStyleSheet("QScrollArea { background: transparent; border: none; }")` e `scroll->viewport()->setAutoFillBackground(false)`. Foi a causa da tela travada na build 95.
- **Nada que dependa de animação durante a leitura de um arquivo**: o `openDocument` trava a tela, e animações congelam.
- **Desfazer tamanhos "otimizados" sem medir**: a textura de 512 px (0075) piorou o estabilizador. Toda mudança de desempenho precisa de medição no `KritaUsageLog.txt` antes e depois.

## O que conferir na revisão (5 minutos)

- [ ] O número é o próximo livre e não repete nenhum existente.
- [ ] A fila inteira aplica numa árvore limpa.
- [ ] Nenhuma marca de conflito em `patches/`.
- [ ] Cada função do Krita usada foi confirmada (ou marcada como não verificada).
- [ ] O patch faz **uma coisa só** e a mensagem diz qual.
- [ ] Texto da interface em português, com alvo de toque de 44 px ou mais.
- [ ] Mensagem de commit em português.

## Mensagem do patch

```
Nanquim: <o que muda, em uma linha>

<por que, em duas ou três linhas: o sintoma no tablet, a causa>
- <mudança 1>
- <mudança 2>
```

## Mudanças de design

Vão primeiro para o protótipo (canvas) e só viram código depois de aprovadas.

## Dividindo o trabalho entre as IAs

- **Uma IA integra:** recebe os patches das outras, aplica a fila inteira, revisa e sobe. As outras propõem.
- **Uma área por vez:** duas IAs não mexem ao mesmo tempo na mesma parte do `NanquimTabletUi.cpp` (ele tem 17 mil linhas e quase tudo está nele).
- **Dê o arquivo atual, não um resumo:** quem escreve um patch precisa do `NanquimTabletUi.cpp` no estado de agora, com a fila aplicada.
- **Atualize o `CONTEXTO-NANQUIM.md`** depois de cada patch que entrar.

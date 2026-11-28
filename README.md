# Nanquim

Fork do [Krita](https://krita.org) para tablets Android, focado em quadrinhos, desempenho e uma interface fácil para quem está começando (inspirada no Clip Studio e no Procreate).

## Como funciona

O repositório não guarda uma cópia inteira do Krita. Ele guarda:

- `KRITA_REF`: a versão estável do Krita usada como base (hoje Krita 5.3.5, `1706bc8`);
- `nanquim-src/`: arquivos próprios do Nanquim (interface de tablet, importador `.sut`, abertura nativa, ícones e tema), copiados direto sobre a árvore do Krita sem gerar conflitos;
- `patches/`: apenas 4 patches temáticos com os ganchos mínimos nos arquivos do Krita (`0001` a `0004`);
- `.github/workflows/android.yml`: o script que baixa o Krita oficial (`invent.kde.org`), aplica os patches, copia `nanquim-src/` e compila o APK no GitHub Actions.

Assim dá para acompanhar atualizações da linha estável (`5.3.x`) com mínimo atrito e sem fila gigante de patches.

## Baixar e instalar

1. Abra a aba **Releases** do repositório no tablet.
2. Baixe o `.apk` da build mais recente.
3. Abra o arquivo e permita a instalação de fontes desconhecidas.

O Nanquim usa o pacote `org.krita.next` (convive lado a lado com o Krita oficial) e atualiza por cima de builds anteriores do Nanquim sem desinstalar.

## Compilar de novo

Aba **Actions** → **Android arm64** → **Run workflow** (escolha o ramo desejado). No `main`, também roda sozinho a cada mudança em `nanquim-src/`, `patches/`, `KRITA_REF` ou no próprio workflow.

## Licença

O Krita é software livre sob a GNU GPL v3 ou posterior, e o Nanquim segue a mesma licença. "Krita" é marca da Krita Foundation. O Nanquim é um projeto independente, sem vínculo com ela.

## Créditos

- **Krita**: Krita Foundation e toda a comunidade do Krita e do KDE (GNU GPL v3).
- **Arte de abertura**: Tyson Tan, ilustração da Kiki feita para o Krita. As artes do Tyson Tan para o Krita são licenciadas em Creative Commons BY-SA e GNU GPL ([krita.org/en/about/mascot](https://krita.org/en/about/mascot/)).
- **Logo do Nanquim** (gota-pena): CC BY-SA 4.0.
- **Design do Modo Foco**: inspirado no [redesign do Krita por u/Mandoart-Studios](https://www.reddit.com/r/krita/comments/1op1lix/my_krita_ui_redesign_with_project_files/) (arte de fundo do post original por Sad_Tea).
- **Design de personalização, retícula e Modo Normal**: inspirado no [redesign do Krita por u/JuhoSprite](https://www.reddit.com/r/krita/comments/1footm7/i_redesigned_kritas_user_interface_ui_for_fun_as/) ("Color Intensity", retícula e imagem no canto do painel de camadas).

## Fontes

A interface usa a **Satoshi** (Indian Type Foundry, distribuída pela Fontshare sob a ITF Free Font License) e os balões de texto usam a **Montserrat** (Julieta Ulanovsky e colaboradores, SIL Open Font License 1.1). As duas são baixadas pelo CI no momento da compilação; se a Satoshi não puder ser baixada, a interface fica com a Montserrat.

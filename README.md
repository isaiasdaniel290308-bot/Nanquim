# Nanquim

Fork do [Krita](https://krita.org) para tablets Android, focado em quadrinhos, desempenho e uma interface fácil para quem está começando (inspirada no Clip Studio e no Procreate).

## Como funciona

O repositório não guarda uma cópia inteira do Krita. Ele guarda:

- `KRITA_REF`: a versão do Krita usada como base (um branch, tag ou commit);
- `patches/`: nossas mudanças, em ordem (`0001-...patch`, `0002-...patch`);
- `.github/workflows/android.yml`: o script que baixa o Krita, aplica os patches e compila o APK no GitHub Actions.

Assim dá para puxar as atualizações da equipe do Krita sem perder as nossas mudanças.

## Baixar e instalar

1. Abra a aba **Releases** do repositório no tablet.
2. Baixe o `.apk` da build mais recente.
3. Abra o arquivo e permita a instalação de fontes desconhecidas.

Se o Krita oficial estiver instalado, desinstale antes: as assinaturas são diferentes.

## Compilar de novo

Aba **Actions** → **Android arm64** → **Run workflow**. Também roda sozinho a cada mudança em `patches/`, `KRITA_REF` ou no próprio workflow.

## Licença

O Krita é software livre sob a GNU GPL v3 ou posterior, e o Nanquim segue a mesma licença. "Krita" é marca da Krita Foundation. O Nanquim é um projeto independente, sem vínculo com ela.

## Créditos

- **Krita**: Krita Foundation e toda a comunidade do Krita e do KDE (GNU GPL v3).
- **Arte de abertura**: Tyson Tan, ilustração da Kiki feita para o Krita. As artes do Tyson Tan para o Krita são licenciadas em Creative Commons BY-SA e GNU GPL ([krita.org/en/about/mascot](https://krita.org/en/about/mascot/)).
- **Logo do Nanquim** (gota-pena): CC BY-SA 4.0.

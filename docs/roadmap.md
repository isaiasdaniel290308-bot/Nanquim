# Roteiro do Nanquim

| Item | Estado | Patch |
| --- | --- | --- |
| Build Android na nuvem, APK assinado | Feito | workflow |
| Tirar plugins pouco usados do Android | Feito | 0001 |
| Cronômetro da abertura no log | Feito | 0002 |
| Nome "Nanquim" no Android | Feito | 0003 |
| Abertura em ~15 s (sem o painel QML de texto) | Feito | 0004 |
| Abrir `.kpp` e `.bundle` pelo "Abrir com" | Em teste | 0005 |
| Abertura com a arte do Tyson Tan e a logo | Em teste | 0006 |
| Medir os ~11 s restantes da janela principal | Próximo | — |
| Interface nova (galeria, barra híbrida, painéis sob demanda) | Projetada | — |
| Importador `.sut` (Clip Studio) | Planejado | — |
| Liquidificar mais rápido (prévia leve + aplicação em resolução total) | Planejado | — |

## Importador `.sut`

Um `.sut` é um banco SQLite. Plano: ler as tabelas da ferramenta (`Node`, `Variant`, `MaterialFile`), extrair a ponta (PNG) e a textura, e converter tamanho, espaçamento, pressão e opacidade para um preset `.kpp` do motor de pincel padrão do Krita. O traço fica parecido, não idêntico. Referência: os scripts de conversão da comunidade do Krita (Freya Lupen).

## Liquidificar

A ferramenta já existe no Krita (Transformar → Liquidificar). Plano:
1. Enquanto o dedo ou a caneta arrasta, deformar uma versão reduzida da camada (nível de detalhe), só na área do pincel.
2. Ao soltar, aplicar a deformação em resolução total, em várias threads.
3. Interface própria: barra inferior com Empurrar, Girar, Inflar, Encolher e Restaurar.

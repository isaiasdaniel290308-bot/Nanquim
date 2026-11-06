# Fase 1: otimização medida

A regra da fase é medir antes de mudar. Cada mudança vira um patch em `patches/` e só fica se melhorar a medição no Galaxy Tab S10 Lite.

## 1. Como medir

O Krita já tem medidores internos, desligados por padrão (`libs/ui/kis_config.cc`):

| Opção no kritarc | Padrão | O que mede |
| --- | --- | --- |
| `enableOpenGLFramerateLogging` | false | Quadros por segundo do canvas |
| `enableBrushSpeedLogging` | false | Velocidade do pincel durante o traço |

Primeiro patch planejado: ligar os dois só na build do Nanquim e mandar a saída para o `logcat`. A build já usa `-DANDROID_ENABLE_STDIO_FORWARDING`, então dá para ler com `adb logcat`, usando a depuração sem fio do tablet.

## 2. Padrões candidatos para tablet

Valores padrão atuais do Krita, que valem para todas as plataformas. Nenhum deles é ajustado para Android hoje.

| Opção | Padrão atual | Candidato no Android | Por que testar |
| --- | --- | --- | --- |
| `OpenGLFilterMode` | 3 (alta qualidade) | 2 ou 1 | O filtro de alta qualidade custa caro numa GPU de celular durante zoom e giro |
| `levelOfDetailEnabled` (Pré-visualização instantânea) | false | true | Acelera muito pincéis grandes em imagens grandes |
| `fpsLimit` | 100 | taxa da tela | Renderizar acima da taxa da tela só gasta bateria (confirmar quantos Hz tem o Tab S10 Lite) |
| `maxNumberOfThreads` | todos os núcleos | testar só os rápidos | Chips de tablet misturam núcleos rápidos e econômicos (confirmar o processador do Tab S10 Lite) |
| `memoryHardLimitPercent` | 50% | medir | O tablet tem pouca RAM livre com o Android aberto |
| `textureSize` | 256 | testar 512 | Texturas maiores significam menos chamadas de upload na GPU |

## 3. Ordem de trabalho

1. Build base funcionando (sem patches) e instalada no tablet.
2. Patch 0001: medidores ligados. Medir a linha de base com a tabela do Passo A do guia da Fase 0.
3. Patches 0002 em diante: um padrão por vez, medindo cada um.
4. Só depois: tirar do APK os plugins que um app de tablet não usa, para reduzir tamanho e tempo de abertura.

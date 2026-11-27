/*
 *  SPDX-FileCopyrightText: 2026 Nanquim
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "NanquimTabletUi.h"
#include "NanquimWheel.h"

#include <QAction>
#include <QApplication>
#include <QBuffer>
#include <QDockWidget>
#include <QEasingCurve>
#include <QEvent>
#include <QHBoxLayout>
#include <QImageReader>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QStackedWidget>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPainter>
#include <QInputDialog>
#include "KisRecentFileIconCache.h"
#include <KisRecentFilesManager.h>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QSharedPointer>
#include <QSpinBox>
#include <cmath>
#include <flake/kis_shape_layer.h>
#include "kis_node_commands_adapter.h"
#include "kis_node_manager.h"
#include <KoShapeControllerBase.h>
#include <kis_group_layer.h>
#include <QCryptographicHash>
#include <QDir>
#include <QDateTime>
#include <QFileInfo>
#include <QElapsedTimer>
#include <QStandardPaths>
#include <QUuid>
#include <QtConcurrent>
#include <atomic>
#include <algorithm>
#include <memory>
#ifdef Q_OS_ANDROID
#include <QAndroidJniObject>
#include <QtAndroid>
#endif
#include <ksharedconfig.h>
#include <kconfiggroup.h>
#include <kis_paint_device.h>
#include <kis_painter.h>
#include <kis_selection.h>
#include <kis_spontaneous_job.h>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QMouseEvent>
#include <QTabletEvent>
#include <QTouchEvent>
#include <QGridLayout>
#include <QPropertyAnimation>
#include <QScrollArea>
#include <QScrollBar>
#include <QComboBox>
#include <QPointer>
#include <QButtonGroup>
#include <QCheckBox>
#include <QPlainTextEdit>
#include <QTextEdit>
#include <QAbstractItemView>
#include <QScroller>
#include <QScrollerProperties>
#include <QScrollBar>
#include <QProgressBar>
#include <QProgressDialog>
#include <QPdfWriter>
#include <QPageSize>
#include <QDesktopServices>
#include <QRegularExpression>
#include <KisVisualColorSelector.h>
#include <KisColorSelectorConfiguration.h>
#include <KoColorSpace.h>
#include "kis_display_color_converter.h"
#include <kis_config_notifier.h>
#include <kis_cubic_curve.h>
#include <kis_image_config.h>
#include "kis_guides_config.h"
#include <kis_paint_layer.h>
#include <QPalette>
#include <QToolTip>
#include <QKeyEvent>
#include <QAbstractButton>
#include <QHash>
#include <QLocale>
#include <QStyle>
#include <QFile>
#include <QPainterPath>
#include <QFontDatabase>
#include <QRandomGenerator>
#include <QUrl>
#include <KisResourceModel.h>
#include <KisTagResourceModel.h>
#include <KisStorageModel.h>
#include "NanquimSutImport.h"
#include <KisResourceUserOperations.h>
#include <KoFileDialog.h>
#include <brushengine/kis_paintop_settings.h>
#include "kis_paintop_box.h"
#include "kis_favorite_resource_manager.h"
#include <QSet>
#include <QUrlQuery>
#include <QPixmapCache>
#include <QScreen>
#include <QSlider>
#include <QTimer>
#include <QGraphicsOpacityEffect>
#include <QToolBar>
#include <QToolButton>
#include <QStatusBar>
#include <QTabBar>
#include <QVBoxLayout>
#include <QVariantAnimation>
#include <QWidgetAction>
#include <functional>
#include <cmath>

#include <kactioncollection.h>
#include <ktoolbar.h>
#include <klocalizedstring.h>

#include <KoCanvasController.h>
#include <KoCanvasResourceProvider.h>
#include <KoCanvasResourcesIds.h>
#include <KoColor.h>
#include <KoToolManager.h>
#include <kis_image.h>

#include "KisDocument.h"
#include "KisImportExportErrorCode.h"
#include "KisImportExportUtils.h"
#include "KisPart.h"
#include "KisResourceServerProvider.h"
#include <KisTagModel.h>
#include <KisResourceTypes.h>
#include <kis_paintop_preset.h>
#include <QDialog>
#include "KisRecentDocumentsModelWrapper.h"
#include <KisKineticScroller.h>
#include <KoColorSpaceRegistry.h>
#include "kis_config.h"
#include "tool/kis_smoothing_options.h"
#include "canvas/kis_canvas2.h"
#include <KoToolBase.h>
#include "KisMainWindow.h"
#include "KisView.h"
#include "KisViewManager.h"
#include "kis_canvas_resource_provider.h"
#include "canvas/kis_canvas2.h"
#include "canvas/kis_coordinates_converter.h"
#include "KisUsageLogger.h"
#include "kis_resources_snapshot.h"
#include "KisAsynchronousStrokeUpdateHelper.h"
#include <strokes/freehand_stroke.h>
#include <strokes/KisFreehandStrokeInfo.h>
#include <kis_paint_information.h>

// KisApplication.cpp: instala .kpp, .bundle e .sut
bool nanquimImportResourcePublic(QWidget *parent, const QString &url);

namespace
{

// ---------------------------------------------------------------------------
// Cores do design (artboards "Desenho" e "Main")
// ---------------------------------------------------------------------------
const QColor C_HEADER(0x24, 0x23, 0x1f);
const QColor C_NAV(0x2e, 0x2d, 0x29);
const QColor C_TEXT(0xee, 0xea, 0xe0);
const QColor C_MUTED(0xa8, 0xa3, 0x98);
const QColor C_DISABLED(0x5e, 0x5b, 0x54);
const QColor C_ACCENT(0x3e, 0x8e, 0x7e);

// ---------------------------------------------------------------------------
// Cores com função fixa. Cada cor tem um papel só; patch novo usa estas, não
// inventa outra (lista completa em docs/CORES.md do repositório).
// Paleta do redesenho: grafite e papel, com verdete como único destaque. Os
// nomes antigos ficaram (AZUL, CIANO) para não mexer em todo o arquivo.
//   AZUL        verdete #3e8e7e: ativo, seleção, borda de foco
//               (botão principal: verdete escuro #2f7568 com texto branco)
//   CIANO       verdete claro #6fbfad: toque, ferramenta ativa, arte-final
//   coral       borracha, erro, ação que apaga
//   verde       pronto, salvo, concluído
//   âmbar       rascunho, atenção
//   AZUL_CLARO  verdete claro: texto e ícone ativos, números de medida
// ---------------------------------------------------------------------------
namespace Cor
{
const QColor AZUL(0x3e, 0x8e, 0x7e);
const QColor CIANO(0x6f, 0xbf, 0xad);
const QColor CORAL(0xff, 0x6b, 0x8b);
const QColor VERDE(0x5e, 0xc9, 0x8c);
const QColor AMBAR(0xe6, 0xb4, 0x5a);
const QColor AZUL_CLARO(0x6f, 0xbf, 0xad);
const QColor TEXTO(0xee, 0xea, 0xe0);
const QColor TEXTO_SUAVE(0xa8, 0xa3, 0x98);
const QColor FUNDO(0x2e, 0x2d, 0x29);
const QColor LINHA(0x4a, 0x48, 0x42);
}

// ---------------------------------------------------------------------------
// Três níveis de relevo: quanto mais perto do dedo, mais clara a borda. As
// superfícies são lisas, sem brilho no topo (fundo #2e2d29).
//   nível 1  barras fixas (topo, lateral): plano, borda #4a4842
//   nível 2  peças sobre a página (dock, cores rápidas, cápsula, cartões)
//   nível 3  pop-ups e janelas (painéis, pincéis, ferramentas, avisos)
// ---------------------------------------------------------------------------
#define NQ_RELEVO2_BG "rgba(46,45,41,0.96)"
#define NQ_RELEVO2_BORDA "#524f48"
#define NQ_RELEVO3_BG "#2e2d29"
#define NQ_RELEVO3_BORDA "#5a574f"

const QString TOP_ID = QStringLiteral("nanquimTop");
const QString NAV_ID = QStringLiteral("nanquimTools");
const QString OLD_TOP_ID = QStringLiteral("BrushesAndStuff");
const QString TOOLBOX_ID = QStringLiteral("ToolBox");
const char *NEEDS_CANVAS = "nanquimNeedsCanvas";
const char *WELCOME_ONLY = "nanquimWelcomeOnly";

bool s_canvasCleaned = false;
bool s_navCollapsed = false;
bool s_focusMode = false;   // modo foco: só a página, o dock do polegar, as 3 cores e a cápsula vertical de ferramentas
bool s_classic = false;   // modo de interface desta sessão (decidido ao abrir o app)

bool isClassicMode()
{
    return KSharedConfig::openConfig()->group("Nanquim").readEntry("uiMode", QStringLiteral("tablet"))
            == QLatin1String("classic");
}

// ---------------------------------------------------------------------------
// Ícones do design, desenhados a partir do SVG (traço 1.8, cantos redondos)
// ---------------------------------------------------------------------------
QPixmap svgPixmap(const QString &body, const QColor &color, int logicalSize)
{
    const qreal dpr = qApp->devicePixelRatio();
    const QString svg = QStringLiteral(
        "<svg xmlns='http://www.w3.org/2000/svg' width='24' height='24' viewBox='0 0 24 24' "
        "fill='none' stroke='%1' stroke-width='1.8' stroke-linecap='round' stroke-linejoin='round'>%2</svg>")
            .arg(color.name(), QString(body).replace(QLatin1String("currentColor"), color.name()));
    QByteArray data = svg.toUtf8();
    QBuffer buffer(&data);
    buffer.open(QIODevice::ReadOnly);
    QImageReader reader(&buffer, "svg");
    const int px = qRound(logicalSize * dpr);
    reader.setScaledSize(QSize(px, px));
    QImage image = reader.read();
    if (image.isNull()) {
        return QPixmap();
    }
    image.setDevicePixelRatio(dpr);
    return QPixmap::fromImage(image);
}

QIcon designIcon(const char *body, int size = 22)
{
    const QString b = QString::fromUtf8(body);
    QIcon icon;
    icon.addPixmap(svgPixmap(b, C_TEXT, size), QIcon::Normal, QIcon::Off);
    icon.addPixmap(svgPixmap(b, C_ACCENT, size), QIcon::Normal, QIcon::On);
    icon.addPixmap(svgPixmap(b, C_DISABLED, size), QIcon::Disabled, QIcon::Off);
    icon.addPixmap(svgPixmap(b, C_DISABLED, size), QIcon::Disabled, QIcon::On);
    // versão maior para o pairar da S Pen (ícone cresce sem borrar)
    const int big = qRound(size * 1.15);
    icon.addPixmap(svgPixmap(b, C_TEXT, big), QIcon::Normal, QIcon::Off);
    icon.addPixmap(svgPixmap(b, C_ACCENT, big), QIcon::Normal, QIcon::On);
    return icon;
}

namespace Svg
{
const char *GALLERY = "<rect x='3.5' y='3.5' width='7' height='7' rx='1.5'/><rect x='13.5' y='3.5' width='7' height='7' rx='1.5'/><rect x='3.5' y='13.5' width='7' height='7' rx='1.5'/><rect x='13.5' y='13.5' width='7' height='7' rx='1.5'/>";
const char *BACK = "<path d='M15 6l-6 6 6 6'/>";
const char *FORWARD = "<path d='M9 6l6 6-6 6'/>";
const char *UNDO = "<path d='M9 14L4 9l5-5'/><path d='M4 9h10.5a5.5 5.5 0 0 1 0 11H11'/>";
const char *REDO = "<path d='M15 14l5-5-5-5'/><path d='M20 9H9.5a5.5 5.5 0 0 0 0 11H13'/>";
const char *PEN = "<path d='M15 4l5 5L9.5 19.5 4 20l.5-5.5z'/><path d='M13 6l5 5'/>";
const char *LAYERS = "<path d='M12 3.5l8.5 4.5-8.5 4.5L3.5 8z'/><path d='M3.5 12.5l8.5 4.5 8.5-4.5'/><path d='M3.5 16.5L12 21l8.5-4.5'/>";
const char *RULER = "<path d='M3 17L17 3l4 4L7 21z'/><path d='M7 13l2 2M10 10l2 2M13 7l2 2'/>";
const char *REFERENCE = "<rect x='3.5' y='4.5' width='17' height='15' rx='2'/><circle cx='9' cy='10' r='1.8'/><path d='M20.5 16l-5-5-8 8.5'/>";
const char *INSERT_IMAGE = "<rect x='3.5' y='5.5' width='13' height='13' rx='2'/><path d='M3.5 15l4-4 5 5'/><path d='M19 3v6M16 6h6'/>";
const char *TIMELAPSE = "<rect x='3.5' y='5.5' width='17' height='13' rx='2'/><path d='M10 9.5v5l4.5-2.5z'/>";
const char *BALLOON = "<path d='M12 4c5 0 9 2.9 9 6.5S17 17 12 17c-.9 0-1.8-.1-2.6-.3L5 19.5l1.2-3.6C4.2 14.7 3 12.7 3 10.5 3 6.9 7 4 12 4z'/>";
const char *PANELS = "<rect x='3.5' y='3.5' width='17' height='17' rx='1.5'/><path d='M3.5 10h17M12 10v10.5'/>";
const char *MORE = "<circle cx='5' cy='12' r='1.8' fill='currentColor' stroke='none'/><circle cx='12' cy='12' r='1.8' fill='currentColor' stroke='none'/><circle cx='19' cy='12' r='1.8' fill='currentColor' stroke='none'/>";
const char *OPTIONS = "<path d='M4 7h9M17 7h3M4 17h3M11 17h9'/><circle cx='15' cy='7' r='2'/><circle cx='9' cy='17' r='2'/>";
const char *ERASER = "<path d='M9 20h11'/><path d='M4.6 14.6l8.8-8.8a2 2 0 0 1 2.8 0l2.6 2.6a2 2 0 0 1 0 2.8L11 19H8.9z'/>";
const char *MOVE = "<path d='M12 3v18M3 12h18'/><path d='M9 6l3-3 3 3M9 18l3 3 3-3M6 9l-3 3 3 3M18 9l3 3-3 3'/>";
const char *BUCKET = "<path d='M4.5 11.5L11 5l7 7-6.5 6.5a2 2 0 0 1-2.8 0l-4.2-4.2a2 2 0 0 1 0-2.8z'/><path d='M4.5 12h13.5'/><path d='M20 15.5c.8 1.2 1.3 2 1.3 2.7a1.3 1.3 0 0 1-2.6 0c0-.7.5-1.5 1.3-2.7z'/>";
const char *SELECT = "<rect x='4' y='4' width='16' height='16' rx='1' stroke-dasharray='3 2.5'/>";
const char *TRANSFORM = "<rect x='6' y='6' width='12' height='12' rx='1'/><rect x='3' y='3' width='4' height='4' rx='.8'/><rect x='17' y='3' width='4' height='4' rx='.8'/><rect x='3' y='17' width='4' height='4' rx='.8'/><rect x='17' y='17' width='4' height='4' rx='.8'/>";
const char *TEXT = "<path d='M12 4c5 0 9 2.9 9 6.5S17 17 12 17c-.9 0-1.8-.1-2.6-.3L5 19.5l1.2-3.6C4.2 14.7 3 12.7 3 10.5 3 6.9 7 4 12 4z'/><path d='M9 9h6M12 9v5'/>";
const char *PICKER = "<path d='M16.5 3.5a2.1 2.1 0 0 1 3 3l-2 2-3-3z'/><path d='M14.5 7.5L6 16v3h3l8.5-8.5'/>";
const char *EXPORT = "<path d='M12 15V4M7.5 8.5L12 4l4.5 4.5'/><path d='M5 13v5a2 2 0 0 0 2 2h10a2 2 0 0 0 2-2v-5'/>";
const char *SETTINGS = "<path d='M18.86 10.61 L21.13 10.90 L21.13 13.10 L18.86 13.39 L17.83 15.87 L19.24 17.68 L17.68 19.24 L15.87 17.83 L13.39 18.86 L13.10 21.13 L10.90 21.13 L10.61 18.86 L8.13 17.83 L6.32 19.24 L4.76 17.68 L6.17 15.87 L5.14 13.39 L2.87 13.10 L2.87 10.90 L5.14 10.61 L6.17 8.13 L4.76 6.32 L6.32 4.76 L8.13 6.17 L10.61 5.14 L10.90 2.87 L13.10 2.87 L13.39 5.14 L15.87 6.17 L17.68 4.76 L19.24 6.32 L17.83 8.13z'/><circle cx='12' cy='12' r='3'/>";
const char *SEARCH = "<circle cx='11' cy='11' r='6.5'/><path d='M16 16l4 4'/>";
const char *FOLDER = "<path d='M3.5 7.5a2 2 0 0 1 2-2h4l2 2h7a2 2 0 0 1 2 2v8a2 2 0 0 1-2 2h-13a2 2 0 0 1-2-2z'/>";
const char *BOOK = "<path d='M4 5.5A1.5 1.5 0 0 1 5.5 4H11v16H5.5A1.5 1.5 0 0 1 4 18.5z'/><path d='M20 5.5A1.5 1.5 0 0 0 18.5 4H13v16h5.5a1.5 1.5 0 0 0 1.5-1.5z'/>";
const char *RETURN = "<path d='M9 6l-5 6 5 6'/><path d='M4 12h11a5 5 0 0 1 5 5'/>";
const char *NIB = "<path d='M12 3l6 10-6 8-6-8z'/><path d='M12 3v9'/><circle cx='12' cy='13.5' r='1.4'/>";
const char *BRUSH = "<path d='M5 19c0-2.5 1.5-3.5 3.5-3.5l9-9 2 2-9 9c0 2-1.5 3-3.5 3-1 0-2-.5-2-1.5z'/>";
const char *STAR = "<path d='M12 3l2.6 6 6.4.5-4.9 4.3 1.5 6.2L12 16.6 6.4 20l1.5-6.2L3 9.5 9.4 9z'/>";
const char *LIST = "<path d='M5 7h14M5 12h14M5 17h14'/>";
const char *IMPORT = "<path d='M12 4v11M7.5 10.5L12 15l4.5-4.5'/><path d='M5 18v1a1 1 0 0 0 1 1h12a1 1 0 0 0 1-1v-1'/>";
const char *ALL_TOOLS = "<circle cx='6' cy='6' r='1.6'/><circle cx='12' cy='6' r='1.6'/><circle cx='18' cy='6' r='1.6'/><circle cx='6' cy='12' r='1.6'/><circle cx='12' cy='12' r='1.6'/><circle cx='18' cy='12' r='1.6'/><circle cx='6' cy='18' r='1.6'/><circle cx='12' cy='18' r='1.6'/><circle cx='18' cy='18' r='1.6'/>";
const char *MAGNET = "<path d='M6 4v7a6 6 0 0 0 12 0V4'/><path d='M6 8h3M15 8h3'/>";
const char *HUB = "<rect x='7.5' y='3.5' width='13' height='13' rx='2'/><path d='M4.5 8v10a2 2 0 0 0 2 2h10M14 3.5v13M7.5 10h13'/>";
const char *TONE = "<circle cx='6' cy='6' r='1.7'/><circle cx='12' cy='6' r='1.7'/><circle cx='18' cy='6' r='1.7'/><circle cx='9' cy='12' r='1.7'/><circle cx='15' cy='12' r='1.7'/><circle cx='6' cy='18' r='1.7'/><circle cx='12' cy='18' r='1.7'/><circle cx='18' cy='18' r='1.7'/>";
const char *SPEED = "<path d='M3 12h6M3 7l5 2.5M3 17l5-2.5M21 12h-6M21 7l-5 2.5M21 17l-5-2.5M12 3v4M12 17v4'/>";
const char *EXTRACT = "<path d='M5 19L14 10M14 4l1.2 2.8L18 8l-2.8 1.2L14 12l-1.2-2.8L10 8l2.8-1.2zM19 14l.7 1.6 1.6.7-1.6.7L19 18.6l-.7-1.6-1.6-.7 1.6-.7z'/>";
const char *OUTLINE = "<rect x='8' y='8' width='8' height='8' rx='1.5'/><rect x='3.5' y='3.5' width='17' height='17' rx='4' stroke-dasharray='2.4 2.4'/>";
const char *DRAFT = "<path d='M4 20l1-4 11-11 3 3-11 11z' stroke-dasharray='2.4 2'/><path d='M14 7l3 3'/>";
const char *PHONE = "<rect x='7' y='3' width='10' height='18' rx='2.5'/><path d='M10.5 18h3'/>";
const char *SLICE = "<rect x='5' y='3.5' width='14' height='17' rx='2'/><path d='M3 9h18M3 15h18' stroke-dasharray='2 2'/>";
const char *TRASH = "<path d='M4.5 7h15M9 7V5a1 1 0 0 1 1-1h4a1 1 0 0 1 1 1v2M7 7l1 12.5a1.5 1.5 0 0 0 1.5 1.5h5a1.5 1.5 0 0 0 1.5-1.5L17 7'/><path d='M10 11v6M14 11v6'/>";
}

// telas do Nanquim (definidas mais adiante)
void showComicsSheet(KisMainWindow *mw, int tab);
void showExport(KisMainWindow *mw);
void showBrushSettings(KisMainWindow *mw);
void showSettings(KisMainWindow *mw);
void showAbout(KisMainWindow *mw);
void showNewPage(KisMainWindow *mw);
void refreshStudioBrushes(KisMainWindow *mw);
void refreshFocusBarInfo(KisMainWindow *mw);
void placeFocusWidgets(KisMainWindow *mw);
void setKritaChromeHidden(KisMainWindow *mw, bool hidden);
void showFocusBarAnimated(KisMainWindow *mw, bool visible, bool animate);
void focusBarOpenPage(KisMainWindow *mw, const QString &path);
void placeStudio(KisMainWindow *mw);
void placeSelectionBar(KisMainWindow *mw);
void updateSelectionBar(KisMainWindow *mw, bool animate = true);
void buildSelectionBar(KisMainWindow *mw);
void markBrushPanelDirty(KisMainWindow *mw);
void warmUpPopups(KisMainWindow *mw);
void populateRulersMenu(KisMainWindow *mw, QMenu *menu, QWidget *anchor);
void showScreentoneSheet(KisMainWindow *mw);
void showExtractLineSheet(KisMainWindow *mw);
void showSpeedlinesSheet(KisMainWindow *mw);
void showLayerOutlineSheet(KisMainWindow *mw);
void applyBlueDraftLayer(KisMainWindow *mw);
void toggleComicBucketPreset(KisMainWindow *mw);
void showWebtoonToolsSheet(KisMainWindow *mw, int initialTab = 0);
void showCommunityCatalogSheet(KisMainWindow *mw);
void showComicHubSheet(KisMainWindow *mw);
void showGalleryAnimated(KisMainWindow *mw);
QWidget *buildCommunityPage(KisMainWindow *mw, QWidget *parent);
bool nqConfirm(KisMainWindow *mw, const QString &section, const QString &title, const QString &text, const QString &confirm);
void showToast(KisMainWindow *mw, const QString &text);

// ---------------------------------------------------------------------------
// Animações
// O Krita desliga as animações nativas de painéis no Android (eram lentas e
// davam crash). Aqui a animação é feita por cima: uma camada leve, que não
// recebe toques, desenha uma "foto" do painel deslizando e aparecendo. O painel
// de verdade já está no lugar, então nada é redimensionado a cada quadro.
// ---------------------------------------------------------------------------
class NanquimOverlay : public QWidget
{
public:
    using Painter = std::function<void(QPainter &, const QRect &, qreal)>;

    NanquimOverlay(QWidget *parent, const QRect &geometry, Painter painter)
        : QWidget(parent)
        , m_painter(std::move(painter))
    {
        setAttribute(Qt::WA_TransparentForMouseEvents);
        setAttribute(Qt::WA_NoSystemBackground);
        setFocusPolicy(Qt::NoFocus);
        setGeometry(geometry);
        raise();
        show();
    }

    void setProgress(qreal p)
    {
        m_progress = p;
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter g(this);
        g.setRenderHint(QPainter::SmoothPixmapTransform);
        m_painter(g, rect(), m_progress);
    }

private:
    Painter m_painter;
    qreal m_progress = 0.0;
};

QPointer<NanquimOverlay> s_panelOverlay;
QPointer<NanquimOverlay> s_navOverlay;
QPointer<NanquimOverlay> s_screenOverlay;

void runOverlay(QPointer<NanquimOverlay> &slot, NanquimOverlay *overlay, int durationMs,
                QEasingCurve::Type easing, std::function<void()> onDone = {})
{
    if (slot) {
        slot->deleteLater();
    }
    slot = overlay;
    QVariantAnimation *anim = new QVariantAnimation(overlay);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->setDuration(durationMs);
    anim->setEasingCurve(easing);
    QObject::connect(anim, &QVariantAnimation::valueChanged, overlay, [overlay](const QVariant &v) {
        overlay->setProgress(v.toReal());
    });
    QObject::connect(anim, &QVariantAnimation::finished, overlay, [overlay, onDone]() {
        if (onDone) {
            onDone();
        }
        overlay->deleteLater();
    });
    anim->start();
}

// filtro de eventos a partir de uma função (build 60.5)
class EventFn : public QObject
{
public:
    using Fn = std::function<bool(QObject *, QEvent *)>;
    EventFn(QObject *parent, Fn fn) : QObject(parent), m_fn(std::move(fn)) {}
protected:
    bool eventFilter(QObject *o, QEvent *e) override { return m_fn(o, e); }
private:
    Fn m_fn;
};

// foto para animação: em tela densa (DPR 2,5+), um grab() de página inteira
// passa de 50 MB de RAM por instantes. A foto vive em movimento: 1,5× de DPR
// é indistinguível e corta o pico pela metade ou mais.
QPixmap lightShot(const QPixmap &shot)
{
    const qreal dpr = shot.devicePixelRatio();
    if (shot.isNull() || dpr <= 1.5) return shot;
    QPixmap light = shot.scaled(shot.size() * (1.5 / dpr), Qt::IgnoreAspectRatio, Qt::FastTransformation);
    light.setDevicePixelRatio(1.5);
    return light;
}

// troca de página com a página antiga sumindo por cima (fade + leve subida):
// uma foto só, então não pesa no tablet
void fadeSwitch(QStackedWidget *stack, QWidget *to, int durationMs = 190)
{
    if (!stack || !to || stack->currentWidget() == to) return;
    if (!stack->isVisible() || !stack->currentWidget()) {
        stack->setCurrentWidget(to);
        return;
    }
    const QPixmap old = lightShot(stack->currentWidget()->grab());
    stack->setCurrentWidget(to);
    NanquimOverlay *o = new NanquimOverlay(stack, stack->rect(), [old](QPainter &g, const QRect &, qreal p) {
        g.setOpacity(1.0 - p);
        g.drawPixmap(QPointF(0, -10.0 * p), old);
    });
    QVariantAnimation *anim = new QVariantAnimation(o);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->setDuration(durationMs);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    QObject::connect(anim, &QVariantAnimation::valueChanged, o, [o](const QVariant &v) { o->setProgress(v.toReal()); });
    QObject::connect(anim, &QVariantAnimation::finished, o, [o]() { o->deleteLater(); });
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

// curva de mola leve, só para pop-ups (o padrão do OutBack é exagerado)
QEasingCurve softSpring()
{
    QEasingCurve c(QEasingCurve::OutBack);
    c.setOvershoot(0.8);
    return c;
}

// aplica já o layout pendente, para saber a posição final antes de desenhar
void flushLayouts()
{
    QApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
}

const int SLIDE = 56;

// direction: +1 entra/sai pela direita, -1 pela esquerda
void animateIn(QPointer<NanquimOverlay> &slot, QWidget *host, QWidget *w, int direction, const QColor &bg)
{
    flushLayouts();
    const QRect geom(w->mapTo(host, QPoint(0, 0)), w->size());
    if (geom.isEmpty()) {
        return;
    }
    const QPixmap shot = lightShot(w->grab());
    NanquimOverlay *o = new NanquimOverlay(host, geom, [shot, bg, direction](QPainter &g, const QRect &r, qreal p) {
        g.fillRect(r, bg);
        g.setOpacity(p);
        g.drawPixmap(QPointF(direction * (1.0 - p) * SLIDE, 0), shot);
    });
    runOverlay(slot, o, 240, QEasingCurve::OutCubic);
}

void animateOut(QPointer<NanquimOverlay> &slot, QWidget *host, const QRect &geom, const QPixmap &shot, int direction)
{
    if (geom.isEmpty() || shot.isNull()) {
        return;
    }
    flushLayouts();
    NanquimOverlay *o = new NanquimOverlay(host, geom, [shot, direction](QPainter &g, const QRect &, qreal p) {
        g.setOpacity(1.0 - p);
        g.drawPixmap(QPointF(direction * p * SLIDE, 0), shot);
    });
    runOverlay(slot, o, 190, QEasingCurve::InCubic);
}

// troca de tela (inicial <-> desenho): a nova tela surge de um fundo escuro
void animateScreenIn(KisMainWindow *mw)
{
    if (!mw) return;
    // quando há transição própria (abertura ou volta à galeria), não empilha outro véu por cima
    if (mw->findChild<QWidget*>(QStringLiteral("nanquimOpeningVeil"))
            || mw->findChild<QWidget*>(QStringLiteral("nanquimReturnFly"))) {
        return;
    }
    QWidget *central = mw->centralWidget();
    if (!central) {
        return;
    }
    const QRect geom(central->mapTo(mw, QPoint(0, 0)), central->size());
    NanquimOverlay *o = new NanquimOverlay(mw, geom, [](QPainter &g, const QRect &r, qreal p) {
        g.fillRect(r, QColor(36, 35, 31, int(255 * (1.0 - p))));
    });
    runOverlay(s_screenOverlay, o, 320, QEasingCurve::OutCubic);
}

// ---------------------------------------------------------------------------
// Peças
// ---------------------------------------------------------------------------
QToolButton *iconButton(QWidget *parent, const QString &tip, const QIcon &icon, bool checkable = false)
{
    QToolButton *b = new QToolButton(parent);
    b->setToolTip(tip);
    b->setIcon(icon);
    b->setIconSize(QSize(22, 22));
    b->setToolButtonStyle(Qt::ToolButtonIconOnly);
    b->setCheckable(checkable);
    b->setAutoRaise(true);
    b->setFocusPolicy(Qt::NoFocus);
    return b;
}

QDockWidget *findDock(KisMainWindow *mw, const QStringList &ids)
{
    Q_FOREACH (const QString &id, ids) {
        if (QDockWidget *dw = mw->dockWidget(id)) {
            return dw;
        }
    }
    return nullptr;
}

QAction *act(KisMainWindow *mw, const char *name)
{
    return mw->actionCollection()->action(QLatin1String(name));
}

// Lado da barra de ferramentas (escolha do artista: canhoto ou destro).
// Os painéis abrem sempre do lado oposto, para não cobrir a barra e o dock.
bool railOnRight()
{
    return KSharedConfig::openConfig()->group("Nanquim").readEntry("railSide", QStringLiteral("left"))
            == QLatin1String("right");
}

Qt::DockWidgetArea panelArea()
{
    return railOnRight() ? Qt::LeftDockWidgetArea : Qt::RightDockWidgetArea;
}

// ---------------------------------------------------------------------------
// Painéis (pendência 3): sobrepostos à página, um de cada vez, com alfinete.
// - solto: flutua do lado oposto ao da barra, sem empurrar a página; fecha com
//   um toque do dedo fora dele ou 300 ms depois que a caneta levanta da página;
// - fixado: vira coluna e a página cede espaço (o painel do Krita de sempre);
// - largura: 320 px em telas largas, 280 px nas médias; só fixa se sobrarem
//   pelo menos 360 px para a página;
// - Cor abre como popover de 280 px ao lado da cor atual.
// ---------------------------------------------------------------------------
const QString PANEL_HOST_ID = QStringLiteral("nanquimPanelHost");
const QString COLOR_DOCK_NAME = QStringLiteral("NanquimColorDock");
const char *SVG_PIN = "<path d='M15 4.5l-4 4-4 1.5-1.5 1.5 7 7 1.5-1.5 1.5-4 4-4M9 15l-4.5 4.5M14.5 4l5.5 5.5'/>";
const char *SVG_CLOSE = "<path d='M6 6l12 12M18 6L6 18'/>";

const char *PANEL_HOST_STYLE =
    "QFrame#nanquimPanelHost { background: " NQ_RELEVO3_BG "; border: 1px solid " NQ_RELEVO3_BORDA "; border-radius: 16px; }"
    "QWidget#nanquimPanelHeader { background: #2e2d29; }"
    "QWidget#nanquimPanelHeader QLabel { color: #eeeae0; font-size: 16px; font-weight: 700; background: transparent; }"
    "QWidget#nanquimPanelHeader QToolButton { border: none; border-radius: 10px; padding: 0;"
    "  min-width: 44px; max-width: 44px; min-height: 44px; max-height: 44px; }"
    "QWidget#nanquimPanelHeader QToolButton:hover { background: rgba(255,255,255,0.06); }"
    "QWidget#nanquimPanelHeader QToolButton:pressed { background: #4a4842; }"
    "QWidget#nanquimPanelHeader QToolButton:checked { background: #2a3d38; }"
    "QWidget#nanquimPanelHeader QToolButton:disabled { background: transparent; }";

QPointer<NanquimOverlay> s_hostOverlay;

bool isPopoverDock(QDockWidget *dw)
{
    return dw && dw->objectName() == COLOR_DOCK_NAME;
}

// build 60: todos os painéis abrem como pop-up preso ao botão (sem dividir a tela)
bool isPinned(QDockWidget *dw)
{
    Q_UNUSED(dw);
    return false;
}

// botão que abriu o pop-up atual (o pop-up abre preso a ele)
QPointer<QWidget> s_popupAnchor;

void setBrushPanelVisible(KisMainWindow *mw, bool visible);
bool canvasOnlyActive(KisMainWindow *mw);

// caneta de verdade (S Pen): o toque dela não é dedo. Fechar um pop-up não
// pode roubar o traço que a caneta estava começando.
static bool nanquimPenDevice(QTabletEvent *te)
{
    switch (te->device()) {
    case QTabletEvent::Stylus:
    case QTabletEvent::Airbrush:
    case QTabletEvent::RotationStylus:
        return true;
    default:
        return false;
    }
}

// tamanho que o conteúdo do painel pede, para abrir inteiro (sem cortar à direita)
QSize popupContentSize(QDockWidget *dw)
{
    QWidget *content = dw->widget();
    QSize want = content ? content->sizeHint() : QSize(280, 360);
    if (content) {
        want = want.expandedTo(content->minimumSizeHint());
        QList<QScrollArea*> areas = content->findChildren<QScrollArea*>();
        if (QScrollArea *self = qobject_cast<QScrollArea*>(content)) areas.prepend(self);
        Q_FOREACH (QScrollArea *sa, areas) {
            if (!sa->widget() || !sa->isVisibleTo(dw)) continue;
            QWidget *inner = sa->widget();
            QSize hint = inner->sizeHint().expandedTo(inner->minimumSizeHint());
            hint += QSize(2 * sa->frameWidth() + sa->verticalScrollBar()->sizeHint().width() + 4, 2 * sa->frameWidth() + 4);
            // a área de rolagem fica dentro de outras margens do painel
            const QSize extra = content->sizeHint() - sa->sizeHint();
            want = want.expandedTo(hint + QSize(qMax(0, extra.width()), qMax(0, extra.height())));
        }
    }
    int header = 0;
    if (QWidget *title = dw->titleBarWidget()) header = title->sizeHint().height();
    return QSize(want.width() + 2, want.height() + header + 2);
}

void storePinned(QDockWidget *dw, bool on)
{
    KConfigGroup cfg = KSharedConfig::openConfig()->group("Nanquim");
    cfg.writeEntry(QStringLiteral("pinned_%1").arg(dw->objectName()).toUtf8().constData(), on);
}

// faixas de largura da pendência 5
int panelWidthFor(KisMainWindow *mw)
{
    return mw->width() >= 1000 ? 320 : 280;
}

bool canPin(KisMainWindow *mw)
{
    int railWidth = 0;
    if (QToolBar *nav = mw->findChild<QToolBar*>(QStringLiteral("nanquimTools"))) {
        if (nav->isVisible()) railWidth = nav->width();
    }
    return mw->width() - railWidth - panelWidthFor(mw) >= 360;
}

QFrame *panelHost(KisMainWindow *mw)
{
    QFrame *host = mw->findChild<QFrame*>(PANEL_HOST_ID);
    if (host) return host;
    host = new QFrame(mw);
    host->setObjectName(PANEL_HOST_ID);
    host->setStyleSheet(QLatin1String(PANEL_HOST_STYLE));
    QVBoxLayout *l = new QVBoxLayout(host);
    l->setContentsMargins(1, 1, 1, 1);
    l->setSpacing(0);
    host->hide();
    return host;
}

QDockWidget *hostedDock(KisMainWindow *mw)
{
    QFrame *host = mw->findChild<QFrame*>(PANEL_HOST_ID);
    if (!host) return nullptr;
    Q_FOREACH (QDockWidget *dw, host->findChildren<QDockWidget*>(QString(), Qt::FindDirectChildrenOnly)) {
        return dw;
    }
    return nullptr;
}

void placePanelHost(KisMainWindow *mw)
{
    QFrame *host = mw->findChild<QFrame*>(PANEL_HOST_ID);
    QWidget *central = mw->centralWidget();
    QDockWidget *dw = hostedDock(mw);
    if (!host || !central || !dw) return;
    const QRect area(central->mapTo(mw, QPoint(0, 0)), central->size());
    const int margin = 12;
    QWidget *anchor = s_popupAnchor;
    const bool anchored = anchor && anchor->isVisible() && anchor->window() == mw
            && anchor->objectName() != QLatin1String("nanquimSwatch");
    if (anchored) {
        // pop-up preso ao botão, do tamanho do conteúdo (até caber na tela)
        const QSize want = popupContentSize(dw);
        const int w = qMin(qMax(qMax(280, panelWidthFor(mw)), want.width()), area.width() - 2 * margin);
        const int h = qMin(qMax(360, want.height()), area.height() - 2 * margin);
        const QRect a(anchor->mapTo(mw, QPoint(0, 0)), anchor->size());
        int x, y;
        const QString pop = anchor->property("nanquimPopup").toString();
        if (pop == QLatin1String("below")) {
            // cápsula do topo do estúdio: abre logo abaixo, alinhado à direita do botão
            x = a.right() - w + 1;
            y = a.bottom() + 10;
        } else if (!pop.isEmpty()) {
            // coluna ("rail") abre para dentro da página; a faixa de camadas ("strip") também
            const bool toLeft = pop == QLatin1String("rail") ? railOnRight() : !railOnRight();
            QWidget *piece = anchor->parentWidget() ? anchor->parentWidget() : anchor;
            const QRect p(piece->mapTo(mw, QPoint(0, 0)), piece->size());
            x = toLeft ? p.left() - 12 - w : p.right() + 13;
            y = p.top();
        } else if (a.bottom() <= area.top() + 4) {
            // botão na barra do topo: abre logo abaixo, centrado no botão
            x = a.center().x() - w / 2;
            y = area.top() + 8;
        } else if (a.right() <= area.left() + 4) {
            x = area.left() + 8;
            y = a.top();
        } else if (a.left() >= area.right() - 4) {
            x = area.right() - 8 - w + 1;
            y = a.top();
        } else {
            // botão sobre a página (dock do polegar): abre ao lado
            x = railOnRight() ? a.left() - 12 - w : a.right() + 13;
            y = a.bottom() - h + 1;
        }
        x = qBound(area.left() + margin, x, area.right() - margin - w + 1);
        y = qBound(area.top() + 8, y, area.bottom() - margin - h + 1);
        host->setGeometry(x, y, w, h);
    } else if (isPopoverDock(dw)) {
        // popover de cor ao lado da cor atual, do lado de dentro da página
        const int w = 280;
        const int h = qMin(440, area.height() - 2 * margin);
        int x = railOnRight() ? area.right() - margin - 80 - w : area.left() + margin + 80;
        int bottom = area.bottom() - 16;
        if (QWidget *swatch = mw->findChild<QWidget*>(QStringLiteral("nanquimSwatch"))) {
            if (swatch->isVisible()) {
                const QRect s(swatch->mapTo(mw, QPoint(0, 0)), swatch->size());
                x = railOnRight() ? s.left() - 12 - w : s.right() + 13;
                bottom = s.bottom();
            }
        }
        host->setGeometry(x, qMax(area.top() + margin, bottom - h + 1), w, h);
    } else {
        const QSize want = popupContentSize(dw);
        const int w = qMin(qMax(panelWidthFor(mw), want.width()), area.width() - 2 * margin);
        const int h = qMin(qMax(360, want.height()), area.height() - 2 * margin);
        const int x = railOnRight() ? area.left() + margin : area.right() - margin - w + 1;
        host->setGeometry(x, area.top() + margin, w, h);
    }
    host->raise();
}

// devolve o painel solto para a janela (o Krita guarda e restaura os painéis dela)
void returnToMainWindow(KisMainWindow *mw, QDockWidget *dw)
{
    if (!dw) return;
    QFrame *host = mw->findChild<QFrame*>(PANEL_HOST_ID);
    if (host && dw->parentWidget() == host) {
        host->layout()->removeWidget(dw);
        dw->hide();
        host->hide();
        mw->addDockWidget(panelArea(), dw);
        dw->hide();
    }
}

void dockPinned(KisMainWindow *mw, QDockWidget *dw)
{
    const Qt::DockWidgetArea area = panelArea();
    if (!mw->tabifiedDockWidgets(dw).isEmpty()) {
        mw->removeDockWidget(dw);
        mw->addDockWidget(area, dw);
    }
    if (dw->isFloating() || mw->dockWidgetArea(dw) != area) {
        dw->setFloating(false);
        mw->addDockWidget(area, dw);
    }
    const int width = panelWidthFor(mw);
    dw->setMinimumWidth(width);
    dw->setMaximumWidth(width);
    dw->show();
    dw->raise();
    flushLayouts();
    mw->resizeDocks({dw}, {width}, Qt::Horizontal);
}

class HostContentWatcher : public QObject
{
public:
    HostContentWatcher(KisMainWindow *mw, QObject *parent) : QObject(parent), m_mw(mw) {}
protected:
    bool eventFilter(QObject *, QEvent *e) override
    {
        if (e->type() == QEvent::LayoutRequest && m_mw && !m_pending) {
            m_pending = true;
            QPointer<HostContentWatcher> self(this);
            QTimer::singleShot(0, this, [self]() {
                if (!self) return;
                self->m_pending = false;
                if (!self->m_mw) return;
                QFrame *host = self->m_mw->findChild<QFrame*>(PANEL_HOST_ID);
                if (host && host->isVisible()) placePanelHost(self->m_mw);
            });
        }
        return false;
    }
private:
    QPointer<KisMainWindow> m_mw;
    bool m_pending = false;
};

void showHosted(KisMainWindow *mw, QDockWidget *dw, bool animate)
{
    QFrame *host = panelHost(mw);
    if (!dw->property("nanquimContentWatch").toBool() && dw->widget()) {
        dw->setProperty("nanquimContentWatch", true);
        HostContentWatcher *watch = new HostContentWatcher(mw, dw);
        dw->widget()->installEventFilter(watch);
        Q_FOREACH (QScrollArea *sa, dw->widget()->findChildren<QScrollArea*>()) {
            if (sa->widget()) sa->widget()->installEventFilter(watch);
        }
    }
    if (dw->parentWidget() != host) {
        mw->removeDockWidget(dw);
        dw->setParent(host);
        dw->setMinimumWidth(0);
        dw->setMaximumWidth(QWIDGETSIZE_MAX);
        host->layout()->addWidget(dw);
    }
    dw->show();
    placePanelHost(mw);
    if (!animate) {
        host->show();
        host->raise();
        return;
    }
    // a "foto" do painel desliza para dentro; o painel de verdade aparece no fim
    host->ensurePolished();
    if (host->layout()) host->layout()->activate();
    const QRect geom = host->geometry();
    const QPixmap shot = lightShot(host->grab());
    const int direction = (isPopoverDock(dw) || s_popupAnchor) ? 0 : (railOnRight() ? -1 : +1);
    QPointer<QFrame> guard(host);
    NanquimOverlay *o = new NanquimOverlay(mw, geom, [shot, direction](QPainter &g, const QRect &, qreal p) {
        g.setOpacity(p);
        if (direction == 0) {
            // popover: cresce a partir da cor atual
            const qreal s = 0.94 + 0.06 * p;
            g.translate(0, shot.height() / shot.devicePixelRatio() * (1.0 - s));
            g.scale(s, s);
            g.drawPixmap(QPointF(0, 0), shot);
        } else {
            g.drawPixmap(QPointF(direction * (1.0 - p) * SLIDE, 0), shot);
        }
    });
    runOverlay(s_hostOverlay, o, direction == 0 ? 130 : 160, QEasingCurve::OutQuint, [guard]() {
        if (guard) {
            guard->show();
            guard->raise();
        }
    });
}

bool panelOpen(KisMainWindow *mw, QDockWidget *dw);

void closePanel(KisMainWindow *mw, QDockWidget *dw, bool animate)
{
    if (!panelOpen(mw, dw)) return;
    s_popupAnchor.clear();   // a âncora é do pop-up aberto; fechou, não sobra
    QFrame *host = mw->findChild<QFrame*>(PANEL_HOST_ID);
    const bool hosted = host && dw->parentWidget() == host;
    QWidget *shown = hosted ? static_cast<QWidget*>(host) : static_cast<QWidget*>(dw);
    const QRect geom(shown->mapTo(mw, QPoint(0, 0)), shown->size());
    const QPixmap shot = animate && shown->isVisible() ? lightShot(shown->grab()) : QPixmap();
    if (hosted) {
        returnToMainWindow(mw, dw);
    } else {
        dw->hide();
    }
    if (animate) {
        const int direction = hosted ? 0 : (railOnRight() ? -1 : +1);
        if (direction == 0) {
            NanquimOverlay *o = new NanquimOverlay(mw, geom, [shot](QPainter &g, const QRect &, qreal p) {
                g.setOpacity(1.0 - p);
                g.drawPixmap(QPointF(0, 0), shot);
            });
            runOverlay(s_hostOverlay, o, 105, QEasingCurve::InCubic);
        } else {
            animateOut(hosted ? s_hostOverlay : s_panelOverlay, mw, geom, shot, direction);
        }
    }
}

void setPanelPinned(KisMainWindow *mw, QDockWidget *dw, bool pinned);

// aberto de verdade (o solto fica escondido durante a animação de entrada)
bool panelOpen(KisMainWindow *mw, QDockWidget *dw)
{
    if (!dw) return false;
    QFrame *host = mw->findChild<QFrame*>(PANEL_HOST_ID);
    if (host && dw->parentWidget() == host) return !dw->isHidden();
    return dw->isVisible();
}

// cabeçalho do painel: título, alfinete e fechar (no lugar do título do Krita)
void ensurePanelHeader(KisMainWindow *mw, QDockWidget *dw)
{
    if (!dw || isPopoverDock(dw) || dw->property("nanquimHeader").toBool()) return;
    dw->setProperty("nanquimHeader", true);
    QWidget *header = new QWidget(dw);
    header->setObjectName(QStringLiteral("nanquimPanelHeader"));
    header->setAttribute(Qt::WA_StyledBackground);
    header->setStyleSheet(QLatin1String(PANEL_HOST_STYLE));
    QHBoxLayout *l = new QHBoxLayout(header);
    l->setContentsMargins(16, 6, 6, 6);
    l->setSpacing(2);
    QLabel *title = new QLabel(dw->windowTitle(), header);
    l->addWidget(title, 1);
    QObject::connect(dw, &QDockWidget::windowTitleChanged, title, &QLabel::setText);
    QToolButton *pin = iconButton(header, i18n("Fixar painel"), designIcon(SVG_PIN, 22), true);
    pin->setObjectName(QStringLiteral("nanquimPin"));
    pin->setChecked(isPinned(dw));
    QToolButton *close = iconButton(header, i18n("Fechar"), designIcon(SVG_CLOSE, 22));
    pin->setVisible(false);   // build 60: pop-ups não se fixam
    l->addWidget(pin);
    l->addWidget(close);
    QPointer<QDockWidget> guard(dw);
    QObject::connect(pin, &QToolButton::clicked, mw, [mw, guard](bool on) {
        if (guard) setPanelPinned(mw, guard, on);
    });
    QObject::connect(close, &QToolButton::clicked, mw, [mw, guard]() {
        if (guard) closePanel(mw, guard, true);
    });
    dw->setTitleBarWidget(header);
}

void syncPinButton(KisMainWindow *mw, QDockWidget *dw)
{
    if (QToolButton *pin = dw->findChild<QToolButton*>(QStringLiteral("nanquimPin"))) {
        QSignalBlocker block(pin);
        pin->setChecked(isPinned(dw));
        pin->setEnabled(isPinned(dw) || canPin(mw));
        pin->setToolTip(pin->isEnabled() ? i18n("Fixar painel") : i18n("Sem espaço para fixar nesta largura"));
    }
}

void setPanelPinned(KisMainWindow *mw, QDockWidget *dw, bool pinned)
{
    if (pinned && !canPin(mw)) pinned = false;
    storePinned(dw, pinned);
    if (pinned) {
        returnToMainWindow(mw, dw);
        dockPinned(mw, dw);
    } else {
        dw->setMinimumWidth(0);
        dw->setMaximumWidth(QWIDGETSIZE_MAX);
        showHosted(mw, dw, false);
    }
    syncPinButton(mw, dw);
}

// Toca num painel: abre do lado oposto ao da barra (um de cada vez) ou fecha.
void togglePanel(KisMainWindow *mw, QDockWidget *dw)
{
    if (!dw) {
        return;
    }
    if (panelOpen(mw, dw)) {
        closePanel(mw, dw, true);
        return;
    }
    setBrushPanelVisible(mw, false);
    Q_FOREACH (QDockWidget *other, mw->dockWidgets()) {
        if (other != dw && panelOpen(mw, other)) {
            closePanel(mw, other, false);
        }
    }
    ensurePanelHeader(mw, dw);
    syncPinButton(mw, dw);
    if (isPinned(dw) && canPin(mw)) {
        dockPinned(mw, dw);
        animateIn(s_panelOverlay, mw, dw, railOnRight() ? -1 : +1, dw->palette().color(QPalette::Window));
    } else {
        showHosted(mw, dw, true);
    }
}

// abre (ou fecha) o painel preso ao botão que foi tocado
void togglePanelFrom(KisMainWindow *mw, QDockWidget *dw, QWidget *anchor)
{
    if (dw && !panelOpen(mw, dw)) s_popupAnchor = anchor;
    togglePanel(mw, dw);
}

// Pop-up fecha com um toque fora dele. Na página, o toque só fecha: não vira
// traço nem move a tela. No botão que abriu, o próprio botão fecha.
const QString BRUSH_PANEL_ID_EARLY = QStringLiteral("nanquimBrushPanel");

class PanelCloser : public QObject
{
public:
    explicit PanelCloser(KisMainWindow *mw) : QObject(mw), m_mw(mw) {}
protected:
    bool eventFilter(QObject *, QEvent *e) override
    {
        const QEvent::Type t = e->type();
        // o app dormiu no meio do gesto: não engolir o desenho ao voltar
        if (t == QEvent::ApplicationStateChange) m_eating = false;
        const bool isMouse = t == QEvent::MouseButtonPress || t == QEvent::MouseMove
                || t == QEvent::MouseButtonRelease || t == QEvent::MouseButtonDblClick;
        if (m_eating) {
            // segurança: se a solta se perder (caneta saiu de proximidade, driver
            // engoliu o evento), o "comer" tem que acabar sozinho
            if (t == QEvent::TabletLeaveProximity
                    || (m_eatStart.isValid() && m_eatStart.elapsed() > 2500)) {
                m_eating = false;
            } else if (t == QEvent::TabletMove || t == QEvent::TouchUpdate || t == QEvent::TabletPress
                    || t == QEvent::TabletRelease || t == QEvent::TouchEnd || t == QEvent::TouchCancel
                    || isMouse) {
                e->accept();
                if (t == QEvent::TabletRelease || t == QEvent::TouchEnd || t == QEvent::TouchCancel
                        || t == QEvent::MouseButtonRelease) {
                    m_eating = false;
                    m_grace.start();   // o Qt pode gerar um clique do mouse depois da caneta
                }
                return true;
            }
        } else if (isMouse && m_grace.isValid() && m_grace.elapsed() < 300) {
            e->accept();
            return true;
        }
        if (t != QEvent::TabletPress && t != QEvent::TouchBegin && t != QEvent::MouseButtonPress) {
            return false;
        }
        if (!m_mw || QApplication::activePopupWidget() || QApplication::activeModalWidget()) return false;
        QFrame *host = this->host();
        QFrame *brushes = this->brushes();
        QFrame *sizeBox = this->sizeBox();
        QToolButton *sizeBtn = this->sizeButton();
        const bool sizeOpen = sizeBox && sizeBtn && sizeBox->isVisible()
                && sizeBox->property("nanquimOpen").toBool();
        // só conta pop-up ABERTO: os painéis existem sempre, escondidos; sem este
        // teste todo toque na página era engolido (build 69: nada abria)
        const bool hostOpen = host && host->isVisible();
        const bool brushesOpen = brushes && brushes->isVisible();
        if (!hostOpen && !brushesOpen && !sizeOpen) return false;

        QPoint global;
        if (t == QEvent::TabletPress) {
            global = static_cast<QTabletEvent*>(e)->globalPos();
        } else if (t == QEvent::MouseButtonPress) {
            global = static_cast<QMouseEvent*>(e)->globalPos();
        } else {
            const auto points = static_cast<QTouchEvent*>(e)->touchPoints();
            if (points.isEmpty()) return false;
            global = points.first().screenPos().toPoint();
        }
        const auto containsW = [](const QWidget *w, const QPoint &g) {
            return w && QRect(w->mapToGlobal(QPoint(0, 0)), w->size()).contains(g);
        };
        // toque dentro de um dos pop-ups abertos: nenhum fecha por aqui
        if ((hostOpen && containsW(host, global)) || (brushesOpen && containsW(brushes, global))
                || (sizeOpen && containsW(sizeBox, global))) {
            return false;
        }
        if (s_popupAnchor && s_popupAnchor->isVisible() && containsW(s_popupAnchor.data(), global)) {
            return false;   // o botão que abriu fecha sozinho
        }
        QWidget *under = qApp->widgetAt(global);
        if (under && under->window() != m_mw) return false;   // menus e janelas do próprio painel
        // fecha todos os de uma vez (um toque, tudo some): coluna de tamanho,
        // janela de pincéis e pop-up hospedeiro
        if (sizeOpen) sizeBtn->click();
        if (brushesOpen) setBrushPanelVisible(m_mw, false);
        if (hostOpen) {
            if (QDockWidget *dw = hostedDock(m_mw)) closePanel(m_mw, dw, true);
        }
        if (!onPage(global)) return false;   // outro botão: fecha e deixa o toque seguir
        if (t == QEvent::TabletPress && nanquimPenDevice(static_cast<QTabletEvent*>(e))) {
            return false;   // caneta desenha: fechar não rouba o traço
        }
        m_eating = true;
        m_eatStart.start();
        m_grace.invalidate();
        e->accept();
        return true;
    }
private:
    bool onPage(const QPoint &global) const
    {
        QWidget *central = m_mw->centralWidget();
        if (!central) return false;
        const QRect page(central->mapToGlobal(QPoint(0, 0)), central->size());
        if (!page.contains(global)) return false;
        for (const char *id : {"nanquimStudioTitle", "nanquimStudioTools", "nanquimStudioRail",
                               "nanquimStudioLayers", "nanquimStudioPages",
                               "nanquimThumbDock", "nanquimSwatch", "nanquimExpandTab", "nanquimToolsFlyout",
                               "nanquimQuickColors", "nanquimFocusBar", "nanquimFocusExit",
                               "nanquimFocusSize", "nanquimFocusSizeButton", "nanquimSelectionBar"}) {
            if (QWidget *w = m_mw->findChild<QWidget*>(QLatin1String(id))) {
                if (w->isVisible() && QRect(w->mapToGlobal(QPoint(0, 0)), w->size()).contains(global)) return false;
            }
        }
        return true;
    }
    // os pop-ups vivem enquanto a janela: procurados uma vez (o QPointer
    // se esvazia sozinho se um deles morrer), não a cada toque na tela
    QFrame *host()
    {
        if (m_mw && !m_host) m_host = m_mw->findChild<QFrame*>(PANEL_HOST_ID);
        return m_host;
    }
    QFrame *brushes()
    {
        if (m_mw && !m_brushes) m_brushes = m_mw->findChild<QFrame*>(BRUSH_PANEL_ID_EARLY);
        return m_brushes;
    }
    QFrame *sizeBox()
    {
        if (m_mw && !m_sizeBox) m_sizeBox = m_mw->findChild<QFrame*>(QStringLiteral("nanquimFocusSize"));
        return m_sizeBox;
    }
    QToolButton *sizeButton()
    {
        if (m_mw && !m_sizeBtn) m_sizeBtn = m_mw->findChild<QToolButton*>(QStringLiteral("nanquimFocusSizeButton"));
        return m_sizeBtn;
    }
    QPointer<KisMainWindow> m_mw;
    QPointer<QFrame> m_host;
    QPointer<QFrame> m_brushes;
    QPointer<QFrame> m_sizeBox;
    QPointer<QToolButton> m_sizeBtn;
    bool m_eating = false;
    QElapsedTimer m_grace;
    QElapsedTimer m_eatStart;
};

// segue a área de desenho (rotação, janela dividida, barra recolhida)
class PanelHostWatcher : public QObject
{
public:
    explicit PanelHostWatcher(KisMainWindow *mw) : QObject(mw), m_mw(mw) {}
protected:
    bool eventFilter(QObject *o, QEvent *e) override
    {
        if (m_mw && o == m_mw && (e->type() == QEvent::Resize || e->type() == QEvent::LayoutRequest)) {
            QPointer<KisMainWindow> guard(m_mw);
            QTimer::singleShot(0, m_mw, [guard]() {
                if (!guard) return;
                QFrame *host = guard->findChild<QFrame*>(PANEL_HOST_ID);
                if (host && host->isVisible()) placePanelHost(guard);
                // fixado sem espaço (janela estreitou): volta a ser solto
                Q_FOREACH (QDockWidget *dw, guard->dockWidgets()) {
                    if (dw->isVisible() && isPinned(dw) && dw->parentWidget() == guard && !canPin(guard)
                            && dw->property("nanquimHeader").toBool()) {
                        dw->setMinimumWidth(0);
                        dw->setMaximumWidth(QWIDGETSIZE_MAX);
                        showHosted(guard, dw, false);
                    }
                }
            });
        }
        return false;
    }
private:
    QPointer<KisMainWindow> m_mw;
};

// ---------------------------------------------------------------------------
// Pairar da S Pen (refeito na build 60.5). Antes o destaque só valia em algumas
// barras, aparecia sem transição (re-polish do estilo a cada troca) e podia
// ficar preso: no Android o Qt nem sempre entrega TabletLeaveProximity quando a
// caneta se afasta. Agora:
//  - um "halo" leve desenhado por cima do botão aparece e some com fade;
//  - o ícone cresce 12% com animação (e afunda 12% no toque);
//  - vale para qualquer botão do Nanquim, inclusive pop-ups e a tela inicial;
//  - sem evento de pairar por 800 ms, o destaque some sozinho.
// ---------------------------------------------------------------------------
static bool isCoralHoverTarget(const QWidget *w)
{
    if (!w) return false;
    const QString name = w->objectName();
    if (name == QLatin1String("nanquimFocusEraser") || name == QLatin1String("nanquimSpringEraser")) return true;
    const QString tip = w->toolTip();
    return tip.contains(QLatin1String("Borracha"), Qt::CaseInsensitive)
           || tip.contains(QLatin1String("Excluir"), Qt::CaseInsensitive)
           || tip.contains(QLatin1String("Remover"), Qt::CaseInsensitive);
}

class HoverHalo : public QWidget
{
public:
    explicit HoverHalo(QWidget *parent) : QWidget(parent)
    {
        setAttribute(Qt::WA_TransparentForMouseEvents);
        setAttribute(Qt::WA_NoSystemBackground);
        setFocusPolicy(Qt::NoFocus);
        m_anim = new QVariantAnimation(this);
        m_anim->setEasingCurve(QEasingCurve::OutCubic);
        QObject::connect(m_anim, &QVariantAnimation::valueChanged, this, [this](const QVariant &v) {
            m_level = v.toReal();
            update();
        });
        QObject::connect(m_anim, &QVariantAnimation::finished, this, [this]() { if (m_level <= 0.0) hide(); });

        m_slide = new QVariantAnimation(this);
        m_slide->setDuration(135);
        m_slide->setStartValue(0.0);
        m_slide->setEndValue(1.0);
        m_slide->setEasingCurve(QEasingCurve::OutCubic);
        QObject::connect(m_slide, &QVariantAnimation::valueChanged, this, [this](const QVariant &v) {
            const qreal t = v.toReal();
            const int x = qRound(m_fromRect.x() + (m_toRect.x() - m_fromRect.x()) * t);
            const int y = qRound(m_fromRect.y() + (m_toRect.y() - m_fromRect.y()) * t);
            const int w = qRound(m_fromRect.width() + (m_toRect.width() - m_fromRect.width()) * t);
            const int h = qRound(m_fromRect.height() + (m_toRect.height() - m_fromRect.height()) * t);
            setGeometry(x, y, qMax(4, w), qMax(4, h));
            update();
        });
        hide();
    }
    static QRect targetRectInParent(QWidget *target, QWidget *parent, qreal *radiusOut = nullptr)
    {
        QRect r(target->mapTo(parent, QPoint(0, 0)), target->size());
        qreal rad = qMin<qreal>(13.0, r.height() / 2.0);
        if (target->objectName() == QLatin1String("nanquimContinueCard")) {
            r.adjust(0, 0, 0, -24);
            rad = 24.0;
        }
        if (radiusOut) *radiusOut = rad;
        return r;
    }
    void showOnRect(const QRect &r, qreal radius, bool coral)
    {
        if (!parentWidget() || r.isEmpty()) return;
        m_radius = radius;
        m_coral = coral;
        const bool wasVisible = isVisible() && m_level > 0.15;
        if (wasVisible && geometry().isValid() && geometry() != r) {
            // deslize magnético entre botões ou cartões vizinhos sem piscar
            m_fromRect = geometry();
            m_toRect = r;
            m_slide->stop();
            m_slide->start();
        } else {
            m_slide->stop();
            m_fromRect = r;
            m_toRect = r;
            setGeometry(r);
        }
        show();
        raise();
        fadeTo(1.0, 115);
    }
    void showOn(QWidget *target)
    {
        if (!target || !parentWidget()) return;
        qreal rad = 13.0;
        const QRect r = targetRectInParent(target, parentWidget(), &rad);
        showOnRect(r, rad, isCoralHoverTarget(target));
    }
    void followRect(const QRect &r)
    {
        if (!isVisible() || !parentWidget() || r.isEmpty()) return;
        if (m_slide->state() == QAbstractAnimation::Running) return;
        if (r != geometry()) {
            m_toRect = r;
            setGeometry(r);
        }
    }
    void follow(QWidget *target)
    {
        if (!isVisible() || !target || !parentWidget()) return;
        followRect(targetRectInParent(target, parentWidget()));
    }
    void fadeOut() { if (isVisible()) fadeTo(0.0, 140); }
protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter g(this);
        g.setRenderHint(QPainter::Antialiasing);
        const QRectF box = QRectF(rect()).adjusted(1.0, 1.0, -1.0, -1.0);
        // superfície lisa sem degradê entre matizes (CORES.md): verdete claro ou coral
        const QColor accent = m_coral ? Cor::CORAL : Cor::CIANO;
        g.setBrush(QColor(accent.red(), accent.green(), accent.blue(), qRound(36 * m_level)));
        g.setPen(QPen(QColor(accent.red(), accent.green(), accent.blue(), qRound(165 * m_level)), 1.2));
        g.drawRoundedRect(box, m_radius, m_radius);
    }
private:
    void fadeTo(qreal to, int ms)
    {
        m_anim->stop();
        m_anim->setStartValue(m_level);
        m_anim->setEndValue(to);
        m_anim->setDuration(ms);
        m_anim->start();
    }
    QVariantAnimation *m_anim = nullptr;
    QVariantAnimation *m_slide = nullptr;
    QRect m_fromRect;
    QRect m_toRect;
    qreal m_level = 0.0;
    qreal m_radius = 12.0;
    bool m_coral = false;
};

// pílula flutuante animada ao pairar a S Pen (substitui o QToolTip estático)
class HoverPill : public QWidget
{
public:
    explicit HoverPill(QWidget *parent) : QWidget(parent)
    {
        setAttribute(Qt::WA_TransparentForMouseEvents);
        setAttribute(Qt::WA_NoSystemBackground);
        setFocusPolicy(Qt::NoFocus);
        m_anim = new QVariantAnimation(this);
        QObject::connect(m_anim, &QVariantAnimation::valueChanged, this, [this](const QVariant &v) {
            m_t = v.toReal();
            const int dx = qRound((1.0 - m_t) * m_slideX);
            const int dy = qRound((1.0 - m_t) * m_slideY);
            move(m_targetPos + QPoint(dx, dy));
            update();
        });
        QObject::connect(m_anim, &QVariantAnimation::finished, this, [this]() {
            if (m_t <= 0.0) hide();
        });
        hide();
    }
    void showAtRect(const QRect &r, const QString &text, const QColor &dotColor, bool onLeftRail, bool onRightRail)
    {
        if (!parentWidget() || r.isEmpty() || text.isEmpty()) return;
        m_text = text;
        m_dot = dotColor.isValid() ? dotColor : Cor::CIANO;
        QFont f = font();
        f.setPixelSize(12);
        f.setWeight(QFont::DemiBold);
        setFont(f);
        const int w = qMin(parentWidget()->width() - 24, QFontMetrics(f).horizontalAdvance(m_text) + 28);
        const int h = 28;
        resize(w, h);

        const QRect bounds = parentWidget()->rect().adjusted(8, 8, -8, -8);
        int x = r.center().x() - w / 2;
        int y = r.bottom() + 8;
        m_slideX = 0;
        m_slideY = -6;
        if (onLeftRail) {
            x = r.right() + 10;
            y = r.center().y() - h / 2;
            m_slideX = -8;
            m_slideY = 0;
        } else if (onRightRail) {
            x = r.left() - 10 - w;
            y = r.center().y() - h / 2;
            m_slideX = 8;
            m_slideY = 0;
        } else if (y + h > bounds.bottom()) {
            y = r.top() - 8 - h;
            m_slideY = 6;
        }
        x = qBound(bounds.left(), x, qMax(bounds.left(), bounds.right() - w + 1));
        y = qBound(bounds.top(), y, qMax(bounds.top(), bounds.bottom() - h + 1));
        m_targetPos = QPoint(x, y);

        m_anim->stop();
        m_t = 0.0;
        move(m_targetPos + QPoint(m_slideX, m_slideY));
        show();
        raise();
        m_anim->setDuration(170);
        m_anim->setStartValue(0.0);
        m_anim->setEndValue(1.0);
        m_anim->setEasingCurve(softSpring());
        m_anim->start();
    }
    void showFor(QWidget *target, const QString &text, bool onLeftRail, bool onRightRail)
    {
        if (!target || !parentWidget() || text.isEmpty()) return;
        const QRect r = HoverHalo::targetRectInParent(target, parentWidget());
        const QColor dot = isCoralHoverTarget(target) ? Cor::CORAL : Cor::CIANO;
        showAtRect(r, text, dot, onLeftRail, onRightRail);
    }
    void fadeOut()
    {
        if (!isVisible()) return;
        m_anim->stop();
        m_anim->setDuration(110);
        m_anim->setStartValue(m_t);
        m_anim->setEndValue(0.0);
        m_anim->setEasingCurve(QEasingCurve::OutCubic);
        m_anim->start();
    }
protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter g(this);
        g.setRenderHint(QPainter::Antialiasing);
        g.setOpacity(qBound(0.0, m_t, 1.0));
        const QRectF box = QRectF(rect()).adjusted(1.0, 1.0, -1.0, -1.0);
        g.setBrush(QColor(0x2e, 0x2d, 0x29, 245));
        g.setPen(QPen(QColor(0x5a, 0x57, 0x4f, 235), 1.0));
        g.drawRoundedRect(box, 13.0, 13.0);

        g.setPen(Qt::NoPen);
        g.setBrush(m_dot);
        g.drawEllipse(QPointF(box.left() + 10.0, box.center().y()), 3.0, 3.0);

        g.setPen(Cor::TEXTO);
        g.setFont(font());
        const QRectF tr = box.adjusted(18.0, 0.0, -8.0, 0.0);
        g.drawText(tr, Qt::AlignVCenter | Qt::AlignLeft,
                   QFontMetrics(font()).elidedText(m_text, Qt::ElideRight, qRound(tr.width())));
    }
private:
    QVariantAnimation *m_anim = nullptr;
    QString m_text;
    QColor m_dot = Cor::CIANO;
    QPoint m_targetPos;
    qreal m_t = 0.0;
    int m_slideX = 0;
    int m_slideY = 0;
};

// anima o tamanho do ícone de um botão sem mexer no tamanho do botão
void animateIconScale(QToolButton *b, qreal factor, int ms, const QEasingCurve &curve)
{
    if (!b || b->icon().isNull()) return;
    if (!b->property("nanquimIconBase").isValid()) {
        if (qFuzzyCompare(factor, 1.0)) return;
        b->setProperty("nanquimIconBase", b->iconSize());
        b->setProperty("nanquimIconScale", 1.0);
        if (b->minimumSize() != b->maximumSize()) {
            b->setProperty("nanquimHoverMin", b->minimumSize());
            b->setProperty("nanquimHoverMax", b->maximumSize());
            b->setFixedSize(b->size());
            b->setProperty("nanquimHoverFixed", true);
        }
    }
    QVariantAnimation *a = b->findChild<QVariantAnimation*>(QStringLiteral("nanquimIconAnim"), Qt::FindDirectChildrenOnly);
    if (!a) {
        a = new QVariantAnimation(b);
        a->setObjectName(QStringLiteral("nanquimIconAnim"));
        QPointer<QToolButton> guard(b);
        QObject::connect(a, &QVariantAnimation::valueChanged, b, [guard](const QVariant &v) {
            if (!guard) return;
            const QSize base = guard->property("nanquimIconBase").toSize();
            if (!base.isValid()) return;
            const qreal f = v.toReal();
            guard->setProperty("nanquimIconScale", f);
            guard->setIconSize(QSize(qRound(base.width() * f), qRound(base.height() * f)));
        });
        QObject::connect(a, &QVariantAnimation::finished, b, [guard]() {
            if (!guard || !qFuzzyCompare(guard->property("nanquimIconScale").toReal(), 1.0)) return;
            const QSize base = guard->property("nanquimIconBase").toSize();
            if (base.isValid()) guard->setIconSize(base);
            if (guard->property("nanquimHoverFixed").toBool()) {
                guard->setMinimumSize(guard->property("nanquimHoverMin").toSize());
                guard->setMaximumSize(guard->property("nanquimHoverMax").toSize());
                guard->setProperty("nanquimHoverFixed", false);
            }
            guard->setProperty("nanquimIconBase", QVariant());
        });
    }
    a->stop();
    a->setStartValue(b->property("nanquimIconScale").toReal());
    a->setEndValue(factor);
    a->setDuration(ms);
    a->setEasingCurve(curve);
    a->start();
}

// onda luminosa curta ao tocar num botão (feedback tátil imediato)
class TouchRipple : public QWidget
{
public:
    explicit TouchRipple(QWidget *parent) : QWidget(parent)
    {
        setAttribute(Qt::WA_TransparentForMouseEvents);
        setAttribute(Qt::WA_NoSystemBackground);
        setFocusPolicy(Qt::NoFocus);
        m_anim = new QVariantAnimation(this);
        m_anim->setDuration(260);
        m_anim->setStartValue(0.0);
        m_anim->setEndValue(1.0);
        m_anim->setEasingCurve(QEasingCurve::OutCubic);
        QObject::connect(m_anim, &QVariantAnimation::valueChanged, this, [this](const QVariant &v) {
            m_t = v.toReal();
            update();
        });
        QObject::connect(m_anim, &QVariantAnimation::finished, this, &QWidget::hide);
        hide();
    }
    void burstOn(QWidget *target)
    {
        if (!target || !parentWidget()) return;
        const QRect r(target->mapTo(parentWidget(), QPoint(0, 0)), target->size());
        setGeometry(r.adjusted(-6, -6, 6, 6));
        m_coral = isCoralHoverTarget(target);
        m_t = 0.0;
        show();
        raise();
        m_anim->stop();
        m_anim->start();
    }
protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter g(this);
        g.setRenderHint(QPainter::Antialiasing);
        const QPointF c(width() * 0.5, height() * 0.5);
        const qreal maxR = qMin(width(), height()) * 0.48;
        const qreal rad = maxR * (0.35 + 0.65 * m_t);
        const int alpha = qRound((1.0 - m_t) * 150.0);
        const QColor col = m_coral ? QColor(0xff, 0x6b, 0x8b, alpha) : QColor(0x6f, 0xbf, 0xad, alpha);
        g.setBrush(Qt::NoBrush);
        g.setPen(QPen(col, 2.0 * (1.0 - m_t * 0.4)));
        g.drawEllipse(c, rad, rad);
    }
private:
    QVariantAnimation *m_anim = nullptr;
    qreal m_t = 0.0;
    bool m_coral = false;
};

class PenHover : public QObject
{
public:
    explicit PenHover(KisMainWindow *mw) : QObject(mw), m_mw(mw)
    {
        m_tip.setSingleShot(true);
        m_tip.setInterval(260);
        QObject::connect(&m_tip, &QTimer::timeout, this, [this]() {
            if (m_button && !m_button->toolTip().isEmpty()) {
                QWidget *win = m_button->window();
                if (!win) return;
                if (!m_pill || m_pill->parentWidget() != win) {
                    if (m_pill) m_pill->deleteLater();
                    m_pill = new HoverPill(win);
                }
                bool onRail = false;
                if (m_mw) {
                    for (const char *id : {"nanquimTools", "nanquimFocusBar", "nanquimStudioRail"}) {
                        QWidget *w = m_mw->findChild<QWidget*>(QLatin1String(id));
                        if (w && w->isAncestorOf(m_button)) {
                            onRail = true;
                            break;
                        }
                    }
                }
                const bool onLeftRail = onRail && !railOnRight();
                const bool onRightRail = onRail && railOnRight();
                m_pill->showFor(m_button, m_button->toolTip(), onLeftRail, onRightRail);
                return;
            }
            if (m_cardList && m_cardRow >= 0 && m_cardList->model()) {
                const QModelIndex idx = m_cardList->model()->index(m_cardRow, 0);
                if (!idx.isValid()) return;
                QWidget *win = m_cardList->window();
                if (!win) return;
                const QRect cardRect = cardRectInWindow(m_cardList, idx, win);
                if (cardRect.isEmpty()) return;
                QString name = idx.data(Qt::DisplayRole).toString();
                if (name.endsWith(QLatin1String(".kra"), Qt::CaseInsensitive)) name.chop(4);
                if (name.isEmpty()) return;
                const QVariant st = idx.data(Qt::UserRole + 60);
                QColor dot = Cor::CIANO;
                QString text = name;
                if (st.isValid()) {
                    const int s = st.toInt();
                    dot = s == 2 ? Cor::VERDE : s == 1 ? Cor::CIANO : Cor::AMBAR;
                    const QString stLabel = s == 2 ? i18n("Pronta") : s == 1 ? i18n("Arte-final") : i18n("Rascunho");
                    text = QStringLiteral("%1 · %2").arg(name, stLabel);
                }
                if (!m_pill || m_pill->parentWidget() != win) {
                    if (m_pill) m_pill->deleteLater();
                    m_pill = new HoverPill(win);
                }
                m_pill->showAtRect(cardRect, text, dot, false, false);
            }
        });
        // caneta afastada sem aviso do sistema: some sozinho
        m_idle.setSingleShot(true);
        m_idle.setInterval(800);
        QObject::connect(&m_idle, &QTimer::timeout, this, [this]() { clear(); });
    }
protected:
    bool eventFilter(QObject *, QEvent *e) override
    {
        const QEvent::Type t = e->type();
        // toque: o ícone afunda e volta (vale também para o dedo)
        if (t == QEvent::TabletPress || t == QEvent::MouseButtonPress || t == QEvent::TouchBegin) {
            clear();
            if (QToolButton *b = buttonAt(pressPos(e))) {
                if (iconGrows(b) && b != m_pressed) {   // o Qt repete o toque como mouse
                    m_pressed = b;
                    animateIconScale(b, 0.86, 80, QEasingCurve(QEasingCurve::OutQuad));
                    QWidget *win = b->window();
                    if (!m_ripple || m_ripple->parentWidget() != win) {
                        if (m_ripple) m_ripple->deleteLater();
                        m_ripple = new TouchRipple(win);
                    }
                    m_ripple->burstOn(b);
                }
            }
            return false;
        }
        if (t == QEvent::TabletRelease || t == QEvent::MouseButtonRelease || t == QEvent::TouchEnd
                || t == QEvent::TouchCancel) {
            if (m_pressed) animateIconScale(m_pressed, 1.0, 240, softSpring());
            m_pressed = nullptr;
            return false;
        }
        if (t == QEvent::TabletLeaveProximity || t == QEvent::ApplicationStateChange
                || t == QEvent::ApplicationDeactivate) {
            clear();
            return false;
        }
        QPoint global;
        if (t == QEvent::TabletMove) {
            QTabletEvent *te = static_cast<QTabletEvent*>(e);
            if (te->buttons() != Qt::NoButton || te->pressure() > 0) return false;
            global = te->globalPos();
        } else if (t == QEvent::MouseMove) {
            QMouseEvent *me = static_cast<QMouseEvent*>(e);
            if (me->buttons() != Qt::NoButton) return false;
            global = me->globalPos();
        } else {
            return false;
        }
        m_idle.start();
        if (global == m_last) return false;
        m_last = global;
        track(global);
        return false;
    }
private:
    static QPoint pressPos(QEvent *e)
    {
        switch (e->type()) {
        case QEvent::TabletPress: return static_cast<QTabletEvent*>(e)->globalPos();
        case QEvent::MouseButtonPress: return static_cast<QMouseEvent*>(e)->globalPos();
        case QEvent::TouchBegin: {
            const auto pts = static_cast<QTouchEvent*>(e)->touchPoints();
            return pts.isEmpty() ? QPoint(-1, -1) : pts.first().screenPos().toPoint();
        }
        default: return QPoint(-1, -1);
        }
    }

    // o widget pertence ao Nanquim (janela principal ou pop-up aberto a partir dela)
    bool ours(QWidget *w) const
    {
        for (QWidget *p = w; p; p = p->parentWidget()) {
            if (p == m_mw) return true;
        }
        return false;
    }

    static QRect cardRectInWindow(QListView *list, const QModelIndex &idx, QWidget *win)
    {
        if (!list || !idx.isValid() || !win) return QRect();
        QWidget *vp = list->viewport();
        if (!vp) return QRect();
        const QRect vr = list->visualRect(idx);
        if (!vr.isValid()) return QRect();
        const int bottomPad = list->property("nanquimCardBottomPad").toInt();
        const QRect local = vr.adjusted(8, 8, -8, -(bottomPad > 0 ? bottomPad : 58));
        const QRect visible = local.intersected(vp->rect());
        if (visible.width() < 24 || visible.height() < 24) return QRect();
        return QRect(vp->mapTo(win, visible.topLeft()), visible.size());
    }

    QAbstractButton *hoverTarget(const QPoint &global) const
    {
        if (!m_mw) return nullptr;
        QWidget *w = qApp->widgetAt(global);
        if (!w || !ours(w)) return nullptr;
        for (QWidget *p = w; p && p != m_mw; p = p->parentWidget()) {
            if (QAbstractButton *b = qobject_cast<QAbstractButton*>(p)) {
                const bool isContinue = b->objectName() == QLatin1String("nanquimContinueCard");
                if (!b->isEnabled() || (!isContinue && (b->width() > 420 || b->height() > 260))) return nullptr;
                return b;
            }
            if (p->isWindow()) break;
        }
        return nullptr;
    }

    QToolButton *buttonAt(const QPoint &global) const
    {
        return qobject_cast<QToolButton*>(hoverTarget(global));
    }

    static bool iconGrows(QToolButton *b)
    {
        if (!b || b->icon().isNull()) return false;
        const QString n = b->objectName();
        return n != QLatin1String("nanquimSwatch") && !(b->parentWidget()
                && b->parentWidget()->objectName() == QLatin1String("nanquimQuickColors"));
    }

    void track(const QPoint &global)
    {
        if (!m_mw) return;
        // Caminho rapido: se a S Pen esta no miolo da folha de desenho e nao ha
        // pop-ups abertos, evita chamar qApp->widgetAt() a 240 Hz.
        if (m_mw->property("nanquimOnCanvas").toBool()) {
            if (QWidget *central = m_mw->centralWidget()) {
                if (central->isVisible() && m_mw->activeView()) {
                    const QRect safeCanvas = QRect(central->mapToGlobal(QPoint(0, 0)), central->size())
                                                 .adjusted(96, 64, -96, -96);
                    if (safeCanvas.contains(global) && !s_popupAnchor && !m_button && !m_list && !m_cardList) {
                        return;
                    }
                }
            }
        }
        QAbstractButton *b = hoverTarget(global);
        if (b != m_button) setButton(b);
        else if (m_halo && b) m_halo->follow(b);   // pop-ups que ainda estão animando

        // linha da lista de pincéis ou cartão de desenho na galeria/capítulo
        QListView *rowList = nullptr;
        QListView *cardList = nullptr;
        if (!b) {
            QWidget *w = qApp->widgetAt(global);
            for (QWidget *p = w; p && ours(p) && p != m_mw; p = p->parentWidget()) {
                if (QListView *lv = qobject_cast<QListView*>(p)) {
                    if (lv->property("nanquimHoverRows").toBool()) rowList = lv;
                    else if (lv->property("nanquimHoverCards").toBool()) cardList = lv;
                    break;
                }
            }
        }
        int row = -1;
        if (rowList) row = rowList->indexAt(rowList->viewport()->mapFromGlobal(global)).row();
        if (rowList != m_list || row != m_row) {
            if (m_list && m_list != rowList) {
                m_list->setProperty("nanquimHoverRow", -1);
                m_list->viewport()->update();
            }
            m_list = rowList;
            m_row = row;
            if (m_list) {
                m_list->setProperty("nanquimHoverRow", row);
                m_list->viewport()->update();
            }
        }

        int cardRow = -1;
        if (cardList) {
            const QModelIndex idx = cardList->indexAt(cardList->viewport()->mapFromGlobal(global));
            if (idx.isValid()) cardRow = idx.row();
        }
        if (cardList != m_cardList || cardRow != m_cardRow) {
            setCard(cardList, cardRow);
        } else if (m_cardList && m_cardRow >= 0 && m_halo && m_cardList->model()) {
            const QModelIndex idx = m_cardList->model()->index(m_cardRow, 0);
            const QRect r = cardRectInWindow(m_cardList, idx, m_cardList->window());
            if (!r.isEmpty()) m_halo->followRect(r);
        }
    }

    void setCard(QListView *list, int row)
    {
        if (m_button) {
            m_cardList = nullptr;
            m_cardRow = -1;
            return;
        }
        m_tip.stop();
        if (m_pill) m_pill->fadeOut();
        m_cardList = (list && row >= 0) ? list : nullptr;
        m_cardRow = m_cardList ? row : -1;
        if (!m_cardList || !m_cardList->model()) {
            if (m_halo) m_halo->fadeOut();
            return;
        }
        const QModelIndex idx = m_cardList->model()->index(m_cardRow, 0);
        QWidget *win = m_cardList->window();
        const QRect r = cardRectInWindow(m_cardList, idx, win);
        if (r.isEmpty() || !win) {
            m_cardList = nullptr;
            m_cardRow = -1;
            if (m_halo) m_halo->fadeOut();
            return;
        }
        if (!m_halo || m_halo->parentWidget() != win) {
            if (m_halo) m_halo->deleteLater();
            m_halo = new HoverHalo(win);
        }
        m_halo->showOnRect(r, 14.0, false);
        m_tip.start();
    }

    void setButton(QAbstractButton *b)
    {
        m_tip.stop();
        QToolTip::hideText();
        if (m_pill) m_pill->fadeOut();
        if (b) {
            m_cardList = nullptr;
            m_cardRow = -1;
        }
        if (m_button) {
            if (QToolButton *tb = qobject_cast<QToolButton*>(m_button.data())) {
                if (iconGrows(tb)) animateIconScale(tb, 1.0, 160, QEasingCurve(QEasingCurve::OutCubic));
            }
        }
        m_button = b;
        if (!b) {
            if (m_halo && !m_cardList) m_halo->fadeOut();
            return;
        }
        QWidget *win = b->window();
        if (!m_halo || m_halo->parentWidget() != win) {
            if (m_halo) m_halo->deleteLater();
            m_halo = new HoverHalo(win);
        }
        m_halo->showOn(b);
        if (QToolButton *tb = qobject_cast<QToolButton*>(b)) {
            if (iconGrows(tb)) animateIconScale(tb, 1.15, 145, softSpring());
        }
        m_tip.start();
    }

    void clear()
    {
        m_last = QPoint();
        m_idle.stop();
        if (m_button) setButton(nullptr);
        m_cardList = nullptr;
        m_cardRow = -1;
        m_tip.stop();
        QToolTip::hideText();
        if (m_pill) m_pill->fadeOut();
        if (m_halo) m_halo->fadeOut();
        if (m_list) {
            m_list->setProperty("nanquimHoverRow", -1);
            m_list->viewport()->update();
            m_list = nullptr;
            m_row = -1;
        }
    }

    QPointer<KisMainWindow> m_mw;
    QPointer<QAbstractButton> m_button;
    QPointer<QToolButton> m_pressed;
    QPointer<HoverHalo> m_halo;
    QPointer<HoverPill> m_pill;
    QPointer<TouchRipple> m_ripple;
    QPointer<QListView> m_list;
    int m_row = -1;
    QPointer<QListView> m_cardList;
    int m_cardRow = -1;
    QPoint m_last;
    QTimer m_tip;
    QTimer m_idle;
};

// ---------------------------------------------------------------------------
// Rolagem suave em todo o aplicativo (build 60.5). O Krita já liga a rolagem
// cinética em parte das listas, mas com valores que deixam o movimento duro:
// a velocidade do arraste não é suavizada (o "peteleco" sai irregular), o
// toque espera até ~0,5 s para chegar ao botão e a freada é curta. Aqui toda
// área rolável do app ganha o mesmo ajuste ao aparecer. Ficam de fora a página
// de desenho e campos de texto editáveis; listas com arrastar-e-soltar só são
// ajustadas se o próprio Krita já tiver ligado a rolagem nelas.
// ---------------------------------------------------------------------------
void tuneSmoothScroller(QScroller *scroller)
{
    if (!scroller) return;
    QScrollerProperties p = scroller->scrollerProperties();
    // build 74: o arraste acompanha mais o dedo, o peteleco vai mais longe e
    // freia antes, e o "elástico" no fim da lista é curto (antes a rolagem
    // parecia lenta e ficava balançando no fim)
    p.setScrollMetric(QScrollerProperties::DragVelocitySmoothingFactor, 0.55);
    p.setScrollMetric(QScrollerProperties::MousePressEventDelay, 0.12);
    p.setScrollMetric(QScrollerProperties::DecelerationFactor, 0.18);
    p.setScrollMetric(QScrollerProperties::ScrollingCurve, QEasingCurve(QEasingCurve::OutCubic));
    p.setScrollMetric(QScrollerProperties::MaximumVelocity, 1.6);
    p.setScrollMetric(QScrollerProperties::FrameRate, QScrollerProperties::Fps60);
    p.setScrollMetric(QScrollerProperties::OvershootDragResistanceFactor, 0.3);
    p.setScrollMetric(QScrollerProperties::OvershootDragDistanceFactor, 0.08);
    p.setScrollMetric(QScrollerProperties::OvershootScrollDistanceFactor, 0.04);
    p.setScrollMetric(QScrollerProperties::OvershootScrollTime, 0.2);
    p.setScrollMetric(QScrollerProperties::SnapTime, 0.2);
    scroller->setScrollerProperties(p);
}

class SmoothScroll : public QObject
{
public:
    explicit SmoothScroll(QObject *parent) : QObject(parent) {}
protected:
    bool eventFilter(QObject *o, QEvent *e) override
    {
        if (e->type() != QEvent::Show) return false;
        QAbstractScrollArea *area = qobject_cast<QAbstractScrollArea*>(o);
        if (!area || area->property("nanquimSmooth").isValid()) return false;
        area->setProperty("nanquimSmooth", true);
        const QString cls = QString::fromLatin1(area->metaObject()->className());
        if (cls.contains(QLatin1String("Canvas"))) return false;   // a página de desenho rola do jeito dela
        if (QTextEdit *t = qobject_cast<QTextEdit*>(area)) { if (!t->isReadOnly()) return false; }
        if (QPlainTextEdit *t = qobject_cast<QPlainTextEdit*>(area)) { if (!t->isReadOnly()) return false; }
        QAbstractItemView *view = qobject_cast<QAbstractItemView*>(area);
        const bool had = QScroller::hasScroller(area->viewport()) || QScroller::hasScroller(area);
        if (!had && view && view->dragEnabled()) return false;   // não roubar o arrastar-e-soltar
        if (view) {
            view->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
            view->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
        }
        // barra e rodinha também andam em passos menores
        area->verticalScrollBar()->setSingleStep(qMax(1, qMin(area->verticalScrollBar()->singleStep(), 24)));
        QScroller *scroller = had ? QScroller::scroller(QScroller::hasScroller(area->viewport()) ? static_cast<QObject*>(area->viewport()) : area)
                                  : KisKineticScroller::createPreconfiguredScroller(area);
        tuneSmoothScroller(scroller);
        return false;
    }
};

void installPanelBehavior(KisMainWindow *mw)
{
    qApp->installEventFilter(new PanelCloser(mw));
    qApp->installEventFilter(new SmoothScroll(mw));
    qApp->installEventFilter(new PenHover(mw));
    mw->installEventFilter(new PanelHostWatcher(mw));
}

KoCanvasResourceProvider *resources(KisMainWindow *mw)
{
    KisViewManager *vm = mw->viewManager();
    return vm && vm->canvasResourceProvider() ? vm->canvasResourceProvider()->resourceManager() : nullptr;
}

// tamanho do pincel: escala logarítmica, 1 a 1000 px num trilho de 0 a 1000
int sizeToSlider(qreal px) { return qBound(0, qRound(std::log(qMax(1.0, px)) / std::log(1000.0) * 1000.0), 1000); }
qreal sliderToSize(int v) { return qMax(1.0, std::round(std::pow(1000.0, v / 1000.0))); }

// ---------------------------------------------------------------------------
// Barra do topo (design "Desenho"): galeria, título, desfazer/refazer, painéis
// ---------------------------------------------------------------------------
const char *TOP_STYLE =
    "QToolBar#nanquimTop { background: #2e2d29; border: none; border-bottom: 1px solid #4a4842;"
    "  spacing: 4px; padding: 4px 10px; }"
    "QToolBar#nanquimTop QToolButton { border: none; border-radius: 10px; padding: 9px; color: #eeeae0;"
    "  min-width: 26px; min-height: 26px; }"
    "QToolBar#nanquimTop QToolButton:hover { background: rgba(255,255,255,0.06); }"
    "QToolBar#nanquimTop QToolButton:pressed { background: #4a4842; }"
    "QToolBar#nanquimTop QToolButton:checked { background: #2f4a43; border: 1px solid #3e8e7e; }"
    "QToolBar#nanquimTop QToolButton::menu-indicator { image: none; width: 0px; }"
    "QToolBar#nanquimTop QToolButton[nanquimHover=\"true\"]:!checked { background: rgba(255,255,255,0.08); }"
    "QToolBar#nanquimTop QLabel#nanquimTitle { color: #eeeae0; font-size: 15px; font-weight: 600; }"
    "QToolBar#nanquimTop QLabel#nanquimInfo { color: #6fbfad; font-size: 11px; font-weight: 600; }"
    "QToolBar#nanquimTop QToolButton#nanquimBack { color: #eeeae0; font-size: 14px; font-weight: 600;"
    "  padding: 8px 14px; border: 1px solid #4a4842; background: #34332e; }";

// Se a janela começa por baixo da barra de status do Android, a barra do topo
// ganha um respiro do tamanho dela (senão o relógio fica por cima do título).
void updateSafeTop(KisMainWindow *mw)
{
    QScreen *screen = mw->screen();
    if (!screen) {
        return;
    }
    const int availTop = screen->availableGeometry().top();
    const int winTop = mw->mapToGlobal(QPoint(0, 0)).y();
    const int inset = qMax(0, availTop - winTop);
    // embaixo: a barra de navegação ou a barra de tarefas da Samsung não cobrem
    // a grade da tela inicial nem a do capítulo
    {
        const int winBottom = mw->mapToGlobal(QPoint(0, mw->height())).y();
        const int bottomInset = qBound(0, winBottom - (screen->availableGeometry().bottom() + 1), 160);
        for (const char *id : {"nanquimHomePage", "nanquimChapterPage"}) {
            QWidget *page = mw->findChild<QWidget*>(QLatin1String(id));
            if (page && page->layout()) page->layout()->setContentsMargins(0, 0, 0, bottomInset);
        }
    }
    if (QWidget *header = mw->findChild<QWidget*>(QStringLiteral("nanquimGalleryHeader"))) {
        header->setFixedHeight(64 + inset);
        if (header->layout()) {
            header->layout()->setContentsMargins(28, inset, 20, 0);
        }
    }
    QToolBar *top = mw->findChild<QToolBar*>(TOP_ID);
    if (!top) {
        return;   // modo Clássico: não há barra do Nanquim
    }
    if (top->property("nanquimInset").toInt() == inset && !top->styleSheet().isEmpty()) {
        return;
    }
    top->setProperty("nanquimInset", inset);
    top->setStyleSheet(QLatin1String(TOP_STYLE)
                       + QStringLiteral("QToolBar#nanquimTop { padding-top: %1px; }").arg(4 + inset));
}

QIcon colorSwatchIcon(const QColor &c)
{
    const qreal dpr = qApp->devicePixelRatio();
    QPixmap pm(QSize(26, 26) * dpr);
    pm.setDevicePixelRatio(dpr);
    pm.fill(Qt::transparent);
    QPainter g(&pm);
    g.setRenderHint(QPainter::Antialiasing);
    g.setPen(QPen(C_TEXT, 2));
    g.setBrush(c);
    g.drawEllipse(QRectF(2, 2, 22, 22));
    g.end();
    return QIcon(pm);
}


// ---------------------------------------------------------------------------
// Time-lapse (como no ibis Paint): grava sozinho um quadro da página inteira
// depois de cada traço. Parado não grava nada, então o vídeo já sai sem as
// pausas. Os quadros ficam numa pasta do app, por desenho, e o vídeo é montado
// pelo codificador do aparelho (NanquimTimelapse.java).
// ---------------------------------------------------------------------------
// Cópia da projeção feita pelo agendador da imagem, como no Gravador do Krita.
// O agendador não roda este job junto com traços, salvamentos ou a montagem da
// imagem; ler a projeção direto de outra thread fechava o app (sinal 11).
class NanquimTimelapseCopyJob : public KisSpontaneousJob
{
public:
    NanquimTimelapseCopyJob(KisPaintDeviceSP device, const QRect &rect, std::function<void(KisPaintDeviceSP)> done)
        : m_device(device), m_rect(rect), m_done(std::move(done)) {}
    void run() override
    {
        KisPaintDeviceSP copy = new KisPaintDevice(m_device->colorSpace());
        copy->makeCloneFromRough(m_device, m_rect);
        m_done(copy);
    }
    bool overrides(const KisSpontaneousJob *) override { return false; }
    int levelOfDetail() const override { return 0; }
    QString debugName() const override { return QStringLiteral("NanquimTimelapseCopyJob"); }
private:
    KisPaintDeviceSP m_device;
    QRect m_rect;
    std::function<void(KisPaintDeviceSP)> m_done;
};

class NanquimTimelapseRecorder : public QObject
{
public:
    explicit NanquimTimelapseRecorder(KisMainWindow *mw)
        : QObject(mw), m_mw(mw)
    {
        m_sinceCapture.start();
        m_sinceChange.start();
        m_sinceTracked.start();
        m_sinceBusy.start();
        QTimer *timer = new QTimer(this);
        timer->setInterval(500);
        QObject::connect(timer, &QTimer::timeout, this, [this]() { tick(); });
        timer->start();
        QtConcurrent::run([]() { removeOrphans(); });
    }

    static bool enabled()
    {
        return KSharedConfig::openConfig()->group("Nanquim").readEntry("timelapseAuto", true);
    }
    static void setEnabled(bool on)
    {
        KSharedConfig::openConfig()->group("Nanquim").writeEntry("timelapseAuto", on);
    }

    // pasta dos quadros do desenho ativo (vazia se não há desenho)
    QString currentDir() const
    {
        KisView *view = m_mw->activeView();
        return view && view->document() ? dirFor(view->document()) : QString();
    }
    // espaço que a gravação do desenho ativo ocupa, em bytes
    qint64 currentBytes() const
    {
        const QString dir = currentDir();
        qint64 total = 0;
        if (!dir.isEmpty()) {
            Q_FOREACH (const QFileInfo &info, QDir(dir).entryInfoList({QStringLiteral("frame_*.jpg")}, QDir::Files)) {
                total += info.size();
            }
        }
        return total;
    }
    int frameCount() const
    {
        const QString dir = currentDir();
        return dir.isEmpty() ? 0 : QDir(dir).entryList({QStringLiteral("frame_*.jpg")}, QDir::Files).count();
    }
    void clearCurrent()
    {
        const QString dir = currentDir();
        if (!dir.isEmpty()) {
            QDir(dir).removeRecursively();
        }
        if (KisView *view = m_mw->activeView()) {
            if (view->document()) view->document()->setProperty("nanquimTimelapseNext", QVariant());
        }
        m_dirty = true;   // recomeça do estado atual do desenho
    }

private:
    static QString baseDir()
    {
        return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/timelapse/");
    }

    // Limite por desenho: passando de 1500 quadros, fica um sim, um não (750).
    // O vídeo continua cobrindo o processo inteiro, só com menos passos.
    static void thinFrames(const QString &dir)
    {
        QDir d(dir);
        const QStringList frames = d.entryList({QStringLiteral("frame_*.jpg")}, QDir::Files, QDir::Name);
        if (frames.size() <= 1500) return;
        for (int i = 1; i < frames.size() - 1; i += 2) {
            d.remove(frames[i]);
        }
    }

    // Gravações de desenhos que nunca foram salvos: somem depois de 3 dias.
    static void removeOrphans()
    {
        const QDateTime limit = QDateTime::currentDateTime().addDays(-3);
        Q_FOREACH (const QFileInfo &info, QDir(baseDir()).entryInfoList({QStringLiteral("novo-*")}, QDir::Dirs | QDir::NoDotAndDotDot)) {
            if (info.lastModified() < limit) {
                QDir(info.absoluteFilePath()).removeRecursively();
            }
        }
    }

    // Desenho salvo: pasta pelo caminho do arquivo (continua entre sessões).
    // Desenho ainda sem nome: pasta provisória, renomeada quando ele for salvo.
    static QString dirFor(KisDocument *doc)
    {
        const QString path = doc->path();
        QString current = doc->property("nanquimTimelapseDir").toString();
        if (path.isEmpty()) {
            if (current.isEmpty()) {
                current = baseDir() + QStringLiteral("novo-") + QUuid::createUuid().toString(QUuid::Id128);
                doc->setProperty("nanquimTimelapseDir", current);
            }
            return current;
        }
        const QString wanted = baseDir() + QString::fromLatin1(
                    QCryptographicHash::hash(path.toUtf8(), QCryptographicHash::Sha1).toHex().left(20));
        if (current != wanted) {
            if (!current.isEmpty() && QDir(current).exists() && !QDir(wanted).exists()) {
                QDir().mkpath(baseDir());
                QDir().rename(current, wanted);
            }
            doc->setProperty("nanquimTimelapseDir", wanted);
            doc->setProperty("nanquimTimelapseNext", QVariant());
        }
        return wanted;
    }

    void tick()
    {
        KisView *view = m_mw->activeView();
        KisImageSP image;
        if (view) image = view->image();
        if (image.data() != m_tracked.data()) {
            if (m_tracked) {
                QObject::disconnect(m_connection);
            }
            m_tracked = image.data();
            m_dirty = !image.isNull();   // primeiro quadro: o estado em que o desenho foi aberto
            m_sinceChange.restart();
            m_sinceTracked.restart();
            if (image) {
                m_connection = QObject::connect(image.data(), &KisImage::sigImageUpdated, this, [this](const QRect &) {
                    m_dirty = true;
                    m_sinceChange.restart();
                });
            }
        }
        if (!image || !view->document() || !m_dirty || !enabled()) return;
        // só grava com a caneta parada de verdade (2,5 s sem mudanças) e no mínimo 6 s
        // entre quadros: antes, cada pausa de 0,7 s entre traços de hachura disparava a
        // redução da página inteira e disputava o processador com o pincel
        if (m_sinceChange.elapsed() < 2500 || m_sinceCapture.elapsed() < 6000) return;
        // desenho recém-aberto: a imagem ainda está sendo montada
        if (m_sinceTracked.elapsed() < 4000) return;
        // nada de quadro enquanto o desenho é salvo (inclusive o salvamento automático)
        if (view->document()->isSaving()) return;
        if (m_busy->load()) {
            if (m_sinceBusy.elapsed() < 30000) return;
            m_busy->store(false);    // um quadro que nunca terminou não trava a gravação
        }

        KisDocument *doc = view->document();
        const QString dir = dirFor(doc);
        int next = doc->property("nanquimTimelapseNext").toInt();
        if (next <= 0) {
            const QStringList existing = QDir(dir).entryList({QStringLiteral("frame_*.jpg")}, QDir::Files, QDir::Name);
            next = existing.isEmpty() ? 1 : existing.last().mid(6, 6).toInt() + 1;
        }
        doc->setProperty("nanquimTimelapseNext", next + 1);

        const QRect bounds = image->bounds();
        if (bounds.isEmpty()) return;
        // lado maior com 1600 px: nítido no celular e leve para gravar
        const qreal scale = qMin(1.0, 1600.0 / qMax(bounds.width(), bounds.height()));
        const int w = qMax(16, qRound(bounds.width() * scale) & ~1);
        const int h = qMax(16, qRound(bounds.height() * scale) & ~1);
        KisPaintDeviceSP projection = image->projection();
        const QString file = dir + QStringLiteral("/frame_%1.jpg").arg(next, 6, 10, QLatin1Char('0'));

        m_dirty = false;
        m_sinceCapture.restart();
        m_sinceBusy.restart();
        std::shared_ptr<std::atomic_bool> busy = m_busy;
        busy->store(true);
        // 1) o agendador da imagem copia a projeção (cópia leve, blocos compartilhados);
        // 2) a redução e a gravação do JPEG rodam fora da interface, só sobre a cópia
        image->addSpontaneousJob(new NanquimTimelapseCopyJob(projection, bounds,
                [bounds, w, h, dir, file, busy, next](KisPaintDeviceSP copy) {
        QtConcurrent::run([copy, bounds, w, h, dir, file, busy, next]() mutable {
            QImage thumb = copy->createThumbnailUncached(w, h, bounds, 1);
            if (!thumb.isNull()) {
                QImage frame(w, h, QImage::Format_RGB32);
                frame.fill(Qt::white);
                QPainter g(&frame);
                g.setRenderHint(QPainter::SmoothPixmapTransform);
                g.drawImage(QRect(0, 0, w, h), thumb);
                g.end();
                QDir().mkpath(dir);
                frame.save(file, "JPG", 82);
                if (next % 100 == 0) {
                    thinFrames(dir);
                }
            }
            busy->store(false);
        });
        }));
    }

    KisMainWindow *m_mw;
    QPointer<KisImage> m_tracked;
    QMetaObject::Connection m_connection;
    bool m_dirty = false;
    QElapsedTimer m_sinceChange;
    QElapsedTimer m_sinceCapture;
    QElapsedTimer m_sinceTracked;
    QElapsedTimer m_sinceBusy;
    std::shared_ptr<std::atomic_bool> m_busy = std::make_shared<std::atomic_bool>(false);
};

void exportTimelapse(KisMainWindow *mw, NanquimTimelapseRecorder *recorder, int seconds)
{
    KisView *view = mw->activeView();
    const QString dir = recorder->currentDir();
    if (!view || !view->document() || dir.isEmpty()) return;
    QString title = view->document()->caption().trimmed();
    if (title.endsWith(QLatin1String(".kra"), Qt::CaseInsensitive)) title.chop(4);
    if (title.startsWith(QLatin1Char('['))) title = i18n("desenho");
#ifdef Q_OS_ANDROID
    QAndroidJniObject::callStaticMethod<void>(
        "org/krita/android/NanquimTimelapse", "export",
        "(Landroid/app/Activity;Ljava/lang/String;Ljava/lang/String;I)V",
        QtAndroid::androidActivity().object(),
        QAndroidJniObject::fromString(dir).object<jstring>(),
        QAndroidJniObject::fromString(title).object<jstring>(),
        jint(seconds));
#else
    Q_UNUSED(seconds);
#endif
}

QPointer<NanquimTimelapseRecorder> s_recorder;

void fillTimelapseMenu(KisMainWindow *mw, QMenu *menu)
{
    menu->clear();
    NanquimTimelapseRecorder *recorder = s_recorder.data();
    if (!recorder) return;
    const int frames = recorder->frameCount();
    const qreal megabytes = recorder->currentBytes() / (1024.0 * 1024.0);
    QAction *info = menu->addAction(frames > 0
            ? i18n("%1 quadros gravados neste desenho (%2 MB)", frames, QString::number(megabytes, 'f', 0))
            : i18n("Nenhum quadro gravado ainda"));
    info->setEnabled(false);
    menu->addSeparator();
    struct Option { const char *label; int seconds; };
    for (const Option &o : {Option{"Exportar vídeo de 30 segundos", 30},
                            Option{"Exportar vídeo de 1 minuto", 60},
                            Option{"Exportar vídeo completo", 0}}) {
        QAction *a = menu->addAction(i18n(o.label));
        a->setEnabled(frames > 0);
        const int seconds = o.seconds;
        QObject::connect(a, &QAction::triggered, mw, [mw, seconds]() {
            if (s_recorder) exportTimelapse(mw, s_recorder.data(), seconds);
        });
    }
    menu->addSeparator();
    QAction *autoRecord = menu->addAction(i18n("Gravar sozinho enquanto desenho"));
    autoRecord->setCheckable(true);
    autoRecord->setChecked(NanquimTimelapseRecorder::enabled());
    QObject::connect(autoRecord, &QAction::toggled, mw, [](bool on) { NanquimTimelapseRecorder::setEnabled(on); });
    QAction *clear = menu->addAction(i18n("Apagar a gravação deste desenho"));
    clear->setEnabled(frames > 0);
    QObject::connect(clear, &QAction::triggered, mw, []() { if (s_recorder) s_recorder->clearCurrent(); });
}


// Inserir imagem como camada (como o "Inserir" do Photoshop): escolhe o arquivo,
// ele entra como camada nova e a ferramenta Transformar já abre para posicionar
// mesmo esquema do activateWhenReady: tenta de vez em quando, com limite
static void waitForLayerThenTransform(KisMainWindow *mw, QPointer<KisView> guard, int before, int tries)
{
    if (!guard || !guard->image()) return;
    if (guard->image()->nlayers() > before) {
        if (QAction *transform = act(mw, "KisToolTransform")) {
            transform->trigger();
        }
        return;
    }
    if (tries <= 0) return;
    QTimer::singleShot(300, mw, [mw, guard, before, tries]() {
        waitForLayerThenTransform(mw, guard, before, tries - 1);
    });
}

void insertImageAsLayer(KisMainWindow *mw)
{
    QAction *import = act(mw, "import_layer_as_paint_layer");
    KisView *view = mw->activeView();
    if (!import || !view || !view->image()) return;
    const int before = view->image()->nlayers();
    import->trigger();
    // espera a camada entrar de verdade (imagem grande pode levar segundos) e
    // só então entra na ferramenta Transformar
    QPointer<KisView> guard(view);
    if (!guard || !guard->image() || guard->image()->nlayers() <= before) {
        waitForLayerThenTransform(mw, guard, before, 16);
    }
}


// ---------------------------------------------------------------------------
// Requadros e balões: formas vetoriais (editáveis com a ferramenta de seleção
// de formas), criadas numa camada vetorial própria, com desfazer.
// As medidas do SVG ficam em pontos (1/72 pol), como no .kra do Krita.
// ---------------------------------------------------------------------------
struct PanelRow { qreal height; QList<qreal> columns; };
struct PanelLayout { const char *name; QList<PanelRow> rows; };

QList<PanelLayout> panelLayouts()
{
    return {
        {"1 quadro", {{1, {1}}}},
        {"2 faixas", {{1, {1}}, {1, {1}}}},
        {"3 faixas", {{1, {1}}, {1, {1}}, {1, {1}}}},
        {"4 quadros (2×2)", {{1, {1, 1}}, {1, {1, 1}}}},
        {"6 quadros (2×3)", {{1, {1, 1}}, {1, {1, 1}}, {1, {1, 1}}}},
        {"Mangá, 5 quadros", {{1.25, {1}}, {1, {0.42, 0.58}}, {1.1, {0.6, 0.4}}}},
        {"Mangá, 6 quadros", {{1, {0.55, 0.45}}, {1.2, {1}}, {1, {0.35, 0.3, 0.35}}}},
        {"Destaque embaixo", {{1, {1, 1}}, {1.6, {1}}}},
        {"Tirinha, 4 quadros", {{1, {1, 1, 1, 1}}}},
    };
}

QList<QRectF> layoutRects(const PanelLayout &layout, const QRectF &area, qreal gutterX, qreal gutterY)
{
    QList<QRectF> rects;
    qreal totalH = 0;
    for (const PanelRow &r : layout.rows) totalH += r.height;
    const qreal usableH = area.height() - gutterY * (layout.rows.size() - 1);
    qreal y = area.top();
    for (const PanelRow &r : layout.rows) {
        const qreal h = usableH * r.height / totalH;
        qreal totalW = 0;
        for (qreal c : r.columns) totalW += c;
        const qreal usableW = area.width() - gutterX * (r.columns.size() - 1);
        qreal x = area.left();
        for (qreal c : r.columns) {
            const qreal w = usableW * c / totalW;
            rects << QRectF(x, y, w, h);
            x += w + gutterX;
        }
        y += h + gutterY;
    }
    return rects;
}

QIcon layoutIcon(const PanelLayout &layout)
{
    const qreal dpr = qApp->devicePixelRatio();
    QPixmap pm(QSize(64, 90) * dpr);
    pm.setDevicePixelRatio(dpr);
    pm.fill(Qt::transparent);
    QPainter g(&pm);
    g.setRenderHint(QPainter::Antialiasing);
    g.fillRect(QRectF(0, 0, 64, 90), QColor(0xfa, 0xfa, 0xf7));
    g.setPen(QPen(QColor(0x15, 0x15, 0x15), 2));
    g.setBrush(Qt::NoBrush);
    for (const QRectF &r : layoutRects(layout, QRectF(6, 7, 52, 76), 3, 4)) {
        g.drawRect(r);
    }
    return QIcon(pm);
}

// A camada nova só pode virar a ativa depois que o Krita cria a forma dela
// (o acréscimo passa pela fila de comandos). Ativar antes dava o aviso
// "shape" do gerenciador de camadas e deixava a seleção num estado errado.
void activateWhenReady(QPointer<KisViewManager> vm, QPointer<KisDocument> doc, KisNodeSP node, int tries)
{
    if (!vm || !doc || !node) return;
    if (doc->shapeForNode(node)) {
        vm->nodeManager()->slotNonUiActivatedNode(node);
        return;
    }
    if (tries <= 0) {
        KisUsageLogger::log(QStringLiteral("[Nanquim] camada nova sem forma; não foi ativada"));
        return;
    }
    QTimer::singleShot(50, vm.data(), [vm, doc, node, tries]() { activateWhenReady(vm, doc, node, tries - 1); });
}

// adiciona formas vindas de um SVG (em pontos) numa camada vetorial
bool addVectorShapes(KisMainWindow *mw, const QString &layerName, const QString &svgBody, bool reuseLayer)
{
    KisView *view = mw->activeView();
    KisViewManager *vm = mw->viewManager();
    if (!view || !view->document() || !view->image() || !vm) return false;
    KisImageSP image = view->image();
    KisDocument *doc = view->document();

    const qreal ptW = image->width() / image->xRes();
    const qreal ptH = image->height() / image->yRes();
    const QString svg = QStringLiteral(
        "<?xml version=\"1.0\" standalone=\"no\"?>"
        "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"%1pt\" height=\"%2pt\" viewBox=\"0 0 %1 %2\">%3</svg>")
            .arg(ptW).arg(ptH).arg(svgBody);
    QByteArray data = svg.toUtf8();
    QBuffer buffer(&data);
    buffer.open(QIODevice::ReadOnly);
    const QList<KoShape*> shapes = KisShapeLayer::createShapesFromSvg(
                &buffer, QString(), image->bounds(), 72.0 * image->xRes(),
                doc->shapeController()->resourceManager(), false, nullptr);
    if (shapes.isEmpty()) {
        KisUsageLogger::log(QStringLiteral("[Nanquim] formas vetoriais: SVG não gerou formas"));
        return false;
    }

    KisShapeLayer *existing = nullptr;
    if (reuseLayer) {
        for (KisNodeSP child = image->root()->firstChild(); child; child = child->nextSibling()) {
            if (child->name() == layerName) {
                existing = dynamic_cast<KisShapeLayer*>(child.data());
                if (existing) break;
            }
        }
    }
    if (existing) {
        Q_FOREACH (KoShape *shape, shapes) {
            existing->addShape(shape);
        }
        vm->nodeManager()->slotNonUiActivatedNode(existing);
        return true;
    }
    KisShapeLayerSP layer = new KisShapeLayer(doc->shapeController(), image, layerName, OPACITY_OPAQUE_U8);
    Q_FOREACH (KoShape *shape, shapes) {
        layer->addShape(shape);
    }
    KisNodeCommandsAdapter adapter(vm);
    adapter.addNode(layer, image->root(), image->root()->lastChild());
    activateWhenReady(QPointer<KisViewManager>(vm), QPointer<KisDocument>(doc), KisNodeSP(layer.data()), 40);
    return true;
}


// balões: forma com contorno preto e fundo branco, no centro da página
QString balloonPath(int kind, const QRectF &r, qreal unit)
{
    const QPointF c = r.center();
    const qreal rx = r.width() / 2, ry = r.height() / 2;
    QString d;
    auto pt = [](const QPointF &p) { return QStringLiteral("%1 %2").arg(p.x()).arg(p.y()); };
    if (kind == 0) {
        // fala: elipse com rabicho para baixo à esquerda
        const qreal a1 = qDegreesToRadians(110.0), a2 = qDegreesToRadians(128.0);
        const QPointF p1(c.x() + rx * std::cos(a1), c.y() + ry * std::sin(a1));
        const QPointF p2(c.x() + rx * std::cos(a2), c.y() + ry * std::sin(a2));
        const QPointF tip(c.x() - rx * 0.75, c.y() + ry * 1.75);
        d = QStringLiteral("M %1 L %2 L %3 A %4 %5 0 1 1 %1 Z")
                .arg(pt(p1), pt(tip), pt(p2)).arg(rx).arg(ry);
        return QStringLiteral("<path d=\"%1\" fill=\"#ffffff\" stroke=\"#000000\" stroke-width=\"%2\" stroke-linejoin=\"round\"/>")
                .arg(d).arg(unit);
    }
    if (kind == 1) {
        // pensamento: nuvem e bolinhas
        const int bumps = 11;
        for (int i = 0; i <= bumps; i++) {
            const qreal a = 2 * M_PI * i / bumps;
            const QPointF p(c.x() + rx * std::cos(a), c.y() + ry * std::sin(a));
            if (i == 0) {
                d += QStringLiteral("M %1 ").arg(pt(p));
            } else {
                const qreal am = 2 * M_PI * (i - 0.5) / bumps;
                const QPointF ctrl(c.x() + rx * 1.32 * std::cos(am), c.y() + ry * 1.32 * std::sin(am));
                d += QStringLiteral("Q %1 %2 ").arg(pt(ctrl), pt(p));
            }
        }
        d += QLatin1String("Z");
        QString out = QStringLiteral("<path d=\"%1\" fill=\"#ffffff\" stroke=\"#000000\" stroke-width=\"%2\" stroke-linejoin=\"round\"/>")
                .arg(d).arg(unit);
        const qreal sizes[] = {0.16, 0.1};
        for (int i = 0; i < 2; i++) {
            const qreal s = rx * sizes[i];
            const QPointF bc(c.x() - rx * (0.65 + 0.3 * i), c.y() + ry * (1.45 + 0.45 * i));
            out += QStringLiteral("<ellipse cx=\"%1\" cy=\"%2\" rx=\"%3\" ry=\"%4\" fill=\"#ffffff\" stroke=\"#000000\" stroke-width=\"%5\"/>")
                    .arg(bc.x()).arg(bc.y()).arg(s).arg(s * 0.8).arg(unit);
        }
        return out;
    }
    if (kind == 2) {
        // grito: estrela irregular
        const int points = 18;
        for (int i = 0; i < points * 2; i++) {
            const qreal a = M_PI * i / points;
            const qreal k = (i % 2 == 0) ? (1.18 + 0.12 * ((i * 7) % 3)) : 0.86;
            const QPointF p(c.x() + rx * k * std::cos(a), c.y() + ry * k * std::sin(a));
            d += (i == 0 ? QStringLiteral("M ") : QStringLiteral("L ")) + pt(p) + QLatin1Char(' ');
        }
        d += QLatin1String("Z");
        return QStringLiteral("<path d=\"%1\" fill=\"#ffffff\" stroke=\"#000000\" stroke-width=\"%2\" stroke-linejoin=\"miter\"/>")
                .arg(d).arg(unit);
    }
    // narração: caixa
    return QStringLiteral("<rect x=\"%1\" y=\"%2\" width=\"%3\" height=\"%4\" fill=\"#ffffff\" stroke=\"#000000\" stroke-width=\"%5\"/>")
            .arg(r.x()).arg(r.y() + r.height() * 0.2).arg(r.width()).arg(r.height() * 0.6).arg(unit);
}


void addBalloonButton(KisMainWindow *mw, QToolBar *top)
{
    // abre "Requadros e balões" já na aba de balões
    QToolButton *b = iconButton(top, i18n("Balões de fala"), designIcon(Svg::BALLOON));
    b->setProperty(NEEDS_CANVAS, true);
    QObject::connect(b, &QToolButton::clicked, mw, [mw]() { showComicsSheet(mw, 1); });
    top->addWidget(b);
}

// ---------------------------------------------------------------------------
// Pincéis no estilo do protótipo: no modo Tablet, uma janela com categorias e
// uma lista com nome, descrição e traço de amostra; no modo Clássico, uma
// coluna de pincéis rápidos ao lado da tela.
// ---------------------------------------------------------------------------
enum StrokeStyle { StrokePressure, StrokeFine, StrokeThick, StrokeTexture, StrokePencil,
                   StrokeFlat, StrokeSoft, StrokeDots, StrokeBlend, StrokeEraser };

struct NanquimBrush {
    const char *file;       // arquivo do pacote padrão do Krita (ou "prefixo*")
    const char *name;
    const char *shortName;  // nome curto, na coluna do modo Clássico
    const char *category;
    const char *detail;
    StrokeStyle style;
    qreal weight;           // espessura da amostra, de 0 a 1
    bool classic;           // aparece na coluna do modo Clássico
};

const NanquimBrush NANQUIM_BRUSHES[] = {
    {"d)_Ink-3_Gpen.kpp",           "Caneta G",          "Caneta G",  "Tinta",    "pressão",   StrokePressure, 0.55, true},
    {"d)_Ink-2_Fineliner.kpp",      "Caneta Maru",       "Maru",      "Tinta",    "fina",      StrokeFine,     0.22, true},
    {"d)_Ink-7_Brush_Rough.kpp",    "Pincel nanquim",    "Nanquim",   "Tinta",    "grossa",    StrokeThick,    0.95, true},
    {"d)_Ink-4_Pen_Rough.kpp",      "Pena seca",         "Pena seca", "Tinta",    "textura",   StrokeTexture,  0.6,  true},
    {"d)_Ink-1_Precision.kpp",      "Caneta técnica",    "Técnica",   "Tinta",    "precisão",  StrokeFine,     0.14, false},
    {"e)_Marker_Details.kpp",       "Marcador",          "Marcador",  "Tinta",    "detalhes",  StrokeFlat,     0.45, false},
    {"c)_Pencil-2.kpp",             "Lápis de rascunho", "Lápis",     "Lápis",    "rascunho",  StrokePencil,   0.35, true},
    {"c)_Pencil-1_Hard.kpp",        "Lápis duro",        "Lápis duro","Lápis",    "fino",      StrokePencil,   0.2,  false},
    {"c)_Pencil-4_Soft.kpp",        "Lápis macio",       "Macio",     "Lápis",    "suave",     StrokePencil,   0.55, false},
    {"c)_Pencil-6_Quick_Shade.kpp", "Sombra rápida",     "Sombra",    "Lápis",    "sombreado", StrokeSoft,     0.7,  false},
    {"b)_Basic-5_Size_default.kpp", "Preenchimento",     "Chapado",   "Pintura",  "chapado",   StrokeFlat,     1.0,  true},
    {"b)_Airbrush_Soft.kpp",        "Aerógrafo",         "Aerógrafo", "Pintura",  "suave",     StrokeSoft,     1.0,  true},
    {"i)_Blend*",                   "Mistura",           "Mistura",   "Pintura",  "esfumar",   StrokeBlend,    0.9,  true},
    {"a)_Eraser_Circle.kpp",        "Borracha",          "Borracha",  "Pintura",  "dura",      StrokeEraser,   0.7,  false},
    {"a)_Eraser_Small.kpp",         "Borracha fina",     "Borr. fina","Pintura",  "detalhe",   StrokeEraser,   0.3,  false},
    {"a)_Eraser_Soft.kpp",          "Borracha macia",    "Borr. macia","Pintura", "suave",     StrokeEraser,   0.9,  false},
    {"y)_Screentone_Pressure.kpp",  "Retícula",          "Retícula",  "Retícula", "pressão",   StrokeDots,     0.8,  true},
    {"y)_Screentones_Regular.kpp",  "Retícula regular",  "Pontos",    "Retícula", "pontos",    StrokeDots,     0.55, false},
};

const QString BRUSH_IMPORTED = QStringLiteral("Importados");
const QString BRUSH_ALL = QStringLiteral("Todos");
const QString BRUSH_FAVORITES = QStringLiteral("Favoritos");

struct BrushItem {
    KisPaintOpPresetSP preset;   // nulo em "Todos" até a linha aparecer
    int id = -1;
    QString name;
    QString shortName;
    QString subtitle;
    StrokeStyle style = StrokePressure;
    qreal weight = 0.5;
};

KisResourceModel *allPresetsModel()
{
    static QPointer<KisResourceModel> s_model;
    if (!s_model) {
        s_model = new KisResourceModel(ResourceType::PaintOpPresets, qApp);
    }
    return s_model;
}

KisPaintOpPresetSP presetForId(int id)
{
    static QHash<int, KisPaintOpPresetSP> s_cache;
    if (id < 0) return KisPaintOpPresetSP();
    auto it = s_cache.find(id);
    if (it != s_cache.end()) return it.value();
    KisPaintOpPresetSP preset = allPresetsModel()->resourceForId(id).dynamicCast<KisPaintOpPreset>();
    if (preset) s_cache.insert(id, preset);
    return preset;
}

qreal weightForPreset(const KisPaintOpPresetSP &preset)
{
    if (!preset || !preset->settings()) return 0.5;
    const qreal size = qMax<qreal>(1.0, preset->settings()->paintOpSize());
    return qBound<qreal>(0.12, std::log(size) / std::log(120.0), 1.0);
}

KisPaintOpPresetSP findBundledPreset(const QString &file)
{
    if (!file.endsWith(QLatin1Char('*'))) {
        KisPaintOpPresetResourceServer *server = KisResourceServerProvider::instance()->paintOpPresetServer();
        return server->resource(QString(), file, QString());
    }
    // pincel cujo nome exato muda entre versões do pacote: o primeiro que bate
    const QString prefix = file.left(file.size() - 1);
    KisResourceModel *model = allPresetsModel();
    for (int row = 0; row < model->rowCount(); ++row) {
        const QModelIndex idx = model->index(row, KisAbstractResourceModel::Filename);
        if (idx.data().toString().startsWith(prefix, Qt::CaseInsensitive)) {
            return model->resourceForIndex(model->index(row, 0)).dynamicCast<KisPaintOpPreset>();
        }
    }
    return KisPaintOpPresetSP();
}

QVector<BrushItem> nanquimBrushItems(const QString &category, bool classicOnly)
{
    static QHash<QString, KisPaintOpPresetSP> s_found;
    QVector<BrushItem> items;
    for (const NanquimBrush &b : NANQUIM_BRUSHES) {
        if (classicOnly ? !b.classic : (category != QString::fromUtf8(b.category))) continue;
        const QString file = QString::fromUtf8(b.file);
        KisPaintOpPresetSP preset = s_found.value(file);
        if (!preset) {
            preset = findBundledPreset(file);
            if (!preset) continue;
            s_found.insert(file, preset);
        }
        BrushItem item;
        item.preset = preset;
        item.id = preset->resourceId();
        item.name = QString::fromUtf8(b.name);
        item.shortName = QString::fromUtf8(b.shortName);
        item.subtitle = QStringLiteral("%1 · %2").arg(QString::fromUtf8(b.category), QString::fromUtf8(b.detail));
        item.style = b.style;
        item.weight = b.weight;
        items << item;
    }
    return items;
}

bool isBundledNanquimFile(const QString &filename)
{
    for (const NanquimBrush &b : NANQUIM_BRUSHES) {
        const QString file = QString::fromUtf8(b.file);
        if (file.endsWith(QLatin1Char('*')) ? filename.startsWith(file.left(file.size() - 1)) : filename == file) {
            return true;
        }
    }
    return false;
}

// um pincel qualquer: usa o nome e a amostra do Nanquim se for um dos nossos
BrushItem itemForPreset(const KisPaintOpPresetSP &preset)
{
    BrushItem item;
    item.preset = preset;
    item.id = preset->resourceId();
    const QString filename = preset->filename();
    for (const NanquimBrush &b : NANQUIM_BRUSHES) {
        const QString file = QString::fromUtf8(b.file);
        if (file.endsWith(QLatin1Char('*')) ? filename.startsWith(file.left(file.size() - 1)) : filename == file) {
            item.name = QString::fromUtf8(b.name);
            item.shortName = QString::fromUtf8(b.shortName);
            item.subtitle = QStringLiteral("%1 · %2").arg(QString::fromUtf8(b.category), QString::fromUtf8(b.detail));
            item.style = b.style;
            item.weight = b.weight;
            return item;
        }
    }
    item.name = preset->name().replace(QLatin1Char('_'), QLatin1Char(' '));
    item.shortName = item.name;
    item.weight = weightForPreset(preset);
    item.subtitle = QStringLiteral("%1 px").arg(qRound(preset->settings() ? preset->settings()->paintOpSize() : 0));
    return item;
}

// pincéis que o artista trouxe (.kpp, .sut ou criados com "+"): ficam na etiqueta "Nanquim"
// Pincéis de pacotes (.bundle) instalados pelo usuário entram na etiqueta "Nanquim",
// que é o que a aba Importados mostra. Os pacotes que vêm com o Krita ficam de fora.
// Roda sempre que a aba é montada, então os pacotes já baixados também aparecem.
int syncImportedBundleBrushes()
{
    KisTagModel tagModel(ResourceType::PaintOpPresets);
    KisTagSP tag = tagModel.tagForUrl(QStringLiteral("Nanquim"));
    if (!tag) tag = tagModel.addTag(QStringLiteral("Nanquim"), false, QVector<KoResourceSP>());
    if (!tag) return 0;

    QSet<int> userStorages;
    KisStorageModel *storages = KisStorageModel::instance();
    for (int r = 0; r < storages->rowCount(); ++r) {
        const QString location = storages->index(r, KisStorageModel::Location).data().toString();
        if (!location.endsWith(QLatin1String(".bundle"), Qt::CaseInsensitive)) continue;
        if (storages->index(r, KisStorageModel::PreInstalled).data().toBool()) continue;
        if (QFileInfo(location).fileName().contains(QLatin1String("Default_Resources"), Qt::CaseInsensitive)) continue;
        userStorages.insert(storages->index(r, KisStorageModel::Id).data().toInt());
    }
    QSet<int> already;
    {
        KisTagResourceModel tagged(ResourceType::PaintOpPresets);
        tagged.setTagsFilter(QVector<int>() << tag->id());
        for (int r = 0; r < tagged.rowCount(); ++r) {
            already.insert(tagged.index(r, KisAbstractResourceModel::Id).data().toInt());
        }
    }
    QVector<int> toTag;
    KisResourceModel *all = allPresetsModel();
    for (int r = 0; r < all->rowCount(); ++r) {
        const int storageId = all->index(r, KisAbstractResourceModel::StorageId).data().toInt();
        const QString fn = all->index(r, KisAbstractResourceModel::Filename).data().toString();
        const bool isUserBundle = userStorages.contains(storageId);
        const bool isSutBrush = fn.startsWith(QLatin1String("sut_"), Qt::CaseInsensitive);
        if (!isUserBundle && !isSutBrush) continue;
        const int id = all->index(r, KisAbstractResourceModel::Id).data().toInt();
        if (id >= 0 && !already.contains(id)) toTag << id;
    }
    if (toTag.isEmpty()) return 0;
    KisTagResourceModel model(ResourceType::PaintOpPresets);
    model.tagResources(tag, toTag);
    KisUsageLogger::log(QStringLiteral("[Nanquim] %1 pincéis de pacotes instalados foram para Importados").arg(toTag.size()));
    return toTag.size();
}

// Instala um pincel ou pacote sem abrir caixa de mensagem nenhuma (as caixas nativas
// saem quebradas no Android). Quem chama mostra o resultado no próprio botão ou num aviso.
bool importBrushFileQuietly(QWidget *parent, const QString &path, QString *message)
{
    const QFileInfo info(path);
    const QString ext = info.suffix().toLower();
    QString msg;
    bool ok = false;
    if (ext == QLatin1String("zip")) {
        ok = NanquimSutImport::importZipArchive(parent, path, &msg);
    } else if (ext == QLatin1String("sut")) {
        ok = NanquimSutImport::importFile(parent, path, &msg);
    } else if (ext == QLatin1String("kpp")) {
        KoResourceSP res = KisResourceUserOperations::importResourceFileWithUserInput(
                    parent, QString(), ResourceType::PaintOpPresets, path);
        ok = !res.isNull();
        if (ok && res->resourceId() >= 0) {
            KisTagModel tagModel(ResourceType::PaintOpPresets);
            if (KisTagSP tag = tagModel.tagForUrl(QStringLiteral("Nanquim"))) {
                KisTagResourceModel tagged(ResourceType::PaintOpPresets);
                tagged.tagResources(tag, QVector<int>() << res->resourceId());
            }
        }
        msg = ok ? i18n("Pincel importado: %1", res->name()) : i18n("Não consegui importar %1.", info.fileName());
    } else if (ext == QLatin1String("bundle")) {
        // reinstalar um pacote que já está aí não é erro
        KisStorageModel *storages = KisStorageModel::instance();
        for (int r = 0; r < storages->rowCount() && !ok; ++r) {
            const QString location = storages->index(r, KisStorageModel::Location).data().toString();
            if (QFileInfo(location).fileName().compare(info.fileName(), Qt::CaseInsensitive) == 0) ok = true;
        }
        if (!ok) ok = KisStorageModel::instance()->importStorage(path, KisStorageModel::None);
        msg = ok ? i18n("Pacote importado: %1", info.completeBaseName())
                 : i18n("Não consegui importar o pacote %1.", info.fileName());
    } else {
        msg = i18n("%1 não é um pincel que o Nanquim abre (.kpp, .bundle, .sut ou .zip).", info.fileName());
    }
    syncImportedBundleBrushes();
    KisUsageLogger::log(QStringLiteral("[Nanquim] importação silenciosa %1: %2").arg(info.fileName()).arg(ok ? "ok" : "falhou"));
    if (message) *message = msg;
    return ok;
}

QVector<BrushItem> importedBrushItems()
{
    syncImportedBundleBrushes();
    QVector<BrushItem> items;
    KisTagModel tagModel(ResourceType::PaintOpPresets);
    KisTagSP tag = tagModel.tagForUrl(QStringLiteral("Nanquim"));
    if (!tag) return items;

    QSet<int> taggedIds;
    {
        KisTagResourceModel tagged(ResourceType::PaintOpPresets);
        tagged.setTagsFilter(QVector<int>() << tag->id());
        for (int row = 0; row < tagged.rowCount(); ++row) {
            const int id = tagged.index(row, KisAbstractResourceModel::Id).data().toInt();
            if (id >= 0) taggedIds.insert(id);
        }
    }
    if (taggedIds.isEmpty()) return items;

    QSet<int> bundleStorages;
    KisStorageModel *storages = KisStorageModel::instance();
    for (int r = 0; r < storages->rowCount(); ++r) {
        const QString loc = storages->index(r, KisStorageModel::Location).data().toString();
        if (loc.endsWith(QLatin1String(".bundle"), Qt::CaseInsensitive)) {
            bundleStorages.insert(storages->index(r, KisStorageModel::Id).data().toInt());
        }
    }

    // só id e nome agora; cada preset é lido sob demanda quando a linha aparece (como em allBrushItems)
    // pincéis .sut e soltos vêm no topo (mais recentes primeiro), antes dos pacotes .bundle
    QVector<BrushItem> sutItems;
    QVector<BrushItem> looseItems;
    QVector<BrushItem> bundleItems;
    KisResourceModel *all = allPresetsModel();
    for (int row = 0; row < all->rowCount(); ++row) {
        const int id = all->index(row, KisAbstractResourceModel::Id).data().toInt();
        if (!taggedIds.contains(id)) continue;
        const QString filename = all->index(row, KisAbstractResourceModel::Filename).data().toString();
        if (isBundledNanquimFile(filename)) continue;
        BrushItem item;
        item.id = id;
        item.name = all->index(row, KisAbstractResourceModel::Name).data().toString().replace(QLatin1Char('_'), QLatin1Char(' '));
        item.shortName = item.name;
        item.weight = -1;   // ainda não lido
        if (filename.startsWith(QLatin1String("sut_"), Qt::CaseInsensitive)) {
            sutItems << item;
        } else if (!bundleStorages.contains(all->index(row, KisAbstractResourceModel::StorageId).data().toInt())) {
            looseItems << item;
        } else {
            bundleItems << item;
        }
    }
    auto newestFirst = [](const BrushItem &a, const BrushItem &b) { return a.id > b.id; };
    std::stable_sort(sutItems.begin(), sutItems.end(), newestFirst);
    std::stable_sort(looseItems.begin(), looseItems.end(), newestFirst);
    items.reserve(sutItems.size() + looseItems.size() + bundleItems.size());
    items << sutItems << looseItems << bundleItems;
    return items;
}

// todos os pincéis do Krita: só nome e id; o resto é lido quando a linha aparece
QVector<BrushItem> allBrushItems()
{
    QVector<BrushItem> items;
    KisResourceModel *model = allPresetsModel();
    items.reserve(model->rowCount());
    for (int row = 0; row < model->rowCount(); ++row) {
        BrushItem item;
        item.id = model->index(row, KisAbstractResourceModel::Id).data().toInt();
        item.name = model->index(row, KisAbstractResourceModel::Name).data().toString().replace(QLatin1Char('_'), QLatin1Char(' '));
        item.shortName = item.name;
        item.weight = -1;   // ainda não lido
        items << item;
    }
    return items;
}

// traço de amostra, desenhado na hora (sem renderizar o pincel de verdade)
void paintStrokeSample(QPainter &g, const QRectF &r, StrokeStyle style, qreal weight,
                       const QColor &inkColor = QColor(238, 234, 224))
{
    g.save();
    g.setRenderHint(QPainter::Antialiasing);
    const int n = 64;
    auto at = [&r](qreal t) {
        return QPointF(r.left() + t * r.width(),
                       r.center().y() + std::sin(t * 2.0 * M_PI + 2.4) * r.height() * 0.26);
    };
    auto normal = [&at](qreal t) {
        const QPointF a = at(qMax(0.0, t - 0.01)), b = at(qMin(1.0, t + 0.01));
        QPointF d = b - a;
        const qreal len = std::hypot(d.x(), d.y());
        return len > 0 ? QPointF(-d.y() / len, d.x() / len) : QPointF(0, 1);
    };
    const qreal maxW = qMax<qreal>(1.4, r.height() * (0.06 + 0.34 * weight));
    QColor ink = inkColor;

    auto taperedPath = [&](qreal widthScale, qreal minPart, qreal offset) {
        QPolygonF left, right;
        for (int i = 0; i <= n; ++i) {
            const qreal t = qreal(i) / n;
            const qreal w = maxW * widthScale * (minPart + (1.0 - minPart) * std::pow(std::sin(M_PI * t), 0.7)) / 2.0;
            const QPointF c = at(t) + normal(t) * offset;
            left << c + normal(t) * w;
            right.prepend(c - normal(t) * w);
        }
        QPolygonF outline = left;
        outline += right;
        QPainterPath path;
        path.addPolygon(outline);
        path.closeSubpath();
        return path;
    };
    QPainterPath line;
    line.moveTo(at(0));
    for (int i = 1; i <= n; ++i) line.lineTo(at(qreal(i) / n));

    switch (style) {
    case StrokePressure:
    case StrokeThick:
        g.fillPath(taperedPath(1.0, 0.08, 0), ink);
        break;
    case StrokeFine:
        g.fillPath(taperedPath(1.0, 0.45, 0), ink);
        break;
    case StrokeTexture: {
        // pena seca: fios paralelos que falham no fim
        const qreal off[] = {-0.3, 0.0, 0.3};
        for (int k = 0; k < 3; ++k) {
            QColor c = ink;
            c.setAlpha(k == 1 ? 235 : 150);
            g.fillPath(taperedPath(0.45, 0.1, off[k] * maxW), c);
        }
        break;
    }
    case StrokePencil: {
        QRandomGenerator rnd(7);
        QColor c(198, 193, 182);
        for (int i = 0; i < n; ++i) {
            const qreal t = qreal(i) / n;
            c.setAlpha(110 + rnd.bounded(110));
            g.setPen(QPen(c, maxW * (0.55 + 0.35 * std::sin(M_PI * t)), Qt::SolidLine, Qt::RoundCap));
            g.drawLine(at(t), at(t + 1.0 / n));
        }
        break;
    }
    case StrokeFlat:
        g.setPen(QPen(ink, maxW, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        g.drawPath(line);
        break;
    case StrokeSoft:
        for (int k = 5; k >= 1; --k) {
            QColor c = ink;
            c.setAlpha(26 + (5 - k) * 12);
            g.setPen(QPen(c, maxW * (0.35 + 0.18 * k), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            g.drawPath(line);
        }
        break;
    case StrokeBlend:
        for (int i = 0; i < n; ++i) {
            const qreal t = qreal(i) / n;
            QColor c = ink;
            c.setAlpha(int(230 * (1.0 - t) + 25));
            g.setPen(QPen(c, maxW * (1.0 - 0.3 * t), Qt::SolidLine, Qt::RoundCap));
            g.drawLine(at(t), at(t + 1.0 / n));
        }
        break;
    case StrokeDots: {
        g.setPen(Qt::NoPen);
        g.setBrush(ink);
        const int dots = 22;
        for (int i = 0; i <= dots; ++i) {
            const qreal t = qreal(i) / dots;
            const qreal rad = maxW * 0.32 * (0.25 + 0.75 * std::sin(M_PI * t));
            g.drawEllipse(at(t), rad, rad);
        }
        break;
    }
    case StrokeEraser: {
        QColor c(145, 140, 130);
        g.setPen(QPen(c, 1.3));
        g.setBrush(Qt::NoBrush);
        g.drawPath(taperedPath(1.0, 0.5, 0));
        break;
    }
    }
    g.restore();
}


enum BrushRoles {
    BrushIdRole = Qt::UserRole + 40,
    BrushNameRole,
    BrushShortRole,
    BrushSubtitleRole,
    BrushStyleRole,
    BrushWeightRole,
    BrushFavoriteRole,
    BrushRemovableRole,
};

const int BRUSH_MORE_ID = -2;   // quadradinho "+ Pincéis" no fim da coluna

// Favoritos: a mesma etiqueta da paleta rápida do Krita (clique com o botão direito na tela)
KisTagSP favoritesTag()
{
    const QString url = KisConfig(true).readEntry<QString>("favoritePresetsTag", QStringLiteral("★ My Favorites"));
    KisTagModel tagModel(ResourceType::PaintOpPresets);
    KisTagSP tag = tagModel.tagForUrl(url);
    if (!tag && url != QStringLiteral("★ My Favorites")) tag = tagModel.tagForUrl(QStringLiteral("★ My Favorites"));
    if (!tag) tag = tagModel.addTag(QStringLiteral("★ My Favorites"), false, QVector<KoResourceSP>());
    return tag;
}

QVector<KisPaintOpPresetSP> favoritePresets()
{
    QVector<KisPaintOpPresetSP> out;
    KisTagSP tag = favoritesTag();
    if (!tag) return out;
    KisTagResourceModel model(ResourceType::PaintOpPresets);
    model.setTagsFilter(QVector<int>() << tag->id());
    for (int row = 0; row < model.rowCount(); ++row) {
        KisPaintOpPresetSP preset = model.resourceForIndex(model.index(row, 0)).dynamicCast<KisPaintOpPreset>();
        if (preset) out << preset;
    }
    return out;
}

QSet<int> s_favoriteIds;

void refreshFavoriteIds()
{
    s_favoriteIds.clear();
    KisTagSP tag = favoritesTag();
    if (!tag) return;
    KisTagResourceModel model(ResourceType::PaintOpPresets);
    model.setTagsFilter(QVector<int>() << tag->id());
    for (int row = 0; row < model.rowCount(); ++row) {
        const int id = model.index(row, KisAbstractResourceModel::Id).data().toInt();
        if (id >= 0) s_favoriteIds.insert(id);
    }
}

void notifyFavoritesChanged(KisMainWindow *mw)
{
    refreshFavoriteIds();
    KisViewManager *vm = mw->viewManager();
    if (vm && vm->paintOpBox() && vm->paintOpBox()->favoriteResourcesManager()) {
        vm->paintOpBox()->favoriteResourcesManager()->updateFavoritePresets();
    }
    if (QTimer *t = mw->findChild<QTimer*>(QStringLiteral("nanquimBrushDockReload"))) t->start();
    markBrushPanelDirty(mw);
    refreshStudioBrushes(mw);
}

void setFavorite(KisMainWindow *mw, int id, bool on)
{
    KisTagSP tag = favoritesTag();
    if (!tag || id < 0) return;
    KisTagResourceModel model(ResourceType::PaintOpPresets);
    if (on) model.tagResources(tag, QVector<int>() << id);
    else model.untagResources(tag, QVector<int>() << id);
    notifyFavoritesChanged(mw);
}

int s_currentBrushId = -1;
QString s_currentBrushName;
void syncCurrentBrush(KisMainWindow *mw);

bool removeImportedBrushesById(KisMainWindow *mw, const QVector<int> &ids)
{
    if (ids.isEmpty()) return false;
    KisTagModel tagModel(ResourceType::PaintOpPresets);
    if (KisTagSP nqTag = tagModel.tagForUrl(QStringLiteral("Nanquim"))) {
        KisTagResourceModel m(ResourceType::PaintOpPresets);
        m.untagResources(nqTag, ids);
    }
    if (KisTagSP favTag = favoritesTag()) {
        KisTagResourceModel m(ResourceType::PaintOpPresets);
        m.untagResources(favTag, ids);
    }
    QSet<int> idSet;
    for (int id : ids) idSet.insert(id);
    KisResourceModel *resModel = allPresetsModel();
    bool anyRemoved = false;
    for (int row = resModel->rowCount() - 1; row >= 0; --row) {
        const QModelIndex idx = resModel->index(row, KisAbstractResourceModel::Id);
        if (idSet.contains(idx.data().toInt())) {
            if (resModel->setResourceInactive(idx)) anyRemoved = true;
        }
    }
    if (mw && idSet.contains(s_currentBrushId)) {
        const QVector<BrushItem> fallback = nanquimBrushItems(QStringLiteral("Tinta"), false);
        KisViewManager *vm = mw->viewManager();
        if (!fallback.isEmpty() && fallback.first().preset && vm && vm->canvasResourceProvider()) {
            vm->canvasResourceProvider()->setPaintOpPreset(fallback.first().preset);
        }
    }
    if (mw) {
        syncCurrentBrush(mw);
        notifyFavoritesChanged(mw);
    }
    return anyRemoved;
}

bool isCurrentBrush(const QModelIndex &index)
{
    const int id = index.data(BrushIdRole).toInt();
    if (id >= 0 && id == s_currentBrushId) return true;
    return s_currentBrushId < 0 && !s_currentBrushName.isEmpty()
            && index.data(BrushNameRole).toString() == s_currentBrushName;
}

void resolveLazyBrush(const QModelIndex &index)
{
    if (index.data(BrushWeightRole).toReal() >= 0) return;
    QAbstractItemModel *model = const_cast<QAbstractItemModel*>(index.model());
    KisPaintOpPresetSP preset = presetForId(index.data(BrushIdRole).toInt());
    const qreal w = weightForPreset(preset);
    model->setData(index, w, BrushWeightRole);
    const int sizePx = qRound(preset && preset->settings() ? preset->settings()->paintOpSize() : 0);
    if (index.data(BrushRemovableRole).toBool()) {
        model->setData(index, QStringLiteral("Importado · %1 px").arg(sizePx), BrushSubtitleRole);
    } else {
        const QString engine = preset && preset->settings() ? preset->paintOp().name() : QString();
        model->setData(index, preset ? QStringLiteral("%1 · %2 px").arg(engine.isEmpty() ? QStringLiteral("Krita") : engine)
                                             .arg(sizePx)
                                     : QStringLiteral("Krita"),
                       BrushSubtitleRole);
    }
}


// a arte do pincel (a miniatura que vem dentro do preset), a mesma que o Krita
// mostra no seletor de presets. Guardada já em quadrado e no dobro do tamanho.
QImage brushThumb(const KisPaintOpPresetSP &preset)
{
    static QHash<int, QImage> s_cache;
    if (!preset) return QImage();
    const int id = preset->resourceId();
    const auto it = s_cache.constFind(id);
    if (it != s_cache.constEnd()) return it.value();
    QImage img = preset->image();
    if (!img.isNull()) {
        const int side = qMin(img.width(), img.height());
        img = img.copy((img.width() - side) / 2, (img.height() - side) / 2, side, side)
                  .scaled(128, 128, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)
                  .convertToFormat(QImage::Format_ARGB32_Premultiplied);
    }
    // teto: ~96 miniaturas de 128 px (~3 MB); com pacotes grandes, a memória
    // não cresce sem limite (e o primeiro a entrar é o primeiro a sair)
    if (s_cache.size() >= 96) s_cache.clear();
    s_cache.insert(id, img);
    return img;
}

void paintBrushThumb(QPainter &g, const QRectF &r, const QModelIndex &index)
{
    g.save();
    g.setRenderHint(QPainter::Antialiasing);
    g.setRenderHint(QPainter::SmoothPixmapTransform);
    QPainterPath shape;
    shape.addRoundedRect(r, 9, 9);
    g.setClipPath(shape);
    g.fillPath(shape, QColor(0xce, 0xce, 0xce));   // o cinza dos ícones do Krita
    const QImage img = brushThumb(presetForId(index.data(BrushIdRole).toInt()));
    if (!img.isNull()) {
        g.drawImage(r, img);
    } else {
        // preset sem arte: a onda desenhada segura o lugar
        paintStrokeSample(g, r.adjusted(5, 8, -5, -8), StrokeStyle(index.data(BrushStyleRole).toInt()),
                          qMax<qreal>(0.12, index.data(BrushWeightRole).toReal()), QColor(0x3a, 0x39, 0x34));
    }
    g.restore();
}

// ---------------------------------------------------------------------------
// Traço de verdade: o próprio pincel desenha a amostra, fora da tela, pelo
// mesmo caminho da prévia ao vivo do editor de pincéis do Krita. Um traço de
// cada vez, com uma pausa entre eles para a lista continuar rolando; o
// resultado fica guardado. Enquanto não fica pronto, a amostra desenhada à
// mão segura o lugar.
// ---------------------------------------------------------------------------
QPointer<KisMainWindow> s_strokeMw;
struct StrokeJob { QString key; int id; QSize px; QColor ink; };
QVector<StrokeJob> s_strokeJobs;
QHash<QString, QImage> s_strokeCache;
QSet<QString> s_strokeFailed;
QList<QPointer<QWidget>> s_strokeViewers;

QImage renderPresetStroke(KisMainWindow *mw, const KisPaintOpPresetSP &preset, const QSize &px, const QColor &ink)
{
    KoCanvasResourceProvider *rm = resources(mw);
    if (!rm || !preset || !preset->settings() || px.isEmpty()) return QImage();
    const QString op = preset->paintOp().id();
    // os mesmos que a prévia do Krita não desenha, e os que precisam de algo por baixo
    if (op == QLatin1String("roundmarker") || op == QLatin1String("experimentbrush")
            || op == QLatin1String("duplicate") || op == QLatin1String("filter")
            || op == QLatin1String("colorsmudge") || op == QLatin1String("deformbrush")) {
        return QImage();
    }
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, px.width(), px.height(), cs, QStringLiteral("nanquim stroke preview"));
    KisPaintLayerSP layer = new KisPaintLayer(image, QStringLiteral("preview"), OPACITY_OPAQUE_U8, cs);

    KisPaintOpPresetSP proxy = preset->clone().dynamicCast<KisPaintOpPreset>();
    if (!proxy || !proxy->settings()) return QImage();
    KisPaintOpSettingsSP settings = proxy->settings();
    // grosso demais não cabe na faixa; fino demais some
    settings->setPaintOpSize(qBound<qreal>(2.0, settings->paintOpSize(), px.height() * 0.42));
    const int textureOffset = qMax(settings->getInt(QStringLiteral("Texture/Pattern/MaximumOffsetX")),
                                   settings->getInt(QStringLiteral("Texture/Pattern/MaximumOffsetY"))) * 2;
    const double textureScale = settings->getDouble(QStringLiteral("Texture/Pattern/Scale"));
    if (textureOffset > 0 && textureOffset * textureScale > 200) {
        settings->setProperty(QStringLiteral("Texture/Pattern/Scale"), 200.0 / textureOffset);
    }
    proxy->setSettings(settings);

    KisResourcesSnapshotSP snapshot = new KisResourcesSnapshot(image, layer, rm, 0, KisNodeList(), proxy);
    snapshot->setOpacity(settings->paintOpOpacity());
    snapshot->setMirroring(false, false);
    snapshot->setFGColorOverride(KoColor(ink, cs));
    KisFreehandStrokeInfo *info = new KisFreehandStrokeInfo();
    KisStrokeId id = image->startStroke(new FreehandStrokeStrategy(snapshot, info, kundo2_noi18n("nanquim_preview")));

    // um S deitado: começa leve, engrossa no meio e afina no fim
    const qreal w = px.width(), h = px.height();
    KisPaintInformation a, b, c;
    a.setPos(QPointF(w * 0.06, h * 0.66));
    a.setPressure(0.05);
    a.setCurrentTime(0);
    b.setPos(QPointF(w * 0.50, h * 0.46));
    b.setPressure(1.0);
    b.setCurrentTime(320);
    c.setPos(QPointF(w * 0.94, h * 0.34));
    c.setPressure(0.05);
    c.setCurrentTime(640);
    image->addJob(id, new FreehandStrokeStrategy::Data(0, a, QPointF(w * 0.20, h * 0.36), QPointF(w * 0.34, h * 0.50), b));
    image->addJob(id, new FreehandStrokeStrategy::Data(0, b, QPointF(w * 0.66, h * 0.42), QPointF(w * 0.80, h * 0.30), c));
    image->addJob(id, new KisAsynchronousStrokeUpdateHelper::UpdateData(true));
    image->endStroke(id);
    image->waitForDone();

    QImage out = layer->paintDevice()->convertToQImage(0, image->bounds());
    // nada desenhado (pincel que depende do que há por baixo): fica a amostra
    bool any = false;
    for (int y = 0; y < out.height() && !any; y += 2) {
        const QRgb *line = reinterpret_cast<const QRgb*>(out.constScanLine(y));
        for (int x = 0; x < out.width(); x += 2) {
            if (qAlpha(line[x]) > 24) { any = true; break; }
        }
    }
    return any ? out : QImage();
}

void runStrokeQueue()
{
    if (s_strokeJobs.isEmpty()) return;
    const StrokeJob job = s_strokeJobs.takeFirst();
    if (!s_strokeCache.contains(job.key) && !s_strokeFailed.contains(job.key)) {
        KisPaintOpPresetSP preset = presetForId(job.id);
        const QImage img = (s_strokeMw && preset) ? renderPresetStroke(s_strokeMw, preset, job.px, job.ink) : QImage();
        if (img.isNull()) {
            s_strokeFailed.insert(job.key);
        } else {
            if (s_strokeCache.size() >= 160) s_strokeCache.clear();   // teto de memória
            s_strokeCache.insert(job.key, img);
            QList<QPointer<QWidget>> alive;
            for (const QPointer<QWidget> &v : s_strokeViewers) {
                if (v) {
                    v->update();
                    alive << v;
                }
            }
            s_strokeViewers = alive;
        }
    }
    if (!s_strokeJobs.isEmpty()) QTimer::singleShot(24, qApp, []() { runStrokeQueue(); });
}

// o traço pronto, ou nulo enquanto ele é desenhado (quem pediu é avisado depois)
// interruptor: o traço de verdade fica desligado até a queda ao abrir páginas
// (build 77) ser explicada; sem ele, a amostra desenhada à mão segura o lugar
const bool NANQUIM_LIVE_STROKES = false;
// build 82: a tela de desenho está sendo refeita do zero. Com isto desligado, as
// peças flutuantes (estúdio) nem são criadas e o desenho abre com a barra do topo
// e a barra lateral, que funcionavam na build 74.
const bool NANQUIM_ESTUDIO = false;

QImage strokePreview(const KisPaintOpPresetSP &preset, const QSizeF &logical, const QColor &ink, QWidget *viewer)
{
    if (!NANQUIM_LIVE_STROKES) return QImage();
    if (!preset || !s_strokeMw) return QImage();
    const qreal dpr = qApp->devicePixelRatio();
    const QSize px(qRound(logical.width() * dpr), qRound(logical.height() * dpr));
    if (px.width() < 8 || px.height() < 8) return QImage();
    const QString key = QStringLiteral("%1:%2:%3x%4:%5").arg(preset->resourceId()).arg(preset->version())
            .arg(px.width()).arg(px.height()).arg(ink.rgba());
    const auto it = s_strokeCache.constFind(key);
    if (it != s_strokeCache.constEnd()) return it.value();
    if (s_strokeFailed.contains(key)) return QImage();
    if (viewer && !s_strokeViewers.contains(QPointer<QWidget>(viewer))) s_strokeViewers << QPointer<QWidget>(viewer);
    for (const StrokeJob &j : s_strokeJobs) {
        if (j.key == key) return QImage();
    }
    const bool idle = s_strokeJobs.isEmpty();
    s_strokeJobs << StrokeJob{key, preset->resourceId(), px, ink};
    if (idle) QTimer::singleShot(0, qApp, []() { runStrokeQueue(); });
    return QImage();
}

// linha da lista (modo Tablet)
class BrushRowDelegate : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    QSize sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const override
    {
        const QListView *view = qobject_cast<const QListView*>(parent());
        return QSize(view ? view->viewport()->width() : 320, 70);
    }
    void paint(QPainter *g, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        resolveLazyBrush(index);
        g->save();
        g->setRenderHint(QPainter::Antialiasing);
        const QRectF card = QRectF(option.rect).adjusted(0, 3, -1, -3);
        if (isCurrentBrush(index)) {
            g->setPen(QPen(QColor(62, 142, 126), 1.4));
            g->setBrush(QColor(42, 61, 56));
            g->drawRoundedRect(card.adjusted(0.7, 0.7, -0.7, -0.7), 10, 10);
        } else if (option.state & QStyle::State_Sunken) {
            g->setPen(Qt::NoPen);
            g->setBrush(QColor(58, 57, 52));
            g->drawRoundedRect(card, 10, 10);
        } else if (parent() && parent()->property("nanquimHoverRow").toInt() == index.row()
                   && parent()->property("nanquimHoverRows").toBool()) {
            g->setPen(Qt::NoPen);
            g->setBrush(QColor(255, 255, 255, 14));
            g->drawRoundedRect(card, 10, 10);
        }
        // arte do pincel à esquerda, nome no meio, traço de amostra e estrela à direita
        const qreal thumbS = card.height() - 12;
        paintBrushThumb(*g, QRectF(card.left() + 8, card.center().y() - thumbS / 2, thumbS, thumbS), index);
        const qreal textL = card.left() + 8 + thumbS + 10;
        const bool removable = index.data(BrushRemovableRole).toBool();
        const qreal rightPad = removable ? 136.0 : 92.0;
        const bool showSample = !removable || NANQUIM_LIVE_STROKES;
        const qreal plateW = showSample ? qMin<qreal>(removable ? 116.0 : 176.0, card.width() * (removable ? 0.26 : 0.4)) : 0.0;
        const QRectF sample(card.right() - rightPad - plateW, card.center().y() - 22, plateW, 44);
        const qreal textW = qMax<qreal>(60.0, (showSample ? sample.left() : (card.right() - rightPad)) - 8 - textL);
        QFont nameFont = option.font;
        nameFont.setPixelSize(15);
        nameFont.setWeight(QFont::DemiBold);
        QFont subFont = option.font;
        subFont.setPixelSize(12);
        const QRectF nameRect(textL, card.top() + 12, textW, 22);
        const QRectF subRect(textL, card.top() + 34, textW, 18);
        g->setFont(nameFont);
        g->setPen(QColor(238, 234, 224));
        g->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter,
                    QFontMetrics(nameFont).elidedText(index.data(BrushNameRole).toString(), Qt::ElideRight, int(nameRect.width())));
        g->setFont(subFont);
        g->setPen(QColor(145, 140, 130));
        g->drawText(subRect, Qt::AlignLeft | Qt::AlignVCenter,
                    QFontMetrics(subFont).elidedText(index.data(BrushSubtitleRole).toString(), Qt::ElideRight, int(subRect.width())));
        if (showSample) {
            // o traço sobre um pedaço de papel: desenhado pelo próprio pincel
            g->setPen(Qt::NoPen);
            g->setBrush(QColor(0xf3, 0xee, 0xe3));
            g->drawRoundedRect(sample, 8, 8);
            const QListView *listView = qobject_cast<const QListView*>(parent());
            const QImage real = strokePreview(presetForId(index.data(BrushIdRole).toInt()), sample.size(),
                                              QColor(0x1b, 0x1a, 0x18), listView ? listView->viewport() : nullptr);
            if (!real.isNull()) {
                g->drawImage(sample, real);
            } else {
                paintStrokeSample(*g, sample.adjusted(8, 6, -8, -6), StrokeStyle(index.data(BrushStyleRole).toInt()),
                                  qMax<qreal>(0.12, index.data(BrushWeightRole).toReal()), QColor(0x1b, 0x1a, 0x18));
            }
        }
        // lixeira (só nos pincéis importados) + engrenagem de ajustes
        if (index.data(BrushIdRole).toInt() >= 0) {
            if (removable) {
                static QPixmap trashIcon;
                if (trashIcon.isNull()) trashIcon = svgPixmap(QString::fromUtf8(Svg::TRASH), QColor(217, 118, 108), 20);
                const QPointF tc(card.right() - 112, card.center().y());
                g->drawPixmap(QPointF(tc.x() - 10, tc.y() - 10), trashIcon);
            }
            static QPixmap gear;
            if (gear.isNull()) gear = svgPixmap(QString::fromUtf8(Svg::SETTINGS), QColor(168, 163, 152), 22);
            const QPointF gc(card.right() - (removable ? 60 : 68), card.center().y());
            g->drawPixmap(QPointF(gc.x() - 11, gc.y() - 11), gear);
        }
        // estrela: toca para pôr ou tirar dos favoritos
        const bool fav = index.data(BrushFavoriteRole).toBool();
        const QPointF c(card.right() - 20, card.center().y());
        QPolygonF star;
        for (int i = 0; i < 10; ++i) {
            const qreal a = -M_PI / 2 + i * M_PI / 5;
            const qreal rad = (i % 2 == 0) ? 8.5 : 3.8;
            star << c + QPointF(std::cos(a) * rad, std::sin(a) * rad);
        }
        g->setPen(QPen(fav ? QColor(111, 191, 173) : QColor(122, 118, 109), 1.4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        g->setBrush(fav ? QColor(111, 191, 173) : QColor(Qt::transparent));
        g->drawPolygon(star);
        g->restore();
    }
};

// quadradinho da coluna (modo Clássico)
class BrushTileDelegate : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    QSize sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const override
    {
        const QListView *view = qobject_cast<const QListView*>(parent());
        return QSize(qMax(64, view ? view->viewport()->width() : 76), 84);
    }
    void paint(QPainter *g, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        resolveLazyBrush(index);
        g->save();
        g->setRenderHint(QPainter::Antialiasing);
        const QRectF tile = QRectF(option.rect).adjusted(5, 4, -5, -4);
        if (index.data(BrushIdRole).toInt() == BRUSH_MORE_ID) {
            QPen dash(QColor(84, 81, 74), 1.2, Qt::DashLine);
            g->setPen(dash);
            g->setBrush(Qt::NoBrush);
            g->drawRoundedRect(tile.adjusted(0.5, 0.5, -0.5, -0.5), 9, 9);
            QFont plus = option.font;
            plus.setPixelSize(22);
            g->setFont(plus);
            g->setPen(QColor(168, 163, 152));
            g->drawText(QRectF(tile.left(), tile.top() + 4, tile.width(), tile.height() * 0.5), Qt::AlignCenter, QStringLiteral("+"));
            QFont f = option.font;
            f.setPixelSize(11);
            g->setFont(f);
            g->drawText(QRectF(tile.left() + 3, tile.bottom() - 22, tile.width() - 6, 18), Qt::AlignCenter, i18n("Pincéis"));
            g->restore();
            return;
        }
        const bool current = isCurrentBrush(index);
        g->setPen(QPen(current ? QColor(62, 142, 126) : QColor(74, 72, 66), current ? 1.4 : 1.0));
        g->setBrush(current ? QColor(42, 61, 56) : QColor(52, 51, 46));
        g->drawRoundedRect(tile.adjusted(0.5, 0.5, -0.5, -0.5), 9, 9);
        const qreal thumbS = qMin(tile.width() - 14, tile.height() - 30);
        paintBrushThumb(*g, QRectF(tile.center().x() - thumbS / 2, tile.top() + 6, thumbS, thumbS), index);
        QFont f = option.font;
        f.setPixelSize(11);
        g->setFont(f);
        g->setPen(current ? QColor(238, 234, 224) : QColor(198, 193, 182));
        const QRectF label(tile.left() + 3, tile.bottom() - 22, tile.width() - 6, 18);
        g->drawText(label, Qt::AlignCenter,
                    QFontMetrics(f).elidedText(index.data(BrushShortRole).toString(), Qt::ElideRight, int(label.width())));
        g->restore();
    }
};

void fillBrushModel(QStandardItemModel *model, const QVector<BrushItem> &items, bool removable = false)
{
    model->clear();
    for (const BrushItem &b : items) {
        QStandardItem *it = new QStandardItem(b.name);
        it->setEditable(false);
        it->setData(b.id, BrushIdRole);
        it->setData(b.name, BrushNameRole);
        it->setData(b.shortName, BrushShortRole);
        it->setData(b.subtitle, BrushSubtitleRole);
        it->setData(int(b.style), BrushStyleRole);
        it->setData(b.weight, BrushWeightRole);
        it->setData(s_favoriteIds.contains(b.id), BrushFavoriteRole);
        it->setData(removable, BrushRemovableRole);
        model->appendRow(it);
    }
}

void selectBrush(KisMainWindow *mw, const QModelIndex &index)
{
    // lido de novo do banco: o pincel pode ter sido salvo com ajustes novos
    const int id = index.data(BrushIdRole).toInt();
    KisPaintOpPresetSP preset = id >= 0 ? allPresetsModel()->resourceForId(id).dynamicCast<KisPaintOpPreset>() : KisPaintOpPresetSP();
    if (!preset) {
        // pincéis do pacote: o id pode ter mudado; procura pelo nome
        KisPaintOpPresetResourceServer *server = KisResourceServerProvider::instance()->paintOpPresetServer();
        preset = server->resource(QString(), QString(), index.data(BrushNameRole).toString());
    }
    KisViewManager *vm = mw->viewManager();
    if (!preset || !vm || !vm->canvasResourceProvider()) return;
    vm->canvasResourceProvider()->setPaintOpPreset(preset);
}

void syncCurrentBrush(KisMainWindow *mw)
{
    KisViewManager *vm = mw->viewManager();
    KisPaintOpPresetSP cur = vm && vm->canvasResourceProvider() ? vm->canvasResourceProvider()->currentPreset() : KisPaintOpPresetSP();
    s_currentBrushId = cur ? cur->resourceId() : -1;
    s_currentBrushName = cur ? cur->name() : QString();
}

QListView *makeBrushList(QWidget *parent, bool tiles)
{
    QListView *list = new QListView(parent);
    list->setModel(new QStandardItemModel(list));
    list->setItemDelegate(tiles ? static_cast<QStyledItemDelegate*>(new BrushTileDelegate(list))
                                : static_cast<QStyledItemDelegate*>(new BrushRowDelegate(list)));
    list->setFrameShape(QFrame::NoFrame);
    list->setSelectionMode(QAbstractItemView::NoSelection);
    list->setFocusPolicy(Qt::NoFocus);
    list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    list->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    list->setUniformItemSizes(true);
    list->setSpacing(0);
    list->setResizeMode(QListView::Adjust);
    list->viewport()->setAutoFillBackground(false);
    list->setStyleSheet(QStringLiteral("QListView { background: transparent; border: none; }"));
    list->setProperty("nanquimHoverRows", !tiles);   // pairar da S Pen destaca a linha
    KisKineticScroller::createPreconfiguredScroller(list);
    return list;
}

// cópia local de um arquivo vindo do seletor do Android (content://)
QString localCopyOf(const QString &file)
{
    if (!file.startsWith(QLatin1String("content:"))) {
        return file.startsWith(QLatin1String("file://")) ? QUrl(file).toLocalFile() : file;
    }
    QString name = QUrl::fromPercentEncoding(file.toUtf8());
    name = name.mid(name.lastIndexOf(QLatin1Char('/')) + 1);
    name = name.mid(name.lastIndexOf(QLatin1Char(':')) + 1);
    if (name.isEmpty()) name = QStringLiteral("pincel");
    QDir dir(QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
    dir.mkpath(QStringLiteral("imports"));
    const QString dst = dir.filePath(QStringLiteral("imports/") + name);
    QFile src(file);
    if (!src.open(QIODevice::ReadOnly)) return file;
    QFile::remove(dst);
    QFile out(dst);
    if (!out.open(QIODevice::WriteOnly)) return file;
    out.write(src.readAll());
    return dst;
}

void importBrushFiles(KisMainWindow *mw)
{
    KoFileDialog dialog(mw, KoFileDialog::OpenFiles, QStringLiteral("NanquimBrushImport"));
    dialog.setCaption(i18n("Importar pincéis (.kpp, .bundle, .sut, .zip)"));
#ifndef Q_OS_ANDROID
    dialog.setNameFilter(i18n("Pincéis (*.kpp *.bundle *.sut *.zip)"));
#endif
    const QStringList files = dialog.filenames();
    QStringList rejected;
    int okCount = 0;
    Q_FOREACH (const QString &f, files) {
        const QString local = localCopyOf(f);
        if (importBrushFileQuietly(mw, local, nullptr)) ++okCount;
        else rejected << QFileInfo(local).fileName();
    }
    if (files.isEmpty()) return;
    markBrushPanelDirty(mw);
    if (rejected.isEmpty()) {
        showToast(mw, i18n("%1 importado(s) · Pincéis → Importados", okCount));
    } else {
        showToast(mw, i18n("Não deu para importar: %1", rejected.join(QStringLiteral(", "))));
    }
}

// "+": guarda o pincel atual, com os ajustes que estão valendo, como um pincel novo
bool saveCurrentAsNewBrush(KisMainWindow *mw)
{
    KisViewManager *vm = mw->viewManager();
    KisPaintOpPresetSP cur = vm && vm->canvasResourceProvider() ? vm->canvasResourceProvider()->currentPreset() : KisPaintOpPresetSP();
    if (!cur) return false;
    bool ok = false;
    const QString name = QInputDialog::getText(mw, i18n("Novo pincel"),
                                               i18n("Guarda o pincel atual, com os ajustes de agora, com outro nome:"),
                                               QLineEdit::Normal, cur->name().replace(QLatin1Char('_'), QLatin1Char(' ')) + i18n(" (meu)"), &ok).trimmed();
    if (!ok || name.isEmpty()) return false;
    KisPaintOpPresetSP preset = cur->clone().dynamicCast<KisPaintOpPreset>();
    QString file = name;
    file.replace(QLatin1Char(' '), QLatin1Char('_')).replace(QLatin1Char('.'), QLatin1Char('_'));
    preset->setResourceId(-1);
    preset->setFilename(file + preset->defaultFileExtension());
    preset->setName(name);
    preset->setValid(true);
    preset->setStorageLocation(QString());
    if (!KisResourceUserOperations::addResourceWithUserInput(mw, preset)) return false;
    KisTagModel tagModel(ResourceType::PaintOpPresets);
    KisTagSP tag = tagModel.tagForUrl(QStringLiteral("Nanquim"));
    if (tag && preset->resourceId() >= 0) {
        KisTagResourceModel tagResources(ResourceType::PaintOpPresets);
        tagResources.tagResources(tag, QVector<int>() << preset->resourceId());
    }
    vm->canvasResourceProvider()->setPaintOpPreset(preset);
    markBrushPanelDirty(mw);
    return true;
}

void openBrushEditor(KisMainWindow *mw)
{
    KisViewManager *vm = mw->viewManager();
    if (!vm || !vm->paintOpBox()) return;
    // o editor do Krita abre preso ao botão dele, que no modo Tablet fica escondido:
    // a animação de entrada o coloca embaixo da barra, à direita
    if (QToolBar *top = mw->findChild<QToolBar*>(TOP_ID)) {
        qApp->setProperty("nanquimEditorAnchor", top->mapToGlobal(QPoint(top->width() - 12, top->height() + 6)));
    }
    QMetaObject::invokeMethod(static_cast<QObject*>(vm->paintOpBox()), "togglePresetEditor");
}

const QString BRUSH_PANEL_ID = QStringLiteral("nanquimBrushPanel");
const QString BRUSH_DOCK_ID = QStringLiteral("NanquimBrushDock");
QPointer<NanquimOverlay> s_brushOverlay;

const char *BRUSH_PANEL_STYLE =
    "QFrame#nanquimBrushPanel { background: " NQ_RELEVO3_BG "; border: 1px solid " NQ_RELEVO3_BORDA "; border-radius: 16px; }"
    "QLabel#nanquimBrushTitle { color: #a8a398; font-size: 13px; font-weight: 700; letter-spacing: 2px; background: transparent; }"
    "QToolButton#nanquimBrushAdd { color: #eeeae0; background: transparent; border: none; border-radius: 10px;"
    "  font-size: 24px; padding: 0px 8px 3px 8px; }"
    "QToolButton#nanquimBrushAdd:pressed { background: #4a4842; }"
    "QPushButton.nanquimChip { color: #eeeae0; background: #34332e; border: 1px solid #4a4842; border-radius: 15px;"
    "  font-size: 13px; font-weight: 600; padding: 6px 14px; min-height: 18px; }"
    "QPushButton.nanquimChip:checked { color: #ffffff; border: none;"
    "  background: #2f7568; }"
    "QPushButton.nanquimTab { background: #34332e; border: 1px solid #4a4842; border-radius: 12px; padding: 0px; }"
    "QPushButton.nanquimTab:checked { border: none;"
    "  background: #2f7568; }"
    "QPushButton.nanquimTab:pressed { background: #4a4842; }"
    "QPushButton.nanquimBrushAction { color: #eeeae0; background: #34332e; border: 1px solid #4a4842; border-radius: 12px;"
    "  font-size: 14px; font-weight: 600; padding: 12px 10px; }"
    "QPushButton.nanquimBrushAction:pressed { background: #4a4842; border-color: #3e8e7e; }"
    "QLineEdit#nanquimBrushSearch { background: #24231f; color: #eeeae0; border: 1px solid #4a4842;"
    "  border-radius: 11px; padding: 0 12px; min-height: 44px; font-size: 14px; }"
    "QLineEdit#nanquimBrushSearch:focus { border: 1.5px solid #3e8e7e; }";

void setBrushPanelVisible(KisMainWindow *mw, bool visible);

void placeBrushPanel(KisMainWindow *mw, QFrame *panel)
{
    for (const char *railId : {"nanquimStudioRail", "nanquimFocusBar"}) {
        QWidget *rail = mw->findChild<QWidget*>(QLatin1String(railId));
        if (rail && rail->isVisible()) {
            // coluna lateral ou barra unificada do modo foco: abre logo ao lado
            const QRect r(rail->mapTo(mw, QPoint(0, 0)), rail->size());
            const int w = qMin(440, mw->width() - r.width() - 48);
            const int h = qMax(qMin(420, mw->height() - r.top() - 12), r.height());
            const int x = railOnRight() ? r.left() - 12 - w : r.right() + 13;
            panel->setGeometry(x, r.top(), w, qMin(h, mw->height() - r.top() - 12));
            return;
        }
    }
    QToolBar *top = mw->findChild<QToolBar*>(TOP_ID);
    QToolButton *button = mw->findChild<QToolButton*>(QStringLiteral("nanquimBrushButton"));
    if (top && button && button->isVisible()) {
        const int topBottom = top->mapTo(mw, QPoint(0, top->height())).y();
        const int w = qMin(460, mw->width() - 24);
        const int h = qMin(640, mw->height() - topBottom - 24);
        const QRect a(button->mapTo(mw, QPoint(0, 0)), button->size());
        const int x = qBound(12, a.center().x() - w / 2, mw->width() - 12 - w);
        panel->setGeometry(x, topBottom + 8, w, h);
        return;
    }
    const int topBottom = top ? top->mapTo(mw, QPoint(0, top->height())).y()
                              : (mw->centralWidget() ? mw->centralWidget()->mapTo(mw, QPoint(0, 0)).y() : 60);
    const int w = qMin(460, mw->width() - 24);
    const int h = qMin(640, mw->height() - topBottom - 24);
    int right = mw->width() - 14;
    if (!top) {
        // modo Clássico: abre ao lado da coluna de pincéis
        if (QDockWidget *dock = mw->findChild<QDockWidget*>(BRUSH_DOCK_ID)) {
            if (dock->isVisible()) right = dock->mapTo(mw, QPoint(0, 0)).x() - 8;
        }
    }
    int x = qMax(8, right - w);
    if (top && railOnRight()) {
        // barra à direita: a janela de pincéis abre do outro lado
        QWidget *central = mw->centralWidget();
        x = (central ? central->mapTo(mw, QPoint(0, 0)).x() : 0) + 14;
    }
    panel->setGeometry(x, topBottom + 8, w, h);
}

void buildBrushPanel(KisMainWindow *mw)
{
    s_strokeMw = mw;
    QFrame *panel = new QFrame(mw);
    panel->setObjectName(BRUSH_PANEL_ID);
    panel->setStyleSheet(QLatin1String(BRUSH_PANEL_STYLE));
    panel->hide();

    QVBoxLayout *root = new QVBoxLayout(panel);
    root->setContentsMargins(16, 14, 14, 14);
    root->setSpacing(10);

    QHBoxLayout *head = new QHBoxLayout();
    QLabel *title = new QLabel(i18n("PINCÉIS"), panel);
    title->setObjectName(QStringLiteral("nanquimBrushTitle"));
    QToolButton *add = new QToolButton(panel);
    add->setObjectName(QStringLiteral("nanquimBrushAdd"));
    add->setText(QStringLiteral("+"));
    add->setToolTip(i18n("Guardar o pincel atual como um pincel novo"));
    add->setFocusPolicy(Qt::NoFocus);
    QPushButton *clearImported = new QPushButton(i18n("Limpar"), panel);
    clearImported->setFocusPolicy(Qt::NoFocus);
    clearImported->setStyleSheet(QStringLiteral(
        "QPushButton { color: #e28b82; background: transparent; border: 1px solid #593533; "
        "border-radius: 10px; font-size: 12px; font-weight: 600; padding: 6px 10px; }"
        "QPushButton:pressed { background: #593533; color: #ffffff; }"));
    clearImported->hide();
    head->addWidget(title);
    head->addStretch(1);
    head->addWidget(clearImported);
    head->addWidget(add);
    root->addLayout(head);

    QListView *list = makeBrushList(panel, false);
    QStandardItemModel *model = static_cast<QStandardItemModel*>(list->model());

    // categorias por ícone: o nome aparece no título da janela
    struct Tab { QString category; const char *icon; };
    const QVector<Tab> tabs = {
        {QStringLiteral("Tinta"), Svg::NIB}, {QStringLiteral("Lápis"), Svg::PEN},
        {QStringLiteral("Pintura"), Svg::BRUSH}, {QStringLiteral("Retícula"), Svg::ALL_TOOLS},
        {BRUSH_FAVORITES, Svg::STAR}, {BRUSH_IMPORTED, Svg::IMPORT}, {BRUSH_ALL, Svg::LIST}};
    QButtonGroup *group = new QButtonGroup(panel);
    group->setExclusive(true);
    QHBoxLayout *tabRow = new QHBoxLayout();
    tabRow->setSpacing(6);
    const QString saved = KSharedConfig::openConfig()->group("Nanquim").readEntry("brushCategory", QStringLiteral("Tinta"));
    for (int i = 0; i < tabs.size(); ++i) {
        QPushButton *tab = new QPushButton(panel);
        tab->setProperty("class", QStringLiteral("nanquimTab"));
        tab->setFixedSize(44, 44);
        tab->setIconSize(QSize(22, 22));
        const QString body = QString::fromUtf8(tabs[i].icon);
        QIcon icon;
        icon.addPixmap(svgPixmap(body, C_TEXT, 22), QIcon::Normal, QIcon::Off);
        icon.addPixmap(svgPixmap(body, QColor(0x13, 0x23, 0x20), 22), QIcon::Normal, QIcon::On);
        tab->setIcon(icon);
        tab->setCheckable(true);
        tab->setFocusPolicy(Qt::NoFocus);
        tab->setToolTip(tabs[i].category);
        tab->setAccessibleName(tabs[i].category);
        tab->setProperty("nanquimCategory", tabs[i].category);
        tab->setChecked(tabs[i].category == saved);
        group->addButton(tab, i);
        tabRow->addWidget(tab);
    }
    if (!group->checkedButton()) group->button(0)->setChecked(true);
    tabRow->addStretch(1);
    root->addLayout(tabRow);

    QLineEdit *searchBox = new QLineEdit(panel);
    searchBox->setObjectName(QStringLiteral("nanquimBrushSearch"));
    searchBox->setPlaceholderText(i18n("Buscar pincel…"));
    searchBox->setClearButtonEnabled(true);
    root->addWidget(searchBox);
    root->addWidget(list, 1);

    QLabel *empty = new QLabel(panel);
    empty->setWordWrap(true);
    empty->setAlignment(Qt::AlignCenter);
    empty->setStyleSheet(QStringLiteral("color: #918c82; font-size: 13px; background: transparent; padding: 24px;"));
    empty->setText(i18n("Nenhum pincel importado ainda.\nToque em \"Importar pincéis\" para trazer arquivos .kpp, .bundle ou .sut, "
                        "ou em + para guardar o pincel atual."));
    empty->hide();
    root->addWidget(empty, 1);

    QHBoxLayout *actions = new QHBoxLayout();
    actions->setSpacing(10);
    QPushButton *settings = new QPushButton(i18n("Ajustes do pincel"), panel);
    QPushButton *importButton = new QPushButton(i18n("Importar pincéis"), panel);
    for (QPushButton *b : {settings, importButton}) {
        b->setProperty("class", QStringLiteral("nanquimBrushAction"));
        b->setFocusPolicy(Qt::NoFocus);
        actions->addWidget(b, 1);
    }
    root->addLayout(actions);

    QPushButton *communityButton = new QPushButton(i18n("Catálogo da comunidade (pincéis e retículas)"), panel);
    communityButton->setProperty("class", QStringLiteral("nanquimBrushAction"));
    communityButton->setFocusPolicy(Qt::NoFocus);
    communityButton->setStyleSheet(QStringLiteral(
        "QPushButton { background: #253d37; border: 1px solid #3e8e7e; color: #d8f2eb; font-weight: 600; }"
        "QPushButton:pressed { background: #2f7568; color: #ffffff; }"));
    root->addWidget(communityButton);

    auto cachedItems = std::make_shared<QVector<BrushItem>>();
    auto cachedCat = std::make_shared<QString>();
    auto applyFilter = [model, list, empty, clearImported, searchBox, cachedItems, cachedCat]() {
        const QString cat = *cachedCat;
        const bool isImportedTab = (cat == BRUSH_IMPORTED);
        const QString q = searchBox->text().trimmed().toLower();
        QVector<BrushItem> shown;
        if (q.isEmpty()) {
            shown = *cachedItems;
        } else {
            shown.reserve(cachedItems->size());
            for (const BrushItem &b : *cachedItems) {
                if (b.name.toLower().contains(q) || b.subtitle.toLower().contains(q)) {
                    shown << b;
                }
            }
        }
        fillBrushModel(model, shown, isImportedTab);
        clearImported->setVisible(isImportedTab && !cachedItems->isEmpty());
        const bool showEmpty = shown.isEmpty() && (!q.isEmpty() || isImportedTab || cat == BRUSH_FAVORITES);
        if (!q.isEmpty() && shown.isEmpty()) {
            empty->setText(i18n("Nenhum pincel encontrado para \"%1\".", searchBox->text().trimmed()));
        } else {
            empty->setText(cat == BRUSH_FAVORITES
                           ? i18n("Nenhum favorito ainda.\nToque na estrela ao lado de um pincel para colocá-lo aqui "
                                  "e na coluna do modo Clássico.")
                           : i18n("Nenhum pincel importado ainda.\nToque em \"Importar pincéis\" para trazer arquivos .kpp, .bundle ou .sut, "
                                  "ou em + para guardar o pincel atual."));
        }
        list->setVisible(!showEmpty);
        empty->setVisible(showEmpty);
    };
    auto reload = [group, title, cachedItems, cachedCat, applyFilter]() {
        QAbstractButton *chip = group->checkedButton();
        const QString cat = chip ? chip->property("nanquimCategory").toString() : QStringLiteral("Tinta");
        title->setText(i18n("PINCÉIS") + QStringLiteral(" · ") + cat.toUpper());
        *cachedCat = cat;
        QVector<BrushItem> items;
        refreshFavoriteIds();
        const bool isImportedTab = (cat == BRUSH_IMPORTED);
        if (isImportedTab) items = importedBrushItems();
        else if (cat == BRUSH_ALL) items = allBrushItems();
        else if (cat == BRUSH_FAVORITES) {
            for (const KisPaintOpPresetSP &p : favoritePresets()) items << itemForPreset(p);
        }
        else items = nanquimBrushItems(cat, false);
        *cachedItems = items;
        applyFilter();
    };
    QObject::connect(searchBox, &QLineEdit::textChanged, panel, [applyFilter](const QString &) { applyFilter(); });
    QObject::connect(group, &QButtonGroup::idClicked, panel, [group, reload](int) {
        if (QAbstractButton *chip = group->checkedButton()) {
            KSharedConfig::openConfig()->group("Nanquim").writeEntry("brushCategory", chip->property("nanquimCategory").toString());
        }
        reload();
    });
    // só recarrega o modelo quando algo mudou (evita reconstruir a lista 2x a cada abertura)
    panel->setProperty("nanquimDirty", true);
    QTimer *warmTrigger = new QTimer(panel);
    warmTrigger->setObjectName(QStringLiteral("nanquimBrushWarmup"));
    warmTrigger->setSingleShot(true);
    warmTrigger->setInterval(0);
    QObject::connect(warmTrigger, &QTimer::timeout, panel, [mw, panel, reload]() {
        if (!panel->property("nanquimDirty").toBool()) return;
        syncCurrentBrush(mw);
        reload();
        panel->setProperty("nanquimDirty", false);
    });
    class ShowReload : public QObject {
    public:
        ShowReload(QObject *parent, std::function<void()> f) : QObject(parent), m_f(std::move(f)) {}
        bool eventFilter(QObject *, QEvent *e) override { if (e->type() == QEvent::Show) m_f(); return false; }
    private:
        std::function<void()> m_f;
    };
    panel->installEventFilter(new ShowReload(panel, [mw, panel, reload]() {
        syncCurrentBrush(mw);
        if (panel->property("nanquimDirty").toBool()) {
            reload();
            panel->setProperty("nanquimDirty", false);
        }
    }));

    // guarda onde o dedo soltou, para saber se o toque foi na estrela
    class ReleaseSpy : public QObject {
    public:
        using QObject::QObject;
        bool eventFilter(QObject *obj, QEvent *e) override {
            if (e->type() == QEvent::MouseButtonRelease || e->type() == QEvent::MouseButtonPress) {
                obj->setProperty("nanquimLastX", static_cast<QMouseEvent*>(e)->pos().x());
            }
            return false;
        }
    };
    list->viewport()->installEventFilter(new ReleaseSpy(list));
    QObject::connect(clearImported, &QPushButton::clicked, panel, [mw, reload]() {
        const QVector<BrushItem> imported = importedBrushItems();
        if (imported.isEmpty()) return;
        if (!nqConfirm(mw, i18n("Pincéis"), i18n("Remover todos os importados?"),
                       i18n("%1 pincel(is) importado(s) vão sair da lista. Os pincéis nativos do Nanquim e do Krita não são apagados.",
                            imported.size()),
                       i18n("Remover tudo"))) {
            return;
        }
        QVector<int> ids;
        ids.reserve(imported.size());
        for (const BrushItem &b : imported) {
            if (b.id >= 0) ids << b.id;
        }
        if (removeImportedBrushesById(mw, ids)) {
            reload();
            showToast(mw, i18n("%1 pincel(is) importado(s) removido(s)", ids.size()));
        }
    });
    QObject::connect(list, &QListView::clicked, panel, [mw, list, reload, group](const QModelIndex &index) {
        const int x = list->viewport()->property("nanquimLastX").toInt();
        const int vw = list->viewport()->width();
        const int brushId = index.data(BrushIdRole).toInt();
        const bool removable = index.data(BrushRemovableRole).toBool();
        if (removable && brushId >= 0 && x > vw - 138 && x <= vw - 92) {
            const QString bName = index.data(BrushNameRole).toString();
            if (nqConfirm(mw, i18n("Pincéis"), i18n("Remover este pincel?"),
                          i18n("\"%1\" vai sair da lista de importados.", bName), i18n("Remover"))) {
                if (removeImportedBrushesById(mw, QVector<int>() << brushId)) {
                    reload();
                    showToast(mw, i18n("Pincel removido · %1", bName));
                }
            }
            return;
        }
        const int gearLeft = removable ? (vw - 88) : (vw - 96);
        if (x > gearLeft && x <= vw - 44 && brushId >= 0) {
            // engrenagem: escolhe o pincel e abre os ajustes dele (com o teste ao vivo)
            selectBrush(mw, index);
            syncCurrentBrush(mw);
            setBrushPanelVisible(mw, false);
            QTimer::singleShot(160, mw, [mw]() {
                // se o modo mudou no meio do intervalo (tela cheia, modo foco), o
                // pop-up não pode aparecer com a âncora invisível
                if (!s_focusMode && !canvasOnlyActive(mw)) showBrushSettings(mw);
            });
            return;
        }
        if (x > vw - 44 && index.data(BrushIdRole).toInt() >= 0) {
            const bool fav = !index.data(BrushFavoriteRole).toBool();
            setFavorite(mw, index.data(BrushIdRole).toInt(), fav);
            QAbstractButton *chip = group->checkedButton();
            if (chip && chip->property("nanquimCategory").toString() == BRUSH_FAVORITES) {
                reload();
            } else {
                const_cast<QAbstractItemModel*>(index.model())->setData(index, fav, BrushFavoriteRole);
            }
            return;
        }
        selectBrush(mw, index);
        syncCurrentBrush(mw);
        list->viewport()->update();
        QTimer::singleShot(120, mw, [mw]() { setBrushPanelVisible(mw, false); });
    });
    QObject::connect(settings, &QPushButton::clicked, mw, [mw]() {
        setBrushPanelVisible(mw, false);
        QTimer::singleShot(200, mw, [mw]() {
            if (!s_focusMode && !canvasOnlyActive(mw)) showBrushSettings(mw);
        });
    });
    QObject::connect(importButton, &QPushButton::clicked, mw, [mw, group]() {
        setBrushPanelVisible(mw, false);
        if (QAbstractButton *chip = group->button(5)) chip->setChecked(true);
        KSharedConfig::openConfig()->group("Nanquim").writeEntry("brushCategory", BRUSH_IMPORTED);
        QTimer::singleShot(200, mw, [mw]() { importBrushFiles(mw); });
    });
    QObject::connect(communityButton, &QPushButton::clicked, mw, [mw]() {
        setBrushPanelVisible(mw, false);
        QTimer::singleShot(160, mw, [mw]() { showCommunityCatalogSheet(mw); });
    });
    QObject::connect(add, &QToolButton::clicked, mw, [mw, group, reload]() {
        if (saveCurrentAsNewBrush(mw)) {
            if (QAbstractButton *chip = group->button(5)) chip->setChecked(true);
            KSharedConfig::openConfig()->group("Nanquim").writeEntry("brushCategory", BRUSH_IMPORTED);
            syncCurrentBrush(mw);
            reload();
        }
    });

    if (KoCanvasResourceProvider *r = resources(mw)) {
        QObject::connect(r, &KoCanvasResourceProvider::canvasResourceChanged, list, [mw, list](int key, const QVariant &) {
            if (key == KoCanvasResource::CurrentPaintOpPreset) {
                syncCurrentBrush(mw);
                list->viewport()->update();
            }
        });
    }
}

void setBrushPanelVisible(KisMainWindow *mw, bool visible)
{
    QFrame *panel = mw->findChild<QFrame*>(BRUSH_PANEL_ID);
    if (!panel) return;
    if (QToolButton *b = mw->findChild<QToolButton*>(QStringLiteral("nanquimBrushButton"))) {
        b->setChecked(visible);
    }
    if (panel->isVisible() == visible) return;
    if (visible) {
        if (!s_classic) {
            if (QDockWidget *dw = hostedDock(mw)) closePanel(mw, dw, false);
            // no estúdio, a coluna inteira é a "dona" da janela: tocar no pincel
            // ativo de novo fecha, tocar em outro só troca o pincel
            QWidget *rail = mw->findChild<QWidget*>(QStringLiteral("nanquimStudioRail"));
            s_popupAnchor = (rail && rail->isVisible()) ? rail
                                                        : mw->findChild<QToolButton*>(QStringLiteral("nanquimBrushButton"));
        }
        placeBrushPanel(mw, panel);
        panel->show();
        panel->raise();
        flushLayouts();
        const QRect geom = panel->geometry();
        const QPixmap shot = lightShot(panel->grab());
        panel->hide();
        QPointer<QFrame> guard(panel);
        // no estúdio ou na barra unificada de foco, a janela sai da coluna: desliza 8 px e cresce de 97%
        QWidget *studioRail = mw->findChild<QWidget*>(QStringLiteral("nanquimStudioRail"));
        QWidget *focusBar = mw->findChild<QWidget*>(QStringLiteral("nanquimFocusBar"));
        const bool fromRail = (studioRail && studioRail->isVisible()) || (focusBar && focusBar->isVisible());
        const int side = fromRail ? (railOnRight() ? +1 : -1) : 0;
        NanquimOverlay *o = new NanquimOverlay(mw, geom, [shot, side](QPainter &g, const QRect &r, qreal p) {
            g.setOpacity(p);
            if (side == 0) {
                g.drawPixmap(QPointF(0, -(1.0 - p) * 18), shot);
                return;
            }
            const qreal scale = 0.97 + 0.03 * p;
            const QPointF origin(side < 0 ? 0 : r.width(), 0);   // canto do lado da coluna
            g.translate(origin + QPointF(side * (1.0 - p) * 8, 0));
            g.scale(scale, scale);
            g.translate(-origin);
            g.drawPixmap(QPointF(0, 0), shot);
        });
        runOverlay(s_brushOverlay, o, side ? 145 : 135, QEasingCurve::OutQuint, [guard]() {
            if (guard) {
                guard->show();
                guard->raise();
            }
        });
    } else {
        const QRect geom = panel->geometry();
        const QPixmap shot = lightShot(panel->grab());
        panel->hide();
        QWidget *studioRail = mw->findChild<QWidget*>(QStringLiteral("nanquimStudioRail"));
        QWidget *focusBar = mw->findChild<QWidget*>(QStringLiteral("nanquimFocusBar"));
        const bool fromRail = (studioRail && studioRail->isVisible()) || (focusBar && focusBar->isVisible());
        const int side = fromRail ? (railOnRight() ? +1 : -1) : 0;
        NanquimOverlay *o = new NanquimOverlay(mw, geom, [shot, side](QPainter &g, const QRect &, qreal p) {
            g.setOpacity(1.0 - p);
            if (side == 0) g.drawPixmap(QPointF(0, -p * 14), shot);
            else g.drawPixmap(QPointF(side * p * 8, 0), shot);   // volta para a coluna
        });
        // fechar é mais rápido que abrir, e também com curva de saída
        runOverlay(s_brushOverlay, o, 110, QEasingCurve::OutCubic);
    }
}

void markBrushPanelDirty(KisMainWindow *mw)
{
    if (!mw) return;
    if (QFrame *panel = mw->findChild<QFrame*>(BRUSH_PANEL_ID)) {
        panel->setProperty("nanquimDirty", true);
    }
}

// pré-aquece os pop-ups (Pincéis, Cor e Camadas) logo após abrir a página para
// que o primeiro toque já abra em 1 quadro, sem montar estilos/modelos na hora
void warmUpPopups(KisMainWindow *mw)
{
    if (!mw || s_classic || mw->property("nanquimPopupsWarmed").toBool()) return;
    mw->setProperty("nanquimPopupsWarmed", true);
    if (QFrame *host = panelHost(mw)) {
        host->ensurePolished();
    }
    if (QFrame *panel = mw->findChild<QFrame*>(BRUSH_PANEL_ID)) {
        panel->ensurePolished();
        if (QTimer *t = panel->findChild<QTimer*>(QStringLiteral("nanquimBrushWarmup"))) {
            t->start();
        }
    }
    for (const QString &id : {QStringLiteral("NanquimColorDock"), QStringLiteral("KisLayerBox")}) {
        if (QDockWidget *dw = mw->dockWidget(id)) {
            ensurePanelHeader(mw, dw);
            dw->ensurePolished();
        }
    }
}

class BrushPanelCloser : public QObject
{
public:
    explicit BrushPanelCloser(KisMainWindow *mw) : QObject(mw), m_mw(mw) {}
protected:
    bool eventFilter(QObject *, QEvent *e) override
    {
        if (e->type() != QEvent::MouseButtonPress && e->type() != QEvent::TabletPress
                && e->type() != QEvent::TouchBegin) {
            return false;
        }
        if (qApp->activeModalWidget()) return false;
        QFrame *panel = m_mw->findChild<QFrame*>(BRUSH_PANEL_ID);
        if (!panel || !panel->isVisible()) return false;
        QPoint global;
        if (e->type() == QEvent::MouseButtonPress) {
            global = static_cast<QMouseEvent*>(e)->globalPos();
        } else if (e->type() == QEvent::TabletPress) {
            global = static_cast<QTabletEvent*>(e)->globalPos();
        } else {
            const auto points = static_cast<QTouchEvent*>(e)->touchPoints();
            if (points.isEmpty()) return false;
            global = points.first().screenPos().toPoint();
        }
        const QRect rect(panel->mapToGlobal(QPoint(0, 0)), panel->size());
        QToolButton *b = m_mw->findChild<QToolButton*>(QStringLiteral("nanquimBrushButton"));
        const QRect bRect = b ? QRect(b->mapToGlobal(QPoint(0, 0)), b->size()) : QRect();
        if (!rect.contains(global) && !bRect.contains(global)) {
            setBrushPanelVisible(m_mw, false);
        }
        return false;
    }
private:
    KisMainWindow *m_mw;
};

// coluna de pincéis do modo Clássico

QDockWidget *buildBrushDock(KisMainWindow *mw)
{
    QDockWidget *dock = new QDockWidget(i18n("Pincéis do Nanquim"), mw);
    dock->setObjectName(BRUSH_DOCK_ID);
    dock->setTitleBarWidget(new QWidget(dock));
    dock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable);
    QWidget *body = new QWidget(dock);
    body->setObjectName(QStringLiteral("nanquimBrushColumn"));
    body->setStyleSheet(QStringLiteral("QWidget#nanquimBrushColumn { background: #2e2d29; }"
                                       "QFrame#nanquimBrushColumnCard { background: #2e2d29; border: 1px solid #4a4842; border-radius: 12px; }"));
    QVBoxLayout *outer = new QVBoxLayout(body);
    outer->setContentsMargins(8, 8, 6, 8);
    QFrame *card = new QFrame(body);
    card->setObjectName(QStringLiteral("nanquimBrushColumnCard"));
    QVBoxLayout *inner = new QVBoxLayout(card);
    inner->setContentsMargins(2, 6, 2, 6);
    QListView *list = makeBrushList(card, true);
    inner->addWidget(list);
    outer->addWidget(card);
    dock->setWidget(body);
    dock->setMinimumWidth(84);
    QStandardItemModel *model = static_cast<QStandardItemModel*>(list->model());

    // a coluna mostra os favoritos do artista; o último quadradinho abre a janela de pincéis
    auto reload = [mw, model]() {
        syncCurrentBrush(mw);
        refreshFavoriteIds();
        QVector<BrushItem> items;
        Q_FOREACH (const KisPaintOpPresetSP &p, favoritePresets()) items << itemForPreset(p);
        BrushItem more;
        more.id = BRUSH_MORE_ID;
        more.name = i18n("Pincéis");
        more.shortName = more.name;
        items << more;
        fillBrushModel(model, items);
    };
    QObject::connect(list, &QListView::clicked, dock, [mw, list](const QModelIndex &index) {
        if (index.data(BrushIdRole).toInt() == BRUSH_MORE_ID) {
            setBrushPanelVisible(mw, true);
            return;
        }
        selectBrush(mw, index);
        syncCurrentBrush(mw);
        list->viewport()->update();
    });
    list->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(list, &QListView::customContextMenuRequested, dock, [mw, list](const QPoint &pos) {
        const QModelIndex index = list->indexAt(pos);
        const int id = index.data(BrushIdRole).toInt();
        if (!index.isValid() || id < 0) return;
        QMenu menu(list);
        QAction *remove = menu.addAction(i18n("Tirar dos favoritos"));
        if (menu.exec(list->viewport()->mapToGlobal(pos)) == remove) setFavorite(mw, id, false);
    });
    if (KoCanvasResourceProvider *r = resources(mw)) {
        QObject::connect(r, &KoCanvasResourceProvider::canvasResourceChanged, list, [mw, list](int key, const QVariant &) {
            if (key == KoCanvasResource::CurrentPaintOpPreset) {
                syncCurrentBrush(mw);
                list->viewport()->update();
            }
        });
    }
    // pincéis novos ou importados entram na coluna
    QTimer *pending = new QTimer(dock);
    pending->setObjectName(QStringLiteral("nanquimBrushDockReload"));
    pending->setSingleShot(true);
    pending->setInterval(400);
    QObject::connect(pending, &QTimer::timeout, dock, reload);
    KisResourceModel *all = allPresetsModel();
    QObject::connect(all, &QAbstractItemModel::rowsInserted, pending, [pending]() { pending->start(); });
    QObject::connect(all, &QAbstractItemModel::modelReset, pending, [pending]() { pending->start(); });
    QObject::connect(dock, &QDockWidget::visibilityChanged, pending, [pending](bool on) { if (on) pending->start(); });

    mw->addDockWidget(Qt::RightDockWidgetArea, dock);
    dock->hide();
    return dock;
}

void setFocusMode(KisMainWindow *mw, bool on);
void showToast(KisMainWindow *mw, const QString &text);
void openPageWithFeedback(KisMainWindow *mw, const QString &path);

// ---------------------------------------------------------------------------
// Gaveta Filmstrip de Capitulo: abre sob o titulo na barra do topo
// ---------------------------------------------------------------------------
const QString FILMSTRIP_ID = QStringLiteral("nanquimFilmstripDrawer");

void toggleFilmstripDrawer(KisMainWindow *mw)
{
    if (!mw) return;
    QFrame *drawer = mw->findChild<QFrame*>(FILMSTRIP_ID);
    if (drawer && drawer->isVisible()) {
        drawer->hide();
        return;
    }
    if (!drawer) {
        drawer = new QFrame(mw);
        drawer->setObjectName(FILMSTRIP_ID);
        drawer->setStyleSheet(QStringLiteral(
            "QFrame#nanquimFilmstripDrawer { background: rgba(46, 45, 41, 0.97);"
            "  border-top: 1px solid #5a574f; border-left: 1px solid #3d3c37;"
            "  border-right: 1px solid #3d3c37; border-bottom: 1px solid #292824;"
            "  border-radius: 16px; }"
            "QPushButton.nanquimStripPage { background: #34332e; border: 1px solid #4a4842;"
            "  border-radius: 10px; color: #eeeae0; font-size: 11px; font-weight: 600; padding: 6px 10px; }"
            "QPushButton.nanquimStripPage:checked { background: rgba(62, 142, 126, 0.22);"
            "  border: 1px solid #3e8e7e; color: #ffffff; }"));
        QVBoxLayout *v = new QVBoxLayout(drawer);
        v->setContentsMargins(14, 10, 14, 12);
        v->setSpacing(8);

        QHBoxLayout *hdr = new QHBoxLayout();
        QLabel *capLabel = new QLabel(drawer);
        capLabel->setObjectName(QStringLiteral("nanquimStripTitle"));
        capLabel->setStyleSheet(QStringLiteral("color: #eeeae0; font-size: 12px; font-weight: 700;"));
        QToolButton *closeBtn = new QToolButton(drawer);
        closeBtn->setIcon(designIcon(SVG_CLOSE, 16));
        closeBtn->setAutoRaise(true);
        QObject::connect(closeBtn, &QToolButton::clicked, drawer, &QWidget::hide);
        hdr->addWidget(capLabel);
        hdr->addStretch();
        hdr->addWidget(closeBtn);
        v->addLayout(hdr);

        QScrollArea *scroll = new QScrollArea(drawer);
        scroll->setObjectName(QStringLiteral("nanquimStripScroll"));
        scroll->setWidgetResizable(true);
        scroll->setFrameShape(QFrame::NoFrame);
        scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        scroll->setStyleSheet(QStringLiteral("background: transparent;"));
        v->addWidget(scroll);
    }

    KisView *view = mw->activeView();
    const QString curPath = (view && view->document()) ? view->document()->localFilePath() : QString();
    QStringList pages;
    QString activeChapterId;
    QString stripHeading = i18n("Páginas recentes");

    if (!curPath.isEmpty()) {
        KConfigGroup cfg = KSharedConfig::openConfig()->group("NanquimChapters");
        Q_FOREACH (const QString &id, cfg.readEntry("list", QStringList())) {
            KConfigGroup c = KSharedConfig::openConfig()->group(QStringLiteral("NanquimChapter-") + id);
            const QStringList chPages = c.readEntry("pages", QStringList());
            if (chPages.contains(curPath)) {
                pages = chPages;
                activeChapterId = id;
                stripHeading = c.readEntry("name", i18n("Capítulo"));
                break;
            }
        }
    }
    if (pages.isEmpty()) {
        Q_FOREACH (const QUrl &u, KisRecentFilesManager::instance()->recentUrlsLatestFirst()) {
            const QString p = u.toLocalFile();
            if (!p.isEmpty() && !pages.contains(p)) pages << p;
            if (pages.size() >= 10) break;
        }
    }

    if (QLabel *capLabel = drawer->findChild<QLabel*>(QStringLiteral("nanquimStripTitle"))) {
        capLabel->setText(stripHeading);
    }
    if (QScrollArea *scroll = drawer->findChild<QScrollArea*>(QStringLiteral("nanquimStripScroll"))) {
        QWidget *stripHost = new QWidget(scroll);
        QHBoxLayout *hl = new QHBoxLayout(stripHost);
        hl->setContentsMargins(0, 0, 0, 0);
        hl->setSpacing(8);
        QStringList statusEntries;
        if (!activeChapterId.isEmpty()) {
            statusEntries = KSharedConfig::openConfig()
                                ->group(QStringLiteral("NanquimChapter-") + activeChapterId)
                                .readEntry("status", QStringList());
        }
        for (int i = 0; i < pages.size(); ++i) {
            const QString pagePath = pages[i];
            int st = 0;
            Q_FOREACH (const QString &entry, statusEntries) {
                const int sep = entry.indexOf(QLatin1Char('|'));
                if (sep > 0 && entry.mid(sep + 1) == pagePath) {
                    st = qBound(0, entry.left(sep).toInt(), 2);
                    break;
                }
            }
            const QString dot = st == 2 ? QStringLiteral("● ") : st == 1 ? QStringLiteral("◐ ") : QStringLiteral("○ ");
            QString baseName = QFileInfo(pagePath).completeBaseName();
            QPushButton *btn = new QPushButton(QStringLiteral("%1%2. %3").arg(dot).arg(i + 1).arg(baseName), stripHost);
            btn->setProperty("class", QStringLiteral("nanquimStripPage"));
            btn->setCheckable(true);
            btn->setChecked(pagePath == curPath);
            btn->setFixedSize(144, 46);
            if (pagePath == curPath) {
                btn->setStyleSheet(QStringLiteral(
                    "QPushButton { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 rgba(62,142,126,0.32), stop:1 rgba(111,191,173,0.22));"
                    " border: 1px solid #6fbfad; border-radius: 10px; color: #ffffff; font-size: 11px; font-weight: 700; padding: 6px 10px; }"));
            } else if (st == 2) {
                btn->setStyleSheet(QStringLiteral(
                    "QPushButton { background: #34332e; border: 1px solid rgba(94,201,140,0.45); border-radius: 10px;"
                    " color: #b8f2d0; font-size: 11px; font-weight: 600; padding: 6px 10px; }"));
            } else if (st == 1) {
                btn->setStyleSheet(QStringLiteral(
                    "QPushButton { background: #34332e; border: 1px solid rgba(111,191,173,0.42); border-radius: 10px;"
                    " color: #cde9e1; font-size: 11px; font-weight: 600; padding: 6px 10px; }"));
            }
            QObject::connect(btn, &QPushButton::clicked, mw, [mw, drawer, pagePath, curPath]() {
                drawer->hide();
                if (!pagePath.isEmpty() && pagePath != curPath) {
                    openPageWithFeedback(mw, pagePath);
                }
            });
            hl->addWidget(btn);
        }
        hl->addStretch();
        scroll->setWidget(stripHost);
    }

    const int w = qMin(mw->width() - 40, 680);
    int drawerTop = 54;
    if (QWidget *pill = mw->findChild<QWidget*>(QStringLiteral("nanquimStudioTitle"))) {
        if (pill->isVisible()) drawerTop = pill->geometry().bottom() + 10;
    }
    const QPoint targetPos((mw->width() - w) / 2, drawerTop);
    drawer->resize(w, 116);
    drawer->move(targetPos - QPoint(0, 14));
    drawer->raise();
    drawer->show();
    QPropertyAnimation *slide = new QPropertyAnimation(drawer, "pos", drawer);
    slide->setDuration(210);
    slide->setStartValue(targetPos - QPoint(0, 14));
    slide->setEndValue(targetPos);
    slide->setEasingCurve(softSpring());
    slide->start(QAbstractAnimation::DeleteWhenStopped);
}

void buildTopBar(KisMainWindow *mw)
{
    QToolBar *top = new QToolBar(i18n("Barra do Nanquim"), mw);
    top->setObjectName(TOP_ID);
    top->setMovable(false);
    top->setFloatable(false);
    top->setContextMenuPolicy(Qt::PreventContextMenu);
    top->setStyleSheet(QLatin1String(TOP_STYLE));
    top->setIconSize(QSize(22, 22));
    mw->addToolBar(Qt::TopToolBarArea, top);

    // galeria (volta para a tela inicial, sem fechar o desenho)
    QToolButton *gallery = iconButton(top, i18n("Galeria"), designIcon(Svg::GALLERY));
    gallery->setProperty(NEEDS_CANVAS, true);
    QObject::connect(gallery, &QToolButton::clicked, mw, [mw]() { showGalleryAnimated(mw); });
    top->addWidget(gallery);

    // "voltar ao desenho" (só na tela inicial, quando há um desenho aberto)
    QToolButton *back = new QToolButton(top);
    back->setObjectName(QStringLiteral("nanquimBack"));
    back->setText(i18n("Voltar ao desenho"));
    back->setIcon(designIcon(Svg::BACK, 18));
    back->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    back->setFocusPolicy(Qt::NoFocus);
    back->setProperty(WELCOME_ONLY, true);
    QObject::connect(back, &QToolButton::clicked, mw, [mw]() { mw->showWelcomeScreen(false); });
    top->addWidget(back);

    // título e informações do desenho
    QToolButton *titleBox = new QToolButton(top);
    titleBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    titleBox->setAutoRaise(true);
    titleBox->setFocusPolicy(Qt::NoFocus);
    QVBoxLayout *tl = new QVBoxLayout(titleBox);
    tl->setContentsMargins(8, 0, 8, 0);
    tl->setSpacing(0);
    QLabel *title = new QLabel(QStringLiteral("NANQUIM ▾"), titleBox);
    title->setObjectName(QStringLiteral("nanquimTitle"));
    title->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    QLabel *info = new QLabel(titleBox);
    info->setObjectName(QStringLiteral("nanquimInfo"));
    info->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    tl->addWidget(title);
    tl->addWidget(info);
    QObject::connect(titleBox, &QToolButton::clicked, mw, [mw]() { toggleFilmstripDrawer(mw); });
    top->addWidget(titleBox);

    QTimer *titleTimer = new QTimer(top);
    titleTimer->setInterval(800);
    QObject::connect(titleTimer, &QTimer::timeout, top, [mw, title, info]() {
        KisView *view = mw->activeView();
        if (!view || !view->document() || !view->image() || mw->centralWidget() == nullptr) {
            return;
        }
        QString name = view->document()->caption().trimmed();
        if (name.endsWith(QLatin1String(".kra"), Qt::CaseInsensitive)) {
            name.chop(4);
        }
        const QString details = QStringLiteral("%1 × %2").arg(view->image()->width()).arg(view->image()->height());
        if (title->text() != name) {
            title->setText(name);
        }
        if (info->text() != details) {
            info->setText(details);
        }
    });
    titleTimer->start();

    // desfazer e refazer ficam no dock do polegar, junto da barra de ferramentas
    top->addSeparator();

    // pincéis: janela no estilo do protótipo (o editor do Krita abre por "Ajustes do pincel")
    QToolButton *brushButton = iconButton(top, i18n("Pincéis"), designIcon(Svg::PEN), true);
    brushButton->setObjectName(QStringLiteral("nanquimBrushButton"));
    brushButton->setProperty(NEEDS_CANVAS, true);
    QObject::connect(brushButton, &QToolButton::clicked, mw, [mw]() {
        QFrame *panel = mw->findChild<QFrame*>(BRUSH_PANEL_ID);
        setBrushPanelVisible(mw, !(panel && panel->isVisible()));
    });
    top->addWidget(brushButton);

    // painéis
    struct Panel { const char *tip; const char *svg; QStringList ids; };
    const QList<Panel> panels = {
        {"Cor", nullptr, {QStringLiteral("NanquimColorDock"), QStringLiteral("ColorSelectorNg"), QStringLiteral("WideGamutColorSelector")}},
        {"Camadas", Svg::LAYERS, {QStringLiteral("KisLayerBox")}},
        {"Opções da ferramenta", Svg::OPTIONS, {QStringLiteral("sharedtooldocker")}},
    };
    Q_FOREACH (const Panel &p, panels) {
        QDockWidget *dw = findDock(mw, p.ids);
        if (!dw) {
            continue;
        }
        QToolButton *b = iconButton(top, i18n(p.tip), p.svg ? designIcon(p.svg) : QIcon(), true);
        // nome estável para quem procura o botão-âncora depois (a roda de
        // atalhos usa o de Camadas; tooltip muda, nome não)
        b->setObjectName(QStringLiteral("nanquimPanel_%1").arg(p.ids.first()));
        b->setProperty(NEEDS_CANVAS, true);
        if (!p.svg) {
            // Cor: o botão é a própria cor de frente
            KisViewManager *vm = mw->viewManager();
            auto refresh = [b, vm]() {
                if (vm && vm->canvasResourceProvider()) {
                    b->setIcon(colorSwatchIcon(vm->canvasResourceProvider()->fgColor().toQColor()));
                }
            };
            refresh();
            if (vm && vm->canvasResourceProvider()) {
                QObject::connect(vm->canvasResourceProvider(), &KisCanvasResourceProvider::sigFGColorChanged, b,
                                 [refresh](const KoColor &) { refresh(); });
            }
            b->setIconSize(QSize(26, 26));
        }
        QObject::connect(b, &QToolButton::clicked, mw, [mw, dw, b]() {
            togglePanelFrom(mw, dw, b);
            b->setChecked(panelOpen(mw, dw));
        });
        QObject::connect(dw->toggleViewAction(), &QAction::toggled, b, [b](bool on) { b->setChecked(on); });
        top->addWidget(b);
    }

    // Central de Quadrinhos (retícula, requadros, webtoon, pincéis…) e o menu de réguas
    QToolButton *hubBtn = iconButton(top, i18n("Quadrinhos e réguas"), designIcon(Svg::HUB));
    hubBtn->setProperty(NEEDS_CANVAS, true);
    QObject::connect(hubBtn, &QToolButton::clicked, mw, [mw]() { showComicHubSheet(mw); });
    top->addWidget(hubBtn);
    addBalloonButton(mw, top);

    if (act(mw, "import_layer_as_paint_layer")) {
        QToolButton *b = iconButton(top, i18n("Inserir imagem como camada"), designIcon(Svg::INSERT_IMAGE));
        b->setProperty(NEEDS_CANVAS, true);
        QObject::connect(b, &QToolButton::clicked, mw, [mw]() { insertImageAsLayer(mw); });
        top->addWidget(b);
    }

    // Referência
    if (QAction *ref = act(mw, "ToolReferenceImages")) {
        QToolButton *b = iconButton(top, i18n("Referência"), designIcon(Svg::REFERENCE), true);
        b->setProperty(NEEDS_CANVAS, true);
        QDockWidget *options = findDock(mw, {QStringLiteral("sharedtooldocker")});
        QObject::connect(b, &QToolButton::clicked, mw, [mw, ref, options, b]() {
            const bool closing = options && panelOpen(mw, options) && s_popupAnchor == b;
            if (!closing) ref->trigger();
            togglePanelFrom(mw, options, b);
            b->setChecked(options && panelOpen(mw, options));
        });
        if (options) {
            QObject::connect(options->toggleViewAction(), &QAction::toggled, b, [b](bool on) { if (!on) b->setChecked(false); });
        }
        top->addWidget(b);
    }

    {
        // exportar: página, capítulo e time-lapse num lugar só
        QToolButton *exportButton = iconButton(top, i18n("Exportar"), designIcon(Svg::EXPORT));
        exportButton->setProperty(NEEDS_CANVAS, true);
        QObject::connect(exportButton, &QToolButton::clicked, mw, [mw]() { showExport(mw); });
        top->addWidget(exportButton);
    }

    // modo foco: esconde as barras (sai pelo botão no canto da página)
    {
        static const char *SVG_FOCUS = "<path d='M4 9V5h4M20 9V5h-4M4 15v4h4M20 15v4h-4'/>";
        QToolButton *focus = iconButton(top, i18n("Tela cheia (como o gesto de quatro dedos)"), designIcon(SVG_FOCUS));
        focus->setObjectName(QStringLiteral("nanquimFocusButton"));
        focus->setProperty(NEEDS_CANVAS, true);
        // build 62.5: usa a tela cheia do próprio Krita em vez do modo foco do Nanquim
        QObject::connect(focus, &QToolButton::clicked, mw, [mw]() {
            if (QAction *a = act(mw, "view_show_canvas_only")) a->trigger();
        });
        top->addWidget(focus);
    }

    // Mais: o menu completo do Krita
    QToolButton *more = iconButton(top, i18n("Mais opções"), designIcon(Svg::MORE));
    QMenu *moreMenu = new QMenu(more);
    QObject::connect(moreMenu, &QMenu::aboutToShow, mw, [mw, moreMenu]() {
        moreMenu->clear();
        Q_FOREACH (QAction *a, mw->menuBar()->actions()) {
            moreMenu->addAction(a);
        }
    });
    more->setMenu(moreMenu);
    more->setPopupMode(QToolButton::InstantPopup);
    top->addWidget(more);
}

// ---------------------------------------------------------------------------
// Ferramentas à esquerda (design "Desenho"): 8 ferramentas, tamanho, opacidade
// ---------------------------------------------------------------------------
const char *NAV_STYLE =
    "QToolBar#nanquimTools { background: #2e2d29; border: none; border-right: 1px solid #4a4842;"
    "  spacing: 4px; padding: 10px 12px; }"
    "QToolBar#nanquimTools QToolButton { border: none; border-radius: 10px; padding: 10px 13px; }"
    "QToolBar#nanquimTools QToolButton:hover { background: rgba(255,255,255,0.06); }"
    "QToolBar#nanquimTools QToolButton:pressed { background: #4a4842; }"
    "QToolBar#nanquimTools QToolButton:checked:pressed { background: #253430; }"
    "QToolBar#nanquimTools QToolButton:checked { background: #2f4a43; border: 1px solid #3e8e7e; }"
    "QToolBar#nanquimTools QToolButton[nanquimHover=\"true\"]:!checked { background: rgba(255,255,255,0.08); }"
    "QToolBar#nanquimTools QLabel { color: #a8a398; font-size: 11px; }"
    "QToolBar#nanquimTools QSlider::groove:vertical { width: 6px; border-radius: 3px; background: #7a766d; }"
    "QToolBar#nanquimTools QSlider::add-page:vertical { width: 6px; border-radius: 3px; background: qlineargradient(x1:0, y1:1, x2:0, y2:0, stop:0 #3e8e7e, stop:1 #6fbfad); }"
    "QToolBar#nanquimTools QSlider::sub-page:vertical { width: 6px; border-radius: 3px; background: #7a766d; }"
    "QToolBar#nanquimTools QSlider::handle:vertical { background: #eeeae0; border: 2px solid #2f7568; height: 20px; width: 20px;"
    "  margin: 0 -9px; border-radius: 12px; }";

QString navStyle()
{
    QString style = QLatin1String(NAV_STYLE);
    if (railOnRight()) {
        style.replace(QLatin1String("border-right: 1px solid"), QLatin1String("border-left: 1px solid"));
    }
    return style;
}

QString expandTabStyle()
{
    return railOnRight()
        ? QStringLiteral("QToolButton { background: rgba(46,45,41,0.94); border: 1px solid #4a4842;"
                         " border-right: none; border-top-left-radius: 12px; border-bottom-left-radius: 12px;"
                         " padding: 14px 6px; }")
        : QStringLiteral("QToolButton { background: rgba(46,45,41,0.94); border: 1px solid #4a4842;"
                         " border-left: none; border-top-right-radius: 12px; border-bottom-right-radius: 12px;"
                         " padding: 14px 6px; }");
}


// ---------------------------------------------------------------------------
// Painel "Todas as ferramentas": grade por categoria, flutuando ao lado da
// barra da esquerda (substitui a caixa de ferramentas do Krita, que deixava
// os ícones encostados num canto e muito espaço vazio)
// ---------------------------------------------------------------------------
const QString FLYOUT_ID = QStringLiteral("nanquimToolsFlyout");

struct FlyoutTool { const char *id; const char *label; };
struct FlyoutGroup { const char *title; QList<FlyoutTool> tools; };

QList<FlyoutGroup> flyoutGroups()
{
    return {
        {"Desenho", {
            {"KritaShape/KisToolBrush", "Pincel"},
            {"KritaShape/KisToolMultiBrush", "Pincel múltiplo"},
            {"KisToolPencil", "Traço livre"},
            {"KarbonCalligraphyTool", "Caligrafia"},
            {"KritaShape/KisToolSmartPatch", "Remendo"},
            {"KritaSelected/KisToolColorSampler", "Conta-gotas"},
        }},
        {"Cor e preenchimento", {
            {"KritaFill/KisToolFill", "Balde"},
            {"KisToolEncloseAndFill", "Envolver"},
            {"KritaFill/KisToolGradient", "Gradiente"},
            {"KritaShape/KisToolLazyBrush", "Colorir"},
        }},
        {"Formas", {
            {"KritaShape/KisToolLine", "Linha"},
            {"KritaShape/KisToolRectangle", "Retângulo"},
            {"KritaShape/KisToolEllipse", "Elipse"},
            {"KisToolPolygon", "Polígono"},
            {"KisToolPolyline", "Polilinha"},
            {"KisToolPath", "Curva"},
            {"KritaShape/KisToolKnife", "Faca"},
        }},
        {"Vetor e texto", {
            {"InteractionTool", "Selecionar"},
            {"PathTool", "Editar nós"},
            {"SvgTextTool", "Texto"},
        }},
        {"Seleção", {
            {"KisToolSelectRectangular", "Retângulo"},
            {"KisToolSelectElliptical", "Elipse"},
            {"KisToolSelectPolygonal", "Polígono"},
            {"KisToolSelectOutline", "Laço"},
            {"KisToolSelectContiguous", "Varinha"},
            {"KisToolSelectSimilar", "Por cor"},
            {"KisToolSelectPath", "Curva"},
            {"KisToolSelectMagnetic", "Magnética"},
        }},
        {"Transformar e ver", {
            {"KisToolTransform", "Transformar"},
            {"KritaTransform/KisToolMove", "Mover"},
            {"KisToolCrop", "Recortar"},
            {"ToolReferenceImages", "Referência"},
            {"KisAssistantTool", "Assistentes"},
            {"KritaShape/KisToolMeasure", "Medir"},
            {"PanTool", "Mão"},
            {"ZoomTool", "Zoom"},
        }},
    };
}

const char *FLYOUT_STYLE =
    "QFrame#nanquimToolsFlyout { background: " NQ_RELEVO3_BG "; border: 1px solid " NQ_RELEVO3_BORDA "; border-radius: 16px; }"
    "QFrame#nanquimToolsFlyout QScrollArea, QFrame#nanquimToolsFlyout QScrollArea > QWidget > QWidget"
    "  { background: transparent; border: none; }"
    "QLabel#nanquimFlyoutTitle { color: #eeeae0; font-size: 16px; font-weight: 600; }"
    "QLabel#nanquimFlyoutSection { color: #a8a398; font-size: 11px; font-weight: 600; letter-spacing: 1px;"
    "  padding-top: 6px; }"
    "QFrame#nanquimToolsFlyout QToolButton { border: none; border-radius: 12px; color: #eeeae0;"
    "  font-size: 11px; padding: 6px 2px; }"
    "QFrame#nanquimToolsFlyout QToolButton:hover { background: rgba(255,255,255,0.06); }"
    "QFrame#nanquimToolsFlyout QToolButton:pressed { background: #4a4842; }"
    "QFrame#nanquimToolsFlyout QToolButton:checked { background: #2a3d38; color: #6fbfad; }";

QPointer<NanquimOverlay> s_flyoutOverlay;

void placeFlyout(KisMainWindow *mw, QFrame *flyout)
{
    QWidget *central = mw->centralWidget();
    if (!central) return;
    const QPoint origin = central->mapTo(mw, QPoint(0, 0));
    const int margin = 10;
    QScrollArea *scroll = flyout->findChild<QScrollArea*>();
    const int want = (scroll && scroll->widget()) ? scroll->widget()->sizeHint().height() + 70
                                                  : flyout->sizeHint().height();
    const int h = qMin(want, central->height() - 2 * margin);
    const int w = flyout->sizeHint().width();
    int x = railOnRight() ? origin.x() + central->width() - margin - w : origin.x() + margin;
    // a coluna do polegar (dock, 3 cores e bolinha) fica na mesma borda: o painel
    // abre ao lado dela, e não por baixo (antes as cores cobriam as ferramentas)
    for (const char *id : {"nanquimThumbDock", "nanquimQuickColors", "nanquimSwatch"}) {
        QWidget *wdg = mw->findChild<QWidget*>(QLatin1String(id));
        if (!wdg || !wdg->isVisible() || wdg == flyout) continue;
        if (railOnRight()) x = qMin(x, wdg->x() - margin - w);
        else x = qMax(x, wdg->x() + wdg->width() + margin);
    }
    flyout->setGeometry(x, origin.y() + margin, w, h);
}

void setFlyoutVisible(KisMainWindow *mw, bool visible)
{
    QFrame *flyout = mw->findChild<QFrame*>(FLYOUT_ID);
    if (!flyout || flyout->isVisible() == visible) return;
    if (visible) {
        placeFlyout(mw, flyout);
        flyout->show();
        flyout->raise();
        // entra deslizando da esquerda, saindo de trás da barra de ferramentas
        const QRect geom = flyout->geometry();
        const QPixmap shot = flyout->grab();
        flyout->hide();
        QPointer<QFrame> guard(flyout);
        NanquimOverlay *o = new NanquimOverlay(mw, geom, [shot](QPainter &g, const QRect &, qreal p) {
            g.setOpacity(p);
            g.drawPixmap(QPointF(-(1.0 - p) * SLIDE, 0), shot);
        });
        runOverlay(s_flyoutOverlay, o, 220, QEasingCurve::OutCubic, [guard]() {
            if (guard) {
                guard->show();
                guard->raise();
            }
        });
    } else {
        const QRect geom = flyout->geometry();
        const QPixmap shot = flyout->grab();
        flyout->hide();
        NanquimOverlay *o = new NanquimOverlay(mw, geom, [shot](QPainter &g, const QRect &, qreal p) {
            g.setOpacity(1.0 - p);
            g.drawPixmap(QPointF(-p * SLIDE, 0), shot);
        });
        runOverlay(s_flyoutOverlay, o, 170, QEasingCurve::InCubic);
    }
    if (QToolButton *toggle = mw->findChild<QToolButton*>(QStringLiteral("nanquimAllToolsButton"))) {
        toggle->setChecked(visible);
    }
}

// fecha o painel ao tocar fora dele
class FlyoutCloser : public QObject
{
public:
    explicit FlyoutCloser(KisMainWindow *mw) : QObject(mw), m_mw(mw) {}
protected:
    bool eventFilter(QObject *, QEvent *e) override
    {
        if (e->type() != QEvent::MouseButtonPress && e->type() != QEvent::TabletPress
                && e->type() != QEvent::TouchBegin) {
            return false;
        }
        if (!m_mw) return false;
        // caminho do toque: ponteiros guardados, sem procurar na árvore toda
        if (!m_flyout) m_flyout = m_mw->findChild<QFrame*>(FLYOUT_ID);
        QFrame *flyout = m_flyout;
        if (!flyout || !flyout->isVisible()) return false;
        QPoint global;
        if (e->type() == QEvent::MouseButtonPress) {
            global = static_cast<QMouseEvent*>(e)->globalPos();
        } else if (e->type() == QEvent::TabletPress) {
            global = static_cast<QTabletEvent*>(e)->globalPos();
        } else {
            const auto points = static_cast<QTouchEvent*>(e)->touchPoints();
            if (points.isEmpty()) return false;
            global = points.first().screenPos().toPoint();
        }
        const QRect flyRect(flyout->mapToGlobal(QPoint(0, 0)), flyout->size());
        if (!m_toggle) m_toggle = m_mw->findChild<QToolButton*>(QStringLiteral("nanquimAllToolsButton"));
        QToolButton *toggle = m_toggle;
        const QRect toggleRect = toggle ? QRect(toggle->mapToGlobal(QPoint(0, 0)), toggle->size()) : QRect();
        if (!flyRect.contains(global) && !toggleRect.contains(global)) {
            setFlyoutVisible(m_mw, false);
        }
        return false;
    }
private:
    QPointer<KisMainWindow> m_mw;
    QPointer<QFrame> m_flyout;
    QPointer<QToolButton> m_toggle;
};

void buildToolsFlyout(KisMainWindow *mw)
{
    KoToolManager *toolManager = KoToolManager::instance();
    QFrame *flyout = new QFrame(mw);
    flyout->setObjectName(FLYOUT_ID);
    flyout->setStyleSheet(QLatin1String(FLYOUT_STYLE));
    flyout->hide();

    QVBoxLayout *root = new QVBoxLayout(flyout);
    root->setContentsMargins(14, 12, 10, 12);
    root->setSpacing(6);

    QHBoxLayout *head = new QHBoxLayout();
    QLabel *title = new QLabel(i18n("Ferramentas"), flyout);
    title->setObjectName(QStringLiteral("nanquimFlyoutTitle"));
    head->addWidget(title, 1);
    QToolButton *close = iconButton(flyout, i18n("Fechar"),
                                    designIcon("<path d='M6 6l12 12M18 6L6 18'/>", 18));
    close->setIconSize(QSize(18, 18));
    QObject::connect(close, &QToolButton::clicked, mw, [mw]() { setFlyoutVisible(mw, false); });
    head->addWidget(close);
    root->addLayout(head);

    QScrollArea *scroll = new QScrollArea(flyout);
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setFrameShape(QFrame::NoFrame);
    KisKineticScroller::createPreconfiguredScroller(scroll);
    QWidget *content = new QWidget(scroll);
    QVBoxLayout *cl = new QVBoxLayout(content);
    cl->setContentsMargins(0, 0, 4, 0);
    cl->setSpacing(2);

    QList<QPair<QToolButton*, QString>> buttons;
    const int columns = 4;
    Q_FOREACH (const FlyoutGroup &group, flyoutGroups()) {
        QList<QPair<QAction*, FlyoutTool>> present;
        Q_FOREACH (const FlyoutTool &t, group.tools) {
            if (QAction *a = act(mw, t.id)) {
                present << qMakePair(a, t);
            }
        }
        if (present.isEmpty()) continue;
        QLabel *section = new QLabel(i18n(group.title).toUpper(), content);
        section->setObjectName(QStringLiteral("nanquimFlyoutSection"));
        cl->addWidget(section);
        QGridLayout *grid = new QGridLayout();
        grid->setSpacing(4);
        for (int i = 0; i < present.size(); i++) {
            QAction *a = present[i].first;
            QToolButton *b = new QToolButton(content);
            b->setIcon(a->icon());
            b->setIconSize(QSize(26, 26));
            b->setText(i18n(present[i].second.label));
            b->setToolTip(a->toolTip());
            b->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
            b->setCheckable(true);
            b->setFocusPolicy(Qt::NoFocus);
            b->setFixedSize(76, 66);
            QObject::connect(b, &QToolButton::clicked, mw, [mw, a]() {
                a->trigger();
                QTimer::singleShot(120, mw, [mw]() { setFlyoutVisible(mw, false); });
            });
            grid->addWidget(b, i / columns, i % columns);
            buttons << qMakePair(b, QString::fromLatin1(present[i].second.id));
        }
        cl->addLayout(grid);
    }
    cl->addStretch(1);
    scroll->setWidget(content);
    root->addWidget(scroll, 1);
    flyout->setFixedWidth(columns * 80 + 30);

    auto sync = [buttons, toolManager]() {
        const QString active = toolManager->activeToolId();
        for (const auto &pair : buttons) {
            pair.first->setChecked(pair.second == active);
        }
    };
    QObject::connect(toolManager, &KoToolManager::changedTool, flyout, [sync](KoCanvasController *) { sync(); });
    QTimer::singleShot(0, flyout, sync);

    qApp->installEventFilter(new FlyoutCloser(mw));
}

// ---------------------------------------------------------------------------
// Menus e janelinhas (Mais, Arquivo, Réguas, pincéis...) entram deslizando
// ---------------------------------------------------------------------------
class PopupAnimator : public QObject
{
public:
    using QObject::QObject;
protected:
    bool eventFilter(QObject *obj, QEvent *e) override
    {
        if (e->type() != QEvent::Show) return false;
        QWidget *w = qobject_cast<QWidget*>(obj);
        if (!w || !w->isWindow() || w->windowType() != Qt::Popup) return false;
        // só menus: as janelas de pincéis do Krita se posicionam depois de aparecer,
        // e animar a posição delas as jogava para o canto da tela
        if (w->property("nanquimAnimating").toBool()) return false;
        if (w->objectName() == QLatin1String("KisPopupButtonFrame")) {
            // janelas de pincéis/editor: o Krita acerta a posição ao mostrar, então a
            // animação parte da posição final, lida logo depois (antes do primeiro desenho)
            QPointer<QWidget> guard(w);
            QTimer::singleShot(0, w, [guard]() {
                if (!guard || !guard->isVisible()) return;
                QPoint end = guard->pos();
                // aberto por "Ajustes do pincel": fica embaixo da barra, encostado à direita
                const QVariant anchor = qApp->property("nanquimEditorAnchor");
                if (anchor.isValid()) {
                    qApp->setProperty("nanquimEditorAnchor", QVariant());
                    const QPoint a = anchor.toPoint();
                    end = QPoint(qMax(8, a.x() - guard->width()), a.y());
                    guard->move(end);
                }
                QPropertyAnimation *anim = new QPropertyAnimation(guard, "pos", guard);
                anim->setDuration(220);
                anim->setStartValue(end + QPoint(0, -14));
                anim->setEndValue(end);
                anim->setEasingCurve(softSpring());
                guard->setProperty("nanquimAnimating", true);
                QObject::connect(anim, &QPropertyAnimation::finished, guard, [guard, end]() {
                    if (guard) {
                        guard->move(end);
                        guard->setProperty("nanquimAnimating", false);
                    }
                });
                anim->start(QAbstractAnimation::DeleteWhenStopped);
            });
            return false;
        }
        if (!w->inherits("QMenu")) return false;
        const QPoint end = w->pos();
        QPropertyAnimation *anim = new QPropertyAnimation(w, "pos", w);
        anim->setDuration(200);
        anim->setStartValue(end + QPoint(0, -10));
        anim->setEndValue(end);
        anim->setEasingCurve(softSpring());
        w->setProperty("nanquimAnimating", true);
        QObject::connect(anim, &QPropertyAnimation::finished, w, [w]() { w->setProperty("nanquimAnimating", false); });
        anim->start(QAbstractAnimation::DeleteWhenStopped);
        return false;
    }
};

// ---------------------------------------------------------------------------
// Indicador (HUD) do pincel ao ajustar tamanho ou opacidade nos controles
// ---------------------------------------------------------------------------
class NanquimBrushHud : public QWidget
{
public:
    explicit NanquimBrushHud(KisMainWindow *mw) : QWidget(mw), m_mw(mw)
    {
        setObjectName(QStringLiteral("nanquimBrushHud"));
        setAttribute(Qt::WA_TranslucentBackground, true);
        setAttribute(Qt::WA_TransparentForMouseEvents, true);
        setFocusPolicy(Qt::NoFocus);
        m_hideTimer.setSingleShot(true);
        m_hideTimer.setInterval(560);
        QObject::connect(&m_hideTimer, &QTimer::timeout, this, &QWidget::hide);
        m_pulse = new QVariantAnimation(this);
        m_pulse->setDuration(150);
        m_pulse->setEasingCurve(softSpring());
        QObject::connect(m_pulse, &QVariantAnimation::valueChanged, this, [this](const QVariant &v) {
            m_scale = v.toReal();
            update();
        });
        hide();
    }

    // brushPx = tamanho real do pincel; screenPx = o mesmo tamanho já com o zoom da página
    void flash(qreal brushPx, qreal screenPx, qreal opacityNorm, bool erasing)
    {
        if (!m_mw) return;
        QWidget *central = m_mw->centralWidget();
        if (!central || !central->isVisible()) return;
        setGeometry(QRect(central->mapTo(m_mw, QPoint(0, 0)), central->size()));

        const bool wasHidden = !isVisible();
        m_rawPx = qBound(1.0, brushPx, 1000.0);
        m_diameter = qBound(6.0, screenPx, qMin(width(), height()) * 0.65);
        m_opacity = qBound(0.01, opacityNorm, 1.0);
        m_erasing = erasing;

        raise();
        show();
        if (wasHidden) {
            m_pulse->stop();
            m_pulse->setStartValue(0.86);
            m_pulse->setEndValue(1.0);
            m_pulse->start();
        }
        update();
        m_hideTimer.start();
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);
        const QPointF center(width() * 0.5, height() * 0.5);
        const qreal radius = m_diameter * 0.5 * m_scale;

        QLinearGradient ringGrad(center - QPointF(radius, radius), center + QPointF(radius, radius));
        if (m_erasing) {
            ringGrad.setColorAt(0.0, QColor(0xff, 0x6b, 0x8b, 240));
            ringGrad.setColorAt(1.0, QColor(0xe5, 0x48, 0x4d, 240));
            p.setBrush(QColor(0xe5, 0x48, 0x4d, qRound(m_opacity * 52)));
        } else {
            ringGrad.setColorAt(0.0, QColor(0x6f, 0xbf, 0xad, 240));
            ringGrad.setColorAt(1.0, QColor(0x3e, 0x8e, 0x7e, 240));
            p.setBrush(QColor(0x3e, 0x8e, 0x7e, qRound(m_opacity * 62)));
        }
        p.setPen(QPen(QBrush(ringGrad), 2.0));
        p.drawEllipse(center, radius, radius);

        p.setPen(QPen(QColor(0xee, 0xea, 0xe0, 150), 1.0));
        p.drawLine(center - QPointF(4, 0), center + QPointF(4, 0));
        p.drawLine(center - QPointF(0, 4), center + QPointF(0, 4));

        const QString label = m_erasing
            ? i18n("Borracha · %1 px", qRound(m_rawPx))
            : QStringLiteral("%1 px  ·  %2%").arg(qRound(m_rawPx)).arg(qRound(m_opacity * 100.0));
        QFont f = font();
        f.setPixelSize(12);
        f.setWeight(QFont::DemiBold);
        p.setFont(f);
        const int textW = QFontMetrics(f).horizontalAdvance(label);
        QRectF pill(0, 0, textW + 24, 28);
        pill.moveCenter(QPointF(center.x(), qMax(24.0, center.y() - radius - 22.0)));

        p.setBrush(QColor(0x2e, 0x2d, 0x29, 238));
        p.setPen(QPen(QColor(0x52, 0x4f, 0x48, 235), 1.0));
        p.drawRoundedRect(pill, 14, 14);
        p.setPen(QColor(0xee, 0xea, 0xe0));
        p.drawText(pill, Qt::AlignCenter, label);
    }

private:
    QPointer<KisMainWindow> m_mw;
    QTimer m_hideTimer;
    qreal m_rawPx = 10.0;
    qreal m_diameter = 10.0;
    QVariantAnimation *m_pulse = nullptr;
    qreal m_scale = 1.0;
    qreal m_opacity = 1.0;
    bool m_erasing = false;
};

void showBrushHud(KisMainWindow *mw)
{
    if (!mw || !mw->activeView()) return;
    KoCanvasResourceProvider *r = resources(mw);
    if (!r) return;
    NanquimBrushHud *hud = mw->findChild<NanquimBrushHud*>(QStringLiteral("nanquimBrushHud"));
    if (!hud) hud = new NanquimBrushHud(mw);
    bool erasing = false;
    if (KisViewManager *vm = mw->viewManager()) {
        if (vm->canvasResourceProvider()) {
            erasing = vm->canvasResourceProvider()->eraserMode();
        }
    }
    const QVariant s = r->resource(KoCanvasResource::Size);
    const QVariant o = r->resource(KoCanvasResource::Opacity);
    const qreal px = s.isValid() ? s.toReal() : 10.0;
    // o círculo acompanha o zoom: mostra o tamanho que o traço terá na tela
    qreal zoom = 1.0;
    if (KisViewManager *vm = mw->viewManager()) {
        if (KisCanvas2 *canvas = vm->canvasBase()) {
            if (const KisCoordinatesConverter *conv = canvas->coordinatesConverter()) {
                const qreal z = conv->effectiveZoom();
                if (z > 0.0) zoom = z;
            }
        }
    }
    hud->flash(px, px * zoom, o.isValid() ? o.toReal() : 1.0, erasing);
}

struct ToolEntry {
    const char *tip;
    const char *svg;
    const char *toolId;   // ação da ferramenta no Krita
    bool eraser;          // pincel com a borracha ligada
};

QWidget *sliderBlock(QWidget *parent, QSlider *slider, QLabel *label, int height)
{
    QWidget *box = new QWidget(parent);
    QVBoxLayout *l = new QVBoxLayout(box);
    l->setContentsMargins(0, 6, 0, 6);
    l->setSpacing(6);
    label->setAlignment(Qt::AlignHCenter);
    slider->setFixedHeight(height);
    l->addWidget(label, 0, Qt::AlignHCenter);
    l->addWidget(slider, 0, Qt::AlignHCenter);
    return box;
}

void positionExpandTab(KisMainWindow *mw, QToolButton *tab)
{
    QWidget *central = mw->centralWidget();
    if (!central || !tab) {
        return;
    }
    const QPoint origin = central->mapTo(mw, QPoint(0, 0));
    const int x = railOnRight() ? origin.x() + central->width() - tab->width() : origin.x();
    tab->move(x, origin.y() + central->height() / 2 - tab->height() / 2);
    tab->raise();
}

class ResizeWatcher : public QObject
{
public:
    ResizeWatcher(KisMainWindow *mw, QToolButton *tab) : QObject(mw), m_mw(mw), m_tab(tab) {}
protected:
    bool eventFilter(QObject *o, QEvent *e) override
    {
        if (o == m_mw && (e->type() == QEvent::Resize || e->type() == QEvent::Move)) {
            positionExpandTab(m_mw, m_tab);
            updateSafeTop(m_mw);
            placeFocusWidgets(m_mw);
        } else if (o == m_mw && e->type() == QEvent::LayoutRequest) {
            positionExpandTab(m_mw, m_tab);
            placeFocusWidgets(m_mw);
        }
        return false;
    }
private:
    KisMainWindow *m_mw;
    QPointer<QToolButton> m_tab;
};

void setNavCollapsed(KisMainWindow *mw, bool collapsed, bool animate);

void buildToolNav(KisMainWindow *mw)
{
    QToolBar *nav = new QToolBar(i18n("Ferramentas do Nanquim"), mw);
    nav->setObjectName(NAV_ID);
    nav->setOrientation(Qt::Vertical);
    nav->setMovable(false);
    nav->setFloatable(false);
    nav->setContextMenuPolicy(Qt::PreventContextMenu);
    nav->setStyleSheet(navStyle());
    nav->setIconSize(QSize(22, 22));
    mw->addToolBar(railOnRight() ? Qt::RightToolBarArea : Qt::LeftToolBarArea, nav);

    const QList<ToolEntry> tools = {
        {"Pincel", Svg::PEN, "KritaShape/KisToolBrush", false},
        {"Borracha", Svg::ERASER, "KritaShape/KisToolBrush", true},
        {"Mover camada", Svg::MOVE, "KritaTransform/KisToolMove", false},
        {"Balde", Svg::BUCKET, "KritaFill/KisToolFill", false},
        {"Seleção", Svg::SELECT, "KisToolSelectRectangular", false},
        {"Transformar", Svg::TRANSFORM, "KisToolTransform", false},
        {"Texto e balões", Svg::TEXT, "SvgTextTool", false},
        {"Conta-gotas", Svg::PICKER, "KritaSelected/KisToolColorSampler", false},
    };

    QAction *erase = act(mw, "erase_action");
    KoToolManager *toolManager = KoToolManager::instance();
    QList<QPair<QToolButton*, ToolEntry>> buttons;

    Q_FOREACH (const ToolEntry &t, tools) {
        QAction *toolAction = act(mw, t.toolId);
        if (!toolAction || (t.eraser && !erase)) {
            continue;
        }
        QToolButton *b = iconButton(nav, i18n(t.tip), designIcon(t.svg), true);
        b->setProperty(NEEDS_CANVAS, true);
        QObject::connect(b, &QToolButton::clicked, mw, [toolAction, erase, t]() {
            toolAction->trigger();
            if (erase && QLatin1String(t.toolId) == QLatin1String("KritaShape/KisToolBrush")
                    && erase->isChecked() != t.eraser) {
                erase->trigger();
            }
        });
        nav->addWidget(b);
        buttons << qMakePair(b, t);
    }

    // marca a ferramenta ativa (inclusive quando ela muda por atalho ou pelo Krita)
    auto sync = [buttons, erase, toolManager]() {
        const QString active = toolManager->activeToolId();
        const bool erasing = erase && erase->isChecked();
        for (const auto &pair : buttons) {
            const bool isBrush = QLatin1String(pair.second.toolId) == QLatin1String("KritaShape/KisToolBrush");
            const bool on = active == QLatin1String(pair.second.toolId) && (!isBrush || pair.second.eraser == erasing);
            pair.first->setChecked(on);
        }
    };
    QObject::connect(toolManager, &KoToolManager::changedTool, nav, [sync](KoCanvasController *) { sync(); });
    if (erase) {
        QObject::connect(erase, &QAction::toggled, nav, [sync](bool) { sync(); });
    }
    QTimer::singleShot(0, nav, sync);

    // todas as ferramentas, num painel flutuante organizado por categoria
    QToolButton *all = iconButton(nav, i18n("Todas as ferramentas"), designIcon(Svg::ALL_TOOLS), true);
    all->setObjectName(QStringLiteral("nanquimAllToolsButton"));
    all->setProperty(NEEDS_CANVAS, true);
    QObject::connect(all, &QToolButton::clicked, mw, [mw, all]() {
        QFrame *flyout = mw->findChild<QFrame*>(FLYOUT_ID);
        setFlyoutVisible(mw, !(flyout && flyout->isVisible()));
        all->setChecked(flyout && flyout->isVisible());
    });
    nav->addWidget(all);

    QWidget *spacer = new QWidget(nav);
    spacer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    nav->addWidget(spacer);

    // tamanho e opacidade do pincel, na vertical, como no design
    QSlider *size = new QSlider(Qt::Vertical, nav);
    size->setRange(0, 1000);
    QLabel *sizeLabel = new QLabel(QStringLiteral("— px"), nav);
    QSlider *opacity = new QSlider(Qt::Vertical, nav);
    opacity->setRange(0, 100);
    QLabel *opacityLabel = new QLabel(QStringLiteral("100%"), nav);
    size->setProperty(NEEDS_CANVAS, true);
    opacity->setProperty(NEEDS_CANVAS, true);
    nav->addWidget(sliderBlock(nav, size, sizeLabel, 130));
    nav->addWidget(sliderBlock(nav, opacity, opacityLabel, 100));

    QObject::connect(size, &QSlider::valueChanged, mw, [mw, sizeLabel](int v) {
        const qreal px = sliderToSize(v);
        sizeLabel->setText(QStringLiteral("%1 px").arg(px));
        if (KoCanvasResourceProvider *r = resources(mw)) {
            r->setResource(KoCanvasResource::Size, px);
            showBrushHud(mw);
        }
    });
    QObject::connect(opacity, &QSlider::valueChanged, mw, [mw, opacityLabel](int v) {
        opacityLabel->setText(QStringLiteral("%1%").arg(v));
        if (KoCanvasResourceProvider *r = resources(mw)) {
            r->setResource(KoCanvasResource::Opacity, v / 100.0);
            showBrushHud(mw);
        }
    });

    auto pull = [mw, size, sizeLabel, opacity, opacityLabel]() {
        KoCanvasResourceProvider *r = resources(mw);
        if (!r) {
            return;
        }
        const QVariant s = r->resource(KoCanvasResource::Size);
        if (s.isValid()) {
            QSignalBlocker block(size);
            size->setValue(sizeToSlider(s.toReal()));
            sizeLabel->setText(QStringLiteral("%1 px").arg(qRound(s.toReal())));
        }
        const QVariant o = r->resource(KoCanvasResource::Opacity);
        if (o.isValid()) {
            QSignalBlocker block(opacity);
            opacity->setValue(qRound(o.toReal() * 100));
            opacityLabel->setText(QStringLiteral("%1%").arg(qRound(o.toReal() * 100)));
        }
    };
    if (KoCanvasResourceProvider *r = resources(mw)) {
        QObject::connect(r, &KoCanvasResourceProvider::canvasResourceChanged, nav, [pull](int key, const QVariant &) {
            if (key == KoCanvasResource::Size || key == KoCanvasResource::Opacity
                    || key == KoCanvasResource::CurrentPaintOpPreset) {
                pull();
            }
        });
    }
    QObject::connect(toolManager, &KoToolManager::changedTool, nav, [pull](KoCanvasController *) { pull(); });
    QTimer::singleShot(0, nav, pull);

    // recolher
    QToolButton *collapse = iconButton(nav, i18n("Recolher ferramentas"),
                                       designIcon(railOnRight() ? Svg::FORWARD : Svg::BACK, 20));
    collapse->setObjectName(QStringLiteral("nanquimCollapse"));
    collapse->setProperty(NEEDS_CANVAS, true);
    QObject::connect(collapse, &QToolButton::clicked, mw, [mw]() { setNavCollapsed(mw, true, true); });
    nav->addWidget(collapse);

    // aba para abrir de novo, na borda esquerda da tela de desenho
    QToolButton *tab = new QToolButton(mw);
    tab->setObjectName(QStringLiteral("nanquimExpandTab"));
    tab->setIcon(designIcon(railOnRight() ? Svg::BACK : Svg::FORWARD, 20));
    tab->setIconSize(QSize(20, 20));
    tab->setToolTip(i18n("Mostrar ferramentas"));
    tab->setFocusPolicy(Qt::NoFocus);
    tab->setStyleSheet(expandTabStyle());
    tab->adjustSize();
    tab->hide();
    QObject::connect(tab, &QToolButton::clicked, mw, [mw]() { setNavCollapsed(mw, false, true); });
    mw->installEventFilter(new ResizeWatcher(mw, tab));
}

void setNavCollapsed(KisMainWindow *mw, bool collapsed, bool animate)
{
    QToolBar *nav = mw->findChild<QToolBar*>(NAV_ID);
    QToolButton *tab = mw->findChild<QToolButton*>(QStringLiteral("nanquimExpandTab"));
    if (!nav) {
        return;
    }
    s_navCollapsed = collapsed;
    if (collapsed) {
        if (nav->isVisible() && animate) {
            const QRect geom(nav->mapTo(mw, QPoint(0, 0)), nav->size());
            const QPixmap shot = nav->grab();
            nav->hide();
            animateOut(s_navOverlay, mw, geom, shot, railOnRight() ? +1 : -1);
        } else {
            nav->hide();
        }
        if (tab) {
            flushLayouts();
            positionExpandTab(mw, tab);
            tab->show();
            tab->raise();
        }
    } else {
        if (tab) {
            tab->hide();
        }
        if (!nav->isVisible()) {
            nav->show();
            if (animate) {
                animateIn(s_navOverlay, mw, nav, railOnRight() ? +1 : -1, C_NAV);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Dock do polegar (pendência 4, ergonomia): Navegar, Refazer e Desfazer
// flutuando junto da barra, na altura do polegar da mão que não desenha, e a
// cor atual logo abaixo. Desfazer fica embaixo de tudo: é o mais usado.
// ---------------------------------------------------------------------------
const QString THUMB_DOCK_ID = QStringLiteral("nanquimThumbDock");
const QString SWATCH_ID = QStringLiteral("nanquimSwatch");
const char *SVG_HAND = "<path d='M8 13V5.5a1.5 1.5 0 0 1 3 0V12M11 11.5v-7a1.5 1.5 0 0 1 3 0V12M14 11.5V6a1.5 1.5 0 0 1 3 0v7.5M8 13.5l-1.6-2.2a1.6 1.6 0 0 0-2.5 2L8.5 19a5 5 0 0 0 4 2H14a5 5 0 0 0 5-5v-3.5'/>";

const char *THUMB_STYLE =
    "QFrame#nanquimThumbDock { background: " NQ_RELEVO2_BG "; border: 1px solid " NQ_RELEVO2_BORDA "; border-radius: 16px; }"
    "QFrame#nanquimThumbDock QToolButton { border: none; border-radius: 12px; padding: 0;"
    "  min-width: 52px; max-width: 52px; min-height: 52px; max-height: 52px; }"
    "QFrame#nanquimThumbDock QToolButton:hover { background: rgba(255,255,255,0.06); }"
    "QFrame#nanquimThumbDock QToolButton:pressed { background: #4a4842; }"
    "QFrame#nanquimThumbDock QToolButton:checked { background: #2f4a43; border: 1px solid #3e8e7e; }"
    "QFrame#nanquimThumbDock QToolButton:checked:pressed { background: #253430; }"
    "QFrame#nanquimThumbDock QToolButton#nanquimSpringEraser:checked { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #e5484d, stop:1 #ff6b8b); border: 1px solid #ff9eb5; }"
    "QFrame#nanquimThumbDock QToolButton[nanquimHover=\"true\"]:!checked { background: rgba(255,255,255,0.08); }"
    "QToolButton#nanquimSwatch { border: none; background: transparent; padding: 0; }";

QIcon swatchIcon(const QColor &c, int size)
{
    const qreal dpr = qApp->devicePixelRatio();
    QPixmap pm(QSize(size, size) * dpr);
    pm.setDevicePixelRatio(dpr);
    pm.fill(Qt::transparent);
    QPainter g(&pm);
    g.setRenderHint(QPainter::Antialiasing);
    // halo da própria cor em volta da bolinha
    {
        const QPointF mid(size / 2.0, size / 2.0);
        QRadialGradient halo(mid, size / 2.0);
        QColor strong(c);
        strong.setAlpha(150);
        QColor none(c);
        none.setAlpha(0);
        halo.setColorAt(0.0, strong);
        halo.setColorAt(0.78, strong);
        halo.setColorAt(1.0, none);
        g.setPen(Qt::NoPen);
        g.setBrush(halo);
        g.drawEllipse(QRectF(0, 0, size, size));
    }
    // anel escuro + anel claro: a cor aparece sobre papel branco e sobre a interface
    g.setPen(QPen(QColor(0x24, 0x23, 0x1f), 3));
    g.setBrush(Qt::NoBrush);
    g.drawEllipse(QRectF(4.5, 4.5, size - 9, size - 9));
    g.setPen(QPen(QColor(0xee, 0xea, 0xe0), 2));
    g.setBrush(c);
    g.drawEllipse(QRectF(6, 6, size - 12, size - 12));
    g.end();
    return QIcon(pm);
}

// ---------------------------------------------------------------------------
// Cores rápidas: três círculos acima do dock do polegar com as cores mais
// usadas. Uso = cor ativa quando a caneta sai da página depois de um traço com
// ferramenta que pinta. Sem histórico: preto, cinza 50% e branco.
// ---------------------------------------------------------------------------
void setForeground(KisMainWindow *mw, const QColor &c);

const QString QUICK_COLORS_ID = QStringLiteral("nanquimQuickColors");
const QString COLOR_USE_GROUP = QStringLiteral("NanquimColorUse");
const int QUICK_COLORS = 3;

// build 60.5: um espaço pode ser fixado com toque longo; os outros seguem as mais usadas
const QString QUICK_PIN_GROUP = QStringLiteral("NanquimQuickPins");

QColor pinnedQuickColor(int slot)
{
    const QString v = KSharedConfig::openConfig()->group(QUICK_PIN_GROUP).readEntry(QStringLiteral("slot%1").arg(slot), QString());
    return QColor::isValidColor(v) ? QColor(v) : QColor();
}

void setPinnedQuickColor(int slot, const QColor &c)
{
    KConfigGroup g = KSharedConfig::openConfig()->group(QUICK_PIN_GROUP);
    const QString key = QStringLiteral("slot%1").arg(slot);
    if (c.isValid()) g.writeEntry(key, c.name());
    else g.deleteEntry(key);
    g.sync();
}

QList<QColor> rankedUsedColors()
{
    KConfigGroup g = KSharedConfig::openConfig()->group(COLOR_USE_GROUP);
    QList<QPair<int, QString>> ranked;
    Q_FOREACH (const QString &key, g.keyList()) {
        const int n = g.readEntry(key, 0);
        if (n > 0 && QColor::isValidColor(key)) ranked << qMakePair(n, key);
    }
    std::stable_sort(ranked.begin(), ranked.end(),
                     [](const QPair<int, QString> &a, const QPair<int, QString> &b) { return a.first > b.first; });
    QList<QColor> out;
    for (const auto &r : ranked) out << QColor(r.second);
    for (const QColor &d : {QColor(0, 0, 0), QColor(0x80, 0x80, 0x80), QColor(0xff, 0xff, 0xff)}) out << d;
    return out;
}

QList<QColor> mostUsedColors()
{
    QList<QColor> out;
    for (int i = 0; i < QUICK_COLORS; ++i) out << pinnedQuickColor(i);
    const QList<QColor> ranked = rankedUsedColors();
    int next = 0;
    for (int i = 0; i < QUICK_COLORS; ++i) {
        if (out[i].isValid()) continue;
        while (next < ranked.size()) {
            const QColor cand = ranked[next++];
            bool has = false;
            for (const QColor &c : out) has = has || (c.isValid() && c.rgb() == cand.rgb());
            if (!has) { out[i] = cand; break; }
        }
    }
    return out;
}

QIcon quickColorIcon(const QColor &c, bool active, int size, bool pinned = false)
{
    const qreal dpr = qApp->devicePixelRatio();
    QPixmap pm(QSize(size, size) * dpr);
    pm.setDevicePixelRatio(dpr);
    pm.fill(Qt::transparent);
    QPainter g(&pm);
    g.setRenderHint(QPainter::Antialiasing);
    if (active) {
        const QRectF ringRect(1.5, 1.5, size - 3, size - 3);
        QLinearGradient rg(ringRect.topLeft(), ringRect.bottomRight());
        rg.setColorAt(0.0, QColor(0x6f, 0xbf, 0xad));
        rg.setColorAt(1.0, QColor(0x3e, 0x8e, 0x7e));
        g.setPen(QPen(QBrush(rg), 3.0));
        g.setBrush(Qt::NoBrush);
        g.drawEllipse(ringRect);
    }
    g.setPen(QPen(QColor(0x5a, 0x57, 0x4f), 1));
    g.setBrush(c);
    g.drawEllipse(QRectF(6, 6, size - 12, size - 12));
    if (pinned) {   // marquinha de "fixada"
        g.setPen(QPen(QColor(0x24, 0x23, 0x1f), 2));
        g.setBrush(QColor(0x6f, 0xbf, 0xad));
        g.drawEllipse(QRectF(size - 15, size - 15, 11, 11));
    }
    g.end();
    return QIcon(pm);
}

void refreshQuickColors(KisMainWindow *mw)
{
    QFrame *bar = mw->findChild<QFrame*>(QUICK_COLORS_ID);
    KisViewManager *vm = mw->viewManager();
    if (!bar || !vm || !vm->canvasResourceProvider()) return;
    const QRgb fg = vm->canvasResourceProvider()->fgColor().toQColor().rgb();
    const QList<QColor> colors = mostUsedColors();
    const QList<QToolButton*> buttons = bar->findChildren<QToolButton*>(QString(), Qt::FindDirectChildrenOnly);
    for (int i = 0; i < buttons.size() && i < colors.size(); ++i) {
        QToolButton *b = buttons[i];
        b->setProperty("nanquimColor", colors[i]);
        b->setIcon(quickColorIcon(colors[i], colors[i].rgb() == fg, 40, pinnedQuickColor(i).isValid()));
        b->setToolTip(colors[i].name());
    }
}

bool isPaintingTool(const QString &id)
{
    for (const char *k : {"Brush", "Fill", "Line", "Rectangle", "Ellipse", "Poly", "Path", "Dyna", "Gradient"}) {
        if (id.contains(QLatin1String(k))) return true;
    }
    return false;
}

// conta um uso da cor ativa a cada traço na página
class ColorUseCounter : public QObject
{
public:
    explicit ColorUseCounter(KisMainWindow *mw) : QObject(mw), m_mw(mw) {}
protected:
    bool eventFilter(QObject *, QEvent *e) override
    {
        if (!m_mw) return false;
        if (e->type() == QEvent::TabletPress) {
            m_onCanvas = onCanvas(static_cast<QTabletEvent*>(e)->globalPos());
        } else if (e->type() == QEvent::TabletRelease && m_onCanvas) {
            m_onCanvas = false;
            KisViewManager *vm = m_mw->viewManager();
            if (!vm || !vm->canvasResourceProvider()) return false;
            if (!isPaintingTool(KoToolManager::instance()->activeToolId())) return false;
            if (vm->canvasResourceProvider()->eraserMode()) {
                return false;
            }
            const QString key = vm->canvasResourceProvider()->fgColor().toQColor().name();
            KConfigGroup g = KSharedConfig::openConfig()->group(COLOR_USE_GROUP);
            g.writeEntry(key, g.readEntry(key, 0) + 1);
            QPointer<KisMainWindow> guard(m_mw);
            QTimer::singleShot(0, m_mw, [guard]() { if (guard) refreshQuickColors(guard); });
        }
        return false;
    }
private:
    bool onCanvas(const QPoint &global) const
    {
        if (!m_mw) return false;
        QWidget *central = m_mw->centralWidget();
        if (!central || !central->isVisible() || !m_mw->activeView()) return false;
        const QRect canvasRect(central->mapToGlobal(QPoint(0, 0)), central->size());
        if (!canvasRect.contains(global)) return false;
        if (canvasRect.adjusted(110, 60, -110, -110).contains(global)) return true;
        QWidget *w = qApp->widgetAt(global);
        return w && central->isAncestorOf(w)
               && QString::fromLatin1(w->metaObject()->className()).contains(QLatin1String("Canvas"));
    }
    QPointer<KisMainWindow> m_mw;
    bool m_onCanvas = false;
};

void buildQuickColors(KisMainWindow *mw)
{
    QFrame *bar = new QFrame(mw);
    bar->setObjectName(QUICK_COLORS_ID);
    bar->setStyleSheet(QStringLiteral(
        "QFrame#nanquimQuickColors { background: " NQ_RELEVO2_BG "; border: 1px solid " NQ_RELEVO2_BORDA "; border-radius: 16px; }"
        "QFrame#nanquimQuickColors QToolButton { border: none; border-radius: 12px; padding: 0; background: transparent; }"
        "QFrame#nanquimQuickColors QToolButton:pressed { background: #4a4842; }"));
    QVBoxLayout *l = new QVBoxLayout(bar);
    l->setContentsMargins(6, 6, 6, 6);
    l->setSpacing(2);
    for (int i = 0; i < QUICK_COLORS; ++i) {
        QToolButton *b = new QToolButton(bar);
        b->setFocusPolicy(Qt::NoFocus);
        b->setFixedSize(52, 48);
        b->setIconSize(QSize(40, 40));
        b->setAutoRaise(true);
        QObject::connect(b, &QToolButton::clicked, mw, [mw, b]() {
            if (b->property("nanquimLongPressed").toBool()) {   // o toque longo já agiu
                b->setProperty("nanquimLongPressed", false);
                return;
            }
            const QColor c = b->property("nanquimColor").value<QColor>();
            if (c.isValid()) setForeground(mw, c);
        });
        // toque longo: fixa a cor atual neste espaço; na cor já fixada, solta
        QTimer *hold = new QTimer(b);
        hold->setSingleShot(true);
        hold->setInterval(500);
        QObject::connect(b, &QToolButton::pressed, hold, [hold, b]() { b->setProperty("nanquimLongPressed", false); hold->start(); });
        QObject::connect(b, &QToolButton::released, hold, [hold]() { hold->stop(); });
        QObject::connect(hold, &QTimer::timeout, mw, [mw, b, i]() {
            KisViewManager *vm = mw->viewManager();
            if (!b->isDown() || !vm || !vm->canvasResourceProvider()) return;
            b->setProperty("nanquimLongPressed", true);
            const QColor fg = vm->canvasResourceProvider()->fgColor().toQColor();
            const QColor pinned = pinnedQuickColor(i);
            if (pinned.isValid() && pinned.rgb() == fg.rgb()) {
                setPinnedQuickColor(i, QColor());
                showToast(mw, i18n("Cor solta: volta a seguir as mais usadas"));
            } else {
                setPinnedQuickColor(i, fg);
                showToast(mw, i18n("Cor fixada neste espaço"));
            }
            refreshQuickColors(mw);
        });
        l->addWidget(b);
    }
    bar->adjustSize();
    bar->hide();
    refreshQuickColors(mw);
    if (KisViewManager *vm = mw->viewManager()) {
        if (vm->canvasResourceProvider()) {
            QPointer<KisMainWindow> guard(mw);
            QObject::connect(vm->canvasResourceProvider(), &KisCanvasResourceProvider::sigFGColorChanged, bar,
                             [guard](const KoColor &) { if (guard) refreshQuickColors(guard); });
        }
    }
    qApp->installEventFilter(new ColorUseCounter(mw));
}

void placeFocusWidgets(KisMainWindow *mw)
{
    if (!mw) return;
    QWidget *central = mw->centralWidget();
    if (!central) return;
    const QRect area(central->mapTo(mw, QPoint(0, 0)), central->size());
    int topInset = 0;
    int bottomInset = 0;
    if (QScreen *screen = mw->screen()) {
        const QRect avail = screen->availableGeometry();
        const int winTop = mw->mapToGlobal(QPoint(0, 0)).y();
        const int winBottom = mw->mapToGlobal(QPoint(0, mw->height())).y();
        topInset = qMax(0, avail.top() - winTop);
        bottomInset = qBound(0, winBottom - (avail.bottom() + 1), 160);
    }
    const bool right = railOnRight();
    if (QFrame *bar = mw->findChild<QFrame*>(QStringLiteral("nanquimFocusBar"))) {
        const int safeTop = area.top() + topInset + 10;
        const int safeBottom = area.bottom() - bottomInset - 10;
        const int availH = qMax(360, safeBottom - safeTop + 1);
        const int minH = bar->layout() ? bar->layout()->minimumSize().height() : 580;
        const int targetH = qMin(availH, qMax(minH, qMin(740, availH - 16)));
        bar->setFixedHeight(targetH);
        const int x = right ? area.right() - 12 - bar->width() + 1 : area.left() + 12;
        const int y = safeTop + qMax(0, (availH - bar->height()) / 2);
        bar->move(x, qBound(safeTop, y, qMax(safeTop, safeBottom - bar->height() + 1)));
        if (bar->isVisible()) bar->raise();
    }
    placeSelectionBar(mw);
}

bool canvasOnlyActive(KisMainWindow *mw);

void placeThumbDock(KisMainWindow *mw)
{
    QFrame *dock = mw->findChild<QFrame*>(THUMB_DOCK_ID);
    QToolButton *swatch = mw->findChild<QToolButton*>(SWATCH_ID);
    QWidget *central = mw->centralWidget();
    if (!dock || !swatch || !central) {
        placeFocusWidgets(mw);
        return;
    }
    const QRect area(central->mapTo(mw, QPoint(0, 0)), central->size());
    dock->adjustSize();
    const bool right = railOnRight();
    const int margin = 12;
    const int x = right ? area.right() - margin - dock->width() + 1 : area.left() + margin;
    const int swatchY = area.bottom() - 16 - swatch->height() + 1;
    swatch->move(x + (dock->width() - swatch->width()) / 2, swatchY);
    dock->move(x, qMax(area.top() + margin, swatchY - 16 - dock->height()));
    dock->raise();
    swatch->raise();
    if (QFrame *quick = mw->findChild<QFrame*>(QUICK_COLORS_ID)) {
        quick->adjustSize();
        // o dock fica também na tela cheia: as 3 cores ficam sempre acima dele
        const int qy = dock->y() - 12 - quick->height();
        quick->move(x + (dock->width() - quick->width()) / 2, qMax(area.top() + margin, qy));
        quick->raise();
    }
    placeFocusWidgets(mw);
}

bool canvasOnlyActive(KisMainWindow *mw)
{
    QAction *a = act(mw, "view_show_canvas_only");
    return a && a->isChecked();
}

// transição suave da tela cheia: as barras (topo e lateral) não somem de repente.
// Ao entrar, uma foto de cada uma desliza para fora e some; ao sair, as barras
// reais reaparecem em fade (o Krita já as colocou no lugar).
QPointer<NanquimOverlay> s_chromeOverlay;

void animateChrome(KisMainWindow *mw, bool entering)
{
    QToolBar *top = mw->findChild<QToolBar*>(TOP_ID);
    QToolBar *nav = mw->findChild<QToolBar*>(NAV_ID);
    if (!top) return;
    QList<QWidget*> bars;
    bars << top;
    if (nav && !s_navCollapsed) bars << nav;
    if (!entering) {
        for (QWidget *w : bars) {
            QGraphicsOpacityEffect *fx = new QGraphicsOpacityEffect(w);
            fx->setOpacity(0.0);
            w->setGraphicsEffect(fx);
            QPointer<QGraphicsOpacityEffect> fxGuard(fx);
            QPointer<QWidget> wGuard(w);
            QVariantAnimation *a = new QVariantAnimation(fx);
            a->setStartValue(0.0);
            a->setEndValue(1.0);
            a->setDuration(260);
            a->setEasingCurve(QEasingCurve::OutCubic);
            QObject::connect(a, &QVariantAnimation::valueChanged, fx, [fxGuard](const QVariant &v) {
                if (fxGuard) fxGuard->setOpacity(v.toReal());
            });
            QObject::connect(a, &QVariantAnimation::finished, fx, [fxGuard, wGuard]() {
                // fora do sinal: apagar o efeito apaga a animação junto
                QTimer::singleShot(0, wGuard.data(), [fxGuard, wGuard]() {
                    if (wGuard && fxGuard && wGuard->graphicsEffect() == fxGuard.data()) wGuard->setGraphicsEffect(nullptr);
                });
            });
            a->start();
        }
        return;
    }
    // entrando: o Krita já escondeu as barras; a foto sai delas (grab funciona escondido)
    const int dir = railOnRight() ? +1 : -1;
    struct Piece { QPixmap shot; QRect geom; bool isTop; int dist; };
    QVector<Piece> pieces;
    QRect reach;
    for (QWidget *w : bars) {
        const QRect g(w->mapTo(mw, QPoint(0, 0)), w->size());
        const QPixmap shot = lightShot(w->grab());
        if (shot.isNull() || g.isEmpty()) continue;
        Piece pc;
        pc.shot = shot;
        pc.geom = g;
        pc.isTop = (w == top);
        pc.dist = pc.isTop ? g.height() + 8 : g.width() + 24;
        QRect r = g;
        if (pc.isTop) r.setTop(r.top() - pc.dist);
        else if (dir < 0) r.setLeft(r.left() - pc.dist);
        else r.setRight(r.right() + pc.dist);
        reach = reach.united(r);
        pieces << pc;
    }
    reach = reach.intersected(mw->rect());
    if (pieces.isEmpty() || reach.isEmpty()) return;
    const QPoint origin = reach.topLeft();
    const QEasingCurve curve(QEasingCurve::InCubic);
    NanquimOverlay *o = new NanquimOverlay(mw, reach, [pieces, origin, dir, curve](QPainter &g, const QRect &, qreal p) {
        const qreal e = curve.valueForProgress(p);
        g.setOpacity(qBound(0.0, 1.0 - e, 1.0));
        for (const Piece &pc : pieces) {
            const QPointF base(pc.geom.topLeft() - origin);
            const QPointF off = pc.isTop ? QPointF(0, -e * pc.dist) : QPointF(dir * e * pc.dist, 0);
            g.drawPixmap(base + off, pc.shot);
        }
    });
    runOverlay(s_chromeOverlay, o, 300, QEasingCurve::Linear);
}

// troca animada dock do polegar <-> cápsula da tela cheia: um se guarda para o
// lado da barra e o outro surge do mesmo lado. São fotos leves (NanquimOverlay):
// os widgets reais já ficam no estado final, então o ThumbDockWatcher e as
// chamadas repetidas da tela cheia (0/120/400 ms) não brigam com a animação.
QPointer<NanquimOverlay> s_focusSwapOverlay;
QPointer<QWidget> s_focusSwapPending;
bool s_focusSwapToFull = false;

// tela cheia: o fundo em volta da página escurece, para o olho ficar no
// desenho. Só a cor de fundo do canvas muda (nada fica por cima da página,
// então a caneta não sente nada). A cor salva nas configurações não muda.
void setCanvasDim(KisMainWindow *mw, bool dim)
{
    KisView *view = mw ? mw->activeView() : nullptr;
    if (!view || !view->canvasBase()) return;
    QWidget *canvasWidget = view->canvasBase()->canvasWidget();
    if (!canvasWidget) return;
    KisConfig cfg(false);
    const QColor normal = cfg.canvasBorderColor();
    if (dim) {
        const QColor deep(0x05, 0x06, 0x08);
        const qreal k = 0.6;
        cfg.setCanvasBorderColor(QColor(qRound(normal.red() * (1 - k) + deep.red() * k),
                                        qRound(normal.green() * (1 - k) + deep.green() * k),
                                        qRound(normal.blue() * (1 - k) + deep.blue() * k)));
    }
    // só este canvas relê a cor (sem reconfigurar o app inteiro)
    QMetaObject::invokeMethod(canvasWidget, "slotConfigChanged");
    if (dim) cfg.setCanvasBorderColor(normal);
    canvasWidget->update();
}

void setThumbDockVisible(KisMainWindow *mw, bool visible)
{
    QFrame *dock = mw->findChild<QFrame*>(THUMB_DOCK_ID);
    QToolButton *swatch = mw->findChild<QToolButton*>(SWATCH_ID);
    if (!dock || !swatch) {
        return;
    }
    if (!visible) {
        // saiu da tela de desenho: não deixa foto da troca voando sobre a galeria
        if (s_focusSwapOverlay) s_focusSwapOverlay->deleteLater();
        s_focusSwapPending.clear();
    }
    if (visible) {
        flushLayouts();
        placeThumbDock(mw);
    }
    const bool full = visible && canvasOnlyActive(mw);
    const QVariant prevFull = mw->property("nanquimDockFull");
    QFrame *bar = mw->findChild<QFrame*>(QStringLiteral("nanquimFocusBar"));
    // o dock do polegar fica na tela cheia; só a cápsula entra e sai, deslizando
    QWidget *outW = full ? nullptr : static_cast<QWidget*>(bar);
    QWidget *inW = full ? static_cast<QWidget*>(bar) : nullptr;
    bool animate = visible && prevFull.isValid() && prevFull.toBool() != full && bar
            && (full || bar->isVisible())
            && mw->centralWidget() && mw->centralWidget()->isVisible();
    QRect outGeom;
    QPixmap outShot;
    if (animate) {
        // troca no meio da animação anterior: o que ia entrar agora sai.
        // Mostra para tirar a foto; a entrada pendente é cancelada.
        if (s_focusSwapPending) {
            s_focusSwapPending->show();
            s_focusSwapPending.clear();
        }
        if (outW) {
            outGeom = QRect(outW->mapTo(mw, QPoint(0, 0)), outW->size());
            outShot = lightShot(outW->grab());
            animate = !outShot.isNull() && !outGeom.isEmpty();
        }
    }
    dock->setVisible(visible);
    swatch->setVisible(visible);
    // tela cheia do Krita: a cápsula vertical (pincel, borracha, lata, transformar,
    // tamanho) entra no lugar do dock, acima das 3 cores
    if (bar) bar->setVisible(full);
    if (!full) {
        if (QFrame *sizeBox = mw->findChild<QFrame*>(QStringLiteral("nanquimFocusSize"))) {
            // para a animação antes de zerar (senão ela continua escrevendo nanquimSlide)
            if (QVariantAnimation *a = sizeBox->findChild<QVariantAnimation*>(QStringLiteral("nanquimSizeAnim"))) a->stop();
            sizeBox->setProperty("nanquimOpen", false);
            sizeBox->setProperty("nanquimSlide", 0.0);
            sizeBox->hide();
        }
        if (QToolButton *b = mw->findChild<QToolButton*>(QStringLiteral("nanquimFocusSizeButton"))) b->setChecked(false);
    }
    if (QFrame *quick = mw->findChild<QFrame*>(QUICK_COLORS_ID)) {
        quick->setVisible(visible);
    }
    if (full) placeThumbDock(mw);   // a cápsula se alinha às 3 cores já visíveis
    mw->setProperty("nanquimDockFull", full);
    // as chamadas repetidas da tela cheia não podem mostrar quem ainda está
    // entrando pela foto
    if (s_focusSwapPending && visible && full == s_focusSwapToFull) {
        s_focusSwapPending->hide();
    }
    if (!animate) {
        return;
    }
    flushLayouts();
    QRect inGeom;
    QPixmap inShot;
    if (inW) {
        inW->ensurePolished();
        inGeom = QRect(inW->mapTo(mw, QPoint(0, 0)), inW->size());
        inShot = lightShot(inW->grab());
        if (inShot.isNull() || inGeom.isEmpty()) {
            return;   // sem foto: tudo já fica no lugar, como era antes
        }
        inW->hide();
        s_focusSwapPending = inW;
        s_focusSwapToFull = full;
    }
    const int dir = railOnRight() ? +1 : -1;
    const int distOut = outW ? outGeom.width() + 24 : 0;
    const int distIn = inW ? inGeom.width() + 24 : 0;
    QRect reach = outGeom.united(inGeom);
    if (dir < 0) reach.setLeft(reach.left() - distOut - distIn);
    else reach.setRight(reach.right() + distOut + distIn);
    const QPoint origin = reach.topLeft();
    const QPointF outBase(outGeom.topLeft() - origin);
    const QPointF inBase(inGeom.topLeft() - origin);
    const QEasingCurve outCurve(QEasingCurve::InCubic);
    const QEasingCurve inCurve = softSpring();
    NanquimOverlay *o = new NanquimOverlay(mw, reach,
            [outShot, inShot, outBase, inBase, dir, distOut, distIn, outCurve, inCurve](QPainter &g, const QRect &, qreal p) {
        const qreal eOut = outCurve.valueForProgress(p);
        const qreal eIn = inCurve.valueForProgress(p);
        g.setOpacity(qBound(0.0, 1.0 - eOut, 1.0));
        g.drawPixmap(outBase + QPointF(dir * eOut * distOut, 0), outShot);
        g.setOpacity(qBound(0.0, eIn, 1.0));
        g.drawPixmap(inBase + QPointF(dir * (1.0 - eIn) * distIn, 0), inShot);
    });
    QPointer<QWidget> inGuard(inW);
    QPointer<KisMainWindow> mwGuard(mw);
    runOverlay(s_focusSwapOverlay, o, 300, QEasingCurve::Linear, [inGuard, mwGuard]() {
        s_focusSwapPending.clear();
        if (!inGuard || !mwGuard) return;
        QWidget *sw = mwGuard->findChild<QWidget*>(SWATCH_ID);
        if (sw && sw->isVisible()) {   // só mostra se ainda estiver na tela de desenho
            inGuard->show();
            inGuard->raise();
        }
    });
}


// mantém o dock no lugar quando a área de desenho muda (painel aberto, rotação, janela dividida)
class ThumbDockWatcher : public QObject
{
public:
    explicit ThumbDockWatcher(KisMainWindow *mw) : QObject(mw), m_mw(mw) {}
protected:
    bool eventFilter(QObject *o, QEvent *e) override
    {
        if (e->type() == QEvent::Resize || e->type() == QEvent::Move || e->type() == QEvent::LayoutRequest) {
            if (m_mw && (o == m_mw || o == m_mw->centralWidget())) {
                QPointer<KisMainWindow> guard(m_mw);
                QTimer::singleShot(0, m_mw, [guard]() { if (guard) placeThumbDock(guard); });
            }
        }
        return false;
    }
private:
    QPointer<KisMainWindow> m_mw;
};

bool openWheel(KisMainWindow *mw, const QPoint &mwPos);

void buildThumbDock(KisMainWindow *mw)
{
    QFrame *dock = new QFrame(mw);
    dock->setObjectName(THUMB_DOCK_ID);
    dock->setStyleSheet(QLatin1String(THUMB_STYLE));
    QVBoxLayout *l = new QVBoxLayout(dock);
    l->setContentsMargins(6, 6, 6, 6);
    l->setSpacing(4);

    auto makeButton = [dock](const QString &tip, const char *svg, bool checkable) {
        QToolButton *b = iconButton(dock, tip, designIcon(svg, 26), checkable);
        b->setIconSize(QSize(26, 26));
        b->setFixedSize(52, 52);
        return b;
    };

    // Navegar: enquanto ligado, a caneta move a página (ferramenta Mão do Krita);
    // desligado, volta para a ferramenta de antes
    KoToolManager *toolManager = KoToolManager::instance();
    QAction *pan = act(mw, "PanTool");
    QToolButton *navigate = makeButton(i18n("Navegar"), SVG_HAND, true);
    navigate->setObjectName(QStringLiteral("nanquimNavigate"));
    QSharedPointer<QString> previousTool(new QString(QStringLiteral("KritaShape/KisToolBrush")));
    QObject::connect(navigate, &QToolButton::clicked, mw, [toolManager, pan, navigate, previousTool](bool on) {
        if (on) {
            const QString current = toolManager->activeToolId();
            if (current != QLatin1String("PanTool") && !current.isEmpty()) {
                *previousTool = current;
            }
            if (pan) {
                pan->trigger();
            } else {
                toolManager->switchToolRequested(QStringLiteral("PanTool"));
            }
        } else {
            toolManager->switchToolRequested(*previousTool);
        }
        Q_UNUSED(navigate);
    });
    QObject::connect(toolManager, &KoToolManager::changedTool, navigate, [toolManager, navigate](KoCanvasController *) {
        navigate->setChecked(toolManager->activeToolId() == QLatin1String("PanTool"));
    });
    l->addWidget(navigate);

    // Espelhar página: vira a folha na horizontal em um toque para checar proporção e simetria
    if (QAction *mirrorAct = act(mw, "mirror_canvas")) {
        static const char *SVG_MIRROR =
            "<path d='M12 3v18' stroke-dasharray='2.5 2.5'/>"
            "<path d='M4 17l5-10v10z'/><path d='M20 17l-5-10v10z'/>";
        QToolButton *mirrorBtn = makeButton(i18n("Espelhar página"), SVG_MIRROR, true);
        mirrorBtn->setObjectName(QStringLiteral("nanquimMirrorCanvas"));
        mirrorBtn->setChecked(mirrorAct->isChecked());
        QObject::connect(mirrorBtn, &QToolButton::clicked, mirrorAct, &QAction::trigger);
        QObject::connect(mirrorAct, &QAction::toggled, mirrorBtn, &QToolButton::setChecked);
        l->addWidget(mirrorBtn);
    }

    // Borracha em Modo Mola: toque curto alterna; segurar (>200 ms) ativa
    // enquanto o polegar pressiona e volta ao pincel ao soltar.
    if (QAction *eraseAct = act(mw, "erase_action")) {
        QToolButton *springEraser = makeButton(i18n("Borracha (segure para Modo Mola)"), Svg::ERASER, true);
        springEraser->setObjectName(QStringLiteral("nanquimSpringEraser"));
        springEraser->setChecked(eraseAct->isChecked());
        QSharedPointer<QElapsedTimer> holdTimer(new QElapsedTimer());
        QSharedPointer<bool> toggledOnPress(new bool(false));

        QObject::connect(eraseAct, &QAction::toggled, springEraser, &QToolButton::setChecked);
        QObject::connect(springEraser, &QToolButton::pressed, mw, [eraseAct, holdTimer, toggledOnPress]() {
            holdTimer->start();
            if (!eraseAct->isChecked()) {
                eraseAct->trigger();
                *toggledOnPress = true;
            } else {
                *toggledOnPress = false;
            }
        });
        QObject::connect(springEraser, &QToolButton::released, mw, [eraseAct, holdTimer, toggledOnPress]() {
            const qint64 heldMs = holdTimer->isValid() ? holdTimer->elapsed() : 0;
            if (heldMs >= 200 && *toggledOnPress && eraseAct->isChecked()) {
                eraseAct->trigger(); // Modo Mola: soltou o polegar, volta ao bico de pena
            } else if (heldMs < 200 && !(*toggledOnPress) && eraseAct->isChecked()) {
                eraseAct->trigger(); // Toque curto quando ja estava ativa: desliga
            }
        });
        l->addWidget(springEraser);
    }

    struct UndoEntry { const char *action; const char *svg; const char *tip; };
    const UndoEntry entries[] = {{"edit_redo", Svg::REDO, "Refazer"}, {"edit_undo", Svg::UNDO, "Desfazer (ou deslize para varrer)"}};
    for (const UndoEntry &entry : entries) {
        QAction *a = act(mw, entry.action);
        QToolButton *b = makeButton(i18n(entry.tip), entry.svg, false);
        if (a) {
            QObject::connect(b, &QToolButton::clicked, a, &QAction::trigger);
            b->setEnabled(a->isEnabled());
            QObject::connect(a, &QAction::changed, b, [a, b]() { b->setEnabled(a->isEnabled()); });
        } else {
            b->setEnabled(false);
        }
        // Scrub horizontal nos botoes de Desfazer/Refazer para varrer o historico
        QSharedPointer<int> scrubAnchorX(new int(0));
        QSharedPointer<bool> scrubbed(new bool(false));
        b->installEventFilter(new EventFn(b, [mw, b, scrubAnchorX, scrubbed](QObject *, QEvent *ev) {
            if (ev->type() == QEvent::MouseButtonPress) {
                *scrubAnchorX = static_cast<QMouseEvent*>(ev)->globalX();
                *scrubbed = false;
            } else if (ev->type() == QEvent::MouseMove && (static_cast<QMouseEvent*>(ev)->buttons() & Qt::LeftButton)) {
                const int dx = static_cast<QMouseEvent*>(ev)->globalX() - *scrubAnchorX;
                if (qAbs(dx) >= 20) {
                    if (QAction *step = act(mw, dx < 0 ? "edit_undo" : "edit_redo")) {
                        if (step->isEnabled()) step->trigger();
                    }
                    *scrubAnchorX = static_cast<QMouseEvent*>(ev)->globalX();
                    *scrubbed = true;
                    return true;
                }
            } else if (ev->type() == QEvent::MouseButtonRelease && *scrubbed) {
                *scrubbed = false;
                b->setDown(false);
                return true;
            }
            return false;
        }));
        l->addWidget(b);
    }
    dock->adjustSize();
    dock->hide();

    // cor atual: abre o painel de cor (vira o popover de 280 px na próxima etapa)
    QToolButton *swatch = new QToolButton(mw);
    swatch->setObjectName(SWATCH_ID);
    swatch->setStyleSheet(QLatin1String(THUMB_STYLE));
    swatch->setToolTip(i18n("Cor"));
    swatch->setFocusPolicy(Qt::NoFocus);
    swatch->setFixedSize(56, 56);
    swatch->setIconSize(QSize(56, 56));
    swatch->setAutoRaise(true);
    KisViewManager *vm = mw->viewManager();
    auto refresh = [swatch, vm]() {
        if (vm && vm->canvasResourceProvider()) {
            swatch->setIcon(swatchIcon(vm->canvasResourceProvider()->fgColor().toQColor(), 56));
        }
    };
    refresh();
    if (vm && vm->canvasResourceProvider()) {
        QObject::connect(vm->canvasResourceProvider(), &KisCanvasResourceProvider::sigFGColorChanged, swatch,
                         [refresh](const KoColor &) { refresh(); });
    }
    // toque: popover de cor; toque longo (400 ms): roda de atalhos centrada na cor
    QTimer *hold = new QTimer(swatch);
    hold->setSingleShot(true);
    hold->setInterval(400);
    QSharedPointer<bool> held(new bool(false));
    QObject::connect(swatch, &QToolButton::pressed, hold, [hold, held]() { *held = false; hold->start(); });
    QObject::connect(swatch, &QToolButton::released, hold, &QTimer::stop);
    QObject::connect(hold, &QTimer::timeout, mw, [mw, swatch, held]() {
        *held = true;
        openWheel(mw, swatch->mapTo(mw, swatch->rect().center()));
    });
    QObject::connect(swatch, &QToolButton::clicked, mw, [mw, held]() {
        if (*held) {
            *held = false;
            return;
        }
        QDockWidget *dw = findDock(mw, {QStringLiteral("NanquimColorDock"), QStringLiteral("ColorSelectorNg"),
                                        QStringLiteral("WideGamutColorSelector")});
        togglePanelFrom(mw, dw, mw->findChild<QWidget*>(SWATCH_ID));
    });
    swatch->hide();

    buildQuickColors(mw);

    ThumbDockWatcher *watcher = new ThumbDockWatcher(mw);
    mw->installEventFilter(watcher);
    QTimer::singleShot(0, mw, [mw, watcher]() {
        if (QWidget *c = mw->centralWidget()) c->installEventFilter(watcher);
    });
}

// ---------------------------------------------------------------------------
// Modo foco: barra vertical única à esquerda (ou à direita para canhotos),
// reunindo voltar/título, ferramentas essenciais, cor, pincéis, tamanho e
// opacidade, camadas, páginas do capítulo, Mais e Sair — na paleta grafite e
// verdete do redesenho. Entra e sai deslizando pela borda da tela.
// ---------------------------------------------------------------------------
QPointer<NanquimOverlay> s_focusBarOverlay;

QFrame *focusDivider(QWidget *parent)
{
    QFrame *d = new QFrame(parent);
    d->setObjectName(QStringLiteral("nanquimFocusDivider"));
    d->setFixedSize(40, 1);
    return d;
}

void closeFocusSizeBox(KisMainWindow *mw)
{
    if (!mw) return;
    if (QFrame *sizeBox = mw->findChild<QFrame*>(QStringLiteral("nanquimFocusSize"))) {
        if (QVariantAnimation *a = sizeBox->findChild<QVariantAnimation*>(QStringLiteral("nanquimSizeAnim"))) a->stop();
        sizeBox->setProperty("nanquimOpen", false);
        sizeBox->setProperty("nanquimSlide", 0.0);
        sizeBox->hide();
    }
    if (QToolButton *b = mw->findChild<QToolButton*>(QStringLiteral("nanquimFocusSizeButton"))) b->setChecked(false);
}

void showFocusBarAnimated(KisMainWindow *mw, bool visible, bool animate)
{
    if (!mw) return;
    QFrame *bar = mw->findChild<QFrame*>(QStringLiteral("nanquimFocusBar"));
    if (!bar) return;

    if (s_focusBarOverlay) {
        s_focusBarOverlay->deleteLater();
        s_focusBarOverlay.clear();
    }

    if (!visible) {
        closeFocusSizeBox(mw);
        if (!bar->isVisible()) return;
        if (!animate || !mw->centralWidget() || !mw->centralWidget()->isVisible()) {
            bar->hide();
            return;
        }
        const QRect geom(bar->mapTo(mw, QPoint(0, 0)), bar->size());
        const QPixmap shot = lightShot(bar->grab());
        bar->hide();
        if (shot.isNull() || geom.isEmpty()) return;

        const int dir = railOnRight() ? +1 : -1;
        const int dist = geom.width() + 22;
        QRect reach = geom;
        if (dir < 0) reach.setLeft(reach.left() - dist);
        else reach.setRight(reach.right() + dist);
        reach = reach.intersected(mw->rect());
        if (reach.isEmpty()) return;

        const QPointF base(geom.topLeft() - reach.topLeft());
        const QEasingCurve curve(QEasingCurve::InCubic);
        NanquimOverlay *o = new NanquimOverlay(mw, reach, [shot, base, dir, dist, curve](QPainter &g, const QRect &, qreal p) {
            const qreal e = curve.valueForProgress(p);
            g.setOpacity(qBound(0.0, 1.0 - e, 1.0));
            g.drawPixmap(base + QPointF(dir * e * dist, 0), shot);
        });
        runOverlay(s_focusBarOverlay, o, 180, QEasingCurve::Linear);
        return;
    }

    refreshFocusBarInfo(mw);
    flushLayouts();
    placeFocusWidgets(mw);

    if (!animate || !mw->centralWidget() || !mw->centralWidget()->isVisible()) {
        bar->show();
        bar->raise();
        return;
    }

    bar->ensurePolished();
    bar->adjustSize();
    placeFocusWidgets(mw);
    const QRect geom(bar->mapTo(mw, QPoint(0, 0)), bar->size());
    const QPixmap shot = lightShot(bar->grab());
    bar->hide();
    if (shot.isNull() || geom.isEmpty()) {
        bar->show();
        bar->raise();
        return;
    }

    const int dir = railOnRight() ? +1 : -1;
    const int dist = geom.width() + 24;
    QRect reach = geom;
    if (dir < 0) reach.setLeft(reach.left() - dist);
    else reach.setRight(reach.right() + dist);
    reach = reach.intersected(mw->rect());
    if (reach.isEmpty()) {
        bar->show();
        bar->raise();
        return;
    }

    const QPointF base(geom.topLeft() - reach.topLeft());
    const QEasingCurve curve = softSpring();
    NanquimOverlay *o = new NanquimOverlay(mw, reach, [shot, base, dir, dist, curve](QPainter &g, const QRect &, qreal p) {
        // espera 18% do tempo para a barra antiga recolher antes de surgir com mola
        const qreal local = qBound(0.0, (p - 0.18) / 0.82, 1.0);
        if (local <= 0.0) return;
        const qreal e = curve.valueForProgress(local);
        g.setOpacity(qBound(0.0, local * 1.6, 1.0));
        g.drawPixmap(base + QPointF(dir * (1.0 - e) * dist, 0), shot);
    });
    QPointer<KisMainWindow> mwGuard(mw);
    QPointer<QFrame> barGuard(bar);
    runOverlay(s_focusBarOverlay, o, 290, QEasingCurve::Linear, [mwGuard, barGuard]() {
        if (!mwGuard || !barGuard) return;
        if (mwGuard->property("nanquimOnCanvas").toBool() && (canvasOnlyActive(mwGuard) || s_focusMode)) {
            placeFocusWidgets(mwGuard);
            barGuard->show();
            barGuard->raise();
        }
    });
}

void buildFocusWidgets(KisMainWindow *mw)
{
    QFrame *bar = new QFrame(mw);
    bar->setObjectName(QStringLiteral("nanquimFocusBar"));
    bar->setFixedWidth(88);
    bar->setStyleSheet(QStringLiteral(
        "QFrame#nanquimFocusBar { background: " NQ_RELEVO2_BG "; border: 1px solid " NQ_RELEVO2_BORDA "; border-radius: 22px; }"
        "QFrame#nanquimFocusDivider { background: #4a4842; border: none; }"
        "QFrame#nanquimFocusBar QToolButton { border: 1px solid transparent; border-radius: 11px; padding: 0;"
        "  min-width: 52px; max-width: 52px; min-height: 34px; max-height: 34px; color: #eeeae0; background: transparent; }"
        "QFrame#nanquimFocusBar QToolButton:pressed { background: #3a3934; }"
        "QFrame#nanquimFocusBar QToolButton:checked { background: #2f7568; border: 1px solid #6fbfad; color: #ffffff; }"
        "QFrame#nanquimFocusBar QToolButton#nanquimFocusEraser:checked { background: #4a2933; border: 1px solid #ff6b8b; color: #ffffff; }"
        "QFrame#nanquimFocusBar QToolButton#nanquimFocusPicker { min-width: 34px; max-width: 34px; min-height: 30px; max-height: 30px;"
        "  background: #3a3934; border: 1px solid #524f48; border-radius: 9px; }"
        "QFrame#nanquimFocusBar QToolButton#nanquimFocusPicker:checked { background: #2f7568; border: 1px solid #6fbfad; }"
        "QFrame#nanquimFocusBar QToolButton#nanquimFocusBack { min-height: 26px; max-height: 26px; }"
        "QFrame#nanquimFocusBar QToolButton#nanquimFocusColor { min-width: 40px; max-width: 40px; min-height: 36px; max-height: 36px; }"
        "QFrame#nanquimFocusBar QToolButton#nanquimFocusUndo, QFrame#nanquimFocusBar QToolButton#nanquimFocusRedo,"
        "QFrame#nanquimFocusBar QToolButton#nanquimFocusBrushesButton, QFrame#nanquimFocusBar QToolButton#nanquimFocusLayersButton,"
        "QFrame#nanquimFocusBar QToolButton#nanquimFocusRulersButton, QFrame#nanquimFocusBar QToolButton#nanquimFocusSnapButton {"
        "  min-width: 38px; max-width: 38px; min-height: 38px; max-height: 38px; border-radius: 10px; }"
        "QFrame#nanquimFocusBar QToolButton#nanquimFocusPrev, QFrame#nanquimFocusBar QToolButton#nanquimFocusNext {"
        "  min-width: 18px; max-width: 18px; min-height: 22px; max-height: 22px; border-radius: 7px; }"
        "QFrame#nanquimFocusBar QToolButton#nanquimFocusMore { min-width: 26px; max-width: 26px; min-height: 26px; max-height: 26px; border-radius: 8px; }"
        "QFrame#nanquimFocusBar QToolButton::menu-indicator { image: none; width: 0px; }"
        "QPushButton#nanquimFocusHeader { border: none; border-radius: 8px; padding: 1px 2px; background: transparent;"
        "  color: #eeeae0; font-size: 11px; font-weight: 600; }"
        "QPushButton#nanquimFocusHeader:pressed { background: #3a3934; }"
        "QLabel#nanquimFocusVal { color: #a8a398; font-size: 11px; font-weight: 600; background: transparent; }"
        "QLabel#nanquimFocusPageLabel { color: #a8a398; font-size: 11px; font-weight: 600; background: transparent; }"
        "QPushButton#nanquimFocusExit { border: none; border-radius: 8px; padding: 2px 3px; min-height: 26px; max-height: 26px;"
        "  min-width: 34px; max-width: 40px; background: transparent; color: #eeeae0; font-size: 11px; font-weight: 600; }"
        "QPushButton#nanquimFocusExit:pressed { background: #3a3934; }"
        "QFrame#nanquimFocusBar QSlider:vertical { width: 32px; background: transparent; }"
        "QFrame#nanquimFocusBar QSlider::groove:vertical { width: 6px; background: #4a4842; border-radius: 3px; }"
        "QFrame#nanquimFocusBar QSlider::sub-page:vertical { width: 6px; background: #4a4842; border-radius: 3px; }"
        "QFrame#nanquimFocusBar QSlider::add-page:vertical { width: 6px; background: #6fbfad; border-radius: 3px; }"
        "QFrame#nanquimFocusBar QSlider::handle:vertical { height: 20px; width: 20px; margin: 0 -7px;"
        "  background: #eeeae0; border: 2px solid #2f7568; border-radius: 10px; }"));

    QVBoxLayout *l = new QVBoxLayout(bar);
    l->setContentsMargins(6, 7, 6, 7);
    l->setSpacing(2);
    l->setAlignment(Qt::AlignHCenter);

    QPointer<KisMainWindow> mwGuard(mw);

    // 1. Voltar à galeria e título/página compactos (tocar no título abre a faixa do capítulo)
    QToolButton *backBtn = iconButton(bar, i18n("Voltar à galeria"), designIcon(Svg::BACK, 16), false);
    backBtn->setObjectName(QStringLiteral("nanquimFocusBack"));
    backBtn->setIconSize(QSize(16, 16));
    QObject::connect(backBtn, &QToolButton::clicked, mw, [mwGuard]() {
        if (!mwGuard) return;
        if (QAction *a = act(mwGuard, "view_show_canvas_only")) {
            if (a->isChecked()) a->trigger();
        }
        showGalleryAnimated(mwGuard);
    });
    l->addWidget(backBtn, 0, Qt::AlignHCenter);

    QPushButton *headerBtn = new QPushButton(QStringLiteral("NANQUIM\np. 1"), bar);
    headerBtn->setObjectName(QStringLiteral("nanquimFocusHeader"));
    headerBtn->setToolTip(i18n("Páginas do capítulo"));
    headerBtn->setFocusPolicy(Qt::NoFocus);
    headerBtn->setFixedWidth(70);
    QObject::connect(headerBtn, &QPushButton::clicked, mw, [mwGuard]() {
        if (mwGuard) toggleFilmstripDrawer(mwGuard);
    });
    l->addWidget(headerBtn, 0, Qt::AlignHCenter);
    l->addWidget(focusDivider(bar), 0, Qt::AlignHCenter);

    // 2. Cor atual (toque abre o popover de cor; toque longo abre a roda de atalhos)
    QToolButton *colorBtn = new QToolButton(bar);
    colorBtn->setObjectName(QStringLiteral("nanquimFocusColor"));
    colorBtn->setProperty("nanquimPopup", QStringLiteral("rail"));
    colorBtn->setToolTip(i18n("Cor"));
    colorBtn->setFocusPolicy(Qt::NoFocus);
    colorBtn->setAutoRaise(true);
    colorBtn->setIconSize(QSize(28, 28));
    auto refreshColor = [mwGuard, colorBtn]() {
        if (!mwGuard || !mwGuard->viewManager() || !mwGuard->viewManager()->canvasResourceProvider()) return;
        colorBtn->setIcon(swatchIcon(mwGuard->viewManager()->canvasResourceProvider()->fgColor().toQColor(), 28));
    };
    refreshColor();
    if (KisViewManager *vm = mw->viewManager()) {
        if (vm->canvasResourceProvider()) {
            QObject::connect(vm->canvasResourceProvider(), &KisCanvasResourceProvider::sigFGColorChanged, colorBtn,
                             [refreshColor](const KoColor &) { refreshColor(); });
        }
    }
    QTimer *colorHold = new QTimer(colorBtn);
    colorHold->setSingleShot(true);
    colorHold->setInterval(400);
    QSharedPointer<bool> colorHeld(new bool(false));
    QObject::connect(colorBtn, &QToolButton::pressed, colorHold, [colorHold, colorHeld]() {
        *colorHeld = false;
        colorHold->start();
    });
    QObject::connect(colorBtn, &QToolButton::released, colorHold, &QTimer::stop);
    QObject::connect(colorHold, &QTimer::timeout, mw, [mwGuard, colorBtn, colorHeld]() {
        if (!mwGuard) return;
        *colorHeld = true;
        openWheel(mwGuard, colorBtn->mapTo(mwGuard, colorBtn->rect().center()));
    });
    QObject::connect(colorBtn, &QToolButton::clicked, mw, [mwGuard, colorBtn, colorHeld]() {
        if (!mwGuard) return;
        if (*colorHeld) {
            *colorHeld = false;
            return;
        }
        QDockWidget *dw = findDock(mwGuard, {QStringLiteral("NanquimColorDock"), QStringLiteral("ColorSelectorNg"),
                                             QStringLiteral("WideGamutColorSelector")});
        togglePanelFrom(mwGuard, dw, colorBtn);
    });
    l->addWidget(colorBtn, 0, Qt::AlignHCenter);

    // 3. Ferramentas essenciais: Pincel, Borracha, Lata, Seleção (Laço) e Transformar
    QAction *erase = act(mw, "erase_action");
    struct Entry { const char *tip; const char *svg; const char *id; bool eraser; const char *objName; };
    const Entry entries[] = {
        {"Pincel", Svg::BRUSH, "KritaShape/KisToolBrush", false, "nanquimFocusBrush"},
        {"Borracha", Svg::ERASER, "KritaShape/KisToolBrush", true, "nanquimFocusEraser"},
        {"Lata de tinta", Svg::BUCKET, "KritaFill/KisToolFill", false, "nanquimFocusBucket"},
        {"Seleção", Svg::SELECT, "KisToolSelectOutline", false, "nanquimFocusSelect"},
        {"Transformar", Svg::TRANSFORM, "KisToolTransform", false, "nanquimFocusTransform"},
    };
    QList<QPair<QToolButton*, Entry>> buttons;
    for (const Entry &en : entries) {
        QAction *tool = act(mw, en.id);
        if (!tool || (en.eraser && !erase)) continue;
        QToolButton *b = iconButton(bar, i18n(en.tip), designIcon(en.svg, 20), true);
        b->setObjectName(QLatin1String(en.objName));
        b->setIconSize(QSize(20, 20));
        const Entry copy = en;
        QObject::connect(b, &QToolButton::clicked, mw, [copy, tool, erase]() {
            tool->trigger();
            if (erase && QLatin1String(copy.id) == QLatin1String("KritaShape/KisToolBrush")
                    && erase->isChecked() != copy.eraser) {
                erase->trigger();
            }
        });
        l->addWidget(b, 0, Qt::AlignHCenter);
        buttons << qMakePair(b, en);
    }
    l->addWidget(focusDivider(bar), 0, Qt::AlignHCenter);

    // 4. Sliders diretos (Tamanho + Conta-gotas no meio + Opacidade) e Desfazer/Refazer
    QLabel *sizeLabel = new QLabel(QStringLiteral("—"), bar);
    sizeLabel->setObjectName(QStringLiteral("nanquimFocusVal"));
    sizeLabel->setAlignment(Qt::AlignCenter);
    QSlider *sizeSlider = new QSlider(Qt::Vertical, bar);
    sizeSlider->setRange(0, 1000);
    sizeSlider->setMinimumHeight(46);
    sizeSlider->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    sizeSlider->setFocusPolicy(Qt::NoFocus);
    sizeSlider->setToolTip(i18n("Tamanho do pincel"));

    QToolButton *pickerBtn = nullptr;
    if (QAction *pickerAct = act(mw, "KritaSelected/KisToolColorSampler")) {
        pickerBtn = iconButton(bar, i18n("Conta-gotas"), designIcon(Svg::PICKER, 17), true);
        pickerBtn->setObjectName(QStringLiteral("nanquimFocusPicker"));
        pickerBtn->setIconSize(QSize(20, 20));
        QObject::connect(pickerBtn, &QToolButton::clicked, pickerAct, &QAction::trigger);
        buttons << qMakePair(pickerBtn, Entry{"Conta-gotas", Svg::PICKER, "KritaSelected/KisToolColorSampler", false, "nanquimFocusPicker"});
    }

    QSlider *opSlider = new QSlider(Qt::Vertical, bar);
    opSlider->setRange(1, 100);
    opSlider->setValue(100);
    opSlider->setMinimumHeight(42);
    opSlider->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    opSlider->setFocusPolicy(Qt::NoFocus);
    opSlider->setToolTip(i18n("Opacidade"));
    QLabel *opLabel = new QLabel(QStringLiteral("100%"), bar);
    opLabel->setObjectName(QStringLiteral("nanquimFocusVal"));
    opLabel->setAlignment(Qt::AlignCenter);

    l->addWidget(sizeLabel, 0, Qt::AlignHCenter);
    l->addWidget(sizeSlider, 2, Qt::AlignHCenter);
    if (pickerBtn) l->addWidget(pickerBtn, 0, Qt::AlignHCenter);
    l->addWidget(opSlider, 2, Qt::AlignHCenter);
    l->addWidget(opLabel, 0, Qt::AlignHCenter);

    QObject::connect(sizeSlider, &QSlider::valueChanged, mw, [mwGuard, sizeLabel](int v) {
        if (!mwGuard) return;
        const qreal px = sliderToSize(v);
        sizeLabel->setText(QString::number(qRound(px)));
        if (KoCanvasResourceProvider *r = resources(mwGuard)) {
            r->setResource(KoCanvasResource::Size, px);
            showBrushHud(mwGuard);
        }
    });
    QObject::connect(opSlider, &QSlider::valueChanged, mw, [mwGuard, opLabel](int v) {
        if (!mwGuard) return;
        opLabel->setText(QStringLiteral("%1%").arg(v));
        if (KoCanvasResourceProvider *r = resources(mwGuard)) {
            r->setResource(KoCanvasResource::Opacity, v / 100.0);
            showBrushHud(mwGuard);
        }
    });
    auto pullSliders = [mwGuard, sizeSlider, sizeLabel, opSlider, opLabel]() {
        if (!mwGuard) return;
        KoCanvasResourceProvider *r = resources(mwGuard);
        if (!r) return;
        const QVariant sv = r->resource(KoCanvasResource::Size);
        if (sv.isValid()) {
            QSignalBlocker block(sizeSlider);
            sizeSlider->setValue(sizeToSlider(sv.toReal()));
            sizeLabel->setText(QString::number(qRound(sv.toReal())));
        }
        const QVariant ov = r->resource(KoCanvasResource::Opacity);
        if (ov.isValid()) {
            QSignalBlocker block(opSlider);
            const int pct = qBound(1, qRound(ov.toReal() * 100.0), 100);
            opSlider->setValue(pct);
            opLabel->setText(QStringLiteral("%1%").arg(pct));
        }
    };
    if (KoCanvasResourceProvider *r = resources(mw)) {
        QObject::connect(r, &KoCanvasResourceProvider::canvasResourceChanged, bar, [pullSliders](int key, const QVariant &) {
            if (key == KoCanvasResource::Size || key == KoCanvasResource::Opacity
                    || key == KoCanvasResource::CurrentPaintOpPreset) {
                pullSliders();
            }
        });
    }
    QTimer::singleShot(0, bar, pullSliders);

    // Desfazer e Refazer logo abaixo dos sliders
    QHBoxLayout *undoRow = new QHBoxLayout();
    undoRow->setContentsMargins(0, 2, 0, 0);
    undoRow->setSpacing(2);
    for (const char *name : {"edit_undo", "edit_redo"}) {
        const bool isUndo = QLatin1String(name) == QLatin1String("edit_undo");
        QToolButton *b = iconButton(bar, isUndo ? i18n("Desfazer") : i18n("Refazer"),
                                    designIcon(isUndo ? Svg::UNDO : Svg::REDO, 16), false);
        b->setObjectName(isUndo ? QStringLiteral("nanquimFocusUndo") : QStringLiteral("nanquimFocusRedo"));
        b->setIconSize(QSize(16, 16));
        if (QAction *a = act(mw, name)) {
            b->setEnabled(a->isEnabled());
            QObject::connect(b, &QToolButton::clicked, mw, [mwGuard, a, isUndo]() {
                a->trigger();
                if (mwGuard) showToast(mwGuard, isUndo ? i18n("Desfazer") : i18n("Refazer"));
            });
            QObject::connect(a, &QAction::changed, b, [a, b]() { b->setEnabled(a->isEnabled()); });
        }
        undoRow->addWidget(b);
    }
    l->addLayout(undoRow);
    l->addWidget(focusDivider(bar), 0, Qt::AlignHCenter);

    // 5. Pincéis e Camadas lado a lado
    QHBoxLayout *panelsRow = new QHBoxLayout();
    panelsRow->setContentsMargins(0, 0, 0, 0);
    panelsRow->setSpacing(2);
    QToolButton *brushesBtn = iconButton(bar, i18n("Pincéis"), designIcon(Svg::PEN, 20), false);
    brushesBtn->setObjectName(QStringLiteral("nanquimFocusBrushesButton"));
    brushesBtn->setIconSize(QSize(20, 20));
    QObject::connect(brushesBtn, &QToolButton::clicked, mw, [mwGuard, brushesBtn]() {
        if (!mwGuard) return;
        QFrame *panel = mwGuard->findChild<QFrame*>(BRUSH_PANEL_ID);
        const bool wasOpen = panel && panel->isVisible();
        setBrushPanelVisible(mwGuard, !wasOpen);
        if (!wasOpen) s_popupAnchor = brushesBtn;
    });
    panelsRow->addWidget(brushesBtn);

    if (QDockWidget *layersDock = findDock(mw, {QStringLiteral("KisLayerBox")})) {
        QToolButton *layersBtn = iconButton(bar, i18n("Camadas"), designIcon(Svg::LAYERS, 20), true);
        layersBtn->setObjectName(QStringLiteral("nanquimFocusLayersButton"));
        layersBtn->setProperty("nanquimPopup", QStringLiteral("rail"));
        layersBtn->setIconSize(QSize(20, 20));
        QObject::connect(layersBtn, &QToolButton::clicked, mw, [mwGuard, layersDock, layersBtn]() {
            if (!mwGuard) return;
            togglePanelFrom(mwGuard, layersDock, layersBtn);
            layersBtn->setChecked(panelOpen(mwGuard, layersDock));
        });
        QObject::connect(layersDock->toggleViewAction(), &QAction::toggled, layersBtn, [layersBtn](bool on) {
            layersBtn->setChecked(on);
        });
        panelsRow->addWidget(layersBtn);
    }
    l->addLayout(panelsRow);

    // 5b. Réguas (Perspectiva/Paralela/Retícula/Extrair linha) e Ímã (Snap) lado a lado
    QHBoxLayout *rulersRow = new QHBoxLayout();
    rulersRow->setContentsMargins(0, 0, 0, 0);
    rulersRow->setSpacing(2);
    QToolButton *rulersBtn = iconButton(bar, i18n("Quadrinhos e réguas"), designIcon(Svg::RULER, 20), false);
    rulersBtn->setObjectName(QStringLiteral("nanquimFocusRulersButton"));
    rulersBtn->setProperty("nanquimPopup", QStringLiteral("rail"));
    rulersBtn->setIconSize(QSize(20, 20));
    QMenu *focusRulerMenu = new QMenu(rulersBtn);
    QObject::connect(focusRulerMenu, &QMenu::aboutToShow, mw, [mwGuard, focusRulerMenu, rulersBtn]() {
        if (mwGuard) populateRulersMenu(mwGuard, focusRulerMenu, rulersBtn);
    });
    rulersBtn->setMenu(focusRulerMenu);
    rulersBtn->setPopupMode(QToolButton::InstantPopup);
    rulersRow->addWidget(rulersBtn);

    QToolButton *snapBtn = iconButton(bar, i18n("Ímã nas réguas (Snap)"), designIcon(Svg::MAGNET, 20), true);
    snapBtn->setObjectName(QStringLiteral("nanquimFocusSnapButton"));
    snapBtn->setIconSize(QSize(20, 20));
    if (QAction *snapAct = act(mw, "toggle_assistant")) {
        snapBtn->setChecked(snapAct->isChecked());
        QObject::connect(snapBtn, &QToolButton::clicked, mw, [mwGuard, snapAct, snapBtn]() {
            snapAct->trigger();
            const bool on = snapAct->isChecked();
            snapBtn->setChecked(on);
            if (mwGuard) showToast(mwGuard, on ? i18n("Ímã nas réguas: ligado") : i18n("Ímã nas réguas: desligado"));
        });
        QObject::connect(snapAct, &QAction::toggled, snapBtn, [snapBtn](bool on) {
            snapBtn->setChecked(on);
        });
        QObject::connect(snapAct, &QAction::changed, snapBtn, [snapAct, snapBtn]() {
            snapBtn->setChecked(snapAct->isChecked());
        });
    } else {
        snapBtn->setEnabled(false);
    }
    rulersRow->addWidget(snapBtn);
    l->addLayout(rulersRow);

    // 6. Paginação compacta em linha única (< 1/1 >)
    QHBoxLayout *pageRow = new QHBoxLayout();
    pageRow->setContentsMargins(0, 0, 0, 0);
    pageRow->setSpacing(1);
    QToolButton *prevBtn = iconButton(bar, i18n("Página anterior"), designIcon(Svg::BACK, 12), false);
    prevBtn->setObjectName(QStringLiteral("nanquimFocusPrev"));
    prevBtn->setIconSize(QSize(12, 12));
    prevBtn->setEnabled(false);
    QLabel *pageLabel = new QLabel(QStringLiteral("1/1"), bar);
    pageLabel->setObjectName(QStringLiteral("nanquimFocusPageLabel"));
    pageLabel->setAlignment(Qt::AlignCenter);
    QToolButton *nextBtn = iconButton(bar, i18n("Próxima página"), designIcon(Svg::FORWARD, 12), false);
    nextBtn->setObjectName(QStringLiteral("nanquimFocusNext"));
    nextBtn->setIconSize(QSize(12, 12));
    nextBtn->setEnabled(false);
    QObject::connect(prevBtn, &QToolButton::clicked, mw, [mwGuard, bar]() {
        if (!mwGuard) return;
        const QString p = bar->property("nanquimFocusPrevPath").toString();
        if (!p.isEmpty()) focusBarOpenPage(mwGuard, p);
    });
    QObject::connect(nextBtn, &QToolButton::clicked, mw, [mwGuard, bar]() {
        if (!mwGuard) return;
        const QString p = bar->property("nanquimFocusNextPath").toString();
        if (!p.isEmpty()) focusBarOpenPage(mwGuard, p);
    });
    pageRow->addWidget(prevBtn);
    pageRow->addWidget(pageLabel, 1);
    pageRow->addWidget(nextBtn);
    l->addLayout(pageRow);
    l->addWidget(focusDivider(bar), 0, Qt::AlignHCenter);

    // 7. Mais opções (...) e Sair lado a lado
    QHBoxLayout *footRow = new QHBoxLayout();
    footRow->setContentsMargins(0, 0, 0, 0);
    footRow->setSpacing(2);
    QToolButton *moreBtn = iconButton(bar, i18n("Mais opções"), designIcon(Svg::MORE, 16), false);
    moreBtn->setObjectName(QStringLiteral("nanquimFocusMore"));
    moreBtn->setIconSize(QSize(16, 16));
    QMenu *moreMenu = new QMenu(moreBtn);
    QObject::connect(moreMenu, &QMenu::aboutToShow, mw, [mwGuard, moreMenu]() {
        moreMenu->clear();
        if (!mwGuard || !mwGuard->menuBar()) return;
        Q_FOREACH (QAction *a, mwGuard->menuBar()->actions()) {
            moreMenu->addAction(a);
        }
    });
    moreBtn->setMenu(moreMenu);
    moreBtn->setPopupMode(QToolButton::InstantPopup);
    footRow->addWidget(moreBtn);

    QPushButton *exitBtn = new QPushButton(i18n("Sair"), bar);
    exitBtn->setObjectName(QStringLiteral("nanquimFocusExit"));
    exitBtn->setToolTip(i18n("Sair do modo foco"));
    exitBtn->setFocusPolicy(Qt::NoFocus);
    QObject::connect(exitBtn, &QPushButton::clicked, mw, [mwGuard]() {
        if (!mwGuard) return;
        if (QAction *a = act(mwGuard, "view_show_canvas_only")) {
            if (a->isChecked()) {
                a->trigger();
                return;
            }
        }
        setFocusMode(mwGuard, false);
    });
    footRow->addWidget(exitBtn);
    l->addLayout(footRow);

    // Sincroniza ferramenta ativa e avisa discretamente ao trocar no modo foco
    KoToolManager *toolManager = KoToolManager::instance();
    auto sync = [buttons, erase, toolManager, pullSliders]() {
        const QString active = toolManager->activeToolId();
        const bool erasing = erase && erase->isChecked();
        for (const auto &pair : buttons) {
            const QLatin1String id(pair.second.id);
            const bool isBrush = id == QLatin1String("KritaShape/KisToolBrush");
            const bool isSelect = id == QLatin1String("KisToolSelectOutline");
            const bool matched = isSelect
                    ? active.startsWith(QLatin1String("KisToolSelect"))
                    : (active == id && (!isBrush || pair.second.eraser == erasing));
            pair.first->setChecked(matched);
        }
        pullSliders();
    };
    QObject::connect(toolManager, &KoToolManager::changedTool, bar, [sync, mwGuard, toolManager, erase](KoCanvasController *) {
        sync();
        static QString last;
        if (!mwGuard || (!canvasOnlyActive(mwGuard) && !s_focusMode)) {
            last.clear();
            return;
        }
        const QString id = toolManager->activeToolId();
        const bool erasing = erase && erase->isChecked();
        QString name;
        if (id == QLatin1String("KritaShape/KisToolBrush")) name = erasing ? i18n("Borracha") : i18n("Pincel");
        else if (id == QLatin1String("KritaFill/KisToolFill")) name = i18n("Lata de tinta");
        else if (id.startsWith(QLatin1String("KisToolSelect"))) name = i18n("Seleção");
        else if (id == QLatin1String("KisToolTransform")) name = i18n("Transformar");
        else if (id == QLatin1String("KritaSelected/KisToolColorSampler")) name = i18n("Conta-gotas");
        if (name.isEmpty()) return;
        const QString key = id + (erasing ? QStringLiteral("-e") : QString());
        if (key == last) return;
        last = key;
        showToast(mwGuard, name);
    });
    if (erase) QObject::connect(erase, &QAction::toggled, bar, [sync](bool) { sync(); });
    QTimer::singleShot(0, bar, sync);
    bar->adjustSize();
    bar->hide();
}

// ---------------------------------------------------------------------------
// Barra contextual de Seleção: cápsula horizontal compacta no rodapé que só
// aparece quando uma ferramenta de seleção está ativa, e some ao voltar para
// o pincel ou sair do desenho.
// ---------------------------------------------------------------------------
const QString SELECTION_BAR_ID = QStringLiteral("nanquimSelectionBar");
QPointer<NanquimOverlay> s_selBarOverlay;

void placeSelectionBar(KisMainWindow *mw)
{
    if (!mw) return;
    QFrame *bar = mw->findChild<QFrame*>(SELECTION_BAR_ID);
    QWidget *central = mw->centralWidget();
    if (!bar || !central) return;
    const QRect area(central->mapTo(mw, QPoint(0, 0)), central->size());
    int bottomInset = 0;
    if (QScreen *screen = mw->screen()) {
        const QRect avail = screen->availableGeometry();
        const int winBottom = mw->mapToGlobal(QPoint(0, mw->height())).y();
        bottomInset = qBound(0, winBottom - (avail.bottom() + 1), 160);
    }
    bar->adjustSize();
    const int x = qBound(area.left() + 12, area.center().x() - bar->width() / 2,
                         qMax(area.left() + 12, area.right() - 12 - bar->width() + 1));
    const int y = qMax(area.top() + 12, area.bottom() - bottomInset - 14 - bar->height() + 1);
    bar->move(x, y);
    if (bar->isVisible()) bar->raise();
}

void updateSelectionBar(KisMainWindow *mw, bool animate)
{
    if (!mw || s_classic) return;
    QFrame *bar = mw->findChild<QFrame*>(SELECTION_BAR_ID);
    if (!bar) return;
    const bool onCanvas = mw->property("nanquimOnCanvas").toBool();
    KoToolManager *tm = KoToolManager::instance();
    const QString active = tm ? tm->activeToolId() : QString();
    const bool want = onCanvas && active.startsWith(QLatin1String("KisToolSelect"));
    const bool cur = bar->property("nanquimSelOpen").toBool();
    if (want == cur) {
        if (want) placeSelectionBar(mw);
        return;
    }
    bar->setProperty("nanquimSelOpen", want);
    if (s_selBarOverlay) {
        s_selBarOverlay->deleteLater();
        s_selBarOverlay.clear();
    }

    if (!want) {
        if (!bar->isVisible()) return;
        if (!animate || !mw->centralWidget() || !mw->centralWidget()->isVisible()) {
            bar->hide();
            return;
        }
        const QRect geom(bar->mapTo(mw, QPoint(0, 0)), bar->size());
        const QPixmap shot = lightShot(bar->grab());
        bar->hide();
        if (shot.isNull() || geom.isEmpty()) return;
        const int dist = geom.height() + 18;
        QRect reach = geom.adjusted(0, 0, 0, dist).intersected(mw->rect());
        if (reach.isEmpty()) return;
        const QPointF base(geom.topLeft() - reach.topLeft());
        const QEasingCurve curve(QEasingCurve::InCubic);
        NanquimOverlay *o = new NanquimOverlay(mw, reach, [shot, base, dist, curve](QPainter &g, const QRect &, qreal p) {
            const qreal e = curve.valueForProgress(p);
            g.setOpacity(qBound(0.0, 1.0 - e, 1.0));
            g.drawPixmap(base + QPointF(0, e * dist), shot);
        });
        runOverlay(s_selBarOverlay, o, 140, QEasingCurve::Linear);
        return;
    }

    placeSelectionBar(mw);
    if (!animate || !mw->centralWidget() || !mw->centralWidget()->isVisible()) {
        bar->show();
        bar->raise();
        return;
    }
    bar->ensurePolished();
    bar->adjustSize();
    placeSelectionBar(mw);
    const QRect geom(bar->mapTo(mw, QPoint(0, 0)), bar->size());
    const QPixmap shot = lightShot(bar->grab());
    bar->hide();
    if (shot.isNull() || geom.isEmpty()) {
        bar->show();
        bar->raise();
        return;
    }
    const int dist = geom.height() + 18;
    QRect reach = geom.adjusted(0, 0, 0, dist).intersected(mw->rect());
    if (reach.isEmpty()) {
        bar->show();
        bar->raise();
        return;
    }
    const QPointF base(geom.topLeft() - reach.topLeft());
    const QEasingCurve curve = softSpring();
    NanquimOverlay *o = new NanquimOverlay(mw, reach, [shot, base, dist, curve](QPainter &g, const QRect &, qreal p) {
        const qreal e = curve.valueForProgress(p);
        g.setOpacity(qBound(0.0, p * 1.8, 1.0));
        g.drawPixmap(base + QPointF(0, (1.0 - e) * dist), shot);
    });
    QPointer<KisMainWindow> mwGuard(mw);
    QPointer<QFrame> barGuard(bar);
    runOverlay(s_selBarOverlay, o, 210, QEasingCurve::Linear, [mwGuard, barGuard]() {
        if (!mwGuard || !barGuard) return;
        if (barGuard->property("nanquimSelOpen").toBool()) {
            placeSelectionBar(mwGuard);
            barGuard->show();
            barGuard->raise();
        }
    });
}

void buildSelectionBar(KisMainWindow *mw)
{
    QFrame *bar = new QFrame(mw);
    bar->setObjectName(SELECTION_BAR_ID);
    bar->setStyleSheet(QStringLiteral(
        "QFrame#nanquimSelectionBar { background: " NQ_RELEVO2_BG "; border: 1px solid " NQ_RELEVO2_BORDA "; border-radius: 20px; }"
        "QFrame#nanquimSelDivider { background: #4a4842; border: none; }"
        "QFrame#nanquimSelectionBar QToolButton { border: 1px solid transparent; border-radius: 12px; padding: 0 8px;"
        "  min-height: 36px; max-height: 36px; color: #eeeae0; font-size: 12px; font-weight: 600; background: transparent; }"
        "QFrame#nanquimSelectionBar QToolButton:pressed { background: #3a3934; }"
        "QFrame#nanquimSelectionBar QToolButton:checked { background: #2f7568; border: 1px solid #6fbfad; color: #ffffff; }"
        "QFrame#nanquimSelectionBar QToolButton#nanquimSelTone { background: #253d37; border: 1px solid #3e8e7e; color: #d8f2eb; }"
        "QFrame#nanquimSelectionBar QToolButton#nanquimSelTone:pressed { background: #2f7568; color: #ffffff; }"
        "QFrame#nanquimSelectionBar QToolButton#nanquimSelClear:pressed { background: #4a2933; border: 1px solid #ff6b8b; }"));

    QHBoxLayout *l = new QHBoxLayout(bar);
    l->setContentsMargins(8, 6, 8, 6);
    l->setSpacing(3);

    QPointer<KisMainWindow> mwGuard(mw);
    struct SelMode { const char *label; const char *id; };
    const SelMode modes[] = {
        {"Laço", "KisToolSelectOutline"},
        {"Retângulo", "KisToolSelectRectangular"},
        {"Varinha", "KisToolSelectContiguous"},
    };
    QList<QPair<QToolButton*, QString>> modeButtons;
    for (const SelMode &m : modes) {
        QAction *a = act(mw, m.id);
        if (!a) continue;
        QToolButton *b = new QToolButton(bar);
        b->setText(i18n(m.label));
        b->setToolTip(i18n(m.label));
        b->setCheckable(true);
        b->setAutoRaise(true);
        b->setFocusPolicy(Qt::NoFocus);
        QObject::connect(b, &QToolButton::clicked, a, &QAction::trigger);
        l->addWidget(b);
        modeButtons << qMakePair(b, QString::fromLatin1(m.id));
    }

    QFrame *div = new QFrame(bar);
    div->setObjectName(QStringLiteral("nanquimSelDivider"));
    div->setFixedSize(1, 22);
    l->addWidget(div, 0, Qt::AlignVCenter);

    // Ação de mangá em 1 toque: retícula recortada pela seleção ativa
    {
        QToolButton *toneBtn = new QToolButton(bar);
        toneBtn->setObjectName(QStringLiteral("nanquimSelTone"));
        toneBtn->setText(i18n("Retícula"));
        toneBtn->setToolTip(i18n("Aplicar retícula de mangá na seleção"));
        toneBtn->setAutoRaise(true);
        toneBtn->setFocusPolicy(Qt::NoFocus);
        QObject::connect(toneBtn, &QToolButton::clicked, mw, [mwGuard]() {
            if (mwGuard) showScreentoneSheet(mwGuard);
        });
        l->addWidget(toneBtn);
    }

    struct SelAct { const char *label; const char *id; const char *toast; const char *obj; };
    const SelAct acts[] = {
        {"Preencher", "fill_selection_foreground_color", "Preenchido com a cor atual", "nanquimSelFill"},
        {"Inverter", "invert_selection", "Seleção invertida", "nanquimSelInvert"},
        {"Copiar", "edit_copy", "Copiado", "nanquimSelCopy"},
        {"Colar", "edit_paste", "Colado", "nanquimSelPaste"},
        {"Limpar", "clear", "Área limpa", "nanquimSelClear"},
        {"Soltar", "deselect", "Seleção removida", "nanquimSelDeselect"},
    };
    for (const SelAct &sa : acts) {
        QAction *a = act(mw, sa.id);
        if (!a) continue;
        QToolButton *b = new QToolButton(bar);
        b->setObjectName(QLatin1String(sa.obj));
        b->setText(i18n(sa.label));
        b->setToolTip(i18n(sa.label));
        b->setAutoRaise(true);
        b->setFocusPolicy(Qt::NoFocus);
        const QString msg = i18n(sa.toast);
        QObject::connect(b, &QToolButton::clicked, mw, [mwGuard, a, msg]() {
            a->trigger();
            if (mwGuard) showToast(mwGuard, msg);
        });
        l->addWidget(b);
    }

    if (KoToolManager *tm = KoToolManager::instance()) {
        auto syncModes = [modeButtons, tm]() {
            const QString active = tm->activeToolId();
            for (const auto &p : modeButtons) {
                p.first->setChecked(active == p.second);
            }
        };
        QObject::connect(tm, &KoToolManager::changedTool, bar, [mwGuard, syncModes](KoCanvasController *) {
            syncModes();
            if (mwGuard) updateSelectionBar(mwGuard, true);
        });
        QTimer::singleShot(0, bar, syncModes);
    }
    bar->setProperty("nanquimSelOpen", false);
    bar->adjustSize();
    bar->hide();
}

void applyFocusVisibility(KisMainWindow *mw)
{
    const bool on = s_focusMode;
    setKritaChromeHidden(mw, on || NANQUIM_ESTUDIO);
    if (QToolBar *top = mw->findChild<QToolBar*>(TOP_ID)) top->setVisible(!on);
    if (QToolBar *nav = mw->findChild<QToolBar*>(NAV_ID)) nav->setVisible(!on && !s_navCollapsed);
    if (QWidget *tab = mw->findChild<QWidget*>(QStringLiteral("nanquimExpandTab"))) tab->setVisible(!on && s_navCollapsed);
    setThumbDockVisible(mw, true);
    flushLayouts();
    showFocusBarAnimated(mw, on, true);
    if (!on && s_navCollapsed) {
        if (QToolButton *tab = mw->findChild<QToolButton*>(QStringLiteral("nanquimExpandTab"))) positionExpandTab(mw, tab);
    }
}

void setFocusMode(KisMainWindow *mw, bool on)
{
    if (s_focusMode == on) return;
    s_focusMode = on;
    if (on) {
        if (QDockWidget *dw = hostedDock(mw)) closePanel(mw, dw, false);
        setBrushPanelVisible(mw, false);
        if (QFrame *flyout = mw->findChild<QFrame*>(FLYOUT_ID)) flyout->hide();
        if (QWidget *wheel = mw->findChild<QWidget*>(QStringLiteral("nanquimWheel"))) wheel->hide();
    }
    applyFocusVisibility(mw);
    if (on) showToast(mw, i18n("Modo foco · dois dedos desfazem, três refazem"));
}

// troca o lado da barra (Configurações → Interface) sem reabrir o app
void setRailSide(KisMainWindow *mw, bool right)
{
    KConfigGroup cfg = KSharedConfig::openConfig()->group("Nanquim");
    cfg.writeEntry("railSide", right ? QStringLiteral("right") : QStringLiteral("left"));
    cfg.sync();
    if (QToolBar *nav = mw->findChild<QToolBar*>(NAV_ID)) {
        const bool visible = nav->isVisible();
        mw->removeToolBar(nav);
        mw->addToolBar(right ? Qt::RightToolBarArea : Qt::LeftToolBarArea, nav);
        nav->setStyleSheet(navStyle());
        nav->setVisible(visible);
        if (QToolButton *collapse = nav->findChild<QToolButton*>(QStringLiteral("nanquimCollapse"))) {
            collapse->setIcon(designIcon(right ? Svg::FORWARD : Svg::BACK, 20));
        }
    }
    if (QToolButton *tab = mw->findChild<QToolButton*>(QStringLiteral("nanquimExpandTab"))) {
        tab->setIcon(designIcon(right ? Svg::BACK : Svg::FORWARD, 20));
        tab->setStyleSheet(expandTabStyle());
        tab->adjustSize();
    }
    // painel aberto vai para o outro lado
    Q_FOREACH (QDockWidget *dw, mw->dockWidgets()) {
        if (dw->isVisible() && !dw->isFloating()) {
            mw->removeDockWidget(dw);
            mw->addDockWidget(panelArea(), dw);
            dw->show();
        }
    }
    if (QFrame *flyout = mw->findChild<QFrame*>(FLYOUT_ID)) flyout->hide();
    flushLayouts();
    if (QToolButton *tab = mw->findChild<QToolButton*>(QStringLiteral("nanquimExpandTab"))) {
        positionExpandTab(mw, tab);
    }
    placeThumbDock(mw);
    placeStudio(mw);
}


// ---------------------------------------------------------------------------
// Galeria (design "Main"): substitui a tela inicial do Krita
// ---------------------------------------------------------------------------
const QString GALLERY_ID = QStringLiteral("nanquimGallery");

QPixmap renderSvg(const QString &svg, const QSize &logical)
{
    const qreal dpr = qApp->devicePixelRatio();
    QByteArray data = svg.toUtf8();
    QBuffer buffer(&data);
    buffer.open(QIODevice::ReadOnly);
    QImageReader reader(&buffer, "svg");
    reader.setScaledSize(logical * dpr);
    QImage image = reader.read();
    if (image.isNull()) {
        return QPixmap();
    }
    image.setDevicePixelRatio(dpr);
    return QPixmap::fromImage(image);
}

// Marca NANQUIM: Chillax Bold (o CI baixa; se falhar, Montserrat Bold), engrossada
// com um contorno de 0,045 em e letras um pouco mais juntas, como no design.
QPixmap brandWordmark(int pixelSize, const QColor &color)
{
    static QString s_family;
    if (s_family.isEmpty()) {
        const int id = QFontDatabase::addApplicationFont(QStringLiteral(":/splash/fonts/brand-bold.otf"));
        const QStringList families = id >= 0 ? QFontDatabase::applicationFontFamilies(id) : QStringList();
        s_family = families.isEmpty() ? QStringLiteral("Montserrat") : families.first();
    }
    QFont f(s_family);
    f.setPixelSize(pixelSize);
    f.setWeight(QFont::Bold);
    f.setLetterSpacing(QFont::PercentageSpacing, 98.5);
    const QString text = QStringLiteral("NANQUIM");
    QPainterPath path;
    path.addText(0, 0, f, text);
    const qreal stroke = 0.045 * pixelSize;
    const QRectF box = path.boundingRect().adjusted(-stroke, -stroke, stroke, stroke);
    const qreal dpr = qApp->devicePixelRatio();
    QPixmap pm((box.size() * dpr).toSize() + QSize(2, 2));
    pm.setDevicePixelRatio(dpr);
    pm.fill(Qt::transparent);
    QPainter g(&pm);
    g.setRenderHint(QPainter::Antialiasing);
    g.translate(-box.topLeft() + QPointF(0.5, 0.5));
    g.setPen(QPen(color, stroke, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    g.setBrush(color);
    g.drawPath(path);
    g.end();
    return pm;
}

const char *DROP_SVG =
    "<svg xmlns='http://www.w3.org/2000/svg' width='64' height='64' viewBox='0 0 64 64'>"
    "<defs><linearGradient id='g' x1='0' y1='0' x2='1' y2='1'>"
    "<stop offset='0' stop-color='#2f7568'/><stop offset='1' stop-color='#6fbfad'/></linearGradient></defs>"
    "<path d='M22 5h20c4 8 11 16 11 25 0 12-13 22-21 30-8-8-21-18-21-30 0-9 7-17 11-25z' fill='url(#g)'/>"
    "<path d='M32 36v22' stroke='#24231f' stroke-width='3.4' stroke-linecap='round'/>"
    "<circle cx='32' cy='28' r='4.6' fill='#24231f'/></svg>";

const char *GALLERY_STYLE =
    "QWidget#nanquimGallery, QWidget#nanquimHomePage, QWidget#nanquimChapterPage { background: #24231f; }"
    "QWidget#nanquimGallery QLabel { color: #eeeae0; }"
    "QWidget#nanquimGalleryHeader { background: #24231f; border-bottom: 1px solid #34332e; }"
    "QLabel#nanquimBrand { font-size: 22px; font-weight: 800; }"
    "QLabel#nanquimSection { color: #a8a398; font-size: 13px; font-weight: 700; letter-spacing: 2px; }"
    "QLabel#nanquimFooter { color: #7a766d; font-size: 12px; }"
    "QLabel#nanquimHint { color: #a8a398; font-size: 13px; }"
    "QLabel#nanquimEmpty { color: #a8a398; font-size: 14px; border: 2px dashed #4a4842; border-radius: 16px; }"
    "QLabel#nanquimKicker { color: #6fbfad; font-size: 12px; font-weight: 700; letter-spacing: 2px; }"
    "QLabel#nanquimChapterTitle { font-size: 26px; font-weight: 800; }"
    "QLabel#nanquimChapterInfo { color: #a8a398; font-size: 13px; }"
    "QLineEdit#nanquimSearch { min-height: 44px; padding: 0 12px; border-radius: 12px; background: #2e2d29;"
    "  border: 1px solid #7a766d; color: #eeeae0; font-size: 14px; }"
    "QLineEdit#nanquimSearch:focus { border: 2px solid #3e8e7e; }"
    "QPushButton.nanquimGhost, QToolButton.nanquimGhost { min-height: 44px; padding: 0 16px; border-radius: 12px;"
    "  border: 1px solid #4a4842; background: #34332e; color: #eeeae0; font-size: 14px; font-weight: 600; }"
    "QToolButton.nanquimGhost::menu-indicator { image: none; width: 0px; }"
    "QPushButton.nanquimGhost:pressed, QToolButton.nanquimGhost:pressed { background: #4a4842; }"
    "QPushButton.nanquimGhost:checked { background: rgba(62, 142, 126, 40); border-color: #3e8e7e; color: #cde9e1; }"
    "QPushButton#nanquimIconButton, QToolButton#nanquimIconButton { min-width: 44px; max-width: 44px; padding: 0; }"
    "QPushButton#nanquimFlatIcon { border: none; background: transparent; min-width: 44px; max-width: 44px; padding: 0; }"
    "QPushButton#nanquimBackToDrawing { min-height: 44px; padding: 0 16px; border-radius: 12px;"
    "  border: 1px solid rgba(62, 142, 126, 115); background: rgba(62, 142, 126, 30); color: #6fbfad;"
    "  font-size: 14px; font-weight: 600; }"
    "QPushButton#nanquimBackToDrawing:pressed { background: rgba(62, 142, 126, 60); }"
    "QPushButton#nanquimPrimary { min-height: 44px; padding: 0 20px; border-radius: 12px; border: none; color: #ffffff;"
    "  font-size: 14px; font-weight: 700; background: #2f7568; }"
    "QPushButton.nanquimChip { min-height: 40px; padding: 0 16px; border-radius: 20px; border: 1px solid #4a4842;"
    "  background: #34332e; color: #cfcabf; font-size: 13px; font-weight: 600; }"
    "QPushButton.nanquimChip:checked { background: rgba(62, 142, 126, 40); color: #cde9e1; border-color: rgba(62, 142, 126, 128); }"
    "QListView#nanquimRecents { background: transparent; border: none; }"
    "QLabel#nanquimH2 { font-size: 18px; font-weight: 700; }"
    "QFrame#nanquimContinue { background: #2e2d29; border: 1px solid #524f48; border-radius: 16px; }"
    "QLabel#nanquimContinueTitle { font-size: 28px; font-weight: 700; }"
    "QLabel#nanquimContinueSub { color: #a8a398; font-size: 15px; }"
    "QLabel#nanquimContinueInfo { color: #a8a398; font-size: 14px; }"
    "QPushButton#nanquimContinueGo { min-height: 56px; border-radius: 12px; border: none; color: #ffffff;"
    "  font-size: 17px; font-weight: 700; background: #2f7568; }"
    "QPushButton#nanquimContinueGo:pressed { background: #2f7568; }"
    "QPushButton#nanquimContinueChapter { min-height: 48px; border-radius: 12px; border: 1px solid #5a574f;"
    "  background: transparent; color: #eeeae0; font-size: 15px; font-weight: 600; }"
    "QPushButton#nanquimContinueChapter:pressed { background: #4a4842; }"
    "QPushButton#nanquimLink { min-height: 44px; padding: 0 8px; border: none; background: transparent; color: #6fbfad;"
    "  font-size: 15px; font-weight: 600; }"
    "QPushButton#nanquimLink:pressed { color: #8fd3c3; }"
    "QWidget#nanquimNewStrip { background: #2e2d29; border-top: 1px solid #524f48; }"
    "QScrollArea#nanquimHomeScroll { background: transparent; border: none; }"
    "QPushButton#nanquimHomeContinue { min-height: 44px; padding: 0 16px; border-radius: 12px; text-align: left;"
    "  border: 1px solid rgba(111, 191, 173, 110); color: #eeeae0; font-size: 14px; font-weight: 600;"
    "  background: #34332e; }"
    "QPushButton#nanquimHomeContinue:pressed { background: #2a3d38; }"
    "QWidget#nanquimHomeBody { background: #24231f; }";

// miniatura de papel com sombra e o nome embaixo
QString chapterOf(const QString &path);
QString chapterNameOf(const QString &path);

// as miniaturas dos recentes são quadradas, com faixas escuras dos lados;
// aqui as faixas são cortadas para a página preencher o cartão inteiro
QPixmap trimmedThumb(const QIcon &icon)
{
    if (icon.isNull()) return QPixmap();
    static QHash<qint64, QPixmap> cache;
    const qint64 key = icon.cacheKey();
    auto it = cache.constFind(key);
    if (it != cache.constEnd()) return it.value();
    const QPixmap pm = icon.pixmap(QSize(512, 512));
    if (pm.isNull()) return QPixmap();
    const QImage img = pm.toImage().convertToFormat(QImage::Format_ARGB32);
    const int w = img.width();
    const int h = img.height();
    const QColor text = qApp->palette().color(QPalette::Text);
    const QColor window = qApp->palette().color(QPalette::Window);
    const QColor blended = QColor::fromRgbF(text.redF() * 0.1 + window.redF() * 0.9,
                                            text.greenF() * 0.1 + window.greenF() * 0.9,
                                            text.blueF() * 0.1 + window.blueF() * 0.9);
    auto same = [](QRgb a, QRgb b) {
        return qAbs(qRed(a) - qRed(b)) < 5 && qAbs(qGreen(a) - qGreen(b)) < 5
                && qAbs(qBlue(a) - qBlue(b)) < 5 && qAbs(qAlpha(a) - qAlpha(b)) < 5;
    };
    auto isBar = [&](QRgb c) {
        return same(c, blended.rgb()) || (qAlpha(c) == 255 && QColor(c).lightness() < 80);
    };
    QRect keep(0, 0, w, h);
    const QRgb side = img.pixel(0, h / 2);
    if (isBar(side) && same(side, img.pixel(w - 1, h / 2))) {
        auto uniformCol = [&](int x) { for (int y = 0; y < h; y += 3) if (!same(img.pixel(x, y), side)) return false; return true; };
        int l = 0, r = w - 1;
        while (l < w / 2 && uniformCol(l)) l++;
        while (r > w / 2 && uniformCol(r)) r--;
        keep.setLeft(l);
        keep.setRight(r);
    }
    const QRgb top = img.pixel(w / 2, 0);
    if (isBar(top) && same(top, img.pixel(w / 2, h - 1))) {
        auto uniformRow = [&](int y) { for (int x = 0; x < w; x += 3) if (!same(img.pixel(x, y), top)) return false; return true; };
        int t = 0, b = h - 1;
        while (t < h / 2 && uniformRow(t)) t++;
        while (b > h / 2 && uniformRow(b)) b--;
        keep.setTop(t);
        keep.setBottom(b);
    }
    QPixmap out = (keep.width() > w / 8 && keep.height() > h / 8) ? QPixmap::fromImage(img.copy(keep)) : pm;
    if (cache.size() > 300) cache.clear();
    cache.insert(key, out);
    return out;
}

// onde a página cabe dentro de 'box', centralizada e encostada embaixo
QRect fitPage(const QSize &content, const QRect &box)
{
    QSize sz = content.isEmpty() ? QSize(707, 1000) : content;
    sz.scale(box.size(), Qt::KeepAspectRatio);
    sz = sz.expandedTo(QSize(8, 8));
    return QRect(box.left() + (box.width() - sz.width()) / 2, box.bottom() - sz.height() + 1, sz.width(), sz.height());
}

// papel com sombra e a miniatura por cima
QRect paintPage(QPainter *g, const QRect &box, const QIcon &icon, int radius)
{
    const QPixmap pm = trimmedThumb(icon);
    const QRect paper = fitPage(pm.isNull() ? QSize() : pm.size(), box);
    g->setPen(Qt::NoPen);
    for (int i = 5; i >= 1; --i) {
        g->setBrush(QColor(0, 0, 0, 12 + 6 * (5 - i)));
        g->drawRoundedRect(paper.adjusted(-i / 2, i, i / 2, i + 1), radius + i, radius + i);
    }
    g->setBrush(QColor(0xfa, 0xfa, 0xf7));
    g->drawRoundedRect(paper, radius, radius);
    if (!pm.isNull()) {
        QPainterPath clip;
        clip.addRoundedRect(paper, radius, radius);
        g->save();
        g->setClipPath(clip);
        g->drawPixmap(paper, pm);
        g->restore();
    }
    return paper;
}

QString relativeWhen(const QDateTime &t)
{
    if (!t.isValid()) return QString();
    const qint64 s = t.secsTo(QDateTime::currentDateTime());
    if (s < 60) return i18n("agora");
    if (s < 3600) return i18n("há %1 min", s / 60);
    if (s < 86400) return i18n("há %1 h", s / 3600);
    const qint64 days = s / 86400;
    if (days == 1) return i18n("ontem");
    if (days < 7) return i18n("há %1 dias", days);
    if (days < 14) return i18n("há 1 semana");
    if (days < 31) return i18n("há %1 semanas", days / 7);
    return QLocale().toString(t.date(), QLocale::ShortFormat);
}

// "há 2 h · Capítulo 2", guardado para não ler o disco a cada desenho da tela
QHash<QString, QString> s_subtitleCache;
QString recentSubtitle(const QString &path)
{
    auto it = s_subtitleCache.constFind(path);
    if (it != s_subtitleCache.constEnd()) return it.value();
    QString s = relativeWhen(QFileInfo(path).lastModified());
    const QString chapter = chapterNameOf(path);
    if (!chapter.isEmpty()) s = s.isEmpty() ? chapter : s + QStringLiteral(" · ") + chapter;
    s_subtitleCache.insert(path, s);
    return s;
}

QPixmap cachedPaper(const QIcon &icon, const QSize &area, int radius);

class RecentDelegate : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    static QSize cell() { return QSize(176, 292); }
    static QRect paperBox(const QRect &cellRect)
    {
        const QRect r = cellRect.adjusted(10, 6, -10, -6);
        return QRect(r.left(), r.top(), r.width(), r.height() - 52);
    }

    QSize sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const override { return cell(); }

    void paint(QPainter *g, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        g->save();
        g->setRenderHint(QPainter::Antialiasing);
        g->setRenderHint(QPainter::SmoothPixmapTransform);
        const QRect r = option.rect.adjusted(10, 6, -10, -6);
        const QRect box = paperBox(option.rect);
        // papel desenhado uma vez e guardado (rolar não reescala as miniaturas)
        const QIcon thumb = index.data(Qt::DecorationRole).value<QIcon>();
        const QPixmap trimmed = trimmedThumb(thumb);
        const QRect paper = fitPage(trimmed.isNull() ? QSize() : trimmed.size(), box);
        g->drawPixmap(box.topLeft() - QPoint(4, 0), cachedPaper(thumb, box.size() + QSize(8, 6), 8));

        if (option.state & QStyle::State_Selected) {
            g->setPen(QPen(C_ACCENT, 2));
            g->setBrush(Qt::NoBrush);
            g->drawRoundedRect(paper, 8, 8);
        }
        const bool chapterView = option.widget && option.widget->property("nanquimChapterView").toBool();
        if (option.widget && option.widget->property("nanquimAddMode").toBool()) {
            const QRectF badge(paper.right() - 34, paper.top() + 8, 26, 26);
            g->setPen(Qt::NoPen);
            g->setBrush(C_ACCENT);
            g->drawEllipse(badge);
            g->setPen(QPen(Qt::white, 2.4, Qt::SolidLine, Qt::RoundCap));
            g->drawLine(badge.center() + QPointF(-6, 0), badge.center() + QPointF(6, 0));
            g->drawLine(badge.center() + QPointF(0, -6), badge.center() + QPointF(0, 6));
        }
        if (option.widget && option.widget->property("nanquimOrderMode").toBool()) {
            // organizar: setas nas laterais e o número da página no meio
            const qreal cy = paper.center().y();
            const QRectF left(paper.left() + 8, cy - 18, 36, 36);
            const QRectF right(paper.right() - 44, cy - 18, 36, 36);
            g->setPen(Qt::NoPen);
            g->setBrush(QColor(36, 35, 31, 220));
            g->drawEllipse(left);
            g->drawEllipse(right);
            g->setPen(QPen(C_ACCENT, 2.6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            g->drawPolyline(QPolygonF() << left.center() + QPointF(4, -7) << left.center() + QPointF(-4, 0) << left.center() + QPointF(4, 7));
            g->drawPolyline(QPolygonF() << right.center() + QPointF(-4, -7) << right.center() + QPointF(4, 0) << right.center() + QPointF(-4, 7));
        }
        if (chapterView) {
            // número da página no canto, como no protótipo
            QFont nf = option.font;
            nf.setPixelSize(11);
            nf.setWeight(QFont::Bold);
            const QString num = QString::number(index.row() + 1).rightJustified(2, QLatin1Char('0'));
            const QRectF pill(paper.left() + 6, paper.top() + 6, QFontMetrics(nf).horizontalAdvance(num) + 14, 22);
            g->setPen(Qt::NoPen);
            g->setBrush(QColor(36, 35, 31, 230));
            g->drawRoundedRect(pill, 11, 11);
            g->setFont(nf);
            g->setPen(C_TEXT);
            g->drawText(pill, Qt::AlignCenter, num);
        }
        const QVariant stage = index.data(Qt::UserRole + 60);
        if (stage.isValid() && chapterView) {
            const int s = stage.toInt();
            const QColor dot = s == 2 ? QColor(0x5e, 0xc9, 0x8c) : s == 1 ? QColor(0x6f, 0xbf, 0xad) : QColor(0xe6, 0xb4, 0x5a);
            const QString label = s == 2 ? i18n("Pronta") : s == 1 ? i18n("Arte-final") : i18n("Rascunho");
            QFont sf = option.font;
            sf.setPixelSize(11);
            sf.setWeight(QFont::DemiBold);
            const int pw = QFontMetrics(sf).horizontalAdvance(label) + 28;
            const QRectF pill(paper.left() + 8, paper.bottom() - 30, pw, 22);
            g->setPen(QPen(QColor(dot.red(), dot.green(), dot.blue(), 110), 1));
            g->setBrush(QColor(46, 45, 41, 232));
            g->drawRoundedRect(pill, 11, 11);
            g->setPen(Qt::NoPen);
            g->setBrush(dot);
            g->drawEllipse(QPointF(pill.left() + 11, pill.center().y()), 4, 4);
            g->setFont(sf);
            g->setPen(QColor(0xee, 0xea, 0xe0));
            g->drawText(pill.adjusted(20, 0, -6, 0), Qt::AlignVCenter | Qt::AlignLeft, label);
        }
        if (option.widget && option.widget->property("nanquimRemoveMode").toBool()) {
            const QRectF badge(paper.right() - 34, paper.top() + 8, 26, 26);
            g->setPen(Qt::NoPen);
            g->setBrush(QColor(0xef, 0x62, 0x62));
            g->drawEllipse(badge);
            g->setPen(QPen(Qt::white, 2.4, Qt::SolidLine, Qt::RoundCap));
            g->drawLine(badge.center() + QPointF(-5, -5), badge.center() + QPointF(5, 5));
            g->drawLine(badge.center() + QPointF(5, -5), badge.center() + QPointF(-5, 5));
        }

        QString name = index.data(Qt::DisplayRole).toString();
        if (name.endsWith(QLatin1String(".kra"), Qt::CaseInsensitive)) name.chop(4);
        QFont f = option.font;
        f.setPixelSize(13);
        f.setWeight(QFont::DemiBold);
        g->setFont(f);
        g->setPen(C_TEXT);
        const QRect nameRect(r.left(), box.bottom() + 10, r.width(), 20);
        g->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter,
                    QFontMetrics(f).elidedText(name, Qt::ElideRight, nameRect.width()));
        if (!chapterView) {
            const QString sub = recentSubtitle(index.data(Qt::ToolTipRole).toString());
            if (!sub.isEmpty()) {
                f.setPixelSize(11);
                f.setWeight(QFont::Normal);
                g->setFont(f);
                g->setPen(QColor(0xa8, 0xa3, 0x98));
                const QRect subRect(r.left(), nameRect.bottom() + 2, r.width(), 16);
                g->drawText(subRect, Qt::AlignLeft | Qt::AlignVCenter,
                            QFontMetrics(f).elidedText(sub, Qt::ElideRight, subRect.width()));
            }
        }
        g->restore();
    }
};




// formato da faixa "Nova página" no rodapé

// ---------------------------------------------------------------------------
// Cartões da tela inicial: "Nova página" em gradiente e formatos com o papel
// desenhado na proporção (também usados na janela de Nova página)
// ---------------------------------------------------------------------------
class PaperTile : public QAbstractButton
{
public:
    PaperTile(const QString &name, const QString &detail, qreal ratio, bool centered, QWidget *parent)
        : QAbstractButton(parent), m_name(name), m_detail(detail), m_ratio(ratio), m_centered(centered)
    {
        setFocusPolicy(Qt::NoFocus);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        setFixedHeight(150);
        setMinimumWidth(130);
    }
    QSize sizeHint() const override { return QSize(m_centered ? 170 : 180, 150); }
protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter g(this);
        g.setRenderHint(QPainter::Antialiasing);
        const QRectF r = QRectF(rect()).adjusted(1, 1, -1, -1);
        const bool on = isChecked();
        const bool down = isDown();
        g.setPen(QPen(on || down ? C_ACCENT : QColor(0x4a, 0x48, 0x42), on ? 2 : 1));
        g.setBrush(on ? QColor(0x2a, 0x3d, 0x38) : down ? QColor(0x34, 0x33, 0x2e) : QColor(0x34, 0x33, 0x2e));
        g.drawRoundedRect(r, 15, 15);

        QFont nf = font();
        nf.setPixelSize(m_centered ? 14 : 15);
        nf.setWeight(QFont::Bold);
        QFont df = font();
        df.setPixelSize(12);
        const qreal maxW = m_centered ? 96 : 64;
        QRectF box;
        if (m_centered) box = QRectF(r.left() + 12, r.top() + 14, r.width() - 24, r.height() - 14 - 62);
        else box = QRectF(r.left() + 16, r.top() + 16, r.width() - 32, 56);
        qreal pw, ph;
        if (m_ratio >= 1.0) { pw = qMin(maxW, box.width()); ph = pw / m_ratio; }
        else { ph = box.height(); pw = qMax(10.0, ph * m_ratio); }
        const QRectF paper(m_centered ? box.center().x() - pw / 2 : box.left(),
                           m_centered ? box.center().y() - ph / 2 : box.bottom() - ph, pw, ph);
        g.setPen(Qt::NoPen);
        g.setBrush(QColor(0, 0, 0, 90));
        g.drawRoundedRect(paper.translated(0, 3), 3, 3);
        g.setBrush(QColor(0xfa, 0xfa, 0xf7));
        g.drawRoundedRect(paper, 3, 3);

        const Qt::Alignment al = m_centered ? Qt::AlignHCenter : Qt::AlignLeft;
        const QRectF nameRect(r.left() + 14, r.bottom() - 52, r.width() - 28, 22);
        const QRectF detRect(r.left() + 14, r.bottom() - 30, r.width() - 28, 18);
        g.setFont(nf);
        g.setPen(C_TEXT);
        g.drawText(nameRect, al | Qt::AlignVCenter, QFontMetrics(nf).elidedText(m_name, Qt::ElideRight, int(nameRect.width())));
        g.setFont(df);
        g.setPen(QColor(0xa8, 0xa3, 0x98));
        g.drawText(detRect, al | Qt::AlignVCenter, QFontMetrics(df).elidedText(m_detail, Qt::ElideRight, int(detRect.width())));
    }
private:
    QString m_name, m_detail;
    qreal m_ratio;
    bool m_centered;
};

class NewPageTile : public QAbstractButton
{
public:
    explicit NewPageTile(QWidget *parent) : QAbstractButton(parent)
    {
        setFocusPolicy(Qt::NoFocus);
        setFixedSize(236, 150);
        setToolTip(i18n("Nova página"));
    }
protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter g(this);
        g.setRenderHint(QPainter::Antialiasing);
        const QRectF r = QRectF(rect()).adjusted(1, 1, -1, -1);
        QLinearGradient grad(r.topLeft(), r.bottomRight());
        grad.setColorAt(0, isDown() ? QColor(0x37, 0x7f, 0x71) : C_ACCENT);
        grad.setColorAt(1, isDown() ? QColor(0x5a, 0xa8, 0x98) : QColor(0x6f, 0xbf, 0xad));
        g.setPen(Qt::NoPen);
        g.setBrush(grad);
        g.drawRoundedRect(r, 16, 16);
        const QPointF c(r.left() + 18 + 26, r.top() + 18 + 26);
        g.setBrush(QColor(255, 255, 255, 235));
        g.drawEllipse(c, 26, 26);
        g.setPen(QPen(QColor(0x2f, 0x75, 0x68), 3, Qt::SolidLine, Qt::RoundCap));
        g.drawLine(c + QPointF(-10, 0), c + QPointF(10, 0));
        g.drawLine(c + QPointF(0, -10), c + QPointF(0, 10));
        QFont f = font();
        f.setPixelSize(19);
        f.setWeight(QFont::ExtraBold);
        g.setFont(f);
        g.setPen(QColor(0x13, 0x23, 0x20));
        g.drawText(QRectF(r.left() + 18, r.bottom() - 58, r.width() - 36, 26), Qt::AlignLeft | Qt::AlignVCenter, i18n("Nova página"));
        f.setPixelSize(12);
        f.setWeight(QFont::DemiBold);
        g.setFont(f);
        g.setPen(QColor(0x13, 0x23, 0x20, 200));
        g.drawText(QRectF(r.left() + 18, r.bottom() - 32, r.width() - 36, 18), Qt::AlignLeft | Qt::AlignVCenter,
                   i18n("Escolher formato e tamanho"));
    }
};

// barra de progresso do capítulo, com a legenda ao lado
class ChapterProgress : public QWidget
{
public:
    explicit ChapterProgress(QWidget *parent) : QWidget(parent) { setFixedHeight(26); }
    void setCounts(int ready, int ink, int draft) { m_ready = ready; m_ink = ink; m_draft = draft; update(); }
protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter g(this);
        g.setRenderHint(QPainter::Antialiasing);
        QFont f = font();
        f.setPixelSize(12);
        g.setFont(f);
        const QFontMetrics fm(f);
        struct Item { QColor c; QString t; };
        const QList<Item> items = {{QColor(0x5e, 0xc9, 0x8c), i18n("%1 prontas", m_ready)},
                                   {QColor(0x6f, 0xbf, 0xad), i18n("%1 arte-final", m_ink)},
                                   {QColor(0xe6, 0xb4, 0x5a), i18n("%1 rascunho", m_draft)}};
        int legendW = 0;
        for (const Item &it : items) legendW += 18 + fm.horizontalAdvance(it.t) + 16;
        const QRectF bar(0, height() / 2 - 4, qMax(80, width() - legendW - 8), 8);
        g.setPen(Qt::NoPen);
        g.setBrush(QColor(0x34, 0x33, 0x2e));
        g.drawRoundedRect(bar, 4, 4);
        const int total = m_ready + m_ink + m_draft;
        if (total > 0) {
            g.save();
            QPainterPath clip;
            clip.addRoundedRect(bar, 4, 4);
            g.setClipPath(clip);
            const qreal rw = bar.width() * m_ready / total;
            const qreal iw = bar.width() * m_ink / total;
            QLinearGradient grad(bar.topLeft(), bar.topRight());
            grad.setColorAt(0, QColor(0x3e, 0x8e, 0x7e));
            grad.setColorAt(1, QColor(0x5e, 0xc9, 0x8c));
            g.setBrush(grad);
            g.drawRect(QRectF(bar.left(), bar.top(), rw, bar.height()));
            g.setBrush(QColor(0x6f, 0xbf, 0xad));
            g.drawRect(QRectF(bar.left() + rw, bar.top(), iw, bar.height()));
            g.restore();
        }
        qreal x = bar.right() + 16;
        for (const Item &it : items) {
            g.setPen(Qt::NoPen);
            g.setBrush(it.c);
            g.drawEllipse(QPointF(x + 5, height() / 2.0), 5, 5);
            g.setPen(QColor(0xcf, 0xca, 0xbf));
            g.drawText(QRectF(x + 16, 0, fm.horizontalAdvance(it.t) + 4, height()), Qt::AlignVCenter | Qt::AlignLeft, it.t);
            x += 18 + fm.horizontalAdvance(it.t) + 16;
        }
    }
private:
    int m_ready = 0, m_ink = 0, m_draft = 0;
};

// ---------------------------------------------------------------------------
// Transição da tela inicial para o capítulo: o resto da tela escurece e as
// páginas saem da capa até os lugares da grade (e o caminho inverso ao voltar)
// ---------------------------------------------------------------------------
class ChapterTransition : public QWidget
{
public:
    ChapterTransition(QWidget *host, const QPixmap &home, const QPixmap &chapter, const QRect &cover,
                      const QRect &grid, const QList<QRect> &cells)
        : QWidget(host), m_home(home), m_chapter(chapter), m_cover(cover), m_grid(grid), m_cells(cells)
    {
        setAttribute(Qt::WA_OpaquePaintEvent);
        setGeometry(host->rect());
    }
    void setProgress(qreal t) { m_t = t; update(); }
protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter g(this);
        g.setRenderHint(QPainter::SmoothPixmapTransform);
        g.fillRect(rect(), QColor(0x24, 0x23, 0x1f));
        g.drawPixmap(0, 0, m_home);
        const qreal veil = qBound(0.0, m_t * 1.5, 1.0);
        g.setOpacity(veil);
        g.setClipRegion(QRegion(rect()) - QRegion(m_grid));
        g.drawPixmap(0, 0, m_chapter);
        g.setClipping(false);
        g.fillRect(m_grid, QColor(0x24, 0x23, 0x1f));
        const qreal dpr = m_chapter.devicePixelRatio();
        const QEasingCurve ease(QEasingCurve::OutCubic);
        for (int i = m_cells.size() - 1; i >= 0; --i) {
            const qreal delay = qMin(i, 10) * 0.03;
            const qreal local = qBound(0.0, (m_t - delay) / 0.7, 1.0);
            if (local <= 0.0) continue;
            const qreal e = ease.valueForProgress(local);
            const QRectF from(m_cover);
            const QRectF to(m_cells[i]);
            const QRectF cur(from.left() + (to.left() - from.left()) * e, from.top() + (to.top() - from.top()) * e,
                             from.width() + (to.width() - from.width()) * e, from.height() + (to.height() - from.height()) * e);
            g.setOpacity(qMin(1.0, local * 3));
            g.drawPixmap(cur, m_chapter, QRectF(to.left() * dpr, to.top() * dpr, to.width() * dpr, to.height() * dpr));
        }
    }
private:
    QPixmap m_home, m_chapter;
    QRect m_cover, m_grid;
    QList<QRect> m_cells;
    qreal m_t = 0;
};


struct PagePreset {
    const char *name;
    const char *detail;
    int width;
    int height;
    int dpi;
};


// ---------------------------------------------------------------------------
// Capítulos: grupos de páginas, guardados nas configurações do app.
// Só guardam o caminho de cada desenho; os arquivos continuam onde estão.
// ---------------------------------------------------------------------------
class ChapterStore : public QObject
{
public:
    struct Chapter { QString id; QString name; QStringList pages; };

    static ChapterStore *instance()
    {
        static ChapterStore *s = new ChapterStore();
        return s;
    }
    std::function<void()> onChanged;   // a galeria redesenha
    QList<std::function<void()>> listeners;

    QList<Chapter> chapters() const
    {
        QList<Chapter> out;
        KConfigGroup cfg = KSharedConfig::openConfig()->group("NanquimChapters");
        Q_FOREACH (const QString &id, cfg.readEntry("list", QStringList())) {
            KConfigGroup c = KSharedConfig::openConfig()->group(QStringLiteral("NanquimChapter-") + id);
            out << Chapter{id, c.readEntry("name", QString()), c.readEntry("pages", QStringList())};
        }
        return out;
    }
    Chapter chapter(const QString &id) const
    {
        Q_FOREACH (const Chapter &c, chapters()) if (c.id == id) return c;
        return Chapter();
    }
    QString create(const QString &name)
    {
        const QString id = QUuid::createUuid().toString(QUuid::Id128).left(12);
        KConfigGroup cfg = KSharedConfig::openConfig()->group("NanquimChapters");
        QStringList list = cfg.readEntry("list", QStringList());
        list.prepend(id);
        cfg.writeEntry("list", list);
        KSharedConfig::openConfig()->group(QStringLiteral("NanquimChapter-") + id).writeEntry("name", name);
        changed();
        return id;
    }
    void rename(const QString &id, const QString &name)
    {
        KSharedConfig::openConfig()->group(QStringLiteral("NanquimChapter-") + id).writeEntry("name", name);
        changed();
    }
    void remove(const QString &id)
    {
        KConfigGroup cfg = KSharedConfig::openConfig()->group("NanquimChapters");
        QStringList list = cfg.readEntry("list", QStringList());
        list.removeAll(id);
        cfg.writeEntry("list", list);
        KSharedConfig::openConfig()->deleteGroup(QStringLiteral("NanquimChapter-") + id);
        changed();
    }
    bool addPage(const QString &id, const QString &path)
    {
        if (id.isEmpty() || path.isEmpty()) return false;
        KConfigGroup c = KSharedConfig::openConfig()->group(QStringLiteral("NanquimChapter-") + id);
        QStringList pages = c.readEntry("pages", QStringList());
        if (pages.contains(path)) return false;
        pages << path;
        c.writeEntry("pages", pages);
        changed();
        return true;
    }
    void removePage(const QString &id, const QString &path)
    {
        KConfigGroup c = KSharedConfig::openConfig()->group(QStringLiteral("NanquimChapter-") + id);
        QStringList pages = c.readEntry("pages", QStringList());
        pages.removeAll(path);
        c.writeEntry("pages", pages);
        changed();
    }
    // etapa da página: 0 rascunho, 1 arte-final, 2 pronta
    int status(const QString &id, const QString &path) const
    {
        KConfigGroup c = KSharedConfig::openConfig()->group(QStringLiteral("NanquimChapter-") + id);
        Q_FOREACH (const QString &entry, c.readEntry("status", QStringList())) {
            const int sep = entry.indexOf(QLatin1Char('|'));
            if (sep > 0 && entry.mid(sep + 1) == path) return qBound(0, entry.left(sep).toInt(), 2);
        }
        return 0;
    }
    void setStatus(const QString &id, const QString &path, int value)
    {
        KConfigGroup c = KSharedConfig::openConfig()->group(QStringLiteral("NanquimChapter-") + id);
        QStringList list = c.readEntry("status", QStringList());
        for (int i = list.size() - 1; i >= 0; --i) {
            const int sep = list[i].indexOf(QLatin1Char('|'));
            if (sep > 0 && list[i].mid(sep + 1) == path) list.removeAt(i);
        }
        if (value > 0) list << QStringLiteral("%1|%2").arg(value).arg(path);
        c.writeEntry("status", list);
        changed();
    }
    // formato das páginas novas (índice em CHAPTER_FORMATS)
    int format(const QString &id) const
    {
        return qBound(0, KSharedConfig::openConfig()->group(QStringLiteral("NanquimChapter-") + id).readEntry("format", 0), 3);
    }
    void setFormat(const QString &id, int value)
    {
        KSharedConfig::openConfig()->group(QStringLiteral("NanquimChapter-") + id).writeEntry("format", value);
        changed();
    }
    // leitura das páginas em dupla: mangá (da direita para a esquerda) ou ocidental.
    // Sem escolha, capítulo de mangá B4 lê como mangá.
    bool rtl(const QString &id) const
    {
        return KSharedConfig::openConfig()->group(QStringLiteral("NanquimChapter-") + id).readEntry("rtl", format(id) == 0);
    }
    void setRtl(const QString &id, bool on)
    {
        KSharedConfig::openConfig()->group(QStringLiteral("NanquimChapter-") + id).writeEntry("rtl", on);
        changed();
    }
    // de que capítulo é a página, a etapa e a posição: lido das configurações uma
    // vez e guardado; a grade da tela inicial pergunta isso a cada página desenhada
    struct PageInfo { QString chapterId; QString chapterName; int stage = 0; int index = -1; };
    PageInfo info(const QString &path) const
    {
        if (m_dirty) rebuildIndex();
        return m_index.value(path);
    }
    void movePage(const QString &id, const QString &path, int delta)
    {
        KConfigGroup c = KSharedConfig::openConfig()->group(QStringLiteral("NanquimChapter-") + id);
        QStringList pages = c.readEntry("pages", QStringList());
        const int i = pages.indexOf(path);
        const int j = i + delta;
        if (i < 0 || j < 0 || j >= pages.size()) return;
        pages.move(i, j);
        c.writeEntry("pages", pages);
        changed();
    }

private:
    void rebuildIndex() const
    {
        m_index.clear();
        Q_FOREACH (const Chapter &c, chapters()) {
            QHash<QString, int> stages;
            KConfigGroup g = KSharedConfig::openConfig()->group(QStringLiteral("NanquimChapter-") + c.id);
            Q_FOREACH (const QString &entry, g.readEntry("status", QStringList())) {
                const int sep = entry.indexOf(QLatin1Char('|'));
                if (sep > 0) stages.insert(entry.mid(sep + 1), qBound(0, entry.left(sep).toInt(), 2));
            }
            for (int i = 0; i < c.pages.size(); i++) {
                if (m_index.contains(c.pages[i])) continue;   // página em dois capítulos: vale o primeiro
                PageInfo pi;
                pi.chapterId = c.id;
                pi.chapterName = c.name;
                pi.stage = stages.value(c.pages[i], 0);
                pi.index = i;
                m_index.insert(c.pages[i], pi);
            }
        }
        m_dirty = false;
    }
    mutable QHash<QString, PageInfo> m_index;
    mutable bool m_dirty = true;

    void changed()
    {
        m_dirty = true;
        KSharedConfig::openConfig()->sync();
        for (const auto &f : listeners) f();
    }
};

QString chapterNameOf(const QString &path)
{
    if (path.isEmpty()) return QString();
    return ChapterStore::instance()->info(path).chapterName;
}

// ícone (miniatura) de um desenho, pelo mesmo cache dos recentes
QUrl urlForPath(const QString &path)
{
    Q_FOREACH (const QUrl &url, KisRecentFilesManager::instance()->recentUrlsLatestFirst()) {
#ifdef Q_OS_ANDROID
        if (url.toLocalFile() == path) return url;
#else
        if (url.toDisplayString(QUrl::PreferLocalFile) == path) return url;
#endif
    }
    return QUrl::fromLocalFile(path);
}

QString displayNameFor(const QString &path)
{
    QString name = QUrl(path).fileName();
    if (name.isEmpty()) name = QFileInfo(path).fileName();
    const QUrl url = urlForPath(path);
    if (!url.fileName().isEmpty()) name = url.fileName();
    return name;
}

// página nova criada dentro de um capítulo: entra nele quando for salva pela primeira vez
void attachDocumentToChapter(KisDocument *doc, const QString &chapterId)
{
    if (!doc || chapterId.isEmpty()) return;
    QPointer<KisDocument> guard(doc);
    auto connection = std::make_shared<QMetaObject::Connection>();
    *connection = QObject::connect(doc, &KisDocument::sigSavingFinished, doc, [guard, chapterId, connection](const QString &filePath) {
        if (filePath.contains(QLatin1String("krita-backup")) || filePath.contains(QLatin1String("autosave"))) return;
        ChapterStore::instance()->addPage(chapterId, filePath);
        QObject::disconnect(*connection);
    });
}

QString s_pendingChapter;   // capítulo aberto na galeria quando a página nova foi criada

// ---------------------------------------------------------------------------
// Abrir página com aviso: páginas grandes levavam ~10 s sem nenhum sinal na
// tela. Um véu escuro com o nome e o tamanho aparece antes de o Krita começar
// a ler o arquivo (o carregamento trava a tela, então o véu é pintado e só
// depois a leitura começa) e some quando a página aparece.
// ---------------------------------------------------------------------------
// De onde a página sai ao abrir (a miniatura tocada, em coordenadas da janela)
// e a foto dela. Quem abre preenche logo antes de chamar openPageWithFeedback.
QRect s_openFrom;
// QPixmap não pode existir antes do QGuiApplication (o Qt aborta ao carregar a
// biblioteca), então ele nasce no primeiro uso e nunca é destruído
QPixmap &openShot() { static QPixmap *p = new QPixmap; return *p; }

// Onde fica a área de desenho na janela (a última vez que um desenho apareceu).
// A página da transição cresce até caber nela, como o "Ajustar visualização".
QRect &canvasAreaSlot() { static QRect r; return r; }

QRect storedCanvasArea(QWidget *window)
{
    QRect r = canvasAreaSlot();
    if (r.isEmpty()) {
        r = KSharedConfig::openConfig()->group("Nanquim").readEntry("canvasArea", QRect());
        canvasAreaSlot() = r;
    }
    // girar o tablet muda a janela: área guardada que não cabe mais não vale
    if (!window || r.isEmpty() || !window->rect().contains(r)) return QRect();
    return r;
}

void rememberCanvasArea(KisMainWindow *mw)
{
    if (!mw) return;
    KisView *view = mw->activeView();
    if (!view || !view->isVisible() || view->width() < 200 || view->height() < 200) return;
    const QRect r(view->mapTo(mw, QPoint(0, 0)), view->size());
    if (r == canvasAreaSlot()) return;
    canvasAreaSlot() = r;
    KSharedConfig::openConfig()->group("Nanquim").writeEntry("canvasArea", r);
}

// a página do tamanho da imagem, centrada e cabendo na área (como o Krita mostra ao abrir)
QRectF fitPageInto(const QSizeF &page, const QRect &area)
{
    if (page.isEmpty() || area.isEmpty()) return QRectF();
    QSizeF to = page;
    to.scale(QSizeF(area.size()), Qt::KeepAspectRatio);
    return QRectF(area.center().x() + 0.5 - to.width() / 2, area.center().y() + 0.5 - to.height() / 2,
                  to.width(), to.height());
}

void setOpenOrigin(QWidget *from, const QRect &local, const QPixmap &shot)
{
    s_openFrom = QRect();
    openShot() = QPixmap();
    if (!from || shot.isNull() || local.isEmpty()) return;
    QWidget *win = from->window();
    s_openFrom = QRect(from->mapTo(win, local.topLeft()), local.size());
    openShot() = shot;
}

class OpeningVeil : public QWidget
{
public:
    OpeningVeil(QWidget *parent, const QString &name, const QString &detail,
                const QRect &from = QRect(), const QPixmap &shot = QPixmap())
        : QWidget(parent), m_name(name), m_detail(detail), m_from(from), m_shot(shot)
    {
        setObjectName(QStringLiteral("nanquimOpeningVeil"));
        setAttribute(Qt::WA_TransparentForMouseEvents, false);
        setFocusPolicy(Qt::NoFocus);
        m_spin.setInterval(16);
        QObject::connect(&m_spin, &QTimer::timeout, this, [this]() { m_angle = (m_angle + 6) % 360; update(); });
        // com a página crescendo não há círculo: a leitura do arquivo trava a tela e ele congelaria
        if (!hasGrowth()) m_spin.start();
        if (hasGrowth()) {
            // a página cresce da miniatura até caber na tela (curva de saída forte)
            m_grow = new QVariantAnimation(this);
            m_grow->setStartValue(0.0);
            m_grow->setEndValue(1.0);
            m_grow->setDuration(GROW_MS);
            m_grow->setEasingCurve(QEasingCurve::OutQuint);
            QObject::connect(m_grow, &QVariantAnimation::valueChanged, this, [this](const QVariant &v) {
                m_p = v.toReal();
                update();
            });
        }
    }
    static const int GROW_MS = 320;
    bool hasGrowth() const { return !m_shot.isNull() && !m_from.isEmpty(); }
    void start() { if (m_grow) m_grow->start(); }
    // some em 180 ms com curva de saída e se apaga (a área de desenho já está por baixo, alinhada)
    void fadeAndClose()
    {
        if (m_closing) return;
        m_closing = true;
        QVariantAnimation *fade = new QVariantAnimation(this);
        fade->setStartValue(1.0);
        fade->setEndValue(0.0);
        fade->setDuration(180);
        fade->setEasingCurve(QEasingCurve::OutCubic);
        QObject::connect(fade, &QVariantAnimation::valueChanged, this, [this](const QVariant &v) {
            m_alpha = v.toReal();
            update();
        });
        QObject::connect(fade, &QVariantAnimation::finished, this, [this]() { deleteLater(); });
        fade->start();
    }
protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter g(this);
        g.setRenderHint(QPainter::Antialiasing);
        g.setRenderHint(QPainter::SmoothPixmapTransform);
        g.setOpacity(m_alpha);
        if (hasGrowth()) {
            // o fundo vira a cor da área de desenho junto com o crescimento; a página vai até o
            // lugar exato em que o Krita vai mostrá-la, então a troca no fim não dá pulo
            g.fillRect(rect(), QColor(36, 35, 31, qRound(255 * qMin(1.0, m_p * 1.4))));
            const QSizeF pageSize = QSizeF(m_shot.size()) / m_shot.devicePixelRatio();
            QRectF target = fitPageInto(pageSize, storedCanvasArea(this));
            if (target.isEmpty()) {
                QSizeF to = pageSize;
                to.scale(QSizeF(rect().adjusted(32, 32, -32, -140).size()), Qt::KeepAspectRatio);
                target = QRectF(width() / 2.0 - to.width() / 2, 32 + (height() - 172 - to.height()) / 2,
                                to.width(), to.height());
            }
            const QRectF start(m_from);
            const QRectF now(start.left() + (target.left() - start.left()) * m_p,
                             start.top() + (target.top() - start.top()) * m_p,
                             start.width() + (target.width() - start.width()) * m_p,
                             start.height() + (target.height() - start.height()) * m_p);
            QPainterPath clip;
            clip.addRoundedRect(now, 4, 4);
            g.save();
            g.setClipPath(clip);
            g.fillPath(clip, QColor(0xf3, 0xee, 0xe3));
            g.drawPixmap(now, m_shot, QRectF(m_shot.rect()));
            g.restore();
            if (m_p < 1.0) return;   // o aviso só aparece com a página já no lugar
            // "Abrindo…" escrito uma vez, embaixo da página (ou numa pílula sobre ela, se não couber)
            QFont lf = font();
            lf.setPixelSize(15);
            lf.setWeight(QFont::DemiBold);
            g.setFont(lf);
            const QString label = QFontMetrics(lf).elidedText(m_name, Qt::ElideMiddle, width() - 64);
            const int lw = QFontMetrics(lf).horizontalAdvance(label) + 32;
            QRectF pill(width() / 2.0 - lw / 2.0, target.bottom() + 12, lw, 34);
            if (pill.bottom() > height() - 8) pill.moveBottom(target.bottom() - 16);
            g.setPen(Qt::NoPen);
            g.setBrush(QColor(36, 35, 31, 215));
            g.drawRoundedRect(pill, 17, 17);
            g.setPen(QColor(0xcf, 0xca, 0xbf));
            g.drawText(pill, Qt::AlignCenter, label);
            return;
        } else {
            g.fillRect(rect(), QColor(36, 35, 31, 215));
        }
        QFont f = font();
        f.setPixelSize(17);
        f.setWeight(QFont::DemiBold);
        QFont s = font();
        s.setPixelSize(13);
        const int textW = qMax(QFontMetrics(f).horizontalAdvance(m_name), QFontMetrics(s).horizontalAdvance(m_detail));
        const int w = qMin(width() - 48, textW + 120);
        const QRectF card((width() - w) / 2.0, height() / 2.0 - 44, w, 88);
        g.setPen(QPen(QColor(0x61, 0x5e, 0x56), 1));
        g.setBrush(QColor(0x3d, 0x3c, 0x37));
        g.drawRoundedRect(card, 18, 18);
        const QRectF ring(card.left() + 24, card.center().y() - 16, 32, 32);
        g.setPen(QPen(QColor(0x5a, 0x57, 0x4f), 4));
        g.drawEllipse(ring);
        g.setPen(QPen(QColor(0x6f, 0xbf, 0xad), 4, Qt::SolidLine, Qt::RoundCap));
        g.drawArc(ring, -m_angle * 16, 100 * 16);
        const QRectF text(ring.right() + 16, card.top() + 18, card.right() - ring.right() - 36, 52);
        g.setFont(f);
        g.setPen(QColor(0xee, 0xea, 0xe0));
        g.drawText(QRectF(text.left(), text.top(), text.width(), 26), Qt::AlignLeft | Qt::AlignVCenter,
                   QFontMetrics(f).elidedText(m_name, Qt::ElideMiddle, int(text.width())));
        g.setFont(s);
        g.setPen(QColor(0xa8, 0xa3, 0x98));
        g.drawText(QRectF(text.left(), text.top() + 28, text.width(), 22), Qt::AlignLeft | Qt::AlignVCenter,
                   QFontMetrics(s).elidedText(m_detail, Qt::ElideRight, int(text.width())));
    }
    void mousePressEvent(QMouseEvent *e) override { e->accept(); }   // toques não passam enquanto abre
private:
    QString m_name, m_detail;
    QRect m_from;
    QPixmap m_shot;
    QTimer m_spin;
    QVariantAnimation *m_grow = nullptr;
    int m_angle = 0;
    qreal m_p = 0.0;
    qreal m_alpha = 1.0;
    bool m_closing = false;
};

void openPageWithFeedback(KisMainWindow *mw, const QString &path)
{
    if (!mw || path.isEmpty()) return;
    if (mw->findChild<QWidget*>(QStringLiteral("nanquimOpeningVeil"))) return;   // já está abrindo uma
    const qint64 bytes = QFileInfo(path).size();
    const QString size = bytes >= 1048576 ? i18n("%1 MB", QLocale().toString(bytes / 1048576.0, 'f', 0))
                                          : QString();
    const QString detail = bytes >= 50 * 1048576
            ? i18n("%1 · páginas grandes levam alguns segundos", size)
            : (size.isEmpty() ? i18n("Só um instante") : size);
    QString name = QFileInfo(path).completeBaseName();
    // a página sai da miniatura tocada (se quem chamou disse de onde)
    const QRect from = s_openFrom;
    const QPixmap shot = openShot();
    s_openFrom = QRect();
    openShot() = QPixmap();
    OpeningVeil *veil = new OpeningVeil(mw, i18n("Abrindo %1…", name), detail, from, shot);
    veil->setGeometry(mw->rect());
    veil->show();
    veil->raise();
    veil->repaint();
    veil->start();
    QPointer<OpeningVeil> veilGuard(veil);
    QPointer<KisMainWindow> mwGuard(mw);
    auto close = [veilGuard]() { if (veilGuard) veilGuard->fadeAndClose(); };
    QObject::connect(KisPart::instance(), &KisPart::sigViewAdded, veil, [close, mwGuard](KisView *) {
        KisUsageLogger::log(QStringLiteral("[Nanquim] abertura: sigViewAdded disparado"));
        QTimer::singleShot(200, close);
        QTimer::singleShot(600, mwGuard.data(), [mwGuard]() { rememberCanvasArea(mwGuard); });
    });
    QTimer::singleShot(60000, veil, close);   // nunca fica preso
    // espera o véu chegar à tela (e a página terminar de crescer) antes de a
    // leitura travar o app
    const int delay = veil->hasGrowth() ? OpeningVeil::GROW_MS + 40 : 60;
    QTimer::singleShot(delay, mw, [mwGuard, path, close]() {
        if (!mwGuard) { close(); return; }
        KisUsageLogger::log(QStringLiteral("[Nanquim] abertura: iniciando openDocument (%1)")
                            .arg(QFileInfo(path).fileName()));
        const bool ok = mwGuard->openDocument(path, KisMainWindow::None);
        KisUsageLogger::log(QStringLiteral("[Nanquim] abertura: openDocument retornou %1")
                            .arg(ok ? QStringLiteral("ok") : QStringLiteral("falhou")));
        if (!ok) close();
        else QTimer::singleShot(400, mwGuard.data(), close);
    });
}


void focusBarOpenPage(KisMainWindow *mw, const QString &path)
{
    openPageWithFeedback(mw, path);
}

void refreshFocusBarInfo(KisMainWindow *mw)
{
    if (!mw) return;
    QFrame *bar = mw->findChild<QFrame*>(QStringLiteral("nanquimFocusBar"));
    if (!bar) return;
    KisView *view = mw->activeView();
    const QString path = (view && view->document()) ? view->document()->path() : QString();
    QString title = path.isEmpty()
            ? (view && view->document() ? view->document()->caption().trimmed() : QStringLiteral("NANQUIM"))
            : displayNameFor(path);
    if (title.endsWith(QLatin1String(".kra"), Qt::CaseInsensitive)) title.chop(4);
    if (title.isEmpty()) title = QStringLiteral("NANQUIM");

    const ChapterStore::PageInfo info = ChapterStore::instance()->info(path);
    int curIdx = 1;
    int total = 1;
    QString prevPath;
    QString nextPath;
    if (!info.chapterId.isEmpty()) {
        const QStringList pages = ChapterStore::instance()->chapter(info.chapterId).pages;
        const int i = pages.indexOf(path);
        total = qMax(1, pages.size());
        curIdx = qMax(1, i + 1);
        if (i > 0) prevPath = pages.at(i - 1);
        if (i >= 0 && i + 1 < pages.size()) nextPath = pages.at(i + 1);
    }
    bar->setProperty("nanquimFocusPrevPath", prevPath);
    bar->setProperty("nanquimFocusNextPath", nextPath);

    if (QPushButton *hdr = bar->findChild<QPushButton*>(QStringLiteral("nanquimFocusHeader"))) {
        const QFontMetrics fm(hdr->font());
        const QString shortName = fm.elidedText(title, Qt::ElideRight, 48);
        hdr->setText(QStringLiteral("%1\np. %2").arg(shortName).arg(curIdx));
    }
    if (QLabel *lbl = bar->findChild<QLabel*>(QStringLiteral("nanquimFocusPageLabel"))) {
        lbl->setText(QStringLiteral("%1/%2").arg(curIdx).arg(total));
    }
    if (QToolButton *prev = bar->findChild<QToolButton*>(QStringLiteral("nanquimFocusPrev"))) {
        prev->setEnabled(!prevPath.isEmpty());
    }
    if (QToolButton *next = bar->findChild<QToolButton*>(QStringLiteral("nanquimFocusNext"))) {
        next->setEnabled(!nextPath.isEmpty());
    }
}

void createPage(const PagePreset &p)
{
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    KisDocument *doc = KisPart::instance()->createDocument();
    KoColor white(Qt::white, cs);
    if (!doc->newImage(i18n(p.name), p.width, p.height, cs, white, KisConfig::RASTER_LAYER, 2, QString(), p.dpi / 72.0)) {
        delete doc;
        return;
    }
    doc->setModified(false);
    attachDocumentToChapter(doc, s_pendingChapter);
    KisPart::instance()->startCustomDocument(doc);
}

QToolButton *makeMoreButton(KisMainWindow *mw, QWidget *parent)
{
    QToolButton *more = new QToolButton(parent);
    more->setIcon(designIcon(Svg::MORE));
    more->setIconSize(QSize(22, 22));
    more->setToolTip(i18n("Mais opções"));
    more->setFocusPolicy(Qt::NoFocus);
    QMenu *moreMenu = new QMenu(more);
    QObject::connect(moreMenu, &QMenu::aboutToShow, mw, [mw, moreMenu]() {
        moreMenu->clear();
        Q_FOREACH (QAction *a, mw->menuBar()->actions()) {
            moreMenu->addAction(a);
        }
    });
    more->setMenu(moreMenu);
    more->setPopupMode(QToolButton::InstantPopup);
    return more;
}


void showWelcome(KisMainWindow *mw, bool startAtGuide);
void showChapterSheet(KisMainWindow *mw, const QString &chapterId, std::function<void(const QString &)> done);


// formato das páginas novas de um capítulo (o "+ Nova página" de dentro dele)
const PagePreset CHAPTER_FORMATS[] = {
    {"Mangá B4", "B4 · 600 dpi", 6071, 8598, 600},
    {"Comic americano", "400 dpi", 2650, 4100, 400},
    {"A4", "300 dpi", 2480, 3508, 300},
    {"Webtoon", "800 px", 800, 12800, 72},
};
const int CHAPTER_FORMAT_COUNT = int(sizeof(CHAPTER_FORMATS) / sizeof(CHAPTER_FORMATS[0]));

// recentes com filtro "sem capítulo" além da busca
const QString LOOSE_ID = QStringLiteral("__loose__");   // aba "Páginas soltas"

// dados extras que a grade da tela inicial lê de cada página
const int ROLE_STAGE = Qt::UserRole + 60;          // etapa (só páginas de capítulo)
const int ROLE_CHAPTER_NAME = Qt::UserRole + 61;
const int ROLE_CONTINUE = Qt::UserRole + 62;       // a página do "Continuar"
const int ROLE_SELECTED = Qt::UserRole + 63;       // marcada no modo Selecionar

// recentes filtrados por capítulo (ou páginas soltas), etapa e busca
class RecentsFilter : public QSortFilterProxyModel
{
public:
    using QSortFilterProxyModel::QSortFilterProxyModel;
    void setLooseOnly(bool on) { setChapter(on ? LOOSE_ID : QString()); }
    // "" todas; LOOSE_ID só as soltas; senão, as páginas desse capítulo, na ordem dele
    void setChapter(const QString &id) { m_chapter = id; refilter(); }
    QString chapter() const { return m_chapter; }
    // -1: todas as etapas; 0 rascunho, 1 arte-final, 2 pronta (só páginas de capítulo)
    void setStage(int stage) { m_stage = stage; invalidateFilter(); }
    int stage() const { return m_stage; }
    void setContinuePath(const QString &path) { if (path != m_continue) { m_continue = path; touchAll(); } }
    void setSelection(const QSet<QString> &paths) { m_selected = paths; touchAll(); }
    void refilter()
    {
        invalidate();
        sort(!m_chapter.isEmpty() && m_chapter != LOOSE_ID ? 0 : -1);
    }
    QVariant data(const QModelIndex &index, int role) const override
    {
        if (role == ROLE_STAGE || role == ROLE_CHAPTER_NAME || role == ROLE_CONTINUE || role == ROLE_SELECTED) {
            const QString path = QSortFilterProxyModel::data(index, Qt::ToolTipRole).toString();
            if (role == ROLE_CONTINUE) return !path.isEmpty() && path == m_continue;
            if (role == ROLE_SELECTED) return m_selected.contains(path);
            const ChapterStore::PageInfo pi = ChapterStore::instance()->info(path);
            if (pi.chapterId.isEmpty()) return QVariant();
            return role == ROLE_STAGE ? QVariant(pi.stage) : QVariant(pi.chapterName);
        }
        return QSortFilterProxyModel::data(index, role);
    }
protected:
    bool filterAcceptsRow(int row, const QModelIndex &parent) const override
    {
        if (!QSortFilterProxyModel::filterAcceptsRow(row, parent)) return false;
        if (m_chapter.isEmpty() && m_stage < 0) return true;
        const QString path = sourceModel()->index(row, 0, parent).data(Qt::ToolTipRole).toString();
        const ChapterStore::PageInfo pi = ChapterStore::instance()->info(path);
        if (m_chapter == LOOSE_ID) {
            if (!pi.chapterId.isEmpty()) return false;
        } else if (!m_chapter.isEmpty() && pi.chapterId != m_chapter) {
            return false;
        }
        if (m_stage >= 0) return !pi.chapterId.isEmpty() && pi.stage == m_stage;
        return true;
    }
    bool lessThan(const QModelIndex &l, const QModelIndex &r) const override
    {
        return ChapterStore::instance()->info(l.data(Qt::ToolTipRole).toString()).index
                < ChapterStore::instance()->info(r.data(Qt::ToolTipRole).toString()).index;
    }
private:
    void touchAll()
    {
        if (rowCount() > 0) Q_EMIT dataChanged(index(0, 0), index(rowCount() - 1, 0));
    }
    QString m_chapter;
    int m_stage = -1;
    QString m_continue;
    QSet<QString> m_selected;
};

// ---------------------------------------------------------------------------
// Tela inicial (build 74): grade de páginas, como numa mesa de desenho.
// Cada papel com sombra é desenhado uma vez e guardado; ao rolar, a grade só
// cola as imagens prontas (antes cada quadro reescalava as miniaturas de
// 512 px, e a rolagem caía para uns 20 quadros por segundo).
// ---------------------------------------------------------------------------
QPixmap cachedPaper(const QIcon &icon, const QSize &area, int radius)
{
    const qreal dpr = qApp->devicePixelRatio();
    const QString key = QStringLiteral("nqpaper-%1-%2x%3-%4").arg(icon.isNull() ? 0 : icon.cacheKey())
            .arg(area.width()).arg(area.height()).arg(radius);
    QPixmap pm;
    if (QPixmapCache::find(key, &pm)) return pm;
    pm = QPixmap((QSizeF(area) * dpr).toSize());
    pm.setDevicePixelRatio(dpr);
    pm.fill(Qt::transparent);
    QPainter g(&pm);
    g.setRenderHint(QPainter::Antialiasing);
    g.setRenderHint(QPainter::SmoothPixmapTransform);
    paintPage(&g, QRect(QPoint(0, 0), area).adjusted(4, 0, -4, -6), icon, radius);
    g.end();
    QPixmapCache::insert(key, pm);
    return pm;
}

QColor stageColor(int s)
{
    return s == 2 ? QColor(0x5e, 0xc9, 0x8c) : s == 1 ? QColor(0x6f, 0xbf, 0xad) : QColor(0xe6, 0xb4, 0x5a);
}

QString stageName(int s)
{
    return s == 2 ? i18n("Pronta") : s == 1 ? i18n("Arte-final") : i18n("Rascunho");
}

// pílula escura com bolinha colorida (etapa) ou em degradê (Continuar)
void paintPill(QPainter *g, const QRectF &pill, const QString &text, const QColor &dot, bool accent, const QFont &base)
{
    QFont f = base;
    f.setPixelSize(11);
    f.setWeight(QFont::Bold);
    g->setPen(Qt::NoPen);
    if (accent) {
        QLinearGradient lg(pill.topLeft(), pill.topRight());
        lg.setColorAt(0, QColor(0x3e, 0x8e, 0x7e));
        lg.setColorAt(1, QColor(0x6f, 0xbf, 0xad));
        g->setBrush(lg);
    } else {
        g->setBrush(QColor(36, 35, 31, 215));
    }
    g->drawRoundedRect(pill, pill.height() / 2, pill.height() / 2);
    qreal tx = pill.left() + 10;
    if (dot.isValid()) {
        g->setBrush(dot);
        g->drawEllipse(QPointF(pill.left() + 11, pill.center().y()), 4, 4);
        tx = pill.left() + 20;
    }
    g->setFont(f);
    g->setPen(accent ? QColor(0x13, 0x23, 0x20) : QColor(0xee, 0xea, 0xe0));
    g->drawText(QRectF(tx, pill.top(), pill.right() - tx - 8, pill.height()), Qt::AlignVCenter | Qt::AlignLeft, text);
}

qreal pillWidth(const QString &text, bool withDot, const QFont &base)
{
    QFont f = base;
    f.setPixelSize(11);
    f.setWeight(QFont::Bold);
    return QFontMetrics(f).horizontalAdvance(text) + (withDot ? 28 : 18);
}

class PageTileDelegate : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    static const int TEXT_H = 50;
    // seis colunas na horizontal do Tab S10 Lite; nunca menos de três
    static QSize cellFor(int viewportWidth)
    {
        const int cols = qBound(3, viewportWidth / 196, 8);
        const int w = qMax(120, viewportWidth / cols);
        return QSize(w, qRound((w - 16) * 1.2) + 16 + TEXT_H);
    }
    static QRect tileRect(const QRect &cell)
    {
        return QRect(cell.left() + 8, cell.top() + 8, cell.width() - 16, cell.height() - 16 - TEXT_H);
    }
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &) const override
    {
        const QListView *v = qobject_cast<const QListView*>(option.widget);
        return v ? v->gridSize() : QSize(196, 300);
    }
    void paint(QPainter *g, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        g->save();
        g->setRenderHint(QPainter::Antialiasing);
        const QRect tile = tileRect(option.rect);
        const bool cont = index.data(ROLE_CONTINUE).toBool();
        const bool selecting = option.widget && option.widget->property("nanquimSelectMode").toBool();
        const bool picked = selecting && index.data(ROLE_SELECTED).toBool();
        const QRectF card = QRectF(tile).adjusted(0.5, 0.5, -0.5, -0.5);
        g->setPen(picked ? QPen(QColor(0x6f, 0xbf, 0xad), 2) : cont ? QPen(C_ACCENT, 2) : QPen(QColor(0x4a, 0x48, 0x42), 1));
        g->setBrush(picked ? QColor(0x2a, 0x3d, 0x38) : QColor(0x2e, 0x2d, 0x29));
        g->drawRoundedRect(card, 14, 14);
        const QRect paperArea = tile.adjusted(12, 12, -12, -8);
        g->drawPixmap(paperArea.topLeft(), cachedPaper(index.data(Qt::DecorationRole).value<QIcon>(), paperArea.size(), 4));

        const QVariant stage = index.data(ROLE_STAGE);
        qreal pillY = tile.top() + 10;
        if (cont) {
            const QString t = i18n("Continuar");
            paintPill(g, QRectF(tile.left() + 10, pillY, pillWidth(t, false, option.font), 24), t, QColor(), true, option.font);
            pillY += 28;
        }
        if (stage.isValid()) {
            const QString t = stageName(stage.toInt());
            paintPill(g, QRectF(tile.left() + 10, pillY, pillWidth(t, true, option.font), 24), t, stageColor(stage.toInt()), false, option.font);
        }
        if (selecting) {
            const QRectF box(tile.right() - 38, tile.top() + 10, 28, 28);
            g->setPen(QPen(picked ? QColor(0x6f, 0xbf, 0xad) : QColor(0xee, 0xea, 0xe0), 2));
            g->setBrush(picked ? QColor(0x6f, 0xbf, 0xad) : QColor(36, 35, 31, 170));
            g->drawEllipse(box);
            if (picked) {
                g->setPen(QPen(QColor(0x13, 0x23, 0x20), 2.6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                g->drawPolyline(QPolygonF() << box.center() + QPointF(-6, 0) << box.center() + QPointF(-1.5, 4.5)
                                            << box.center() + QPointF(6.5, -4.5));
            }
        }

        QString name = index.data(Qt::DisplayRole).toString();
        if (name.endsWith(QLatin1String(".kra"), Qt::CaseInsensitive)) name.chop(4);
        QFont f = option.font;
        f.setPixelSize(15);
        f.setWeight(QFont::DemiBold);
        g->setFont(f);
        g->setPen(C_TEXT);
        const QRect nameRect(tile.left() + 2, tile.bottom() + 8, tile.width() - 4, 21);
        g->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, QFontMetrics(f).elidedText(name, Qt::ElideRight, nameRect.width()));
        const QString sub = recentSubtitle(index.data(Qt::ToolTipRole).toString());
        if (!sub.isEmpty()) {
            QFont sf = option.font;
            sf.setPixelSize(13);
            g->setFont(sf);
            g->setPen(QColor(0xa8, 0xa3, 0x98));
            const QRect subRect(nameRect.left(), nameRect.bottom() + 1, nameRect.width(), 18);
            g->drawText(subRect, Qt::AlignLeft | Qt::AlignVCenter, QFontMetrics(sf).elidedText(sub, Qt::ElideRight, subRect.width()));
        }
        g->restore();
    }
};

// páginas em dupla, como o leitor vê o capítulo aberto (uma linha = duas páginas)
const int ROLE_SPREAD_PATHS = Qt::UserRole + 70;   // QStringList {esquerda, direita}; "" = lugar vazio
const int ROLE_SPREAD_NUMS = Qt::UserRole + 71;    // QVariantList {número esquerda, número direita}

class SpreadDelegate : public QStyledItemDelegate
{
public:
    explicit SpreadDelegate(QObject *parent, std::function<QIcon(const QString &)> icon, std::function<int(const QString &)> stage)
        : QStyledItemDelegate(parent), m_icon(std::move(icon)), m_stage(std::move(stage)) {}
    static QSize cellFor(int viewportWidth, qreal aspect)
    {
        const int cols = viewportWidth >= 900 ? 3 : 2;
        const int w = qMax(240, viewportWidth / cols);
        const int pageW = (w - 16 - 28) / 2;
        return QSize(w, qRound(pageW / qMax(0.3, aspect)) + 16 + 28 + 30);
    }
    // metade da dupla que o dedo tocou: 0 esquerda, 1 direita
    static int sideAt(const QRect &cell, const QPoint &p) { return p.x() < cell.center().x() ? 0 : 1; }
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &) const override
    {
        const QListView *v = qobject_cast<const QListView*>(option.widget);
        return v ? v->gridSize() : QSize(300, 260);
    }
    void paint(QPainter *g, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        g->save();
        g->setRenderHint(QPainter::Antialiasing);
        const QRect cell = option.rect.adjusted(8, 8, -8, -8);
        const QRect card(cell.left(), cell.top(), cell.width(), cell.height() - 30);
        g->setPen(QPen(QColor(0x4a, 0x48, 0x42), 1));
        g->setBrush(QColor(0x2e, 0x2d, 0x29));
        g->drawRoundedRect(QRectF(card).adjusted(0.5, 0.5, -0.5, -0.5), 14, 14);
        const QStringList paths = index.data(ROLE_SPREAD_PATHS).toStringList();
        const QVariantList nums = index.data(ROLE_SPREAD_NUMS).toList();
        const int pageW = (card.width() - 28) / 2;
        const int pageH = card.height() - 28;
        for (int side = 0; side < 2 && side < paths.size(); side++) {
            const QRect r(card.center().x() - (side == 0 ? pageW : 0) + (side == 0 ? -1 : 1), card.top() + 14, pageW, pageH);
            if (paths[side].isEmpty()) {
                g->setPen(QPen(QColor(0x5a, 0x57, 0x4f), 1.4, Qt::DashLine));
                g->setBrush(Qt::NoBrush);
                g->drawRoundedRect(QRectF(r).adjusted(4, 0, -4, -6), 3, 3);
                continue;
            }
            const QRect paper = fitPage(QSize(r.width() - 8, r.height() - 6), r.adjusted(4, 0, -4, -6));
            const QPixmap pm = cachedPaper(m_icon(paths[side]), paper.size() + QSize(8, 6), 3);
            g->drawPixmap(paper.topLeft() - QPoint(4, 0), pm);
            const int st = m_stage(paths[side]);
            if (st >= 0) {
                g->setPen(Qt::NoPen);
                g->setBrush(stageColor(st));
                g->drawRect(QRectF(paper.left(), paper.bottom() - 4, paper.width(), 5));
            }
            QFont nf = option.font;
            nf.setPixelSize(11);
            nf.setWeight(QFont::Bold);
            const QString num = QString::number(side < nums.size() ? nums[side].toInt() : 0);
            const QRectF pill(paper.left() + 6, paper.top() + 6, QFontMetrics(nf).horizontalAdvance(num) + 14, 22);
            g->setPen(Qt::NoPen);
            g->setBrush(QColor(36, 35, 31, 230));
            g->drawRoundedRect(pill, 11, 11);
            g->setFont(nf);
            g->setPen(C_TEXT);
            g->drawText(pill, Qt::AlignCenter, num);
        }
        QFont f = option.font;
        f.setPixelSize(13);
        f.setWeight(QFont::DemiBold);
        g->setFont(f);
        g->setPen(QColor(0xa8, 0xa3, 0x98));
        g->drawText(QRect(cell.left(), card.bottom() + 4, cell.width(), 24), Qt::AlignCenter, index.data(Qt::DisplayRole).toString());
        g->restore();
    }
private:
    std::function<QIcon(const QString &)> m_icon;
    std::function<int(const QString &)> m_stage;
};

// botão "voltar" do Android (e Esc) fecha o capítulo aberto
class BackKeyFilter : public QObject
{
public:
    BackKeyFilter(QObject *parent, std::function<bool()> handler) : QObject(parent), m_handler(handler) {}
protected:
    bool eventFilter(QObject *, QEvent *e) override
    {
        if (e->type() != QEvent::KeyPress) return false;
        const int key = static_cast<QKeyEvent*>(e)->key();
        if (key != Qt::Key_Back && key != Qt::Key_Escape) return false;
        return m_handler();
    }
private:
    std::function<bool()> m_handler;
};

QPushButton *galleryButton(const QString &text, const char *svg, QWidget *parent)
{
    QPushButton *b = new QPushButton(text, parent);
    b->setProperty("class", QStringLiteral("nanquimGhost"));
    b->setFocusPolicy(Qt::NoFocus);
    if (svg) {
        b->setIcon(designIcon(svg, 18));
        b->setIconSize(QSize(18, 18));
    }
    return b;
}

// faz a troca entre tela inicial e capítulo com a animação das páginas
void runChapterTransition(QStackedWidget *stack, QWidget *homePage, QWidget *chapterPage,
                          QWidget *origin, QListView *grid, bool opening)
{
    auto collectCells = [chapterPage, grid]() {
        QList<QRect> cells;
        QAbstractItemModel *m = grid->model();
        const QRect vp = grid->viewport()->rect();
        for (int i = 0; m && i < qMin(m->rowCount(), 40); i++) {
            const QRect vr = grid->visualRect(m->index(i, 0));
            if (!vr.isValid() || !vr.intersects(vp)) continue;
            cells << QRect(grid->viewport()->mapTo(chapterPage, vr.topLeft()), vr.size());
        }
        return cells;
    };
    // as páginas saem do botão que abriu o capítulo (ou do meio da tela)
    QPointer<QWidget> originGuard(origin);
    auto coverRect = [homePage, originGuard]() {
        if (originGuard && originGuard->isVisible()) {
            const QPoint c = originGuard->mapTo(homePage, originGuard->rect().center());
            return QRect(c.x() - 30, c.y() - 42, 60, 84);
        }
        const QPoint c = homePage->rect().center();
        return QRect(c.x() - 50, c.y() - 70, 100, 140);
    };
    QPixmap homeShot, chapterShot;
    QList<QRect> cells;
    QRect gridArea;
    if (opening) {
        homeShot = lightShot(homePage->grab());
        stack->setCurrentWidget(chapterPage);
        if (chapterPage->layout()) chapterPage->layout()->activate();
        grid->doItemsLayout();
        chapterShot = lightShot(chapterPage->grab());
    } else {
        chapterShot = lightShot(chapterPage->grab());
    }
    cells = collectCells();
    gridArea = QRect(grid->viewport()->mapTo(chapterPage, QPoint(0, 0)), grid->viewport()->size());
    if (!opening) {
        stack->setCurrentWidget(homePage);
        if (homePage->layout()) homePage->layout()->activate();
        homeShot = lightShot(homePage->grab());
    }
    const QRect cover = coverRect();

    ChapterTransition *overlay = new ChapterTransition(stack, homeShot, chapterShot, cover, gridArea, cells);
    overlay->setProgress(opening ? 0.0 : 1.0);
    overlay->show();
    overlay->raise();
    QVariantAnimation *anim = new QVariantAnimation(overlay);
    anim->setDuration(opening ? 320 : 260);
    anim->setEasingCurve(opening ? QEasingCurve::OutCubic : QEasingCurve::InOutCubic);
    anim->setStartValue(opening ? 0.0 : 1.0);
    anim->setEndValue(opening ? 1.0 : 0.0);
    QObject::connect(anim, &QVariantAnimation::valueChanged, overlay, [overlay](const QVariant &v) { overlay->setProgress(v.toReal()); });
    QObject::connect(anim, &QVariantAnimation::finished, overlay, [overlay]() { overlay->deleteLater(); });
    anim->start();
}

// "Continuar de onde parou" (redesenho): a última página, grande, à esquerda
// da grade. Um toque volta para ela.
class ContinueCard : public QAbstractButton
{
public:
    explicit ContinueCard(QWidget *parent) : QAbstractButton(parent)
    {
        setFocusPolicy(Qt::NoFocus);
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    }
    void setContent(const QString &label, const QString &title, const QString &sub, int stage, const QPixmap &page)
    {
        m_label = label;
        m_title = title;
        m_sub = sub;
        m_stage = stage;
        m_page = page;
        setAccessibleName(QStringLiteral("%1: %2").arg(label, title));
        setToolTip(stage >= 0 ? QStringLiteral("%1 · %2").arg(title, stageName(stage)) : title);
        update();
    }
protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter g(this);
        g.setRenderHint(QPainter::Antialiasing);
        g.setRenderHint(QPainter::SmoothPixmapTransform);
        const QRectF r = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -24.5);
        g.setPen(QPen(QColor(238, 234, 224, 20), 1));
        g.setBrush(isDown() ? QColor(0x34, 0x33, 0x2e) : QColor(0x2e, 0x2d, 0x29));
        g.drawRoundedRect(r, 24, 24);

        QFont small = font();
        small.setPixelSize(14);
        QFont big = font();
        big.setPixelSize(22);
        big.setWeight(QFont::DemiBold);
        const qreal pad = 28;
        g.setFont(small);
        g.setPen(C_MUTED);
        g.drawText(QRectF(r.left() + pad, r.top() + 22, r.width() - 2 * pad, 20), Qt::AlignLeft | Qt::AlignVCenter, m_label);

        // a página, em pé, do maior tamanho que couber
        const QRectF area(r.left() + pad, r.top() + 60, r.width() - 2 * pad, r.height() - 60 - 96);
        QSizeF paper = m_page.isNull() ? QSizeF(257, 364) : QSizeF(m_page.size()) / m_page.devicePixelRatio();
        paper.scale(area.size(), Qt::KeepAspectRatio);
        const QRectF pr(area.center().x() - paper.width() / 2, area.center().y() - paper.height() / 2,
                        paper.width(), paper.height());
        m_paperRect = pr.toRect();
        QPainterPath clip;
        clip.addRoundedRect(pr, 4, 4);
        g.fillPath(clip, QColor(0xf3, 0xee, 0xe3));
        if (!m_page.isNull()) {
            g.save();
            g.setClipPath(clip);
            g.drawPixmap(pr, m_page, QRectF(m_page.rect()));
            g.restore();
        }

        const QRectF titleRect(r.left() + pad, r.bottom() - 78, r.width() - 2 * pad, 28);
        const QRectF subRect(r.left() + pad, r.bottom() - 48, r.width() - 2 * pad, 20);
        QString stage;
        QColor stageCol;
        if (m_stage >= 0) {
            stage = stageName(m_stage);
            stageCol = stageColor(m_stage);
        }
        const qreal stageW = stage.isEmpty() ? 0 : QFontMetrics(small).horizontalAdvance(stage) + 12;
        g.setFont(big);
        g.setPen(C_TEXT);
        g.drawText(titleRect, Qt::AlignLeft | Qt::AlignVCenter,
                   QFontMetrics(big).elidedText(m_title, Qt::ElideRight, int(titleRect.width())));
        g.setFont(small);
        g.setPen(C_MUTED);
        g.drawText(subRect.adjusted(0, 0, -stageW, 0), Qt::AlignLeft | Qt::AlignVCenter,
                   QFontMetrics(small).elidedText(m_sub, Qt::ElideRight, int(subRect.width() - stageW)));
        if (!stage.isEmpty()) {
            g.setPen(stageCol);
            g.drawText(subRect, Qt::AlignRight | Qt::AlignVCenter, stage);
        }
    }
public:
    QRect paperRect() const { return m_paperRect; }
    QPixmap page() const { return m_page; }
private:
    QString m_label, m_title, m_sub;
    int m_stage = -1;
    QPixmap m_page;
    QRect m_paperRect;
};


// ---------------------------------------------------------------------------
// Volta à galeria animada: a página sai da área de desenho e encolhe até o
// cartão dela na grade (o caminho inverso da abertura). Sem cartão visível,
// só esmaece. Nunca segura toques por mais de ~0,3 s.
// ---------------------------------------------------------------------------
class GalleryReturnFly : public QWidget
{
public:
    GalleryReturnFly(QWidget *parent, const QPixmap &page, const QRectF &from)
        : QWidget(parent), m_page(page), m_from(from), m_to(from)
    {
        setObjectName(QStringLiteral("nanquimReturnFly"));
        setGeometry(parent->rect());
    }
    void flyTo(const QRectF &to) { run(to, false); }
    void fadeOut() { run(m_from, true); }
protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter g(this);
        g.setRenderHint(QPainter::Antialiasing);
        g.setRenderHint(QPainter::SmoothPixmapTransform);
        g.fillRect(rect(), QColor(36, 35, 31, qRound(255 * (1.0 - m_p))));
        if (m_fadeOnly) g.setOpacity(1.0 - m_p);
        const QRectF now(m_from.left() + (m_to.left() - m_from.left()) * m_p,
                         m_from.top() + (m_to.top() - m_from.top()) * m_p,
                         m_from.width() + (m_to.width() - m_from.width()) * m_p,
                         m_from.height() + (m_to.height() - m_from.height()) * m_p);
        QPainterPath clip;
        clip.addRoundedRect(now, 4, 4);
        g.setClipPath(clip);
        g.fillPath(clip, QColor(0xf3, 0xee, 0xe3));
        g.drawPixmap(now, m_page, QRectF(m_page.rect()));
    }
    void mousePressEvent(QMouseEvent *e) override { e->accept(); }
private:
    void run(const QRectF &to, bool fadeOnly)
    {
        if (m_running) return;
        m_running = true;
        m_to = to;
        m_fadeOnly = fadeOnly;
        QVariantAnimation *a = new QVariantAnimation(this);
        a->setStartValue(0.0);
        a->setEndValue(1.0);
        a->setDuration(fadeOnly ? 180 : 260);
        a->setEasingCurve(QEasingCurve::OutCubic);
        QObject::connect(a, &QVariantAnimation::valueChanged, this, [this](const QVariant &v) {
            m_p = v.toReal();
            update();
        });
        QObject::connect(a, &QVariantAnimation::finished, this, [this]() { deleteLater(); });
        a->start(QAbstractAnimation::DeleteWhenStopped);
        // segurança: some de qualquer jeito
        QTimer::singleShot(1200, this, [this]() { deleteLater(); });
    }
    QPixmap m_page;
    QRectF m_from;
    QRectF m_to;
    qreal m_p = 0.0;
    bool m_fadeOnly = false;
    bool m_running = false;
};

void showGalleryAnimated(KisMainWindow *mw)
{
    if (!mw) return;
    if (mw->findChild<QWidget*>(QStringLiteral("nanquimReturnFly"))) return;
    KisView *view = mw->activeView();
    QString path;
    QRectF from;
    if (view && view->isVisible() && view->image() && view->document()) {
        rememberCanvasArea(mw);
        path = view->document()->localFilePath();
        const QRect area(view->mapTo(mw, QPoint(0, 0)), view->size());
        from = fitPageInto(QSizeF(view->image()->width(), view->image()->height()), area);
    }
    mw->showWelcomeScreen(true);
    if (path.isEmpty() || from.isEmpty()) return;
    QListView *grid = mw->findChild<QListView*>(QStringLiteral("nanquimRecents"));
    if (!grid || !grid->model()) return;
    QModelIndex found;
    QAbstractItemModel *model = grid->model();
    for (int row = 0; row < model->rowCount() && !found.isValid(); ++row) {
        const QModelIndex idx = model->index(row, 0);
        if (idx.data(Qt::ToolTipRole).toString() == path) found = idx;
    }
    if (!found.isValid()) return;   // página fora da lista: troca simples, sem animação
    const QPixmap page = found.data(Qt::DecorationRole).value<QIcon>().pixmap(QSize(420, 600));
    if (page.isNull()) return;
    GalleryReturnFly *fly = new GalleryReturnFly(mw, page, from);
    fly->show();
    fly->raise();
    QPointer<GalleryReturnFly> flyGuard(fly);
    QPointer<QListView> gridGuard(grid);
    const QPersistentModelIndex target(found);
    // um respiro para a grade se arrumar na tela antes de medir o cartão
    QTimer::singleShot(30, fly, [flyGuard, gridGuard, target]() {
        if (!flyGuard) return;
        if (!gridGuard || !target.isValid() || !gridGuard->isVisible()) {
            flyGuard->fadeOut();
            return;
        }
        const QRect paper = PageTileDelegate::tileRect(gridGuard->visualRect(target)).adjusted(12, 12, -12, -8);
        if (paper.isEmpty() || !gridGuard->viewport()->rect().intersects(paper)) {
            flyGuard->fadeOut();
            return;
        }
        flyGuard->flyTo(QRectF(QRect(gridGuard->viewport()->mapTo(flyGuard->parentWidget(), paper.topLeft()),
                                     paper.size())));
    });
}

QWidget *buildGallery(KisMainWindow *mw)
{
    // papéis prontos da grade: espaço para umas três telas de páginas
    if (QPixmapCache::cacheLimit() < 64 * 1024) QPixmapCache::setCacheLimit(64 * 1024);

    QWidget *gallery = new QWidget();
    gallery->setObjectName(GALLERY_ID);
    gallery->setAttribute(Qt::WA_StyledBackground);
    gallery->setStyleSheet(QLatin1String(GALLERY_STYLE));
    QVBoxLayout *root = new QVBoxLayout(gallery);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);
    QStackedWidget *pages = new QStackedWidget(gallery);
    root->addWidget(pages);

    // ===================== tela inicial =====================
    // build 74: uma grade só, com todas as páginas (como a galeria do Procreate
    // e do ibis Paint). Os capítulos viram abas em cima da grade; a etapa e o
    // "Continuar" aparecem em cada página. Sem cartões fixos: não sobram
    // cartões vazios quando uma página sai da lista.
    QWidget *home = new QWidget(pages);
    home->setObjectName(QStringLiteral("nanquimHomePage"));
    home->setAttribute(Qt::WA_StyledBackground);
    QVBoxLayout *homeLayout = new QVBoxLayout(home);
    homeLayout->setContentsMargins(0, 0, 0, 0);
    homeLayout->setSpacing(0);

    QWidget *header = new QWidget(home);
    header->setObjectName(QStringLiteral("nanquimGalleryHeader"));
    header->setAttribute(Qt::WA_StyledBackground);
    header->setFixedHeight(72);
    QHBoxLayout *hl = new QHBoxLayout(header);
    hl->setContentsMargins(28, 0, 24, 0);
    hl->setSpacing(10);
    QLabel *logo = new QLabel(header);
    logo->setPixmap(renderSvg(QLatin1String(DROP_SVG), QSize(32, 32)));
    QLabel *brand = new QLabel(header);
    brand->setObjectName(QStringLiteral("nanquimBrand"));
    brand->setPixmap(brandWordmark(24, QColor(0xee, 0xea, 0xe0)));
    hl->addWidget(logo);
    hl->addWidget(brand);
    hl->addSpacing(14);
    // seções da tela inicial: as páginas do usuário e a Comunidade (pincéis para baixar)
    QButtonGroup *sectionGroup = new QButtonGroup(header);
    sectionGroup->setExclusive(true);
    QPushButton *pagesTab = galleryButton(i18n("Páginas"), nullptr, header);
    pagesTab->setCheckable(true);
    pagesTab->setChecked(true);
    QPushButton *communityTab = galleryButton(i18n("Comunidade"), nullptr, header);
    communityTab->setCheckable(true);
    sectionGroup->addButton(pagesTab, 0);
    sectionGroup->addButton(communityTab, 1);
    hl->addWidget(pagesTab);
    hl->addWidget(communityTab);
    hl->addSpacing(10);
    QLineEdit *search = new QLineEdit(header);
    search->setObjectName(QStringLiteral("nanquimSearch"));
    search->setPlaceholderText(i18n("Buscar páginas"));
    search->setClearButtonEnabled(true);
    search->addAction(designIcon(Svg::SEARCH, 18), QLineEdit::LeadingPosition);
    search->setMinimumWidth(160);
    search->setMaximumWidth(420);
    hl->addWidget(search, 1);
    hl->addStretch(0);

    // Continuar: volta ao desenho aberto ou abre a página mais recente
    QPushButton *contGo = new QPushButton(header);
    contGo->setObjectName(QStringLiteral("nanquimHomeContinue"));
    contGo->setFocusPolicy(Qt::NoFocus);
    contGo->setIcon(QIcon(svgPixmap(QString::fromUtf8(Svg::NIB), QColor(0x6f, 0xbf, 0xad), 20)));
    contGo->setIconSize(QSize(20, 20));
    contGo->setMaximumWidth(320);
    hl->addWidget(contGo);
    QPushButton *open = galleryButton(i18n("Abrir"), Svg::FOLDER, header);
    QObject::connect(open, &QPushButton::clicked, mw, [mw]() {
        if (QAction *a = act(mw, "file_open")) a->trigger();
    });
    hl->addWidget(open);
    QPushButton *guide = galleryButton(QString(), Svg::BOOK, header);
    guide->setObjectName(QStringLiteral("nanquimIconButton"));
    guide->setToolTip(i18n("Guia"));
    QObject::connect(guide, &QPushButton::clicked, mw, [mw]() { showWelcome(mw, false); });
    hl->addWidget(guide);
    QPushButton *settingsButton = galleryButton(QString(), Svg::SETTINGS, header);
    settingsButton->setToolTip(i18n("Configurações"));
    settingsButton->setObjectName(QStringLiteral("nanquimIconButton"));
    QObject::connect(settingsButton, &QPushButton::clicked, mw, [mw]() { showSettings(mw); });
    hl->addWidget(settingsButton);
    QToolButton *more = makeMoreButton(mw, header);
    more->setProperty("class", QStringLiteral("nanquimGhost"));
    more->setObjectName(QStringLiteral("nanquimIconButton"));
    hl->addWidget(more);
    QPushButton *newPage = new QPushButton(i18n("Nova página"), header);
    newPage->setObjectName(QStringLiteral("nanquimPrimary"));
    newPage->setFocusPolicy(Qt::NoFocus);
    hl->addWidget(newPage);
    homeLayout->addWidget(header);

    QStackedWidget *homeSections = new QStackedWidget(home);
    QWidget *pagesSection = new QWidget(homeSections);
    QVBoxLayout *sectionLayout = new QVBoxLayout(pagesSection);
    sectionLayout->setContentsMargins(0, 0, 0, 0);
    sectionLayout->setSpacing(0);
    homeSections->addWidget(pagesSection);

    // --- abas: Tudo, capítulos, páginas soltas; filtros de etapa e Selecionar ---
    QWidget *tabsRow = new QWidget(home);
    QHBoxLayout *trl = new QHBoxLayout(tabsRow);
    trl->setContentsMargins(32, 12, 32, 6);
    trl->setSpacing(10);
    QScrollArea *tabsScroll = new QScrollArea(tabsRow);
    tabsScroll->setObjectName(QStringLiteral("nanquimHomeScroll"));
    tabsScroll->setFrameShape(QFrame::NoFrame);
    tabsScroll->setWidgetResizable(true);
    tabsScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    tabsScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    tabsScroll->setFixedHeight(48);
    QWidget *tabsHost = new QWidget();
    tabsHost->setObjectName(QStringLiteral("nanquimHomeBody"));
    tabsHost->setAttribute(Qt::WA_StyledBackground);
    QHBoxLayout *tabsLayout = new QHBoxLayout(tabsHost);
    tabsLayout->setContentsMargins(0, 0, 0, 0);
    tabsLayout->setSpacing(8);
    tabsScroll->setWidget(tabsHost);
    trl->addWidget(tabsScroll, 1);
    QButtonGroup *tabGroup = new QButtonGroup(tabsRow);
    tabGroup->setExclusive(true);

    QList<QPushButton*> stageChips;
    const QStringList stageLabels = {i18n("Rascunhos"), i18n("Arte-final"), i18n("Prontas")};
    for (int i = 0; i < 3; i++) {
        QPushButton *chip = new QPushButton(stageLabels[i], tabsRow);
        QPixmap dot(12, 12);
        dot.fill(Qt::transparent);
        QPainter dp(&dot);
        dp.setRenderHint(QPainter::Antialiasing);
        dp.setPen(Qt::NoPen);
        dp.setBrush(stageColor(i));
        dp.drawEllipse(QRectF(1, 1, 10, 10));
        dp.end();
        chip->setIcon(QIcon(dot));
        chip->setIconSize(QSize(12, 12));
        chip->setProperty("class", QStringLiteral("nanquimChip"));
        chip->setCheckable(true);
        chip->setFocusPolicy(Qt::NoFocus);
        trl->addWidget(chip);
        stageChips << chip;
    }
    QPushButton *selectMode = galleryButton(i18n("Selecionar"), nullptr, tabsRow);
    selectMode->setCheckable(true);
    trl->addWidget(selectMode);
    sectionLayout->addWidget(tabsRow);

    // --- faixa do capítulo escolhido nas abas, ou das páginas selecionadas -------
    QWidget *chapterBar = new QWidget(home);
    QHBoxLayout *cbarl = new QHBoxLayout(chapterBar);
    cbarl->setContentsMargins(32, 2, 32, 4);
    cbarl->setSpacing(10);
    ChapterProgress *homeProgress = new ChapterProgress(chapterBar);
    cbarl->addWidget(homeProgress, 1);
    QPushButton *chapterAdd = galleryButton(i18n("+ Página no capítulo"), nullptr, chapterBar);
    QPushButton *chapterOpen = galleryButton(i18n("Abrir capítulo"), nullptr, chapterBar);
    cbarl->addWidget(chapterAdd);
    cbarl->addWidget(chapterOpen);
    chapterBar->hide();
    sectionLayout->addWidget(chapterBar);

    QWidget *selectBar = new QWidget(home);
    QHBoxLayout *sbarl = new QHBoxLayout(selectBar);
    sbarl->setContentsMargins(32, 2, 32, 4);
    sbarl->setSpacing(10);
    QLabel *selectInfo = new QLabel(selectBar);
    selectInfo->setObjectName(QStringLiteral("nanquimHint"));
    sbarl->addWidget(selectInfo, 1);
    QPushButton *selToChapter = galleryButton(i18n("Pôr no capítulo…"), nullptr, selectBar);
    QPushButton *selRemove = galleryButton(i18n("Tirar da lista"), nullptr, selectBar);
    sbarl->addWidget(selToChapter);
    sbarl->addWidget(selRemove);
    selectBar->hide();
    sectionLayout->addWidget(selectBar);

    // --- grade ------------------------------------------------------------------
    QStackedWidget *gridStack = new QStackedWidget(home);
    RecentsFilter *proxy = new RecentsFilter(gallery);
    proxy->setSourceModel(&KisRecentDocumentsModelWrapper::instance()->model());
    proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    QObject::connect(search, &QLineEdit::textChanged, proxy, &QSortFilterProxyModel::setFilterFixedString);

    QListView *grid = new QListView(gridStack);
    grid->setObjectName(QStringLiteral("nanquimRecents"));
    grid->setItemDelegate(new PageTileDelegate(grid));
    grid->setViewMode(QListView::IconMode);
    grid->setResizeMode(QListView::Adjust);
    grid->setMovement(QListView::Static);
    grid->setUniformItemSizes(true);
    grid->setGridSize(PageTileDelegate::cellFor(1200));
    grid->setSelectionMode(QAbstractItemView::NoSelection);
    grid->setEditTriggers(QAbstractItemView::NoEditTriggers);
    grid->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    grid->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);   // a largura não muda ao rolar
    grid->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    grid->setFocusPolicy(Qt::NoFocus);
    grid->setMouseTracking(false);
    grid->setModel(proxy);
    grid->setProperty("nanquimSmooth", true);   // ajustada aqui, não pelo filtro geral
    grid->setProperty("nanquimHoverCards", true);
    grid->setProperty("nanquimCardBottomPad", PageTileDelegate::TEXT_H + 8);
    tuneSmoothScroller(KisKineticScroller::createPreconfiguredScroller(grid));
    QWidget *gridWrap = new QWidget(gridStack);
    QVBoxLayout *gwl = new QVBoxLayout(gridWrap);
    gwl->setContentsMargins(24, 0, 24, 0);
    gwl->setSpacing(0);
    gwl->addWidget(grid);
    auto fitGrid = [grid]() {
        const QSize cell = PageTileDelegate::cellFor(qMax(300, grid->viewport()->width() - 2));
        if (grid->gridSize() != cell) grid->setGridSize(cell);
    };
    grid->installEventFilter(new EventFn(grid, [fitGrid](QObject *, QEvent *e) {
        if (e->type() == QEvent::Resize || e->type() == QEvent::Show) fitGrid();
        return false;
    }));

    QWidget *emptyPage = new QWidget(gridStack);
    QVBoxLayout *epl = new QVBoxLayout(emptyPage);
    epl->setContentsMargins(32, 24, 32, 24);
    epl->setSpacing(10);
    QLabel *empty = new QLabel(emptyPage);
    empty->setObjectName(QStringLiteral("nanquimEmpty"));
    empty->setAlignment(Qt::AlignCenter);
    empty->setWordWrap(true);
    epl->addWidget(empty, 1);
    QLabel *footer = new QLabel(i18n("O Nanquim é feito sobre o Krita, software livre da comunidade KDE."), emptyPage);
    footer->setObjectName(QStringLiteral("nanquimFooter"));
    footer->setAlignment(Qt::AlignCenter);
    epl->addWidget(footer);
    gridStack->addWidget(gridWrap);
    gridStack->addWidget(emptyPage);
    // à esquerda, a última página grande ("Continuar de onde parou"); a grade ao lado
    QWidget *homeBody = new QWidget(home);
    QHBoxLayout *hbl = new QHBoxLayout(homeBody);
    hbl->setContentsMargins(32, 8, 0, 0);
    hbl->setSpacing(8);
    ContinueCard *continueCard = new ContinueCard(homeBody);
    continueCard->setObjectName(QStringLiteral("nanquimContinueCard"));
    continueCard->setFixedWidth(380);
    continueCard->hide();
    hbl->addWidget(continueCard, 0);
    hbl->addWidget(gridStack, 1);
    sectionLayout->addWidget(homeBody, 1);
    homeLayout->addWidget(homeSections, 1);
    // a Comunidade só é montada no primeiro toque (não baixa nada ao abrir o app)
    QObject::connect(sectionGroup, &QButtonGroup::idClicked, home, [mw, homeSections, pagesSection, search](int id) {
        if (id == 1) {
            QWidget *community = homeSections->findChild<QWidget*>(QStringLiteral("nanquimCommunityPage"));
            if (!community) {
                community = buildCommunityPage(mw, homeSections);
                homeSections->addWidget(community);
            }
            homeSections->setCurrentWidget(community);
        } else {
            homeSections->setCurrentWidget(pagesSection);
        }
        search->setVisible(id == 0);
    });
    // em pé ou em tela estreita a grade precisa do espaço: o cartão sai
    homeBody->installEventFilter(new EventFn(homeBody, [homeBody, continueCard](QObject *, QEvent *e) {
        if (e->type() == QEvent::Resize) {
            continueCard->setVisible(continueCard->property("nanquimHas").toBool() && homeBody->width() >= 900);
        }
        return false;
    }));

    // ===================== capítulo aberto =====================
    QWidget *chapterPage = new QWidget(pages);
    chapterPage->setObjectName(QStringLiteral("nanquimChapterPage"));
    chapterPage->setAttribute(Qt::WA_StyledBackground);
    QVBoxLayout *cv = new QVBoxLayout(chapterPage);
    cv->setContentsMargins(0, 0, 0, 0);
    cv->setSpacing(0);
    QWidget *chHeader = new QWidget(chapterPage);
    chHeader->setObjectName(QStringLiteral("nanquimGalleryHeader"));
    chHeader->setAttribute(Qt::WA_StyledBackground);
    chHeader->setFixedHeight(100);
    QHBoxLayout *chl = new QHBoxLayout(chHeader);
    chl->setContentsMargins(20, 0, 24, 0);
    chl->setSpacing(10);
    QPushButton *chBack = galleryButton(QString(), Svg::BACK, chHeader);
    chBack->setObjectName(QStringLiteral("nanquimFlatIcon"));
    chBack->setToolTip(i18n("Voltar para a tela inicial"));
    chl->addWidget(chBack);
    QVBoxLayout *titles = new QVBoxLayout();
    titles->setSpacing(2);
    QLabel *kicker = new QLabel(i18n("CAPÍTULO"), chHeader);
    kicker->setObjectName(QStringLiteral("nanquimKicker"));
    QLabel *chTitle = new QLabel(chHeader);
    chTitle->setObjectName(QStringLiteral("nanquimChapterTitle"));
    QLabel *chInfo = new QLabel(chHeader);
    chInfo->setObjectName(QStringLiteral("nanquimChapterInfo"));
    titles->addWidget(kicker);
    titles->addWidget(chTitle);
    titles->addWidget(chInfo);
    chl->addLayout(titles, 1);
    // grade ou duplas (como o leitor vê)
    QButtonGroup *viewGroup = new QButtonGroup(chHeader);
    QPushButton *viewGrid = new QPushButton(i18n("Grade"), chHeader);
    QPushButton *viewSpreads = new QPushButton(i18n("Duplas"), chHeader);
    int viewId = 0;
    for (QPushButton *b : {viewGrid, viewSpreads}) {
        b->setProperty("class", QStringLiteral("nanquimChip"));
        b->setCheckable(true);
        b->setFocusPolicy(Qt::NoFocus);
        viewGroup->addButton(b, viewId++);
        chl->addWidget(b);
    }
    viewGrid->setChecked(true);
    chl->addSpacing(8);
    QPushButton *stageMode = galleryButton(i18n("Marcar etapas"), nullptr, chHeader);
    stageMode->setCheckable(true);
    QPushButton *orderMode = galleryButton(i18n("Organizar páginas"), nullptr, chHeader);
    orderMode->setCheckable(true);
    QPushButton *addPages = galleryButton(i18n("Adicionar páginas"), nullptr, chHeader);
    addPages->setCheckable(true);
    QPushButton *chapterRemove = galleryButton(i18n("Concluir"), nullptr, chHeader);
    chapterRemove->setCheckable(true);
    QPushButton *chMore = galleryButton(QString(), Svg::MORE, chHeader);
    chMore->setObjectName(QStringLiteral("nanquimIconButton"));
    chMore->setToolTip(i18n("Mais opções do capítulo"));
    QPushButton *chNewPage = new QPushButton(i18n("Nova página"), chHeader);
    chNewPage->setObjectName(QStringLiteral("nanquimPrimary"));
    chNewPage->setFocusPolicy(Qt::NoFocus);
    for (QWidget *w : {static_cast<QWidget*>(stageMode), static_cast<QWidget*>(orderMode), static_cast<QWidget*>(addPages),
                       static_cast<QWidget*>(chapterRemove), static_cast<QWidget*>(chMore), static_cast<QWidget*>(chNewPage)}) {
        chl->addWidget(w);
    }
    cv->addWidget(chHeader);

    QWidget *chBody = new QWidget(chapterPage);
    QVBoxLayout *cbl = new QVBoxLayout(chBody);
    cbl->setContentsMargins(32, 16, 32, 10);
    cbl->setSpacing(10);
    QHBoxLayout *progressRow = new QHBoxLayout();
    progressRow->setSpacing(12);
    ChapterProgress *progress = new ChapterProgress(chBody);
    progressRow->addWidget(progress, 1);
    QPushButton *mangaSwitch = new QPushButton(i18n("Leitura de mangá"), chBody);
    mangaSwitch->setProperty("class", QStringLiteral("nanquimChip"));
    mangaSwitch->setCheckable(true);
    mangaSwitch->setFocusPolicy(Qt::NoFocus);
    mangaSwitch->setToolTip(i18n("Duplas da direita para a esquerda, com a página 1 sozinha à esquerda"));
    progressRow->addWidget(mangaSwitch);
    cbl->addLayout(progressRow);
    QLabel *chHint = new QLabel(chBody);
    chHint->setObjectName(QStringLiteral("nanquimHint"));
    chHint->hide();
    cbl->addWidget(chHint);
    auto makeGrid = [](QWidget *parent) {
        QListView *v = new QListView(parent);
        v->setObjectName(QStringLiteral("nanquimRecents"));
        v->setItemDelegate(new RecentDelegate(v));
        v->setViewMode(QListView::IconMode);
        v->setResizeMode(QListView::Adjust);
        v->setMovement(QListView::Static);
        v->setUniformItemSizes(true);
        v->setGridSize(RecentDelegate::cell());
        v->setSelectionMode(QAbstractItemView::NoSelection);
        v->setEditTriggers(QAbstractItemView::NoEditTriggers);
        v->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        v->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
        v->setFocusPolicy(Qt::NoFocus);
        v->setMouseTracking(false);
        KisKineticScroller::createPreconfiguredScroller(v);
        return v;
    };
    QStandardItemModel *pagesModel = new QStandardItemModel(gallery);
    QListView *pagesView = makeGrid(chBody);
    pagesView->setProperty("nanquimChapterView", true);
    pagesView->setProperty("nanquimHoverCards", true);
    pagesView->setProperty("nanquimCardBottomPad", 58);
    pagesView->setModel(pagesModel);

    // duplas: ícones e etapas lidos do modelo das páginas do capítulo
    QStandardItemModel *spreadsModel = new QStandardItemModel(gallery);
    auto iconOf = [pagesModel](const QString &path) {
        for (int i = 0; i < pagesModel->rowCount(); i++) {
            if (pagesModel->item(i)->data(Qt::ToolTipRole).toString() == path)
                return pagesModel->item(i)->data(Qt::DecorationRole).value<QIcon>();
        }
        return QIcon();
    };
    auto stageOf = [pagesModel](const QString &path) {
        for (int i = 0; i < pagesModel->rowCount(); i++) {
            if (pagesModel->item(i)->data(Qt::ToolTipRole).toString() == path)
                return pagesModel->item(i)->data(Qt::UserRole + 60).toInt();
        }
        return -1;
    };
    QListView *spreadsView = new QListView(chBody);
    spreadsView->setObjectName(QStringLiteral("nanquimRecents"));
    spreadsView->setItemDelegate(new SpreadDelegate(spreadsView, iconOf, stageOf));
    spreadsView->setViewMode(QListView::IconMode);
    spreadsView->setResizeMode(QListView::Adjust);
    spreadsView->setMovement(QListView::Static);
    spreadsView->setUniformItemSizes(true);
    spreadsView->setSelectionMode(QAbstractItemView::NoSelection);
    spreadsView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    spreadsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    spreadsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    spreadsView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    spreadsView->setFocusPolicy(Qt::NoFocus);
    spreadsView->setModel(spreadsModel);
    spreadsView->setProperty("nanquimSmooth", true);
    spreadsView->setProperty("nanquimHoverCards", true);
    spreadsView->setProperty("nanquimCardBottomPad", 38);
    tuneSmoothScroller(KisKineticScroller::createPreconfiguredScroller(spreadsView));
    spreadsView->hide();

    QLabel *chEmpty = new QLabel(chBody);
    chEmpty->setObjectName(QStringLiteral("nanquimEmpty"));
    chEmpty->setAlignment(Qt::AlignCenter);
    chEmpty->setWordWrap(true);
    chEmpty->setText(i18n("<b style='color:#eeeae0; font-size:16px'>Capítulo vazio</b><br><br>"
                          "Toque em <b style='color:#6fbfad'>+ Nova página</b> ou em \"Adicionar páginas\" "
                          "para trazer desenhos que você já tem."));
    cbl->addWidget(pagesView, 1);
    cbl->addWidget(spreadsView, 1);
    cbl->addWidget(chEmpty, 1);
    cv->addWidget(chBody, 1);

    pages->addWidget(home);
    pages->addWidget(chapterPage);
    pages->setCurrentWidget(home);

    // --- estado ----------------------------------------------------------------
    struct GalleryState {
        QString chapter;        // capítulo aberto (tela do capítulo)
        QString homeChapter;    // aba da tela inicial: "" tudo, LOOSE_ID soltas
        bool addMode = false;
        bool animating = false;
        QSet<QString> selected;
    };
    auto state = std::make_shared<GalleryState>();
    QPointer<QWidget> galleryGuard(gallery);

    auto chapterAspect = [state]() {
        const PagePreset &f = CHAPTER_FORMATS[ChapterStore::instance()->format(state->chapter)];
        return f.height > 0 ? double(f.width) / f.height : 0.7;
    };
    auto fitSpreads = [spreadsView, chapterAspect]() {
        const QSize cell = SpreadDelegate::cellFor(qMax(300, spreadsView->viewport()->width() - 2), chapterAspect());
        if (spreadsView->gridSize() != cell) spreadsView->setGridSize(cell);
    };
    spreadsView->installEventFilter(new EventFn(spreadsView, [fitSpreads](QObject *, QEvent *e) {
        if (e->type() == QEvent::Resize || e->type() == QEvent::Show) fitSpreads();
        return false;
    }));

    // --- dados do capítulo aberto -----------------------------------------------
    auto rebuildPages = [state, pagesModel]() {
        pagesModel->clear();
        const ChapterStore::Chapter c = ChapterStore::instance()->chapter(state->chapter);
        for (int i = 0; i < c.pages.size(); i++) {
            const QString path = c.pages[i];
            QStandardItem *item = new QStandardItem(displayNameFor(path));
            item->setData(path, Qt::ToolTipRole);
            item->setData(ChapterStore::instance()->status(state->chapter, path), Qt::UserRole + 60);
            const QIcon icon = KisRecentFileIconCache::instance()->getOrQueueFileIcon(urlForPath(path));
            if (!icon.isNull()) item->setData(icon, Qt::DecorationRole);
            pagesModel->appendRow(item);
        }
    };
    // páginas em dupla. Ocidental: a 1 sozinha à direita, depois 2|3, 4|5...
    // Mangá: a 1 sozinha à esquerda, depois 3|2, 5|4...
    auto rebuildSpreads = [state, spreadsModel]() {
        spreadsModel->clear();
        const ChapterStore::Chapter c = ChapterStore::instance()->chapter(state->chapter);
        if (c.pages.isEmpty()) return;
        const bool rtl = ChapterStore::instance()->rtl(state->chapter);
        auto add = [spreadsModel, rtl](const QString &label, int a, const QString &pa, int b, const QString &pb) {
            // a e pa: a página que vem antes na leitura
            QStandardItem *item = new QStandardItem(label);
            const QStringList paths = rtl ? QStringList{pb, pa} : QStringList{pa, pb};
            const QVariantList nums = rtl ? QVariantList{b, a} : QVariantList{a, b};
            item->setData(paths, ROLE_SPREAD_PATHS);
            item->setData(nums, ROLE_SPREAD_NUMS);
            spreadsModel->appendRow(item);
        };
        // a primeira página fica sozinha, no lado em que a leitura começa
        {
            QStandardItem *item = new QStandardItem(i18n("Pág. 1"));
            item->setData(rtl ? QStringList{c.pages[0], QString()} : QStringList{QString(), c.pages[0]}, ROLE_SPREAD_PATHS);
            item->setData(rtl ? QVariantList{1, 0} : QVariantList{0, 1}, ROLE_SPREAD_NUMS);
            spreadsModel->appendRow(item);
        }
        for (int i = 1; i < c.pages.size(); i += 2) {
            const bool pair = i + 1 < c.pages.size();
            add(pair ? i18n("Págs. %1 e %2", i + 1, i + 2) : i18n("Pág. %1", i + 1),
                i + 1, c.pages[i], i + 2, pair ? c.pages[i + 1] : QString());
        }
    };
    QObject::connect(KisRecentFileIconCache::instance(), &KisRecentFileIconCache::fileIconChanged, pagesModel,
                     [pagesModel, spreadsView](const QUrl &url, const QIcon &icon) {
        bool hit = false;
        for (int i = 0; i < pagesModel->rowCount(); i++) {
            QStandardItem *item = pagesModel->item(i);
            if (urlForPath(item->data(Qt::ToolTipRole).toString()) == url) {
                item->setData(icon, Qt::DecorationRole);
                hit = true;
            }
        }
        if (hit && spreadsView->isVisible()) spreadsView->viewport()->update();
    });

    // --- abas da tela inicial ----------------------------------------------------
    auto continuePath = std::make_shared<QString>();
    auto rebuildTabs = [state, tabsLayout, tabsHost, tabGroup]() {
        // apaga as abas antigas (botões e o "+ Capítulo")
        while (QLayoutItem *it = tabsLayout->takeAt(0)) {
            if (QWidget *w = it->widget()) {
                tabGroup->removeButton(qobject_cast<QAbstractButton*>(w));
                w->deleteLater();
            }
            delete it;
        }
        auto addTab = [tabsLayout, tabsHost, tabGroup, state](const QString &text, const QString &id) {
            QPushButton *chip = new QPushButton(text, tabsHost);
            chip->setProperty("class", QStringLiteral("nanquimChip"));
            chip->setProperty("nanquimChapterId", id);
            chip->setCheckable(true);
            chip->setFocusPolicy(Qt::NoFocus);
            chip->setChecked(id == state->homeChapter);
            tabGroup->addButton(chip);
            tabsLayout->addWidget(chip);
        };
        addTab(i18n("Tudo"), QString());
        bool found = state->homeChapter.isEmpty() || state->homeChapter == LOOSE_ID;
        Q_FOREACH (const ChapterStore::Chapter &c, ChapterStore::instance()->chapters()) {
            int ready = 0;
            Q_FOREACH (const QString &p, c.pages) if (ChapterStore::instance()->info(p).stage == 2) ready++;
            addTab(c.pages.isEmpty() ? c.name : QStringLiteral("%1  %2/%3").arg(c.name).arg(ready).arg(c.pages.size()), c.id);
            if (c.id == state->homeChapter) found = true;
        }
        addTab(i18n("Páginas soltas"), LOOSE_ID);
        QPushButton *newChapter = new QPushButton(i18n("+ Capítulo"), tabsHost);
        newChapter->setObjectName(QStringLiteral("nanquimLink"));
        newChapter->setFocusPolicy(Qt::NoFocus);
        newChapter->setProperty("nanquimNewChapter", true);
        tabsLayout->addWidget(newChapter);
        tabsLayout->addStretch(1);
        if (!found) {   // capítulo da aba foi apagado
            state->homeChapter.clear();
            if (QAbstractButton *first = tabGroup->buttons().value(0)) first->setChecked(true);
        }
        return newChapter;
    };

    auto refreshHome = [=]() {
        s_subtitleCache.clear();
        // Continuar: o desenho aberto ou, sem nenhum aberto, o mais recente
        QStandardItemModel &model = KisRecentDocumentsModelWrapper::instance()->model();
        QString path;
        QString name;
        KisView *view = mw->activeView();
        if (view && view->document()) {
            path = view->document()->localFilePath();
            name = path.isEmpty() ? i18n("página nova") : displayNameFor(path);
        }
        if (!view && model.rowCount() > 0) {
            path = model.index(0, 0).data(Qt::ToolTipRole).toString();
            name = displayNameFor(path);
        }
        if (name.endsWith(QLatin1String(".kra"), Qt::CaseInsensitive)) name.chop(4);
        *continuePath = path;
        proxy->setContinuePath(path);
        // o cartão grande faz o papel do "Continuar" do cabeçalho
        contGo->setVisible(false);
        {
            const bool has = view || !path.isEmpty();
            continueCard->setProperty("nanquimHas", has);
            if (has) {
                QPixmap page;
                if (view && view->document()) {
                    page = view->document()->generatePreview(QSize(640, 900));
                } else {
                    for (int row = 0; row < model.rowCount(); ++row) {
                        const QModelIndex idx = model.index(row, 0);
                        if (idx.data(Qt::ToolTipRole).toString() == path) {
                            page = trimmedThumb(idx.data(Qt::DecorationRole).value<QIcon>());
                            break;
                        }
                    }
                }
                const ChapterStore::PageInfo info = ChapterStore::instance()->info(path);
                const QString sub = info.chapterId.isEmpty() ? i18n("Página solta")
                                                             : i18n("%1, página %2", info.chapterName, info.index + 1);
                continueCard->setContent(view ? i18n("Voltar ao desenho") : i18n("Continuar de onde parou"),
                                         name, sub, info.chapterId.isEmpty() ? -1 : info.stage, page);
            }
            continueCard->setVisible(has && homeBody->width() >= 900);
        }

        // grade ou tela vazia
        const int total = model.rowCount();
        const int rows = proxy->rowCount();
        if (rows > 0) {
            gridStack->setCurrentWidget(gridWrap);
        } else {
            gridStack->setCurrentWidget(emptyPage);
            footer->setVisible(total == 0);
            if (total == 0) {
                empty->setText(i18n("<b style='color:#eeeae0; font-size:17px'>Nenhuma página ainda</b><br><br>"
                                    "Toque em <b style='color:#6fbfad'>+ Nova página</b> para escolher o formato, "
                                    "ou em <b style='color:#6fbfad'>Abrir</b> para trazer um arquivo do tablet."));
            } else if (!search->text().isEmpty()) {
                empty->setText(i18n("Nenhuma página com \"%1\" no nome.", search->text().toHtmlEscaped()));
            } else if (proxy->stage() >= 0) {
                empty->setText(i18n("Nenhuma página nesta etapa. As etapas são marcadas dentro dos capítulos."));
            } else if (state->homeChapter == LOOSE_ID) {
                empty->setText(i18n("Todas as páginas recentes estão em capítulos."));
            } else {
                empty->setText(i18n("<b style='color:#eeeae0; font-size:16px'>Nenhuma página deste capítulo nos recentes</b><br><br>"
                                    "Toque em <b style='color:#6fbfad'>Abrir capítulo</b> para ver todas, "
                                    "ou em <b style='color:#6fbfad'>+ Página no capítulo</b>."));
            }
        }

        // faixa do capítulo da aba
        const bool selecting = selectMode->isChecked();
        const ChapterStore::Chapter hc = (state->homeChapter.isEmpty() || state->homeChapter == LOOSE_ID)
                ? ChapterStore::Chapter() : ChapterStore::instance()->chapter(state->homeChapter);
        chapterBar->setVisible(!selecting && !hc.id.isEmpty());
        if (!hc.id.isEmpty()) {
            int counts[3] = {0, 0, 0};
            Q_FOREACH (const QString &p, hc.pages) counts[ChapterStore::instance()->status(hc.id, p)]++;
            homeProgress->setCounts(counts[2], counts[1], counts[0]);
        }
        // faixa da seleção
        selectBar->setVisible(selecting);
        const int n = state->selected.size();
        selectInfo->setText(n == 0 ? i18n("Toque nas páginas para marcar")
                                   : (n == 1 ? i18n("1 página marcada") : i18n("%1 páginas marcadas", n)));
        selToChapter->setEnabled(n > 0);
        selRemove->setEnabled(n > 0);
        selectMode->setText(selecting ? i18n("Concluir") : i18n("Selecionar"));
        grid->setProperty("nanquimSelectMode", selecting);
        grid->viewport()->update();
    };
    QObject::connect(KisRecentFileIconCache::instance(), &KisRecentFileIconCache::fileIconChanged, grid,
                     [grid](const QUrl &, const QIcon &) { grid->viewport()->update(); });

    auto refreshing = std::make_shared<bool>(false);
    auto refreshChapter = [=]() {
        if (*refreshing) return;
        *refreshing = true;
        struct Unlock { std::shared_ptr<bool> f; ~Unlock() { *f = false; } } unlock{refreshing};
        const ChapterStore::Chapter c = ChapterStore::instance()->chapter(state->chapter);
        if (c.id.isEmpty()) {   // capítulo apagado
            state->chapter.clear();
            state->addMode = false;
            pages->setCurrentWidget(home);
            return;
        }
        s_pendingChapter = c.id;
        rebuildPages();
        rebuildSpreads();
        int counts[3] = {0, 0, 0};
        Q_FOREACH (const QString &p, c.pages) counts[ChapterStore::instance()->status(c.id, p)]++;
        chTitle->setText(c.name);
        chInfo->setText(c.pages.size() == 1 ? i18n("1 página · %1", i18n(CHAPTER_FORMATS[ChapterStore::instance()->format(c.id)].name))
                                            : i18n("%1 páginas · %2 prontas · %3", c.pages.size(), counts[2],
                                                   i18n(CHAPTER_FORMATS[ChapterStore::instance()->format(c.id)].name)));
        progress->setCounts(counts[2], counts[1], counts[0]);
        progress->setVisible(!c.pages.isEmpty() && !state->addMode);

        const bool marking = stageMode->isChecked();
        const bool ordering = orderMode->isChecked();
        const bool removing = chapterRemove->isChecked();
        const bool adding = state->addMode;
        const bool busy = marking || ordering || removing || adding;
        // webtoon é uma tira só: não tem dupla
        const bool canSpread = chapterAspect() > 0.3;
        const bool spreads = canSpread && viewSpreads->isChecked() && !busy;
        viewGrid->setVisible(canSpread && !busy);
        viewSpreads->setVisible(canSpread && !busy);
        stageMode->setVisible(!spreads && (!busy || marking));
        orderMode->setVisible(!spreads && (!busy || ordering));
        addPages->setVisible(!spreads && (!busy || adding));
        chapterRemove->setVisible(removing);
        chMore->setVisible(!busy);
        chNewPage->setVisible(!busy);
        stageMode->setText(marking ? i18n("Concluir") : i18n("Marcar etapas"));
        orderMode->setText(ordering ? i18n("Concluir") : i18n("Organizar páginas"));
        addPages->setText(adding ? i18n("Concluir") : i18n("Adicionar páginas"));
        addPages->setChecked(adding);
        {
            QSignalBlocker block(mangaSwitch);
            mangaSwitch->setChecked(ChapterStore::instance()->rtl(c.id));
        }
        mangaSwitch->setVisible(spreads);

        pagesView->setModel(adding ? static_cast<QAbstractItemModel*>(proxy) : static_cast<QAbstractItemModel*>(pagesModel));
        pagesView->setProperty("nanquimChapterView", !adding);
        pagesView->setProperty("nanquimAddMode", adding);
        pagesView->setProperty("nanquimOrderMode", ordering);
        pagesView->setProperty("nanquimRemoveMode", removing);
        pagesView->viewport()->update();
        chHint->setVisible(busy);
        chHint->setText(marking ? i18n("Toque numa página para passar de rascunho para arte-final e para pronta")
                        : ordering ? i18n("Toque na seta da esquerda ou da direita para mudar a página de lugar")
                        : adding ? i18n("Toque nos desenhos para pôr neste capítulo")
                        : i18n("Toque nas páginas que quer tirar do capítulo (os arquivos não são apagados)"));
        const int rows = pagesView->model()->rowCount();
        chEmpty->setVisible(rows == 0);
        pagesView->setVisible(rows > 0 && !spreads);
        spreadsView->setVisible(rows > 0 && spreads);
        if (spreads) fitSpreads();
    };

    auto resetChapterModes = [state, stageMode, orderMode, chapterRemove]() {
        state->addMode = false;
        for (QPushButton *b : {stageMode, orderMode, chapterRemove}) {
            QSignalBlocker block(b);
            b->setChecked(false);
        }
    };

    // "Adicionar páginas" mostra todas as páginas recentes, sem os filtros da aba;
    // ao terminar, a tela inicial volta com a aba e a etapa que estavam escolhidas
    auto homeFilters = [state, proxy, stageChips](bool restore) {
        int stage = -1;
        for (int i = 0; i < stageChips.size(); i++) if (stageChips[i]->isChecked()) stage = i;
        proxy->setStage(restore ? stage : -1);
        proxy->setChapter(restore ? state->homeChapter : QString());
    };

    auto openChapter = [=](const QString &id, QWidget *origin) {
        if (state->animating) return;
        state->chapter = id;
        resetChapterModes();
        refreshChapter();
        state->animating = true;
        runChapterTransition(pages, home, chapterPage, origin, spreadsView->isHidden() ? pagesView : spreadsView, true);
        QTimer::singleShot(450, gallery, [state]() { state->animating = false; });
    };
    auto closeChapter = [=]() {
        if (state->animating || pages->currentWidget() != chapterPage) return;
        if (state->addMode) homeFilters(true);
        resetChapterModes();
        refreshChapter();
        refreshHome();
        state->animating = true;
        runChapterTransition(pages, home, chapterPage, chapterOpen, spreadsView->isHidden() ? pagesView : spreadsView, false);
        state->chapter.clear();
        s_pendingChapter.clear();
        QTimer::singleShot(370, gallery, [state]() { state->animating = false; });
    };

    // abas: a lista é refeita quando os capítulos mudam
    std::shared_ptr<std::function<void()>> wireTabs = std::make_shared<std::function<void()>>();
    *wireTabs = [=]() {
        QPushButton *newChapter = rebuildTabs();
        QObject::connect(newChapter, &QPushButton::clicked, mw, [mw, state, proxy, galleryGuard, wireTabs, refreshHome]() {
            showChapterSheet(mw, QString(), [state, proxy, galleryGuard, wireTabs, refreshHome](const QString &newId) {
                if (!galleryGuard || newId.isEmpty()) return;
                // espera a lista de capítulos se refazer e abre a aba nova
                QTimer::singleShot(30, galleryGuard, [state, proxy, wireTabs, refreshHome, newId]() {
                    state->homeChapter = newId;
                    proxy->setChapter(newId);
                    (*wireTabs)();
                    refreshHome();
                });
            });
        });
    };
    QObject::connect(tabGroup, QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked), mw,
                     [state, proxy, refreshHome, grid](QAbstractButton *b) {
        const QString id = b->property("nanquimChapterId").toString();
        if (id == state->homeChapter) return;
        state->homeChapter = id;
        proxy->setChapter(id);
        grid->scrollToTop();
        refreshHome();
    });
    for (int i = 0; i < stageChips.size(); i++) {
        QPushButton *chip = stageChips[i];
        QObject::connect(chip, &QPushButton::clicked, mw, [stageChips, i, proxy, refreshHome](bool on) {
            for (int j = 0; j < stageChips.size(); j++) {
                if (j != i) { QSignalBlocker block(stageChips[j]); stageChips[j]->setChecked(false); }
            }
            proxy->setStage(on ? i : -1);
            refreshHome();
        });
    }
    QObject::connect(chapterOpen, &QPushButton::clicked, mw, [state, openChapter, chapterOpen]() {
        if (!state->homeChapter.isEmpty() && state->homeChapter != LOOSE_ID) openChapter(state->homeChapter, chapterOpen);
    });
    QObject::connect(chapterAdd, &QPushButton::clicked, mw, [state]() {
        if (state->homeChapter.isEmpty() || state->homeChapter == LOOSE_ID) return;
        s_pendingChapter = state->homeChapter;
        createPage(CHAPTER_FORMATS[ChapterStore::instance()->format(state->homeChapter)]);
    });
    QObject::connect(newPage, &QPushButton::clicked, mw, [mw, state]() {
        // a janela de formatos já vem com o capítulo da aba escolhido
        s_pendingChapter = (state->homeChapter == LOOSE_ID) ? QString() : state->homeChapter;
        showNewPage(mw);
    });

    // seleção
    QObject::connect(selectMode, &QPushButton::toggled, mw, [state, proxy, refreshHome](bool) {
        state->selected.clear();
        proxy->setSelection(state->selected);
        refreshHome();
    });
    QObject::connect(selRemove, &QPushButton::clicked, mw, [mw, state, selectMode]() {
        const int n = state->selected.size();
        Q_FOREACH (const QString &p, state->selected) mw->removeRecentFile(p);   // os arquivos continuam no tablet
        selectMode->setChecked(false);
        showToast(mw, n == 1 ? i18n("1 página saiu da lista; o arquivo continua no tablet")
                             : i18n("%1 páginas saíram da lista; os arquivos continuam no tablet", n));
    });
    QMenu *toChapterMenu = new QMenu(selToChapter);
    toChapterMenu->setStyleSheet(QStringLiteral(
        "QMenu { background: #2e2d29; border: 1px solid #4a4842; border-radius: 12px; padding: 6px; }"
        "QMenu::item { color: #eeeae0; padding: 12px 22px; border-radius: 8px; font-size: 14px; }"
        "QMenu::item:selected { background: #4a4842; }"
        "QMenu::separator { height: 1px; background: #4a4842; margin: 6px 10px; }"));
    QObject::connect(selToChapter, &QPushButton::clicked, mw, [mw, state, selectMode, selToChapter, toChapterMenu, galleryGuard]() {
        toChapterMenu->clear();
        const QStringList paths = state->selected.values();
        auto putIn = [mw, paths, selectMode](const QString &id) {
            int added = 0;
            Q_FOREACH (const QString &p, paths) if (ChapterStore::instance()->addPage(id, p)) added++;
            selectMode->setChecked(false);
            showToast(mw, added == 1 ? i18n("1 página foi para %1", ChapterStore::instance()->chapter(id).name)
                                     : i18n("%1 páginas foram para %2", added, ChapterStore::instance()->chapter(id).name));
        };
        Q_FOREACH (const ChapterStore::Chapter &c, ChapterStore::instance()->chapters()) {
            const QString id = c.id;
            QObject::connect(toChapterMenu->addAction(c.name), &QAction::triggered, mw, [putIn, id]() { putIn(id); });
        }
        toChapterMenu->addSeparator();
        QObject::connect(toChapterMenu->addAction(i18n("Novo capítulo…")), &QAction::triggered, mw, [mw, putIn, galleryGuard]() {
            showChapterSheet(mw, QString(), [putIn, galleryGuard](const QString &newId) {
                if (!galleryGuard || newId.isEmpty()) return;
                QTimer::singleShot(30, galleryGuard, [putIn, newId]() { putIn(newId); });
            });
        });
        toChapterMenu->popup(selToChapter->mapToGlobal(QPoint(selToChapter->width() - toChapterMenu->sizeHint().width(),
                                                              selToChapter->height() + 6)));
    });

    QObject::connect(grid, &QListView::clicked, mw, [mw, grid, state, proxy, selectMode, refreshHome](const QModelIndex &index) {
        const QString path = index.data(Qt::ToolTipRole).toString();
        if (path.isEmpty()) return;
        if (selectMode->isChecked()) {
            if (state->selected.contains(path)) state->selected.remove(path);
            else state->selected.insert(path);
            proxy->setSelection(state->selected);
            refreshHome();
            return;
        }
        // a página aberta agora: só volta para ela
        if (KisView *view = mw->activeView()) {
            if (view->document() && view->document()->localFilePath() == path) {
                mw->showWelcomeScreen(false);
                return;
            }
        }
        s_pendingChapter.clear();
        {
            const QRect paperArea = PageTileDelegate::tileRect(grid->visualRect(index)).adjusted(12, 12, -12, -8);
            const QIcon deco = index.data(Qt::DecorationRole).value<QIcon>();
            QPixmap shot = trimmedThumb(deco);
            if (shot.isNull()) shot = cachedPaper(deco, paperArea.size(), 4);
            setOpenOrigin(grid->viewport(), paperArea, shot);
        }
        openPageWithFeedback(mw, path);
    });
    auto continueNow = [mw, continuePath, continueCard]() {
        if (mw->activeView()) {   // desenho aberto: só volta para ele
            mw->showWelcomeScreen(false);
            return;
        }
        if (continuePath->isEmpty()) return;
        s_pendingChapter.clear();
        setOpenOrigin(continueCard, continueCard->paperRect(), continueCard->page());
        openPageWithFeedback(mw, *continuePath);
    };
    QObject::connect(contGo, &QPushButton::clicked, mw, continueNow);
    QObject::connect(continueCard, &QAbstractButton::clicked, mw, continueNow);

    // voltar do Android: sai da seleção, limpa a busca, volta para a aba Tudo
    qApp->installEventFilter(new BackKeyFilter(gallery, [galleryGuard, pages, home, selectMode, search, state, proxy, tabGroup, refreshHome]() {
        if (!galleryGuard || !galleryGuard->isVisible() || pages->currentWidget() != home) return false;
        if (QApplication::activeModalWidget()) return false;
        if (selectMode->isChecked()) { selectMode->setChecked(false); return true; }
        if (!search->text().isEmpty()) { search->clear(); return true; }
        if (!state->homeChapter.isEmpty()) {
            state->homeChapter.clear();
            proxy->setChapter(QString());
            if (QAbstractButton *first = tabGroup->buttons().value(0)) first->setChecked(true);
            refreshHome();
            return true;
        }
        return false;
    }));

    // --- ações do capítulo aberto ------------------------------------------------
    QObject::connect(chBack, &QPushButton::clicked, mw, [closeChapter]() { closeChapter(); });
    qApp->installEventFilter(new BackKeyFilter(gallery, [galleryGuard, pages, chapterPage, closeChapter]() {
        if (!galleryGuard || !galleryGuard->isVisible() || pages->currentWidget() != chapterPage) return false;
        if (QApplication::activeModalWidget()) return false;
        closeChapter();
        return true;
    }));
    QObject::connect(viewGroup, QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked), mw,
                     [refreshChapter](QAbstractButton *) { refreshChapter(); });
    QObject::connect(mangaSwitch, &QPushButton::toggled, mw, [state](bool on) {
        if (!state->chapter.isEmpty()) ChapterStore::instance()->setRtl(state->chapter, on);
    });
    QObject::connect(stageMode, &QPushButton::toggled, mw, [refreshChapter](bool) { refreshChapter(); });
    QObject::connect(orderMode, &QPushButton::toggled, mw, [refreshChapter](bool) { refreshChapter(); });
    QObject::connect(chapterRemove, &QPushButton::toggled, mw, [refreshChapter](bool) { refreshChapter(); });
    QObject::connect(addPages, &QPushButton::clicked, mw, [state, refreshChapter, homeFilters]() {
        state->addMode = !state->addMode;
        homeFilters(!state->addMode);
        refreshChapter();
    });
    QObject::connect(chNewPage, &QPushButton::clicked, mw, [state]() {
        if (state->chapter.isEmpty()) return;
        s_pendingChapter = state->chapter;
        createPage(CHAPTER_FORMATS[ChapterStore::instance()->format(state->chapter)]);
    });

    QMenu *chMenu = new QMenu(chMore);
    chMenu->setStyleSheet(toChapterMenu->styleSheet());
    QAction *renameAction = chMenu->addAction(i18n("Renomear e formato…"));
    QAction *otherFormat = chMenu->addAction(i18n("Nova página em outro formato…"));
    chMenu->addSeparator();
    QAction *takeOut = chMenu->addAction(i18n("Tirar páginas do capítulo"));
    QAction *deleteAction = chMenu->addAction(i18n("Excluir capítulo"));
    QObject::connect(chMore, &QPushButton::clicked, mw, [chMore, chMenu]() {
        chMenu->popup(chMore->mapToGlobal(QPoint(chMore->width() - chMenu->sizeHint().width(), chMore->height() + 6)));
    });
    QObject::connect(renameAction, &QAction::triggered, mw, [mw, state]() {
        if (!state->chapter.isEmpty()) showChapterSheet(mw, state->chapter, [](const QString &) {});
    });
    QObject::connect(otherFormat, &QAction::triggered, mw, [mw, state]() {
        s_pendingChapter = state->chapter;
        showNewPage(mw);
    });
    QObject::connect(takeOut, &QAction::triggered, mw, [chapterRemove, viewGrid, refreshChapter]() {
        viewGrid->setChecked(true);
        chapterRemove->setChecked(true);
        refreshChapter();
    });
    QObject::connect(deleteAction, &QAction::triggered, mw, [mw, state, closeChapter]() {
        const ChapterStore::Chapter c = ChapterStore::instance()->chapter(state->chapter);
        if (QMessageBox::question(mw, i18n("Excluir capítulo"),
                                  i18n("Excluir o capítulo \"%1\"? Os desenhos continuam salvos no tablet e nos recentes.", c.name))
                == QMessageBox::Yes) {
            closeChapter();
            ChapterStore::instance()->remove(c.id);
        }
    });

    // lembra onde o dedo tocou, para saber qual seta (ou qual lado da dupla) foi tocado
    class PressSpy : public QObject {
    public:
        using QObject::QObject;
        bool eventFilter(QObject *obj, QEvent *e) override {
            if (e->type() == QEvent::MouseButtonRelease || e->type() == QEvent::MouseButtonPress) {
                obj->setProperty("nanquimLastPos", static_cast<QMouseEvent*>(e)->pos());
            }
            return false;
        }
    };
    pagesView->viewport()->installEventFilter(new PressSpy(pagesView));
    spreadsView->viewport()->installEventFilter(new PressSpy(spreadsView));

    QObject::connect(pagesView, &QListView::clicked, mw, [mw, state, stageMode, orderMode, chapterRemove, pagesView](const QModelIndex &index) {
        const QString path = index.data(Qt::ToolTipRole).toString();
        if (path.isEmpty() || state->chapter.isEmpty()) return;
        if (state->addMode) {
            ChapterStore::instance()->addPage(state->chapter, path);
            return;
        }
        if (stageMode->isChecked()) {
            const int s = ChapterStore::instance()->status(state->chapter, path);
            ChapterStore::instance()->setStatus(state->chapter, path, (s + 1) % 3);
            return;
        }
        if (orderMode->isChecked()) {
            const QPoint p = pagesView->viewport()->property("nanquimLastPos").toPoint();
            const QRect r = pagesView->visualRect(index);
            if (p.x() < r.center().x() - r.width() / 6) ChapterStore::instance()->movePage(state->chapter, path, -1);
            else if (p.x() > r.center().x() + r.width() / 6) ChapterStore::instance()->movePage(state->chapter, path, +1);
            return;
        }
        if (chapterRemove->isChecked()) {
            ChapterStore::instance()->removePage(state->chapter, path);
            return;
        }
        s_pendingChapter = state->chapter;
        setOpenOrigin(pagesView->viewport(), RecentDelegate::paperBox(pagesView->visualRect(index)),
                      trimmedThumb(index.data(Qt::DecorationRole).value<QIcon>()));
        openPageWithFeedback(mw, path);
    });
    QObject::connect(spreadsView, &QListView::clicked, mw, [mw, state, spreadsView](const QModelIndex &index) {
        const QStringList paths = index.data(ROLE_SPREAD_PATHS).toStringList();
        const QPoint p = spreadsView->viewport()->property("nanquimLastPos").toPoint();
        const int side = SpreadDelegate::sideAt(spreadsView->visualRect(index), p);
        const QString path = paths.value(side);
        if (path.isEmpty() || state->chapter.isEmpty()) return;
        s_pendingChapter = state->chapter;
        openPageWithFeedback(mw, path);
    });

    // --- avisos de mudança -------------------------------------------------------
    ChapterStore::instance()->listeners << [refreshHome, refreshChapter, pages, chapterPage, proxy, galleryGuard, wireTabs]() {
        if (!galleryGuard) return;
        QTimer::singleShot(0, galleryGuard, [refreshHome, refreshChapter, pages, chapterPage, proxy, wireTabs]() {
            (*wireTabs)();
            proxy->refilter();   // capítulo e etapa das páginas mudam
            refreshHome();
            if (pages->currentWidget() == chapterPage) refreshChapter();
        });
    };
    auto later = [refreshHome, refreshChapter, pages, chapterPage, state, galleryGuard]() {
        if (!galleryGuard) return;
        QTimer::singleShot(0, galleryGuard, [refreshHome, refreshChapter, pages, chapterPage, state]() {
            refreshHome();
            if (pages->currentWidget() == chapterPage && state->addMode) refreshChapter();
        });
    };
    QObject::connect(proxy, &QAbstractItemModel::rowsInserted, gallery, later);
    QObject::connect(proxy, &QAbstractItemModel::rowsRemoved, gallery, later);
    QObject::connect(proxy, &QAbstractItemModel::modelReset, gallery, later);
    QObject::connect(proxy, &QAbstractItemModel::layoutChanged, gallery, later);
    QObject::connect(KisPart::instance(), &KisPart::sigViewAdded, gallery, [later](KisView *) { later(); });
    QObject::connect(KisPart::instance(), &KisPart::sigViewRemoved, gallery, [later](KisView *) { later(); });
    (*wireTabs)();
    refreshHome();
    return gallery;
}

// troca a tela inicial do Krita pela galeria, no mesmo lugar da pilha
void installGallery(KisMainWindow *mw)
{
    QStackedWidget *stack = qobject_cast<QStackedWidget*>(mw->centralWidget());
    if (!stack || stack->count() < 2) {
        KisUsageLogger::log(QStringLiteral("[Nanquim] pilha de telas não encontrada; tela inicial do Krita mantida"));
        return;
    }
    const bool wasCurrent = stack->currentIndex() == 0;
    QWidget *old = stack->widget(0);
    QWidget *gallery = buildGallery(mw);
    stack->removeWidget(old);
    old->hide();
    stack->insertWidget(0, gallery);
    if (wasCurrent) {
        stack->setCurrentIndex(0);
    }
}


// ---------------------------------------------------------------------------
// Pincéis do Nanquim: uma etiqueta com pincéis leves (motor nativo do Krita),
// escolhidos para rascunho, arte-final e retícula. Criada uma vez; os pincéis
// MyPaint, mais pesados, continuam disponíveis em "Tudo".
// ---------------------------------------------------------------------------
void setupNanquimBrushes(KisMainWindow *mw)
{
    KConfigGroup cfg = KSharedConfig::openConfig()->group("Nanquim");
    if (cfg.readEntry("brushSetV1", false)) return;

    const QStringList files = {
        QStringLiteral("c)_Pencil-2.kpp"),            // rascunho
        QStringLiteral("c)_Pencil-1_Hard.kpp"),
        QStringLiteral("c)_Pencil-4_Soft.kpp"),
        QStringLiteral("c)_Pencil-6_Quick_Shade.kpp"),
        QStringLiteral("d)_Ink-1_Precision.kpp"),     // arte-final
        QStringLiteral("d)_Ink-2_Fineliner.kpp"),
        QStringLiteral("d)_Ink-3_Gpen.kpp"),
        QStringLiteral("d)_Ink-4_Pen_Rough.kpp"),
        QStringLiteral("d)_Ink-7_Brush_Rough.kpp"),
        QStringLiteral("e)_Marker_Details.kpp"),      // preto chapado e cinzas
        QStringLiteral("b)_Basic-5_Size_default.kpp"),
        QStringLiteral("b)_Airbrush_Soft.kpp"),
        QStringLiteral("y)_Screentone_Pressure.kpp"), // retícula
        QStringLiteral("y)_Screentones_Regular.kpp"),
        QStringLiteral("a)_Eraser_Circle.kpp"),       // borrachas
        QStringLiteral("a)_Eraser_Small.kpp"),
        QStringLiteral("a)_Eraser_Soft.kpp"),
    };
    KisPaintOpPresetResourceServer *server = KisResourceServerProvider::instance()->paintOpPresetServer();
    QVector<KoResourceSP> resources;
    KisPaintOpPresetSP pencil;
    Q_FOREACH (const QString &file, files) {
        KisPaintOpPresetSP preset = server->resource(QString(), file, QString());
        if (preset) {
            resources << preset;
            if (!pencil) pencil = preset;
        }
    }
    if (resources.isEmpty()) {
        KisUsageLogger::log(QStringLiteral("[Nanquim] pincéis padrão não encontrados; etiqueta não criada"));
        return;
    }
    KisTagModel tags(ResourceType::PaintOpPresets);
    KisTagSP tag = tags.addTag(QStringLiteral("Nanquim"), true, resources);
    if (tag) {
        // a janela de pincéis abre nessa etiqueta; "Tudo" continua a um toque
        KSharedConfig::openConfig()->group("SelectedTags").writeEntry(ResourceType::PaintOpPresets, tag->url());
        if (pencil && mw->viewManager() && mw->viewManager()->canvasResourceProvider()) {
            mw->viewManager()->canvasResourceProvider()->setPaintOpPreset(pencil);
        }
        cfg.writeEntry("brushSetV1", true);
        KisUsageLogger::log(QStringLiteral("[Nanquim] etiqueta de pincéis criada com %1 pincéis").arg(resources.size()));
    }
}

// Favoritos começam com a coluna do protótipo (Caneta G, Maru, Nanquim...), uma vez.
// Depois, quem manda é a estrela da janela de pincéis.
void seedNanquimFavorites(KisMainWindow *mw)
{
    KConfigGroup cfg = KSharedConfig::openConfig()->group("Nanquim");
    if (cfg.readEntry("favoritesSeedV1", false)) return;
    KisTagSP tag = favoritesTag();
    if (!tag) return;
    QVector<int> ids;
    Q_FOREACH (const BrushItem &b, nanquimBrushItems(QString(), true)) {
        if (b.id >= 0) ids << b.id;
    }
    if (ids.isEmpty()) return;   // pacote padrão ainda não carregou: tenta na próxima abertura
    QVector<int> old;
    Q_FOREACH (const KisPaintOpPresetSP &p, favoritePresets()) old << p->resourceId();
    KisTagResourceModel model(ResourceType::PaintOpPresets);
    if (!old.isEmpty()) model.untagResources(tag, old);
    model.tagResources(tag, ids);
    cfg.writeEntry("favoritesSeedV1", true);
    notifyFavoritesChanged(mw);
    KisUsageLogger::log(QStringLiteral("[Nanquim] favoritos iniciais: %1 pincéis").arg(ids.size()));
}

// ---------------------------------------------------------------------------
// Boas-vindas (primeira abertura) e guia rápido
// ---------------------------------------------------------------------------
const char *WELCOME_STYLE =
    "QDialog#nanquimWelcome { background: #2e2d29; border: 1px solid #4a4842; border-radius: 18px; }"
    "QDialog#nanquimWelcome QLabel { color: #eeeae0; background: transparent; }"
    "QLabel#nanquimWelcomeTitle { font-size: 24px; font-weight: 700; }"
    "QLabel#nanquimWelcomeBody { font-size: 15px; color: #cfcabf; }"
    "QLabel#nanquimWelcomeSmall { font-size: 12px; color: #918c82; }"
    "QLabel#nanquimGuideTitle { font-size: 15px; font-weight: 600; }"
    "QLabel#nanquimGuideText { font-size: 13px; color: #a8a398; }"
    "QPushButton#nanquimPrimary { min-height: 46px; padding: 0 22px; border: none; border-radius: 12px;"
    "  color: #ffffff; font-size: 15px; font-weight: 600;"
    "  background: #2f7568; }"
    "QPushButton#nanquimSecondary { min-height: 46px; padding: 0 18px; border-radius: 12px;"
    "  border: 1px solid #4a4842; background: #34332e; color: #eeeae0; font-size: 15px; }";

QWidget *guideRow(QWidget *parent, const char *svg, const QString &title, const QString &text)
{
    QWidget *row = new QWidget(parent);
    QHBoxLayout *l = new QHBoxLayout(row);
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(14);
    QLabel *icon = new QLabel(row);
    icon->setPixmap(svgPixmap(QString::fromUtf8(svg), C_ACCENT, 26));
    icon->setFixedSize(44, 44);
    icon->setAlignment(Qt::AlignCenter);
    icon->setStyleSheet(QStringLiteral("background: #2a3d38; border-radius: 12px;"));
    QVBoxLayout *t = new QVBoxLayout();
    t->setSpacing(2);
    QLabel *a = new QLabel(title, row);
    a->setObjectName(QStringLiteral("nanquimGuideTitle"));
    QLabel *b = new QLabel(text, row);
    b->setObjectName(QStringLiteral("nanquimGuideText"));
    b->setWordWrap(true);
    t->addWidget(a);
    t->addWidget(b);
    l->addWidget(icon, 0, Qt::AlignTop);
    l->addLayout(t, 1);
    return row;
}

void showWelcome(KisMainWindow *mw, bool startAtGuide)
{
    QDialog *dlg = new QDialog(mw);
    dlg->setObjectName(QStringLiteral("nanquimWelcome"));
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    dlg->setStyleSheet(QLatin1String(WELCOME_STYLE));
    dlg->setModal(true);
    dlg->setFixedWidth(qMin(680, mw->width() - 80));

    QVBoxLayout *root = new QVBoxLayout(dlg);
    root->setContentsMargins(32, 28, 32, 24);
    root->setSpacing(16);
    QStackedWidget *pages = new QStackedWidget(dlg);
    root->addWidget(pages);

    // --- página 1: a mensagem ------------------------------------------------
    QWidget *p1 = new QWidget(pages);
    QVBoxLayout *l1 = new QVBoxLayout(p1);
    l1->setContentsMargins(0, 0, 0, 0);
    l1->setSpacing(14);
    QHBoxLayout *brand = new QHBoxLayout();
    QLabel *logo = new QLabel(p1);
    logo->setPixmap(renderSvg(QLatin1String(DROP_SVG), QSize(44, 44)));
    QLabel *title = new QLabel(i18n("Bem-vindo ao Nanquim"), p1);
    title->setObjectName(QStringLiteral("nanquimWelcomeTitle"));
    brand->addWidget(logo);
    brand->addSpacing(8);
    brand->addWidget(title, 1);
    l1->addLayout(brand);
    QLabel *body = new QLabel(i18n(
        "O Nanquim nasceu de um carinho antigo pelo Krita. Foi nele que eu aprendi que um programa "
        "de desenho pode ser livre, feito por uma comunidade inteira e, ainda assim, estar à altura "
        "de qualquer trabalho profissional.\n\n"
        "Este aplicativo é a minha forma de agradecer: o mesmo motor de pincéis do Krita, com uma "
        "interface pensada para quem desenha quadrinhos no tablet. Nada aqui existiria sem as pessoas "
        "que constroem o Krita há mais de vinte anos, nem sem a arte do Tyson Tan, que dá rosto ao projeto.\n\n"
        "Se o Nanquim for útil para você, conheça e apoie o Krita em krita.org."), p1);
    body->setObjectName(QStringLiteral("nanquimWelcomeBody"));
    body->setWordWrap(true);
    l1->addWidget(body);
    QLabel *credit = new QLabel(i18n("Nanquim é um projeto independente, não é o Krita oficial. "
                                     "Código livre sob a GNU GPL. Arte da abertura: Tyson Tan (CC BY-SA)."), p1);
    credit->setObjectName(QStringLiteral("nanquimWelcomeSmall"));
    credit->setWordWrap(true);
    l1->addWidget(credit);
    QHBoxLayout *b1 = new QHBoxLayout();
    b1->addStretch(1);
    QPushButton *next = new QPushButton(i18n("Ver o guia rápido"), p1);
    next->setObjectName(QStringLiteral("nanquimPrimary"));
    b1->addWidget(next);
    l1->addLayout(b1);
    pages->addWidget(p1);

    // --- página 2: guia rápido -----------------------------------------------
    QWidget *p2 = new QWidget(pages);
    QVBoxLayout *l2 = new QVBoxLayout(p2);
    l2->setContentsMargins(0, 0, 0, 0);
    l2->setSpacing(12);
    QLabel *gt = new QLabel(i18n("Guia rápido"), p2);
    gt->setObjectName(QStringLiteral("nanquimWelcomeTitle"));
    l2->addWidget(gt);
    l2->addWidget(guideRow(p2, Svg::GALLERY, i18n("Galeria"),
        i18n("Crie uma página pelos modelos (mangá, comic, tirinha) ou toque num desenho recente. "
             "O botão de quatro quadrados volta para cá sem fechar o desenho.")));
    l2->addWidget(guideRow(p2, Svg::PEN, i18n("Ferramentas à esquerda"),
        i18n("Pincel, borracha, balde, seleção, transformar e texto. O botão de pontinhos abre todas as "
             "ferramentas. Embaixo ficam o tamanho e a opacidade do pincel.")));
    l2->addWidget(guideRow(p2, Svg::LAYERS, i18n("Painéis no topo"),
        i18n("Pincéis (a miniatura), cor, camadas, opções, réguas e referência abrem com um toque e "
             "fecham com outro. O menu completo do Krita fica nos três pontinhos.")));
    l2->addWidget(guideRow(p2, Svg::MOVE, i18n("Gestos"),
        i18n("Dois dedos para mover, dar zoom e girar a página. A caneta desenha; o dedo navega.")));
    l2->addWidget(guideRow(p2, Svg::INSERT_IMAGE, i18n("Imagens e pincéis"),
        i18n("Insira uma imagem como camada pelo botão com \"+\". Arquivos .kpp e .bundle abrem direto "
             "no Nanquim pelo \"Abrir com\" do tablet.")));
    l2->addWidget(guideRow(p2, Svg::TIMELAPSE, i18n("Time-lapse"),
        i18n("O Nanquim grava o processo sozinho, sem as pausas. Exporte o vídeo pelo botão de play.")));
    QHBoxLayout *b2 = new QHBoxLayout();
    QPushButton *backBtn = new QPushButton(i18n("Voltar"), p2);
    backBtn->setObjectName(QStringLiteral("nanquimSecondary"));
    QPushButton *done = new QPushButton(i18n("Começar a desenhar"), p2);
    done->setObjectName(QStringLiteral("nanquimPrimary"));
    b2->addWidget(backBtn);
    b2->addStretch(1);
    b2->addWidget(done);
    l2->addLayout(b2);
    pages->addWidget(p2);

    QObject::connect(next, &QPushButton::clicked, pages, [pages]() { pages->setCurrentIndex(1); });
    QObject::connect(backBtn, &QPushButton::clicked, pages, [pages]() { pages->setCurrentIndex(0); });
    QObject::connect(done, &QPushButton::clicked, dlg, &QDialog::accept);
    QObject::connect(dlg, &QDialog::finished, mw, [](int) {
        KSharedConfig::openConfig()->group("Nanquim").writeEntry("welcomeShownV1", true);
    });
    pages->setCurrentIndex(startAtGuide ? 1 : 0);
    dlg->open();
}


// ---------------------------------------------------------------------------
// Menu "Nanquim" (na barra de menus; no modo Tablet ele aparece dentro de "Mais")
// e troca entre os modos de interface Tablet e Clássico
// ---------------------------------------------------------------------------
void switchUiMode(KisMainWindow *mw)
{
    const bool toClassic = !isClassicMode();
    KConfigGroup cfg = KSharedConfig::openConfig()->group("Nanquim");
    cfg.writeEntry("uiMode", toClassic ? QStringLiteral("classic") : QStringLiteral("tablet"));
    cfg.writeEntry("classicArrangedV2", false);
    cfg.sync();
    QMessageBox::information(mw, QStringLiteral("Nanquim"),
        (toClassic ? i18n("Modo Clássico escolhido: menus no topo, barras do Krita e painéis fixos.")
                   : i18n("Modo Tablet escolhido: barra do Nanquim, ferramentas à esquerda e painéis em um toque."))
        + QStringLiteral("\n\n")
        + i18n("Salve o seu desenho, feche o Nanquim (tire-o da lista de apps recentes) e abra de novo para trocar."));
}

void arrangeClassic(KisMainWindow *mw);

void fillNanquimMenu(KisMainWindow *mw, QMenu *menu)
{
    menu->clear();
    QAction *mode = menu->addAction(isClassicMode() ? i18n("Interface: mudar para o modo Tablet")
                                                    : i18n("Interface: mudar para o modo Clássico"));
    QObject::connect(mode, &QAction::triggered, mw, [mw]() { switchUiMode(mw); });
    if (s_classic) {
        QAction *arrange = menu->addAction(i18n("Arrumar os painéis do modo Clássico"));
        arrange->setEnabled(mw->activeView() != nullptr);
        QObject::connect(arrange, &QAction::triggered, mw, [mw]() { arrangeClassic(mw); });
    }
    menu->addSeparator();
    QAction *insert = menu->addAction(i18n("Inserir imagem como camada…"));
    insert->setEnabled(mw->activeView() != nullptr);
    QObject::connect(insert, &QAction::triggered, mw, [mw]() { insertImageAsLayer(mw); });
    QMenu *chapterMenu = menu->addMenu(i18n("Adicionar este desenho a um capítulo"));
    KisView *activeView = mw->activeView();
    const QString docPath = activeView && activeView->document() ? activeView->document()->path() : QString();
    chapterMenu->setEnabled(activeView != nullptr);
    if (docPath.isEmpty()) {
        QAction *needSave = chapterMenu->addAction(i18n("Salve o desenho primeiro"));
        needSave->setEnabled(false);
    } else {
        Q_FOREACH (const ChapterStore::Chapter &c, ChapterStore::instance()->chapters()) {
            QAction *a = chapterMenu->addAction(c.name);
            a->setCheckable(true);
            a->setChecked(c.pages.contains(docPath));
            const QString id = c.id;
            QObject::connect(a, &QAction::triggered, mw, [id, docPath](bool on) {
                if (on) ChapterStore::instance()->addPage(id, docPath);
                else ChapterStore::instance()->removePage(id, docPath);
            });
        }
        chapterMenu->addSeparator();
        QAction *newChapter = chapterMenu->addAction(i18n("Novo capítulo…"));
        QObject::connect(newChapter, &QAction::triggered, mw, [mw, docPath]() {
            showChapterSheet(mw, QString(), [docPath](const QString &id) { ChapterStore::instance()->addPage(id, docPath); });
        });
    }
    QAction *hubAction = menu->addAction(i18n("Quadrinhos: mangá, requadros e webtoon…"));
    hubAction->setEnabled(mw->activeView() != nullptr);
    QObject::connect(hubAction, &QAction::triggered, mw, [mw]() { showComicHubSheet(mw); });
    QAction *panelsAction = menu->addAction(i18n("Requadros e balões…"));
    panelsAction->setEnabled(mw->activeView() != nullptr);
    QObject::connect(panelsAction, &QAction::triggered, mw, [mw]() { showComicsSheet(mw, 0); });
    QAction *exportAction = menu->addAction(i18n("Exportar…"));
    exportAction->setEnabled(mw->activeView() != nullptr);
    QObject::connect(exportAction, &QAction::triggered, mw, [mw]() { showExport(mw); });
    QAction *brushSettings = menu->addAction(i18n("Ajustes do pincel…"));
    brushSettings->setEnabled(mw->activeView() != nullptr);
    QObject::connect(brushSettings, &QAction::triggered, mw, [mw]() { showBrushSettings(mw); });
    QMenu *timelapse = menu->addMenu(i18n("Time-lapse"));
    fillTimelapseMenu(mw, timelapse);
    menu->addSeparator();
    QAction *guide = menu->addAction(i18n("Boas-vindas e guia rápido"));
    QObject::connect(guide, &QAction::triggered, mw, [mw]() { showWelcome(mw, false); });
    QAction *settings = menu->addAction(i18n("Configurações do Nanquim…"));
    QObject::connect(settings, &QAction::triggered, mw, [mw]() { showSettings(mw); });
    QAction *about = menu->addAction(i18n("Sobre e créditos"));
    QObject::connect(about, &QAction::triggered, mw, [mw]() { showAbout(mw); });
}

void buildNanquimMenu(KisMainWindow *mw)
{
    QMenu *menu = new QMenu(QStringLiteral("Nanquim"), mw->menuBar());
    menu->setObjectName(QStringLiteral("nanquimMenu"));
    mw->menuBar()->addMenu(menu);
    fillNanquimMenu(mw, menu);
    QObject::connect(menu, &QMenu::aboutToShow, mw, [mw, menu]() { fillNanquimMenu(mw, menu); });
}


// Painéis do modo Clássico no estilo do protótipo: fundo #34332e, títulos discretos,
// campos e botões arredondados e o azul do Nanquim na seleção. Só nos painéis, para
// não pesar o resto da janela (folha de estilo em tudo deixaria a tela mais lenta).
const char *CLASSIC_DOCK_STYLE =
    "QDockWidget { color: #cfcabf; font-size: 12px; }"
    "QDockWidget > QWidget { background: #34332e; }"
    "KoDockWidgetTitleBar { background: #34332e; border-top: 1px solid #4a4842; min-height: 26px; }"
    "KoDockWidgetTitleBar QLabel { color: #cfcabf; font-weight: 600; font-size: 13px; padding-left: 4px; }"
    "QMenu { background: #2e2d29; border: 1px solid #4a4842; color: #eeeae0; padding: 4px; }"
    "QMenu::item { padding: 6px 18px; border-radius: 6px; }"
    "QMenu::item:selected { background: #2a3d38; }"
    "QAbstractScrollArea, QAbstractScrollArea > QWidget { background: #34332e; border: none; }"
    "QAbstractItemView { background: #34332e; alternate-background-color: #36352f; border: none;"
    "  selection-background-color: #2a3d38; selection-color: #eeeae0; outline: 0; }"
    "QAbstractItemView::item { padding: 3px 4px; border-radius: 6px; color: #bdb8ad; }"
    "QAbstractItemView::item:selected { background: #2a3d38; color: #eeeae0; }"
    "QLineEdit, QComboBox, QSpinBox, QDoubleSpinBox { background: #2e2d29; border: 1px solid #7a766d;"
    "  border-radius: 6px; color: #e3dfd5; padding: 3px 8px; min-height: 20px; }"
    "QComboBox::drop-down { border: none; width: 18px; }"
    "QComboBox QAbstractItemView { background: #2e2d29; border: 1px solid #4a4842; }"
    "QPushButton, QToolButton { background: #4a4842; border: 1px solid #524f48; border-radius: 6px;"
    "  color: #e3dfd5; padding: 3px 8px; min-height: 20px; }"
    "QToolButton { padding: 3px; }"
    "QPushButton:pressed, QToolButton:pressed { background: #4a4842; border-color: #3e8e7e; }"
    "QPushButton:checked, QToolButton:checked { background: #2a3d38; border-color: #3e8e7e; }"
    "KoDockWidgetTitleBar QAbstractButton, KoDockWidgetTitleBarButton { background: transparent; border: none; min-height: 0; padding: 0; }"
    "QTabBar::tab { background: transparent; color: #918c82; padding: 6px 12px; border: none; }"
    "QTabBar::tab:selected { color: #eeeae0; border-bottom: 2px solid #3e8e7e; }"
    "QScrollBar:vertical { background: transparent; width: 8px; margin: 2px; }"
    "QScrollBar::handle:vertical { background: #7a766d; border-radius: 3px; min-height: 30px; }"
    "QScrollBar:horizontal { background: transparent; height: 8px; margin: 2px; }"
    "QScrollBar::handle:horizontal { background: #7a766d; border-radius: 3px; min-width: 30px; }"
    "QScrollBar::add-line, QScrollBar::sub-line, QScrollBar::add-page, QScrollBar::sub-page { background: none; border: none; width: 0; height: 0; }";

void styleClassicDocks(KisMainWindow *mw)
{
    const QStringList ids = {QStringLiteral("ColorSelectorNg"), QStringLiteral("WideGamutColorSelector"),
                             QStringLiteral("History"), QStringLiteral("sharedtooldocker"),
                             QStringLiteral("KisLayerBox"), QStringLiteral("PresetDocker")};
    Q_FOREACH (const QString &id, ids) {
        QDockWidget *dw = mw->dockWidget(id);
        if (!dw || dw->property("nanquimStyled").toBool()) continue;
        dw->setProperty("nanquimStyled", true);
        dw->setStyleSheet(QLatin1String(CLASSIC_DOCK_STYLE));
        // a lista de camadas pinta a seleção com a cor de destaque da paleta
        QPalette pal = dw->palette();
        pal.setColor(QPalette::Highlight, QColor(0x2a, 0x3d, 0x38));
        pal.setColor(QPalette::HighlightedText, QColor(0xee, 0xea, 0xe0));
        pal.setColor(QPalette::Base, QColor(0x2e, 0x2d, 0x29));
        pal.setColor(QPalette::AlternateBase, QColor(0x34, 0x33, 0x2e));
        pal.setColor(QPalette::Window, QColor(0x2e, 0x2d, 0x29));
        dw->setPalette(pal);
    }
    // títulos em português como no protótipo
    const QList<QPair<QString, QString>> titles = {
        {QStringLiteral("ColorSelectorNg"), i18n("Cor")},
        {QStringLiteral("WideGamutColorSelector"), i18n("Cor")},
        {QStringLiteral("History"), i18n("Histórico")},
        {QStringLiteral("KisLayerBox"), i18n("Camadas")},
        {QStringLiteral("sharedtooldocker"), i18n("Opções")},
    };
    for (const auto &p : titles) {
        if (QDockWidget *dw = mw->dockWidget(p.first)) dw->setWindowTitle(p.second);
    }
}

// Modo Clássico: arranjo de desktop. Menus e barras do Krita no topo, caixa de
// ferramentas à esquerda e, à direita, cor (com as opções da ferramenta em aba)
// em cima das camadas. Aplicado uma vez a cada troca de modo; depois vale o que
// o usuário arrumar.
void arrangeClassic(KisMainWindow *mw)
{
    Q_FOREACH (KisToolBar *tb, mw->toolBars()) {
        const QString name = tb->objectName();
        tb->setVisible(name == QLatin1String("mainToolBar") || name == QLatin1String("editToolBar")
                       || name == OLD_TOP_ID);
    }
    Q_FOREACH (QDockWidget *dw, mw->dockWidgets()) {
        dw->hide();
        dw->setMinimumWidth(0);
    }
    auto place = [mw](const QStringList &ids, Qt::DockWidgetArea area) -> QDockWidget* {
        QDockWidget *dw = findDock(mw, ids);
        if (dw) {
            dw->setFloating(false);
            mw->addDockWidget(area, dw);
            dw->show();
        }
        return dw;
    };
    // Arranjo do protótipo "Clássico": ferramentas à esquerda; ao lado da tela,
    // a coluna de pincéis rápidos; à direita, cor em cima, histórico e opções
    // da ferramenta em abas no meio, camadas embaixo.
    QDockWidget *toolbox = place({TOOLBOX_ID}, Qt::LeftDockWidgetArea);
    // coluna de pincéis do Nanquim (a lista do Krita continua em Configuração → Áreas Acopláveis)
    QDockWidget *brushes = mw->findChild<QDockWidget*>(BRUSH_DOCK_ID);
    if (brushes) {
        brushes->setFloating(false);
        mw->addDockWidget(Qt::RightDockWidgetArea, brushes);
        brushes->show();
    } else {
        brushes = place({QStringLiteral("PresetDocker")}, Qt::RightDockWidgetArea);
    }
    QDockWidget *color = place({QStringLiteral("ColorSelectorNg"), QStringLiteral("WideGamutColorSelector")},
                               Qt::RightDockWidgetArea);
    QDockWidget *history = place({QStringLiteral("History")}, Qt::RightDockWidgetArea);
    QDockWidget *options = place({QStringLiteral("sharedtooldocker")}, Qt::RightDockWidgetArea);
    QDockWidget *layers = place({QStringLiteral("KisLayerBox")}, Qt::RightDockWidgetArea);
    if (brushes && color) {
        mw->splitDockWidget(brushes, color, Qt::Horizontal);
    }
    QDockWidget *middle = history ? history : options;
    if (color && middle) {
        mw->splitDockWidget(color, middle, Qt::Vertical);
    }
    if (middle && layers) {
        mw->splitDockWidget(middle, layers, Qt::Vertical);
    } else if (color && layers) {
        mw->splitDockWidget(color, layers, Qt::Vertical);
    }
    if (history && options) {
        mw->tabifyDockWidget(history, options);
        history->raise();
    }
    const int side = qBound(280, mw->width() / 5, 380);
    if (brushes && color) {
        mw->resizeDocks({brushes, color}, {qBound(88, mw->width() / 20, 110), side}, Qt::Horizontal);
    } else if (color) {
        mw->resizeDocks({color}, {side}, Qt::Horizontal);
    }
    if (color && middle && layers) {
        const int h = mw->height();
        mw->resizeDocks({color, middle, layers}, {h * 30 / 100, h * 25 / 100, h * 45 / 100}, Qt::Vertical);
    }
    Q_UNUSED(toolbox);
    styleClassicDocks(mw);
}


// ===========================================================================
// Identidade visual do Nanquim em todas as áreas (protótipo "Identidade visual")
// ===========================================================================

// Folha de estilo comum às janelas do Nanquim: fundo #2e2d29, títulos de seção
// em maiúsculas espaçadas, botão principal com o degradê da gota, chips em pílula.
const char *SHEET_STYLE =
    "QDialog#nanquimSheet { background: " NQ_RELEVO3_BG "; border: 1px solid " NQ_RELEVO3_BORDA "; border-radius: 18px; }"
    "QDialog#nanquimSheet QLabel { color: #eeeae0; background: transparent; }"
    "QLabel#nqSection { color: #a8a398; font-size: 13px; font-weight: 700; letter-spacing: 2px; }"
    "QLabel#nqTitle { font-size: 24px; font-weight: 700; }"
    "QLabel#nqHint { color: #a8a398; font-size: 12px; }"
    "QLabel#nqBody { color: #cfcabf; font-size: 14px; }"
    "QLabel#nqItem { font-size: 14px; font-weight: 600; }"
    "QLabel#nqSmall { color: #a8a398; font-size: 12px; }"
    "QPushButton { min-height: 44px; padding: 0 18px; border-radius: 10px; border: 1px solid #4a4842;"
    "  background: #34332e; color: #eeeae0; font-size: 14px; font-weight: 600; }"
    "QPushButton:pressed { background: #4a4842; border-color: #3e8e7e; }"
    "QPushButton:disabled { color: #7a766d; }"
    "QPushButton#nqPrimary { border: none; color: #ffffff; font-weight: 700;"
    "  background: #2f7568; }"
    "QPushButton#nqGhost { border: none; background: transparent; }"
    "QPushButton#nqDanger { border: none; background: transparent; color: #ff8585; }"
    "QPushButton#nqClose { border: none; background: transparent; min-width: 44px; max-width: 44px; padding: 0; font-size: 20px; }"
    "QPushButton.nqChip { min-height: 34px; padding: 0 14px; border-radius: 17px; font-size: 13px; }"
    "QPushButton.nqChip:checked { border: none; color: #ffffff;"
    "  background: #2f7568; }"
    "QPushButton.nqCard { min-height: 64px; border-radius: 12px; padding: 8px; }"
    "QPushButton.nqCard:checked { background: #2a3d38; border: 2px solid #3e8e7e; }"
    "QPushButton.nqSwatch { min-height: 44px; max-height: 44px; min-width: 44px; max-width: 44px; padding: 0; border-radius: 10px; }"
    "QPushButton.nqSwatch:checked { border: 2px solid #3e8e7e; }"
    "QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox, QPlainTextEdit { min-height: 40px; padding: 0 10px; border-radius: 10px;"
    "  border: 1px solid #7a766d; background: #2e2d29; color: #eeeae0; font-size: 14px; }"
    "QLineEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus, QPlainTextEdit:focus { border: 2px solid #3e8e7e; }"
    "QPlainTextEdit { padding: 8px 10px; }"
    "QComboBox::drop-down { border: none; width: 22px; }"
    "QComboBox QAbstractItemView { background: #2e2d29; color: #eeeae0; border: 1px solid #4a4842; selection-background-color: #2a3d38; }"
    "QCheckBox { color: #eeeae0; font-size: 14px; spacing: 12px; min-height: 36px; }"
    "QCheckBox::indicator { width: 22px; height: 22px; border-radius: 6px; border: 2px solid #7a766d; background: #2e2d29; }"
    "QCheckBox::indicator:checked { border-color: #6fbfad;"
    "  background: #2f7568; }"
    "QSlider { min-height: 30px; }"
    "QSlider::groove:horizontal { height: 6px; border-radius: 3px; background: #7a766d; }"
    "QSlider::sub-page:horizontal { border-radius: 3px;"
    "  background: #6fbfad; }"
    "QSlider::handle:horizontal { width: 22px; height: 22px; margin: -8px 0; border-radius: 11px; background: #eeeae0; }"
    "QScrollArea, QScrollArea > QWidget > QWidget { background: transparent; border: none; }"
    "QFrame#nqPanel { background: #2e2d29; border: 1px solid #4a4842; border-radius: 14px; }"
    "QFrame#nqDivider { background: #4a4842; max-height: 1px; min-height: 1px; border: none; }";

QDialog *makeSheet(KisMainWindow *mw, int width, int height = 0)
{
    QDialog *d = new QDialog(mw);
    d->setObjectName(QStringLiteral("nanquimSheet"));
    d->setAttribute(Qt::WA_DeleteOnClose);
    d->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    d->setStyleSheet(QLatin1String(SHEET_STYLE));
    d->setModal(true);
    d->setFixedWidth(qMin(width, mw->width() - 40));
    if (height > 0) d->setFixedHeight(qMin(height, mw->height() - 40));
    return d;
}

QLabel *nqLabel(const QString &text, const char *name, QWidget *parent, bool wrap = false)
{
    QLabel *l = new QLabel(text, parent);
    l->setObjectName(QLatin1String(name));
    l->setWordWrap(wrap);
    return l;
}

QPushButton *nqButton(const QString &text, QWidget *parent, const char *name = nullptr)
{
    QPushButton *b = new QPushButton(text, parent);
    if (name) b->setObjectName(QLatin1String(name));
    b->setFocusPolicy(Qt::NoFocus);
    return b;
}

QPushButton *nqChip(const QString &text, QWidget *parent, QButtonGroup *group, int id)
{
    QPushButton *b = nqButton(text, parent);
    b->setProperty("class", QStringLiteral("nqChip"));
    b->setCheckable(true);
    if (group) group->addButton(b, id);
    return b;
}

QFrame *nqDivider(QWidget *parent)
{
    QFrame *f = new QFrame(parent);
    f->setObjectName(QStringLiteral("nqDivider"));
    return f;
}

// cabeçalho: seção em maiúsculas, título e o X de fechar
QHBoxLayout *sheetHeader(QDialog *d, const QString &section, const QString &title)
{
    QHBoxLayout *h = new QHBoxLayout();
    QVBoxLayout *t = new QVBoxLayout();
    t->setSpacing(4);
    t->addWidget(nqLabel(section.toUpper(), "nqSection", d));
    if (!title.isEmpty()) t->addWidget(nqLabel(title, "nqTitle", d));
    h->addLayout(t, 1);
    QPushButton *close = nqButton(QStringLiteral("×"), d, "nqClose");
    close->setToolTip(i18n("Fechar"));
    QObject::connect(close, &QPushButton::clicked, d, &QDialog::reject);
    h->addWidget(close, 0, Qt::AlignTop);
    return h;
}

// confirmação no visual das folhas do Nanquim (devolve true se o usuário confirmou)
bool nqConfirm(KisMainWindow *mw, const QString &section, const QString &title, const QString &text, const QString &confirm)
{
    QDialog *d = makeSheet(mw, 460);
    QVBoxLayout *root = new QVBoxLayout(d);
    root->setContentsMargins(24, 20, 24, 20);
    root->setSpacing(14);
    root->addLayout(sheetHeader(d, section, title));
    root->addWidget(nqLabel(text, "nqSmall", d, true));
    QHBoxLayout *row = new QHBoxLayout();
    row->setSpacing(10);
    QPushButton *cancel = nqButton(i18n("Cancelar"), d, "nqGhost");
    QPushButton *ok = nqButton(confirm, d, "nqPrimary");
    ok->setStyleSheet(QStringLiteral("background: #b4534a; color: #ffffff;"));
    QObject::connect(cancel, &QPushButton::clicked, d, &QDialog::reject);
    QObject::connect(ok, &QPushButton::clicked, d, &QDialog::accept);
    row->addStretch(1);
    row->addWidget(cancel);
    row->addWidget(ok);
    root->addLayout(row);
    return d->exec() == QDialog::Accepted;
}

QHBoxLayout *chipRow(QWidget *parent, QButtonGroup *group, const QStringList &labels, int checked)
{
    QHBoxLayout *row = new QHBoxLayout();
    row->setSpacing(8);
    for (int i = 0; i < labels.size(); i++) {
        QPushButton *c = nqChip(labels[i], parent, group, i);
        c->setChecked(i == checked);
        row->addWidget(c);
    }
    row->addStretch(1);
    return row;
}

QPushButton *swatchButton(QWidget *parent, const QColor &c, const QString &tip)
{
    QPushButton *b = nqButton(QString(), parent);
    b->setProperty("class", QStringLiteral("nqSwatch"));
    b->setToolTip(tip);
    b->setStyleSheet(QStringLiteral("background: %1;").arg(c.name()));
    return b;
}

// ---------------------------------------------------------------------------
// Aviso discreto no topo central (estilo Procreate): nunca cobre o desenho
// ---------------------------------------------------------------------------
void showToast(KisMainWindow *mw, const QString &text)
{
    if (!mw) return;
    QFrame *old = mw->findChild<QFrame*>(QStringLiteral("nanquimToast"));
    if (old) old->deleteLater();
    QFrame *toast = new QFrame(mw);
    toast->setObjectName(QStringLiteral("nanquimToast"));
    toast->setAttribute(Qt::WA_TransparentForMouseEvents);
    toast->setStyleSheet(QStringLiteral(
        "QFrame#nanquimToast { background: rgba(46,45,41,0.92);"
        "  border: 1px solid #524f48; border-radius: 15px; }"
        "QFrame#nanquimToastDot { background: #6fbfad; border-radius: 3px; }"
        "QLabel { color: #eeeae0; font-size: 13px; font-weight: 600; background: transparent; }"));
    QHBoxLayout *l = new QHBoxLayout(toast);
    l->setContentsMargins(12, 6, 14, 6);
    l->setSpacing(8);
    QFrame *dot = new QFrame(toast);
    dot->setObjectName(QStringLiteral("nanquimToastDot"));
    dot->setFixedSize(6, 6);
    l->addWidget(dot, 0, Qt::AlignVCenter);
    l->addWidget(new QLabel(text, toast));
    toast->adjustSize();

    int topY = 14;
    if (QScreen *screen = mw->screen()) {
        const int winTop = mw->mapToGlobal(QPoint(0, 0)).y();
        topY = qMax(topY, screen->availableGeometry().top() - winTop + 10);
    }
    if (QToolBar *top = mw->findChild<QToolBar*>(TOP_ID)) {
        if (top->isVisible()) {
            topY = qMax(topY, top->mapTo(mw, QPoint(0, top->height())).y() + 8);
        }
    }
    const int x = (mw->width() - toast->width()) / 2;
    const int yEnd = topY;
    const int yStart = yEnd - 22;
    toast->move(x, yStart);
    toast->show();
    toast->raise();
    QPropertyAnimation *in = new QPropertyAnimation(toast, "pos", toast);
    in->setDuration(210);
    in->setStartValue(QPoint(x, yStart));
    in->setEndValue(QPoint(x, yEnd));
    in->setEasingCurve(softSpring());
    in->start(QAbstractAnimation::DeleteWhenStopped);
    QPointer<QFrame> guard(toast);
    QTimer::singleShot(1700, toast, [guard, x, yEnd, yStart]() {
        if (!guard) return;
        QPropertyAnimation *out = new QPropertyAnimation(guard, "pos", guard);
        out->setDuration(150);
        out->setStartValue(QPoint(x, yEnd));
        out->setEndValue(QPoint(x, yStart - 8));
        out->setEasingCurve(QEasingCurve::InCubic);
        QObject::connect(out, &QPropertyAnimation::finished, guard, [guard]() { if (guard) guard->deleteLater(); });
        out->start(QAbstractAnimation::DeleteWhenStopped);
    });
}

// ---------------------------------------------------------------------------
// Pílula de salvamento: no canto da página, do lado oposto à barra. Gira em
// ciano enquanto salva, estala verde ao terminar e fica coral se falhar, com
// o motivo. Some sozinha. Substitui o aviso "Página salva".
// ---------------------------------------------------------------------------
class SavePill : public QWidget
{
public:
    enum State { Busy, Ok, Failed };

    explicit SavePill(KisMainWindow *mw)
        : QWidget(mw), m_mw(mw)
    {
        setObjectName(QStringLiteral("nanquimSavePill"));
        setAttribute(Qt::WA_TransparentForMouseEvents);
        setAttribute(Qt::WA_TranslucentBackground);
        setAttribute(Qt::WA_NoSystemBackground);
        setFocusPolicy(Qt::NoFocus);
        m_spin.setInterval(16);
        QObject::connect(&m_spin, &QTimer::timeout, this, [this]() {
            m_angle = (m_angle + 9) % 360;
            update();
        });
        m_hide.setSingleShot(true);
        QObject::connect(&m_hide, &QTimer::timeout, this, [this]() { slideOut(); });
        m_pop.setStartValue(0.0);
        m_pop.setEndValue(1.0);
        m_pop.setDuration(320);
        m_pop.setEasingCurve(softSpring());
        QObject::connect(&m_pop, &QVariantAnimation::valueChanged, this, [this](const QVariant &v) {
            m_popValue = v.toReal();
            update();
        });
        hide();
    }

    void showState(State st, const QString &title, const QString &sub)
    {
        const bool wasHidden = !isVisible() || m_leaving;
        m_state = st;
        m_title = title;
        m_sub = sub;
        m_leaving = false;
        if (m_slide) m_slide->stop();
        resizeToText();
        place();
        if (st == Busy) {
            m_spin.start();
            m_hide.start(15000);   // segurança: o Krita pode não salvar (nada mudou)
        } else {
            m_spin.stop();
            m_popValue = 0.0;
            m_pop.stop();
            m_pop.start();
            m_hide.start(st == Ok ? 2000 : 5000);
        }
        if (wasHidden) slideIn();
        show();
        raise();
        update();
    }

    void place()
    {
        if (!m_mw) return;
        QWidget *central = m_mw->centralWidget();
        if (!central) return;
        const QRect area(central->mapTo(m_mw, QPoint(0, 0)), central->size());
        const int x = railOnRight() ? area.left() + 20 : area.right() - 20 - width() + 1;
        m_rest = QPoint(x, area.bottom() - 20 - height() + 1);
        if (!m_slide || m_slide->state() != QAbstractAnimation::Running) move(m_rest);
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter g(this);
        g.setRenderHint(QPainter::Antialiasing);
        const QRectF r = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
        const QColor tint = m_state == Busy ? Cor::CIANO : m_state == Ok ? Cor::VERDE : Cor::CORAL;
        QLinearGradient bg(r.topLeft(), r.bottomLeft());
        bg.setColorAt(0.0, QColor(0x3d, 0x3c, 0x37));
        bg.setColorAt(0.45, QColor(0x34, 0x33, 0x2e));
        bg.setColorAt(1.0, QColor(0x2e, 0x2d, 0x29));
        QColor border = m_state == Busy ? QColor(0x5a, 0x57, 0x4f) : tint.darker(220);
        g.setPen(QPen(border, 1));
        g.setBrush(bg);
        g.drawRoundedRect(r, r.height() / 2, r.height() / 2);

        const QRectF dot(8, (height() - 36) / 2.0, 36, 36);
        QColor wash(tint);
        wash.setAlpha(40);
        g.setPen(Qt::NoPen);
        g.setBrush(wash);
        g.drawEllipse(dot);
        const QPointF c = dot.center();
        if (m_state == Busy) {
            g.setPen(QPen(tint, 2.6, Qt::SolidLine, Qt::RoundCap));
            g.setBrush(Qt::NoBrush);
            g.drawArc(QRectF(c.x() - 9, c.y() - 9, 18, 18), -m_angle * 16, 270 * 16);
        } else if (m_state == Ok) {
            const qreal s = 0.6 + 0.4 * m_popValue;
            g.save();
            g.translate(c);
            g.scale(s, s);
            g.setPen(QPen(tint, 2.6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            g.setBrush(Qt::NoBrush);
            g.drawPolyline(QPolygonF() << QPointF(-7, 0.5) << QPointF(-2, 5.5) << QPointF(7.5, -5));
            g.restore();
        } else {
            g.setPen(QPen(tint, 2.8, Qt::SolidLine, Qt::RoundCap));
            g.drawLine(QPointF(c.x(), c.y() - 7), QPointF(c.x(), c.y() + 2));
            g.drawPoint(QPointF(c.x(), c.y() + 7));
        }

        const int tx = int(dot.right()) + 12;
        g.setPen(Cor::TEXTO);
        g.setFont(titleFont());
        g.drawText(QRect(tx, 8, width() - tx - 18, height() / 2 - 6), Qt::AlignLeft | Qt::AlignBottom, m_title);
        g.setPen(Cor::TEXTO_SUAVE);
        g.setFont(subFont());
        g.drawText(QRect(tx, height() / 2, width() - tx - 18, height() / 2 - 8), Qt::AlignLeft | Qt::AlignTop,
                   QFontMetrics(subFont()).elidedText(m_sub, Qt::ElideRight, width() - tx - 18));
    }

private:
    QFont titleFont() const
    {
        QFont f = font();
        f.setPixelSize(14);
        f.setWeight(QFont::Bold);
        return f;
    }
    QFont subFont() const
    {
        QFont f = font();
        f.setPixelSize(12);
        return f;
    }
    void resizeToText()
    {
        const int text = qMax(QFontMetrics(titleFont()).horizontalAdvance(m_title),
                              qMin(260, QFontMetrics(subFont()).horizontalAdvance(m_sub)));
        resize(8 + 36 + 12 + text + 20, 52);
    }
    void slideIn()
    {
        if (!m_slide) m_slide = new QPropertyAnimation(this, "pos", this);
        m_slide->stop();
        m_slide->setDuration(260);
        m_slide->setStartValue(m_rest + QPoint(0, 30));
        m_slide->setEndValue(m_rest);
        m_slide->setEasingCurve(softSpring());
        move(m_rest + QPoint(0, 30));
        m_slide->start();
    }
    void slideOut()
    {
        m_spin.stop();
        if (!isVisible()) return;
        m_leaving = true;
        if (!m_slide) m_slide = new QPropertyAnimation(this, "pos", this);
        m_slide->stop();
        m_slide->setDuration(180);
        m_slide->setStartValue(pos());
        m_slide->setEndValue(pos() + QPoint(0, 40));
        m_slide->setEasingCurve(QEasingCurve::InCubic);
        QPointer<SavePill> guard(this);
        QObject::disconnect(m_slide, &QPropertyAnimation::finished, nullptr, nullptr);
        QObject::connect(m_slide, &QPropertyAnimation::finished, this, [guard]() {
            if (guard && guard->m_leaving) {
                guard->hide();
                guard->m_leaving = false;
            }
        });
        m_slide->start();
    }

    QPointer<KisMainWindow> m_mw;
    State m_state = Ok;
    QString m_title, m_sub;
    QTimer m_spin, m_hide;
    QVariantAnimation m_pop;
    QPointer<QPropertyAnimation> m_slide;
    QPoint m_rest;
    qreal m_popValue = 1.0;
    int m_angle = 0;
    bool m_leaving = false;
};

void showSaveState(KisMainWindow *mw, int state, const QString &title, const QString &sub)
{
    if (!mw || s_classic) return;
    SavePill *pill = mw->findChild<SavePill*>(QStringLiteral("nanquimSavePill"));
    if (!pill) pill = new SavePill(mw);
    pill->showState(SavePill::State(state), title, sub);
}

// perguntas do Krita (salvar, fechar...) com a cara do Nanquim. No Android o
// Qt entregava essas perguntas para o próprio sistema desenhar (a caixa cinza
// com "YES/NO"); o Nanquim pede ao Qt que as desenhe, e aqui elas ganham o estilo.
const char *MESSAGE_STYLE =
    "QMessageBox, QInputDialog { background: #2e2d29; border: 1px solid #4a4842; }"
    "QMessageBox QLabel, QInputDialog QLabel { color: #eeeae0; font-size: 15px; background: transparent; }"
    "QMessageBox QLabel#qt_msgbox_label { font-size: 16px; font-weight: 600; min-width: 320px; }"
    "QMessageBox QLabel#qt_msgbox_informativelabel { color: #a8a398; font-size: 14px; }"
    "QMessageBox QPushButton, QInputDialog QPushButton { min-height: 44px; min-width: 104px; padding: 0 18px; border-radius: 12px;"
    "  border: 1px solid #4a4842; background: #34332e; color: #eeeae0; font-size: 14px; font-weight: 600; }"
    "QMessageBox QPushButton:pressed, QInputDialog QPushButton:pressed { background: #4a4842; border-color: #3e8e7e; }"
    "QMessageBox QPushButton:default, QInputDialog QPushButton:default { border: none; color: #ffffff; font-weight: 700;"
    "  background: #2f7568; }"
    "QMessageBox QPushButton#nqDanger { color: #ff8585; background: transparent; border: 1px solid #4a2c31; }"
    "QMessageBox QCheckBox { color: #cfcabf; font-size: 14px; }"
    "QMessageBox QTextEdit { background: #2e2d29; color: #cfcabf; border: 1px solid #4a4842; border-radius: 10px; }"
    "QInputDialog QLineEdit, QInputDialog QComboBox, QInputDialog QSpinBox, QInputDialog QDoubleSpinBox {"
    "  min-height: 44px; padding: 0 12px; border-radius: 12px; border: 1.5px solid #3e8e7e; background: #24231f;"
    "  color: #eeeae0; font-size: 15px; }"
    "QInputDialog QListView { background: #2e2d29; color: #eeeae0; border: 1px solid #4a4842; border-radius: 12px; font-size: 15px; }"
    "QInputDialog QListView::item { min-height: 44px; padding: 0 12px; }"
    "QInputDialog QListView::item:selected { background: #2a3d38; color: #ffffff; }";

// botões em português e o ícone do Nanquim no lugar do ícone do sistema
void dressMessageBox(QMessageBox *mb)
{
    struct Name { QMessageBox::StandardButton button; const char *text; };
    const Name names[] = {
        {QMessageBox::Ok, "OK"}, {QMessageBox::Save, "Salvar"}, {QMessageBox::SaveAll, "Salvar tudo"},
        {QMessageBox::Open, "Abrir"}, {QMessageBox::Yes, "Sim"}, {QMessageBox::YesToAll, "Sim para todos"},
        {QMessageBox::No, "Não"}, {QMessageBox::NoToAll, "Não para todos"}, {QMessageBox::Abort, "Interromper"},
        {QMessageBox::Retry, "Tentar de novo"}, {QMessageBox::Ignore, "Ignorar"}, {QMessageBox::Close, "Fechar"},
        {QMessageBox::Cancel, "Cancelar"}, {QMessageBox::Discard, "Não salvar"}, {QMessageBox::Help, "Ajuda"},
        {QMessageBox::Apply, "Aplicar"}, {QMessageBox::Reset, "Redefinir"}, {QMessageBox::RestoreDefaults, "Restaurar padrões"},
    };
    for (const Name &n : names) {
        if (QAbstractButton *b = mb->button(n.button)) b->setText(i18n(n.text));
    }
    auto danger = [](QAbstractButton *b) {
        if (!b) return;
        b->setObjectName(QStringLiteral("nqDanger"));
        b->style()->unpolish(b);
        b->style()->polish(b);
    };
    // "O documento foi modificado. Deseja gravá-lo?": Salvar / Não salvar / Cancelar
    const QString text = mb->text() + mb->informativeText();
    if (mb->button(QMessageBox::Yes) && mb->button(QMessageBox::No)
            && (text.contains(QLatin1String("modific"), Qt::CaseInsensitive) || text.contains(QLatin1String("grav"), Qt::CaseInsensitive))) {
        mb->button(QMessageBox::Yes)->setText(i18n("Salvar"));
        mb->button(QMessageBox::No)->setText(i18n("Não salvar"));
        danger(mb->button(QMessageBox::No));
    }
    danger(mb->button(QMessageBox::Discard));

    const char *path = nullptr;
    QColor color = C_ACCENT;
    switch (mb->icon()) {
    case QMessageBox::Warning:
        path = "<path d='M12 3.5l9.2 16.2H2.8z'/><path d='M12 10v4.6'/><circle cx='12' cy='17.3' r='.7' fill='currentColor'/>";
        color = QColor(0xf5, 0xb7, 0x4f);
        break;
    case QMessageBox::Critical:
        path = "<circle cx='12' cy='12' r='9'/><path d='M9 9l6 6M15 9l-6 6'/>";
        color = QColor(0xff, 0x85, 0x85);
        break;
    case QMessageBox::Question:
        path = "<circle cx='12' cy='12' r='9'/><path d='M9.6 9.4a2.5 2.5 0 1 1 3.5 2.3c-.7.4-1.1 1-1.1 1.8v.4'/><circle cx='12' cy='17' r='.7' fill='currentColor'/>";
        break;
    case QMessageBox::Information:
        path = "<circle cx='12' cy='12' r='9'/><path d='M12 11v6'/><circle cx='12' cy='7.8' r='.7' fill='currentColor'/>";
        break;
    default:
        break;
    }
    if (path) mb->setIconPixmap(svgPixmap(QString::fromUtf8(path), color, 40));

    // no Android a caixa aparecia com faixa branca e o texto cortado à direita
    QPalette pal = mb->palette();
    pal.setColor(QPalette::Window, QColor(0x2e, 0x2d, 0x29));
    pal.setColor(QPalette::Base, QColor(0x2e, 0x2d, 0x29));
    pal.setColor(QPalette::WindowText, QColor(0xee, 0xea, 0xe0));
    mb->setPalette(pal);
    mb->setAutoFillBackground(true);
    for (const char *name : {"qt_msgbox_label", "qt_msgbox_informativelabel"}) {
        if (QLabel *l = mb->findChild<QLabel*>(QLatin1String(name))) {
            l->setWordWrap(true);
            l->setMaximumWidth(520);
        }
    }
    mb->adjustSize();
}

// A caixa nativa do Krita sai quebrada no Android (faixa branca, texto cortado à direita).
// Ela some da tela e uma folha do Nanquim, com o mesmo texto e os mesmos botões, toma o
// lugar; o botão escolhido é repassado para a caixa original, que fecha com a resposta certa.
void replaceMessageBox(QPointer<QMessageBox> mb, QPointer<KisMainWindow> mw)
{
    if (!mb || !mw || mb->property("nanquimReplacing").toBool()) return;
    mb->setProperty("nanquimReplacing", true);
    KisUsageLogger::log(QStringLiteral("[Nanquim] caixa do Krita trocada por folha: %1").arg(mb->text().left(160)));

    auto prepare = [](QString t) {
        if (Qt::mightBeRichText(t)) return t;
        // o pacote instalado agora aparece na aba Importados
        t.replace(QStringLiteral("estão em \"Tudo\", na janela de pincéis"),
                  QStringLiteral("estão em Pincéis → Importados"));
        // nomes de arquivo longos podem quebrar linha (U+200B explícito)
        const QString zwsp = QString(QChar(0x200B));
        t.replace(QLatin1Char('_'), QStringLiteral("_") + zwsp);
        t.replace(QLatin1Char('/'), QStringLiteral("/") + zwsp);
        return t;
    };

    QDialog *d = makeSheet(mw, 540);
    d->setMaximumHeight(qMax(240, mw->height() - 48));
    QVBoxLayout *root = new QVBoxLayout(d);
    root->setContentsMargins(24, 22, 24, 20);
    root->setSpacing(16);

    QHBoxLayout *top = new QHBoxLayout();
    top->setSpacing(14);
    const bool hasIcon = !mb->iconPixmap().isNull();
    if (hasIcon) {
        QLabel *icon = new QLabel(d);
        icon->setPixmap(mb->iconPixmap());
        top->addWidget(icon, 0, Qt::AlignTop);
    }

    // Texto direto na folha, no fundo escuro dela. A versão com área de rolagem
    // deixava o fundo da rolagem bege (cor de papel do tema) e o texto claro sumia;
    // ao abrir uma página, a folha ficava presa atrás do véu "Abrindo…".
    QVBoxLayout *texts = new QVBoxLayout();
    texts->setSpacing(6);
    QLabel *main = nqLabel(prepare(mb->text()), "nqItem", d, true);
    main->setTextFormat(mb->textFormat());
    main->setMinimumWidth(0);
    main->setTextInteractionFlags(Qt::NoTextInteraction);
    texts->addWidget(main);
    if (!mb->informativeText().isEmpty()) {
        QLabel *info = nqLabel(prepare(mb->informativeText()), "nqSmall", d, true);
        info->setMinimumWidth(0);
        texts->addWidget(info);
    }
    top->addLayout(texts, 1);
    root->addLayout(top);

    auto chosen = std::make_shared<QPointer<QAbstractButton>>();
    QHBoxLayout *row = new QHBoxLayout();
    row->setSpacing(10);
    row->addStretch(1);
    const QList<QAbstractButton*> buttons = mb->buttons();
    QAbstractButton *def = mb->defaultButton();
    for (QAbstractButton *src : buttons) {
        const bool isDanger = (src->objectName() == QLatin1String("nqDanger"));
        const bool primary = !isDanger && ((src == def) || buttons.size() == 1);
        QPushButton *b = nqButton(src->text().remove(QLatin1Char('&')), d, primary ? "nqPrimary" : nullptr);
        if (isDanger) b->setStyleSheet(QStringLiteral("color: #ff8585; border-color: #593533;"));
        QPointer<QAbstractButton> srcGuard(src);
        QObject::connect(b, &QPushButton::clicked, d, [d, chosen, srcGuard]() {
            *chosen = srcGuard;
            d->accept();
        });
        row->addWidget(b);
    }
    root->addLayout(row);
    d->exec();

    if (!mb) return;
    mb->setProperty("nanquimReplacing", false);
    mb->setProperty("nanquimDressed", false);
    QAbstractButton *target = chosen->data();
    if (!target) target = mb->escapeButton();
    const QList<QAbstractButton*> currentButtons = mb->buttons();
    if (!target && currentButtons.size() == 1) target = currentButtons.first();
    if (target) target->click();
    else mb->reject();
}

class MessageStyler : public QObject
{
public:
    using QObject::QObject;
protected:
    bool eventFilter(QObject *obj, QEvent *e) override
    {
        if (e->type() != QEvent::Polish && e->type() != QEvent::Show) return false;
        QWidget *w = qobject_cast<QWidget*>(obj);
        if (!w || !w->isWindow()) return false;
        const bool box = w->inherits("QMessageBox");
        if (!box && !w->inherits("QInputDialog")) return false;
        if (e->type() == QEvent::Polish) {
            if (w->property("nanquimStyled").toBool()) return false;
            w->setProperty("nanquimStyled", true);
            // sem a moldura branca do sistema em volta
            if (!w->testAttribute(Qt::WA_WState_Created) && !(w->windowFlags() & Qt::FramelessWindowHint)) {
                w->setWindowFlags(w->windowFlags() | Qt::FramelessWindowHint);
            }
            w->setAttribute(Qt::WA_StyledBackground);
            if (w->styleSheet().isEmpty()) w->setStyleSheet(QLatin1String(MESSAGE_STYLE));
            return false;
        }
        if (w->property("nanquimDressed").toBool()) return false;
        w->setProperty("nanquimDressed", true);
        if (box) {
            QMessageBox *mb = static_cast<QMessageBox*>(w);
            dressMessageBox(mb);
            KisUsageLogger::log(QStringLiteral("[Nanquim] QMessageBox exibida: %1").arg(mb->text().left(160)));
            KisMainWindow *mw = qobject_cast<KisMainWindow*>(parent());
            // caixas com caixa de seleção ou detalhes ficam nativas (são raras)
            // durante a abertura de uma página fica a caixa nativa: a troca nunca pode travar a abertura
            QWidget *veilWidget = mw ? mw->findChild<QWidget*>(QStringLiteral("nanquimOpeningVeil")) : nullptr;
            const bool opening = veilWidget != nullptr;
            // o véu da abertura sai da frente para a caixa aparecer; a abertura continua depois da resposta
            if (OpeningVeil *veil = dynamic_cast<OpeningVeil*>(veilWidget)) veil->fadeAndClose();
            if (mw && !opening && !mb->checkBox() && mb->detailedText().isEmpty() && !mb->buttons().isEmpty()
                    && !mb->property("nanquimReplacing").toBool()) {
                mb->setWindowOpacity(0.0);
                mb->resize(1, 1);
                mb->move(-20000, -20000);
                QPointer<QMessageBox> mbGuard(mb);
                QPointer<KisMainWindow> mwGuard(mw);
                QTimer::singleShot(0, mb, [mbGuard, mwGuard]() { replaceMessageBox(mbGuard, mwGuard); });
            }
        } else if (QInputDialog *in = qobject_cast<QInputDialog*>(w)) {
            const QString cancel = in->cancelButtonText().remove(QLatin1Char('&'));
            if (cancel.compare(QLatin1String("Cancel"), Qt::CaseInsensitive) == 0) in->setCancelButtonText(i18n("Cancelar"));
            if (in->labelText() == QLatin1String("Save As:")) in->setLabelText(i18n("Salvar como:"));
        }
        return false;
    }
};

// ---------------------------------------------------------------------------
// Aviso de memória: páginas muito grandes fazem o Android fechar o app
// ---------------------------------------------------------------------------
qint64 estimateImageBytes(KisImageSP image)
{
    if (!image) return 0;
    const qint64 pixels = qint64(image->width()) * image->height();
    const int pixelSize = image->colorSpace() ? int(image->colorSpace()->pixelSize()) : 4;
    int layers = 1;   // a projeção
    std::function<void(KisNodeSP)> walk = [&](KisNodeSP node) {
        for (KisNodeSP child = node->firstChild(); child; child = child->nextSibling()) {
            if (child->paintDevice() && child->inherits("KisPaintLayer")) layers++;
            walk(child);
        }
    };
    walk(image->root());
    return pixels * pixelSize * layers;
}

void showMemoryWarning(KisMainWindow *mw, KisView *view)
{
    if (!mw || !view || !view->image() || !view->document()) return;
    static QSet<QString> warned;
    const QString key = view->document()->path().isEmpty() ? view->document()->caption() : view->document()->path();
    if (warned.contains(key)) return;
    const qint64 bytes = estimateImageBytes(view->image());
    const qint64 limit = qint64(1200) * 1024 * 1024;
    if (bytes < limit) return;
    warned.insert(key);

    QDialog *d = makeSheet(mw, 560);
    QVBoxLayout *root = new QVBoxLayout(d);
    root->setContentsMargins(26, 22, 26, 22);
    root->setSpacing(14);
    root->addLayout(sheetHeader(d, i18n("Cuidado com a memória"),
                                i18n("Esta página usa cerca de %1 GB", QString::number(bytes / (1024.0 * 1024 * 1024), 'f', 1))));
    root->addWidget(nqLabel(i18n("O tablet pode fechar o Nanquim ao transformar ou liquidificar uma página deste tamanho. "
                                 "Juntar camadas que já estão prontas, ou reduzir a página para 600 dpi, deixa tudo mais leve."),
                            "nqBody", d, true));
    QProgressBar *bar = new QProgressBar(d);
    bar->setRange(0, 100);
    bar->setValue(qBound(10, int(100.0 * bytes / (2.0 * limit)), 100));
    bar->setTextVisible(false);
    bar->setFixedHeight(10);
    bar->setStyleSheet(QStringLiteral("QProgressBar { background: #4a4842; border: none; border-radius: 5px; }"
                                      "QProgressBar::chunk { background: #ffb020; border-radius: 5px; }"));
    root->addWidget(bar);
    QHBoxLayout *actions = new QHBoxLayout();
    actions->setSpacing(8);
    QPushButton *reduce = nqButton(i18n("Reduzir a página…"), d);
    QPushButton *ok = nqButton(i18n("Entendi"), d, "nqPrimary");
    actions->addWidget(reduce);
    actions->addStretch(1);
    actions->addWidget(ok);
    root->addLayout(actions);
    QObject::connect(ok, &QPushButton::clicked, d, &QDialog::accept);
    QObject::connect(reduce, &QPushButton::clicked, d, [mw, d]() {
        d->accept();
        if (QAction *a = act(mw, "imagesize")) a->trigger();
    });
    d->open();
}

// ---------------------------------------------------------------------------
// Sobre e créditos
// ---------------------------------------------------------------------------
void showAbout(KisMainWindow *mw)
{
    QDialog *d = makeSheet(mw, 940, 580);
    QHBoxLayout *root = new QHBoxLayout(d);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // arte do Krita (a mesma da abertura), cortada para a coluna
    const int artW = qMin(400, d->width() / 2 - 40);
    const int artH = d->height();
    QLabel *art = new QLabel(d);
    art->setFixedSize(artW, artH);
    QPixmap src(QStringLiteral(":/splash/hd.jpg"));
    QPixmap canvas(artW, artH);
    canvas.fill(QColor(0x24, 0x23, 0x1f));
    {
        QPainter g(&canvas);
        g.setRenderHint(QPainter::SmoothPixmapTransform);
        g.setRenderHint(QPainter::Antialiasing);
        if (!src.isNull()) {
            const QPixmap scaled = src.scaledToHeight(artH, Qt::SmoothTransformation);
            const int sx = qMax(0, (scaled.width() - artW) / 2 - scaled.width() / 8);
            g.drawPixmap(0, 0, scaled, sx, 0, artW, artH);
        }
        QLinearGradient fade(0, artH * 0.45, 0, artH);
        fade.setColorAt(0, QColor(36, 35, 31, 0));
        fade.setColorAt(1, QColor(36, 35, 31, 235));
        g.fillRect(QRect(0, 0, artW, artH), fade);
        g.drawPixmap(28, artH - 116, renderSvg(QLatin1String(DROP_SVG), QSize(56, 56)));
        QFont f = g.font();
        f.setPixelSize(34);
        f.setWeight(QFont::Black);
        g.setFont(f);
        g.setPen(QColor(0xee, 0xea, 0xe0));
        g.drawText(QRect(96, artH - 118, artW - 110, 44), Qt::AlignLeft | Qt::AlignVCenter, QStringLiteral("Nanquim"));
        f.setPixelSize(13);
        f.setWeight(QFont::Normal);
        g.setFont(f);
        g.setPen(QColor(0xa8, 0xa3, 0x98));
        g.drawText(QRect(96, artH - 76, artW - 110, 22), Qt::AlignLeft | Qt::AlignVCenter, i18n("Desenho e quadrinhos no tablet"));
        f.setPixelSize(11);
        g.setFont(f);
        g.setPen(QColor(0x8a, 0x85, 0x7b));
        g.drawText(QRect(28, artH - 40, artW - 40, 20), Qt::AlignLeft | Qt::AlignVCenter, i18n("Arte: Tyson Tan · CC BY-SA"));
    }
    art->setPixmap(canvas);
    root->addWidget(art);

    QWidget *right = new QWidget(d);
    QVBoxLayout *rl = new QVBoxLayout(right);
    rl->setContentsMargins(34, 28, 28, 24);
    rl->setSpacing(14);
    rl->addLayout(sheetHeader(d, i18n("Sobre e créditos"), QString()));
    rl->addWidget(nqLabel(i18n("O Nanquim existe porque o Krita existe. Cada traço passa pelo motor de pintura que a "
                               "comunidade do Krita constrói há mais de vinte anos; aqui só mudamos a casca, para quem "
                               "desenha quadrinhos no tablet."), "nqBody", right, true));
    struct Credit { const char *badge; const char *name; const char *info; const char *license; };
    const Credit credits[] = {
        {"K", "Krita", "Motor de pintura, pincéis e formato .kra · comunidade KDE", "GNU GPL"},
        {"TT", "Tyson Tan", "Arte da abertura e a mascote Kiki", "CC BY-SA"},
        {"S", "Satoshi", "Fonte da interface · Indian Type Foundry (Fontshare)", "ITF FFL"},
        {"M", "Montserrat", "Textos dos balões · Julieta Ulanovsky", "SIL OFL"},
        {"N", "Nanquim", "Interface de tablet, capítulos, balões e time-lapse", "GNU GPL"},
    };
    for (const Credit &c : credits) {
        QWidget *row = new QWidget(right);
        row->setMinimumHeight(58);
        QHBoxLayout *l = new QHBoxLayout(row);
        l->setContentsMargins(0, 6, 0, 6);
        l->setSpacing(14);
        QLabel *badge = new QLabel(QString::fromUtf8(c.badge), row);
        badge->setFixedSize(42, 42);
        badge->setAlignment(Qt::AlignCenter);
        badge->setStyleSheet(QStringLiteral("background: #2e2d29; border-radius: 12px; color: #6fbfad; font-size: 14px; font-weight: 800;"));
        QVBoxLayout *t = new QVBoxLayout();
        t->setSpacing(2);
        t->addWidget(nqLabel(QString::fromUtf8(c.name), "nqItem", row));
        t->addWidget(nqLabel(i18n(c.info), "nqSmall", row, true));
        l->addWidget(badge);
        l->addLayout(t, 1);
        l->addWidget(nqLabel(QString::fromUtf8(c.license), "nqSmall", row));
        rl->addWidget(row);
        rl->addWidget(nqDivider(right));
    }
    rl->addStretch(1);
    QHBoxLayout *actions = new QHBoxLayout();
    actions->setSpacing(8);
    QPushButton *support = nqButton(i18n("Apoiar o Krita"), right, "nqPrimary");
    QPushButton *code = nqButton(i18n("Código do Nanquim"), right);
    QPushButton *krita = nqButton(i18n("Sobre o Krita"), right, "nqGhost");
    actions->addWidget(support);
    actions->addWidget(code);
    actions->addWidget(krita);
    actions->addStretch(1);
    rl->addLayout(actions);
    root->addWidget(right, 1);

    QObject::connect(support, &QPushButton::clicked, d, []() {
        QDesktopServices::openUrl(QUrl(QStringLiteral("https://krita.org/en/donations/")));
    });
    QObject::connect(code, &QPushButton::clicked, d, []() {
        QDesktopServices::openUrl(QUrl(QStringLiteral("https://github.com/isaiasdaniel290308-bot/Nanquim")));
    });
    QObject::connect(krita, &QPushButton::clicked, d, [mw, d]() {
        d->accept();
        if (QAction *a = act(mw, "help_about_app")) a->trigger();
    });
    d->open();
}

// ---------------------------------------------------------------------------
// Cor: roda de cor do Krita, tinta e correção, cinzas de mangá e cores recentes
// ---------------------------------------------------------------------------
const QString COLOR_DOCK_ID = QStringLiteral("NanquimColorDock");

void setForeground(KisMainWindow *mw, const QColor &c)
{
    KisViewManager *vm = mw->viewManager();
    if (!vm || !vm->canvasResourceProvider()) return;
    vm->canvasResourceProvider()->setFGColor(KoColor(c, KoColorSpaceRegistry::instance()->rgb8()));
}

// ---------------------------------------------------------------------------
// Roda de atalhos (versão A): botão lateral da S Pen ou toque longo na cor
// atual. Centro na ponta da caneta; camadas, matiz e triângulo de cor.
// ---------------------------------------------------------------------------
// pincéis usados por último (ids), o mais novo primeiro
QList<int> s_recentBrushes;
QVector<KisPaintOpPresetSP> s_wheelPresets;   // um por setor; nulo = borracha ou vazio

void trackRecentBrushes(KisMainWindow *mw)
{
    {
        const QStringList saved = KSharedConfig::openConfig()->group("Nanquim").readEntry("recentBrushes", QStringList());
        for (const QString &id : saved) if (id.toInt() > 0) s_recentBrushes << id.toInt();
    }
    KoCanvasResourceProvider *r = resources(mw);
    if (!r) return;
    QPointer<KisMainWindow> guard(mw);
    QObject::connect(r, &KoCanvasResourceProvider::canvasResourceChanged, mw, [guard](int key, const QVariant &) {
        if (key != KoCanvasResource::CurrentPaintOpPreset || !guard) return;
        KisViewManager *vm = guard->viewManager();
        KisPaintOpPresetSP p = vm && vm->canvasResourceProvider() ? vm->canvasResourceProvider()->currentPreset() : KisPaintOpPresetSP();
        if (!p || p->resourceId() < 0) return;
        s_recentBrushes.removeAll(p->resourceId());
        s_recentBrushes.prepend(p->resourceId());
        while (s_recentBrushes.size() > 12) s_recentBrushes.removeLast();
        QStringList out;
        for (int id : s_recentBrushes) out << QString::number(id);
        KSharedConfig::openConfig()->group("Nanquim").writeEntry("recentBrushes", out);
    });
}

void togglePanelFrom(KisMainWindow *mw, QDockWidget *dw, QWidget *anchor);

NanquimWheel *wheelFor(KisMainWindow *mw)
{
    NanquimWheel *wheel = mw->findChild<NanquimWheel*>(QStringLiteral("nanquimWheel"));
    if (wheel) return wheel;
    wheel = new NanquimWheel(mw);
    wheel->hide();
    QPointer<KisMainWindow> guard(mw);
    wheel->onColor = [guard](const QColor &c) { if (guard) setForeground(guard, c); };
    wheel->onBrush = [guard, wheel](int index) {
        if (!guard || index < 0 || index >= s_wheelPresets.size()) return;
        KisViewManager *vm = guard->viewManager();
        if (!vm || !vm->canvasResourceProvider()) return;
        QAction *erase = act(guard, "erase_action");
        if (QAction *brush = act(guard, "KritaShape/KisToolBrush")) brush->trigger();
        KisPaintOpPresetSP preset = s_wheelPresets[index];
        if (preset) {
            vm->canvasResourceProvider()->setPaintOpPreset(preset);
            if (erase && erase->isChecked()) erase->trigger();
        } else if (erase && !erase->isChecked()) {
            erase->trigger();   // borracha: o mesmo pincel com a borracha ligada
        }
        QTimer::singleShot(140, wheel, [wheel]() { wheel->hide(); });
    };
    wheel->onLayers = [guard, wheel]() {
        wheel->hide();
        if (!guard) return;
        QDockWidget *layers = findDock(guard, {QStringLiteral("KisLayerBox")});
        // âncora: o botão da barra tem nome estável (o tooltip é tradução)
        QToolButton *anchor = guard->findChild<QToolButton*>(QStringLiteral("nanquimPanel_KisLayerBox"));
        if (!anchor) {
            if (QToolBar *top = guard->findChild<QToolBar*>(TOP_ID)) {
                Q_FOREACH (QToolButton *b, top->findChildren<QToolButton*>()) {
                    if (b->toolTip() == i18n("Camadas")) anchor = b;
                }
            }
        }
        if (layers && !panelOpen(guard, layers)) togglePanelFrom(guard, layers, anchor);
    };
    wheel->onClose = [wheel]() { wheel->hide(); };
    return wheel;
}

bool openWheel(KisMainWindow *mw, const QPoint &mwPos)
{
    if (s_classic || !mw->activeView()) return false;
    KisViewManager *vm = mw->viewManager();
    if (!vm || !vm->image() || !vm->canvasResourceProvider()) return false;
    NanquimWheel *wheel = wheelFor(mw);
    if (wheel->isVisible()) {
        wheel->hide();
        return true;
    }
    // borracha no topo, três favoritos e quatro recentes (sem repetir)
    QVector<NanquimWheel::Brush> brushes;
    s_wheelPresets.clear();
    QAction *erase = act(mw, "erase_action");
    const bool erasing = erase && erase->isChecked();
    const KisPaintOpPresetSP current = vm->canvasResourceProvider()->currentPreset();
    int active = -1;
    {
        NanquimWheel::Brush eraser;
        eraser.name = i18n("Borracha");
        eraser.kind = NanquimWheel::Eraser;
        brushes << eraser;
        s_wheelPresets << KisPaintOpPresetSP();
        if (erasing) active = 0;
    }
    QList<int> used;
    auto add = [&](const KisPaintOpPresetSP &p, int kind) {
        if (!p || used.contains(p->resourceId())) return false;
        used << p->resourceId();
        const BrushItem item = itemForPreset(p);
        NanquimWheel::Brush b;
        b.name = item.name;
        b.size = QStringLiteral("%1 px").arg(qRound(p->settings() ? p->settings()->paintOpSize() : 0));
        b.thumb = brushThumb(p);
        b.kind = kind;
        if (!erasing && current && current->resourceId() == p->resourceId()) active = brushes.size();
        brushes << b;
        s_wheelPresets << p;
        return true;
    };
    refreshFavoriteIds();
    const QVector<KisPaintOpPresetSP> favs = favoritePresets();
    for (int i = 0, n = 0; i < favs.size() && n < 3; i++) if (add(favs[i], NanquimWheel::Favorite)) n++;
    for (int i = 0, n = 0; i < s_recentBrushes.size() && brushes.size() < NanquimWheel::SECTORS; i++) {
        KisPaintOpPresetSP p = presetForId(s_recentBrushes[i]);
        if (add(p, NanquimWheel::Recent)) n++;
    }
    // ainda sobrou lugar: mais favoritos
    for (int i = 0; i < favs.size() && brushes.size() < NanquimWheel::SECTORS; i++) add(favs[i], NanquimWheel::Favorite);
    while (brushes.size() < NanquimWheel::SECTORS) {
        brushes << NanquimWheel::Brush();
        s_wheelPresets << KisPaintOpPresetSP();
    }
    const KisNodeSP node = vm->nodeManager()->activeNode();
    wheel->setBrushes(brushes, active, node ? node->name() : QString());
    wheel->setColor(vm->canvasResourceProvider()->fgColor().toQColor());
    wheel->showAt(mwPos);
    return true;
}

// fecha a roda com um toque fora dela; o toque não vira traço
class WheelCloser : public QObject
{
public:
    explicit WheelCloser(KisMainWindow *mw) : QObject(mw), m_mw(mw) {}
protected:
    bool eventFilter(QObject *, QEvent *e) override
    {
        const QEvent::Type t = e->type();
        if (t == QEvent::ApplicationStateChange) m_eating = false;
        if (m_eating) {
            // segurança: a solta pode se perder (caneta saiu de proximidade etc.)
            if (t == QEvent::TabletLeaveProximity
                    || (m_eatStart.isValid() && m_eatStart.elapsed() > 2500)) {
                m_eating = false;
            } else if (t == QEvent::TabletMove || t == QEvent::MouseMove || t == QEvent::TouchUpdate
                    || t == QEvent::TabletPress || t == QEvent::TabletRelease || t == QEvent::MouseButtonRelease
                    || t == QEvent::TouchEnd || t == QEvent::TouchCancel) {
                if (t == QEvent::TabletRelease || t == QEvent::MouseButtonRelease || t == QEvent::TouchEnd
                        || t == QEvent::TouchCancel) {
                    if (t != QEvent::MouseButtonRelease || !m_waitMouse) {
                        m_eating = false;
                    }
                    m_waitMouse = false;
                }
                return true;
            }
        }
        if (t != QEvent::TabletPress && t != QEvent::MouseButtonPress && t != QEvent::TouchBegin) return false;
        if (!m_mw || QApplication::activePopupWidget() || QApplication::activeModalWidget()) return false;
        NanquimWheel *wheel = this->wheel();
        if (!wheel || !wheel->isVisible()) return false;
        QPoint global;
        if (t == QEvent::MouseButtonPress) {
            global = static_cast<QMouseEvent*>(e)->globalPos();
        } else if (t == QEvent::TabletPress) {
            global = static_cast<QTabletEvent*>(e)->globalPos();
        } else {
            const auto points = static_cast<QTouchEvent*>(e)->touchPoints();
            if (points.isEmpty()) return false;
            global = points.first().screenPos().toPoint();
        }
        const QPoint local = wheel->mapFromGlobal(global);
        if (wheel->hits(local)) return false;
        wheel->hide();
        if (t == QEvent::TabletPress && nanquimPenDevice(static_cast<QTabletEvent*>(e))) {
            return false;   // caneta: fechar a roda não rouba o traço
        }
        m_eating = true;
        m_waitMouse = t == QEvent::TabletPress;
        m_eatStart.start();
        return true;
    }
private:
    NanquimWheel *wheel()
    {
        if (m_mw && !m_wheel) m_wheel = m_mw->findChild<NanquimWheel*>(QStringLiteral("nanquimWheel"));
        return m_wheel;
    }
    QPointer<KisMainWindow> m_mw;
    QPointer<NanquimWheel> m_wheel;
    bool m_eating = false;
    bool m_waitMouse = false;
    QElapsedTimer m_eatStart;
};


QDockWidget *buildColorDock(KisMainWindow *mw)
{
    QDockWidget *dock = new QDockWidget(i18n("Cor"), mw);
    dock->setObjectName(COLOR_DOCK_ID);
    dock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable);
    dock->setTitleBarWidget(new QWidget(dock));
    QWidget *body = new QWidget(dock);
    body->setObjectName(QStringLiteral("nanquimColorBody"));
    body->setAttribute(Qt::WA_StyledBackground);
    body->setStyleSheet(QLatin1String(SHEET_STYLE) + QStringLiteral(
        "QWidget#nanquimColorBody { background: #2e2d29; }"
        "QWidget#nanquimColorBody QLabel { color: #eeeae0; background: transparent; }"));
    QVBoxLayout *root = new QVBoxLayout(body);
    root->setContentsMargins(18, 14, 18, 16);
    root->setSpacing(12);

    QHBoxLayout *head = new QHBoxLayout();
    head->addWidget(nqLabel(i18n("Cor"), "nqSection", body), 1);
    QPushButton *picker = nqButton(QString(), body, "nqGhost");
    picker->setIcon(designIcon(Svg::PICKER, 20));
    picker->setToolTip(i18n("Conta-gotas"));
    picker->setFixedWidth(48);
    head->addWidget(picker);
    root->addLayout(head);

    KisVisualColorSelector *selector = new KisVisualColorSelector(body);
    selector->setAcceptTabletEvents(true);
    selector->setDisplayRenderer(KisDisplayColorConverter::dumbConverterInstance()->displayRendererInterface());
    // redesenho: quadrado de saturação e brilho, com a faixa de matiz em cima
    selector->setConfig(false, false);
    {
        const KisColorSelectorConfiguration squareHue(KisColorSelectorConfiguration::Square,
                                                      KisColorSelectorConfiguration::Slider,
                                                      KisColorSelectorConfiguration::SV,
                                                      KisColorSelectorConfiguration::H);
        selector->setConfiguration(&squareHue);
        selector->setSliderPosition(Qt::TopEdge);
    }
    selector->setMinimumSize(220, 220);
    selector->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    root->addWidget(selector, 1);

    QHBoxLayout *hexRow = new QHBoxLayout();
    hexRow->setSpacing(10);
    QLabel *current = new QLabel(body);
    current->setFixedSize(36, 36);
    QLineEdit *hex = new QLineEdit(body);
    hex->setMaxLength(7);
    hexRow->addWidget(current);
    hexRow->addWidget(hex, 1);
    root->addLayout(hexRow);

    root->addWidget(nqLabel(i18n("Tinta e correção"), "nqSmall", body));
    QHBoxLayout *ink = new QHBoxLayout();
    ink->setSpacing(8);
    struct Swatch { QColor c; const char *tip; };
    const QList<Swatch> inks = {
        {QColor(0x15, 0x15, 0x15), "Preto nanquim"}, {QColor(0xfa, 0xfa, 0xf7), "Branco de correção"},
        {QColor(0x8f, 0xb3, 0xff), "Azul de rascunho"}, {QColor(0xe5, 0x48, 0x4d), "Vermelho de revisão"},
    };
    for (const Swatch &s : inks) {
        QPushButton *b = swatchButton(body, s.c, i18n(s.tip));
        const QColor c = s.c;
        QObject::connect(b, &QPushButton::clicked, mw, [mw, c]() { setForeground(mw, c); });
        ink->addWidget(b);
    }
    ink->addStretch(1);
    root->addLayout(ink);

    root->addWidget(nqLabel(i18n("Cinzas para mangá"), "nqSmall", body));
    QGridLayout *grays = new QGridLayout();
    grays->setSpacing(4);
    for (int i = 1; i <= 9; i++) {
        const int v = qRound(255 * (1.0 - i / 10.0));
        const QColor c(v, v, v);
        QPushButton *b = nqButton(QString(), body);
        b->setToolTip(i18n("Cinza %1%", i * 10));
        b->setFixedHeight(34);
        b->setMinimumWidth(24);
        b->setStyleSheet(QStringLiteral("QPushButton { background: %1; border: 1px solid #4a4842; border-radius: 6px; min-height: 34px; padding: 0; }")
                         .arg(c.name()));
        QObject::connect(b, &QPushButton::clicked, mw, [mw, c]() { setForeground(mw, c); });
        grays->addWidget(b, 0, i - 1);
    }
    root->addLayout(grays);

    // prévia: como o tom da cor atual fica em retícula (60 linhas, 45°)
    QHBoxLayout *toneRow = new QHBoxLayout();
    toneRow->setSpacing(12);
    QLabel *tonePreview = new QLabel(body);
    tonePreview->setFixedSize(64, 64);
    tonePreview->setStyleSheet(QStringLiteral("border: 1px solid #524f48; border-radius: 10px;"));
    QVBoxLayout *toneText = new QVBoxLayout();
    toneText->setSpacing(2);
    QLabel *toneTitle = nqLabel(QString(), "nqItem", body);
    toneText->addStretch(1);
    toneText->addWidget(toneTitle);
    toneText->addWidget(nqLabel(i18n("Como esta cor fica em retícula, para impressão em P&B"), "nqSmall", body, true));
    toneText->addStretch(1);
    toneRow->addWidget(tonePreview);
    toneRow->addLayout(toneText, 1);
    root->addLayout(toneRow);

    root->addWidget(nqLabel(i18n("Usadas há pouco"), "nqSmall", body));
    QHBoxLayout *recent = new QHBoxLayout();
    recent->setSpacing(8);
    QList<QPushButton*> recentButtons;
    for (int i = 0; i < 7; i++) {
        QPushButton *b = nqButton(QString(), body);
        b->setFixedSize(32, 32);
        recentButtons << b;
        recent->addWidget(b);
    }
    recent->addStretch(1);
    root->addLayout(recent);

    QPushButton *full = nqButton(i18n("Seletor completo do Krita"), body, "nqGhost");
    root->addWidget(full, 0, Qt::AlignLeft);
    dock->setWidget(body);

    auto recentColors = std::make_shared<QList<QColor>>();
    auto sync = [mw, selector, current, hex, recentButtons, recentColors, tonePreview, toneTitle]() {
        KisViewManager *vm = mw->viewManager();
        if (!vm || !vm->canvasResourceProvider()) return;
        const KoColor fg = vm->canvasResourceProvider()->fgColor();
        {
            QSignalBlocker block(selector);
            selector->slotSetColor(fg);
        }
        const QColor qc = fg.toQColor();
        current->setStyleSheet(QStringLiteral("background: %1; border-radius: 18px; border: 2px solid #eeeae0;").arg(qc.name()));
        if (!hex->hasFocus()) hex->setText(qc.name());
        {
            // cobertura = quanto a cor escurece o papel
            const qreal cover = qBound(0.0, 1.0 - qGray(qc.rgb()) / 255.0, 1.0);
            const qreal dpr = qApp->devicePixelRatio();
            QPixmap pm(QSize(62, 62) * dpr);
            pm.setDevicePixelRatio(dpr);
            pm.fill(QColor(0xfa, 0xfa, 0xf7));
            QPainter tp(&pm);
            tp.setRenderHint(QPainter::Antialiasing);
            tp.setPen(Qt::NoPen);
            tp.setBrush(QColor(0x1a, 0x1a, 0x1a));
            const qreal step = 7.0;
            const qreal r = qMin(step * 0.72, step * std::sqrt(cover / 3.14159265358979));
            tp.translate(31, 31);
            tp.rotate(45);
            if (r > 0.25) {
                for (qreal y = -48; y <= 48; y += step) {
                    for (qreal x = -48; x <= 48; x += step) tp.drawEllipse(QPointF(x, y), r, r);
                }
            }
            tp.end();
            tonePreview->setPixmap(pm);
            toneTitle->setText(i18n("Retícula de %1%", qRound(cover * 100)));
        }
        recentColors->clear();
        if (vm->paintOpBox() && vm->paintOpBox()->favoriteResourcesManager()) {
            KisFavoriteResourceManager *fav = vm->paintOpBox()->favoriteResourcesManager();
            for (int i = 0; i < fav->recentColorsTotal() && recentColors->size() < recentButtons.size(); i++) {
                *recentColors << fav->recentColorAt(i).toQColor();
            }
        }
        for (int i = 0; i < recentButtons.size(); i++) {
            const bool has = i < recentColors->size();
            recentButtons[i]->setVisible(has);
            if (has) {
                // a cor em uso fica maior, com anel ciano
                const bool cur = recentColors->at(i).rgb() == qc.rgb();
                const int d = cur ? 40 : 32;
                recentButtons[i]->setFixedSize(d, d);
                recentButtons[i]->setStyleSheet(QStringLiteral(
                    "QPushButton { background: %1; border: %2; border-radius: %3px; min-height: %4px; padding: 0; }")
                    .arg(recentColors->at(i).name())
                    .arg(cur ? QStringLiteral("3px solid #6fbfad") : QStringLiteral("1px solid #7a766d"))
                    .arg(d / 2).arg(d));
            }
        }
    };
    for (int i = 0; i < recentButtons.size(); i++) {
        QObject::connect(recentButtons[i], &QPushButton::clicked, mw, [mw, recentColors, i]() {
            if (i < recentColors->size()) setForeground(mw, recentColors->at(i));
        });
    }
    QObject::connect(selector, &KisVisualColorSelector::sigNewColor, mw, [mw](const KoColor &c) {
        KisViewManager *vm = mw->viewManager();
        if (vm && vm->canvasResourceProvider()) vm->canvasResourceProvider()->setFGColor(c);
    });
    QObject::connect(hex, &QLineEdit::editingFinished, mw, [mw, hex]() {
        const QColor c(hex->text().trimmed());
        if (c.isValid()) setForeground(mw, c);
    });
    QObject::connect(picker, &QPushButton::clicked, mw, [mw]() {
        if (QAction *a = act(mw, "KritaSelected/KisToolColorSampler")) a->trigger();
    });
    QObject::connect(full, &QPushButton::clicked, mw, [mw, dock]() {
        QDockWidget *krita = findDock(mw, {QStringLiteral("ColorSelectorNg"), QStringLiteral("WideGamutColorSelector")});
        if (krita) togglePanel(mw, krita);
        Q_UNUSED(dock);
    });
    if (KisViewManager *vm = mw->viewManager()) {
        if (vm->canvasResourceProvider()) {
            QObject::connect(vm->canvasResourceProvider(), &KisCanvasResourceProvider::sigFGColorChanged, dock,
                             [sync](const KoColor &) { sync(); });
        }
    }
    QObject::connect(dock, &QDockWidget::visibilityChanged, dock, [sync](bool on) { if (on) sync(); });

    mw->addDockWidget(Qt::RightDockWidgetArea, dock);
    dock->hide();
    return dock;
}

// ---------------------------------------------------------------------------
// Ajustes do pincel: o essencial do editor do Krita, num toque
// ---------------------------------------------------------------------------
struct PressureCurve { const char *name; const char *curve; };
const PressureCurve PRESSURE_CURVES[] = {
    {"Leve", "0,0;0.25,0.55;1,1;"},
    {"Normal", "0,0;1,1;"},
    {"Firme", "0,0;0.6,0.3;1,1;"},
};

int currentPressureCurve()
{
    const QString cur = KisConfig(true).pressureTabletCurve().remove(QLatin1Char(' '));
    for (int i = 0; i < 3; i++) {
        if (cur == QLatin1String(PRESSURE_CURVES[i].curve)) return i;
    }
    return -1;
}

void setPressureCurve(int index)
{
    if (index < 0 || index > 2) return;
    KisConfig cfg(false);
    cfg.setPressureTabletCurve(QLatin1String(PRESSURE_CURVES[index].curve));
    KisConfigNotifier::instance()->notifyConfigChanged();
}

// curva de pressão desenhada (só para mostrar)
class CurvePreview : public QWidget
{
public:
    explicit CurvePreview(QWidget *parent) : QWidget(parent) { setFixedSize(220, 150); }
    void setCurve(const QString &curve) { m_curve = curve; update(); }
protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter g(this);
        g.setRenderHint(QPainter::Antialiasing);
        const QRectF r = QRectF(rect()).adjusted(1, 1, -1, -1);
        g.setPen(QPen(QColor(0x4a, 0x48, 0x42), 1));
        g.setBrush(QColor(0x2e, 0x2d, 0x29));
        g.drawRoundedRect(r, 12, 12);
        for (int i = 1; i < 4; i++) {
            g.drawLine(QPointF(r.left() + r.width() * i / 4, r.top() + 6), QPointF(r.left() + r.width() * i / 4, r.bottom() - 6));
            g.drawLine(QPointF(r.left() + 6, r.top() + r.height() * i / 4), QPointF(r.right() - 6, r.top() + r.height() * i / 4));
        }
        const KisCubicCurve curve(m_curve.isEmpty() ? QStringLiteral("0,0;1,1;") : m_curve);
        const QRectF a = r.adjusted(14, 14, -14, -14);
        QPainterPath path;
        for (int i = 0; i <= 48; i++) {
            const qreal x = i / 48.0;
            const QPointF p(a.left() + x * a.width(), a.bottom() - qBound(0.0, curve.value(x), 1.0) * a.height());
            if (i == 0) path.moveTo(p); else path.lineTo(p);
        }
        g.setPen(QPen(QColor(0x6f, 0xbf, 0xad), 2.6, Qt::SolidLine, Qt::RoundCap));
        g.setBrush(Qt::NoBrush);
        g.drawPath(path);
    }
private:
    QString m_curve;
};

// Ajustes do pincel como painel (pendência 3): abre fixado, a página cede
// espaço e dá para testar o traço sem fechar nada. A prévia mostra a cor,
// o tamanho e a opacidade atuais com a curva de pressão escolhida.
const QString BRUSH_SETTINGS_DOCK_ID = QStringLiteral("NanquimBrushSettingsDock");

class StrokePreview : public QWidget
{
public:
    explicit StrokePreview(QWidget *parent) : QWidget(parent) { setFixedHeight(112); }
    void set(const QColor &c, qreal size, qreal opacity, const QString &curve)
    {
        m_color = c; m_size = size; m_opacity = opacity; m_curve = curve;
        update();
    }
protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter g(this);
        g.setRenderHint(QPainter::Antialiasing);
        const QRectF r = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
        g.setPen(Qt::NoPen);
        g.setBrush(QColor(0xfa, 0xfa, 0xf7));
        g.drawRoundedRect(r, 12, 12);
        const KisCubicCurve curve(m_curve.isEmpty() ? QStringLiteral("0,0;1,1;") : m_curve);
        // largura na prévia: proporcional, mas cabendo no quadro
        const qreal maxW = qBound(1.5, m_size * 0.5, 30.0);
        QColor c = m_color;
        c.setAlphaF(qBound(0.05, m_opacity, 1.0));
        const QRectF a = r.adjusted(22, 18, -22, -18);
        QPointF prev;
        const int n = 90;
        QImage layer(size() * devicePixelRatioF(), QImage::Format_ARGB32_Premultiplied);
        layer.setDevicePixelRatio(devicePixelRatioF());
        layer.fill(Qt::transparent);
        QPainter l(&layer);
        l.setRenderHint(QPainter::Antialiasing);
        QColor solid = m_color;
        solid.setAlpha(255);
        for (int i = 0; i <= n; i++) {
            const qreal t = qreal(i) / n;
            const QPointF p(a.left() + t * a.width(), a.center().y() + std::sin(t * M_PI * 2.0) * a.height() * 0.38);
            // pressão sobe e desce ao longo do traço, como uma mão de verdade
            const qreal pressure = std::sin(t * M_PI);
            const qreal w = qMax(0.8, maxW * qBound(0.0, curve.value(pressure), 1.0));
            if (i > 0) {
                l.setPen(QPen(solid, w, Qt::SolidLine, Qt::RoundCap));
                l.drawLine(prev, p);
            }
            prev = p;
        }
        l.end();
        g.setOpacity(c.alphaF());
        g.drawImage(QPointF(0, 0), layer);
    }
private:
    QColor m_color = Qt::black;
    qreal m_size = 10;
    qreal m_opacity = 1;
    QString m_curve;
};

// ---------------------------------------------------------------------------
// Estabilizador por pincel: liga o estabilizador do Krita na ferramenta Pincel
// e guarda a escolha com o nome do pincel (volta quando o pincel volta).
// ---------------------------------------------------------------------------
const QString STAB_GROUP = QStringLiteral("NanquimStabilizer");

qreal stabDistance(int strength) { return 10.0 + qBound(0, strength, 100) * 1.9; }

QString currentPresetName(KisMainWindow *mw)
{
    KisViewManager *vm = mw->viewManager();
    KisPaintOpPresetSP p = vm && vm->canvasResourceProvider() ? vm->canvasResourceProvider()->currentPreset() : KisPaintOpPresetSP();
    return p ? p->name() : QString();
}

// lê o estado: o guardado deste pincel ou, sem nada guardado, o da ferramenta
void readStabilizer(KisMainWindow *mw, bool *on, int *strength)
{
    KConfigGroup g = KSharedConfig::openConfig()->group(STAB_GROUP);
    const QString name = currentPresetName(mw);
    if (!name.isEmpty() && g.hasKey(name)) {
        const QStringList parts = g.readEntry(name, QString()).split(QLatin1Char(';'));
        *on = parts.value(0) == QLatin1String("1");
        *strength = qBound(0, parts.value(1, QStringLiteral("45")).toInt(), 100);
        return;
    }
    KisConfig cfg(true);
    *on = cfg.lineSmoothingType() == KisSmoothingOptions::STABILIZER;
    *strength = qBound(0, qRound((cfg.lineSmoothingDistanceMax() - 10.0) / 1.9), 100);
}

void applyStabilizer(KisMainWindow *mw, bool on, int strength)
{
    KisViewManager *vm = mw->viewManager();
    if (!vm || !vm->canvasBase()) return;
    KoToolBase *tool = KoToolManager::instance()->toolById(vm->canvasBase(), QStringLiteral("KritaShape/KisToolBrush"));
    if (!tool) return;
    tool->optionWidgets();   // o Pincel do Krita cria os controles sob demanda; os ajustes passam por eles
    tool->setProperty("smoothingType", on ? int(KisSmoothingOptions::STABILIZER) : int(KisSmoothingOptions::NO_SMOOTHING));
    if (on) tool->setProperty("smoothnessQualityMax", qRound(stabDistance(strength)));
}

void storeStabilizer(KisMainWindow *mw, bool on, int strength)
{
    const QString name = currentPresetName(mw);
    if (name.isEmpty()) return;
    KConfigGroup g = KSharedConfig::openConfig()->group(STAB_GROUP);
    g.writeEntry(name, QStringLiteral("%1;%2").arg(on ? 1 : 0).arg(strength));
}

// teste ao vivo: risque aqui; a linha cinza é a mão, a clara é com o estabilizador
class StabilizerPad : public QWidget
{
public:
    explicit StabilizerPad(QWidget *parent) : QWidget(parent)
    {
        setFixedHeight(120);
        setAttribute(Qt::WA_TabletTracking, false);
        setToolTip(i18n("Risque aqui para testar o traço"));
        // risco de exemplo até o primeiro teste
        for (int i = 0; i < 64; i++) {
            const qreal t = i / 63.0;
            const qreal r = std::sin((i + 1) * 12.9898) * 43758.5453;
            m_points << Pt{QPointF(t, 0.5 + 0.2 * std::sin(t * M_PI * 2.2 + 0.4) + (r - std::floor(r) - 0.5) * 0.13),
                           std::sin(t * M_PI) * 0.7 + 0.3};
        }
    }
    void set(bool on, int strength, qreal size, qreal opacity, const QColor &c)
    {
        m_on = on; m_strength = strength; m_size = size; m_opacity = opacity; m_color = c;
        update();
    }
protected:
    struct Pt { QPointF p; qreal pressure; };   // p em fração do quadro (0..1)

    void add(const QPointF &local, qreal pressure, bool start)
    {
        const QPointF f(qBound(0.0, local.x() / qMax(1, width()), 1.0), qBound(0.0, local.y() / qMax(1, height()), 1.0));
        if (start) m_points.clear();
        if (!m_points.isEmpty() && QLineF(m_points.last().p, f).length() < 0.003) return;
        m_points << Pt{f, pressure};
        update();
    }
    void tabletEvent(QTabletEvent *e) override
    {
        if (e->type() == QEvent::TabletPress) { m_down = true; add(e->posF(), e->pressure(), true); }
        else if (e->type() == QEvent::TabletMove && m_down) add(e->posF(), e->pressure(), false);
        else if (e->type() == QEvent::TabletRelease) m_down = false;
        e->accept();
    }
    void mousePressEvent(QMouseEvent *e) override { m_down = true; add(e->localPos(), 0.7, true); }
    void mouseMoveEvent(QMouseEvent *e) override { if (m_down) add(e->localPos(), 0.7, false); }
    void mouseReleaseEvent(QMouseEvent *) override { m_down = false; }
    void paintEvent(QPaintEvent *) override
    {
        QPainter g(this);
        g.setRenderHint(QPainter::Antialiasing);
        const QRectF r = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
        g.setPen(QPen(QColor(0x4a, 0x48, 0x42), 1));
        g.setBrush(QColor(0x24, 0x23, 0x1f));
        g.drawRoundedRect(r, 12, 12);
        if (m_points.size() < 2) {
            g.setPen(QColor(0x7a, 0x76, 0x6d));
            g.drawText(r, Qt::AlignCenter, i18n("Risque aqui"));
            return;
        }
        auto toWidget = [this](const QPointF &f) { return QPointF(f.x() * width(), f.y() * height()); };
        // mão
        QPainterPath raw;
        raw.moveTo(toWidget(m_points.first().p));
        for (int i = 1; i < m_points.size(); i++) raw.lineTo(toWidget(m_points[i].p));
        g.setPen(QPen(QColor(0x7a, 0x76, 0x6d), 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        g.setBrush(Qt::NoBrush);
        g.drawPath(raw);
        // com estabilizador: média dos pontos vizinhos, mais larga quanto mais força
        const int w = m_on ? 1 + qRound(m_strength / 100.0 * 18) : 0;
        QVector<QPointF> smooth;
        for (int i = 0; i < m_points.size(); i++) {
            if (!w) { smooth << toWidget(m_points[i].p); continue; }
            QPointF acc; int n = 0;
            for (int k = i - w; k <= i + w; k++) {
                if (k >= 0 && k < m_points.size()) { acc += m_points[k].p; n++; }
            }
            smooth << toWidget(acc / n);
        }
        QColor ink = m_color.lightnessF() < 0.25 ? QColor(0xee, 0xea, 0xe0) : m_color;
        ink.setAlphaF(qBound(0.15, m_opacity, 1.0));
        const qreal maxW = qBound(1.5, 2.0 + m_size * 0.15, 12.0);
        for (int i = 1; i < smooth.size(); i++) {
            g.setPen(QPen(ink, qMax(1.0, maxW * m_points[i].pressure), Qt::SolidLine, Qt::RoundCap));
            g.drawLine(smooth[i - 1], smooth[i]);
        }
    }
private:
    QVector<Pt> m_points;
    bool m_down = false;
    bool m_on = true;
    int m_strength = 45;
    qreal m_size = 10;
    qreal m_opacity = 1;
    QColor m_color = Qt::white;
};

// interruptor (liga/desliga) desenhado como no design
class NqSwitch : public QAbstractButton
{
public:
    explicit NqSwitch(QWidget *parent) : QAbstractButton(parent)
    {
        setCheckable(true);
        setFixedSize(52, 32);
        setFocusPolicy(Qt::NoFocus);
    }
    QSize sizeHint() const override { return QSize(52, 32); }
protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter g(this);
        g.setRenderHint(QPainter::Antialiasing);
        g.setPen(Qt::NoPen);
        if (isChecked()) {
            QLinearGradient grad(0, 0, width(), 0);
            grad.setColorAt(0, QColor(0x3e, 0x8e, 0x7e));
            grad.setColorAt(1, QColor(0x6f, 0xbf, 0xad));
            g.setBrush(grad);
        } else {
            g.setBrush(QColor(0x5a, 0x57, 0x4f));
        }
        g.drawRoundedRect(rect(), 16, 16);
        g.setBrush(isChecked() ? QColor(0x13, 0x23, 0x20) : QColor(0xee, 0xea, 0xe0));
        g.drawEllipse(QRectF(isChecked() ? 24 : 4, 4, 24, 24));
    }
};

QDockWidget *buildBrushSettingsDock(KisMainWindow *mw)
{
    QDockWidget *dock = new QDockWidget(i18n("Ajustes do pincel"), mw);
    dock->setObjectName(BRUSH_SETTINGS_DOCK_ID);
    dock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable);
    QScrollArea *scroll = new QScrollArea(dock);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QWidget *body = new QWidget(scroll);
    body->setObjectName(QStringLiteral("nanquimBrushSettingsBody"));
    body->setAttribute(Qt::WA_StyledBackground);
    body->setStyleSheet(QLatin1String(SHEET_STYLE) + QStringLiteral(
        "QWidget#nanquimBrushSettingsBody { background: #2e2d29; }"
        "QWidget#nanquimBrushSettingsBody QLabel { background: transparent; }"
        "QPushButton#nqRow { border: none; border-top: 1px solid #4a4842; border-radius: 0; background: transparent;"
        "  text-align: left; padding: 0; font-size: 16px; min-height: 52px; }"));
    scroll->setStyleSheet(QStringLiteral("QScrollArea { background: #2e2d29; border: none; }"));
    KisKineticScroller::createPreconfiguredScroller(scroll);
    QVBoxLayout *root = new QVBoxLayout(body);
    root->setContentsMargins(20, 6, 20, 16);
    root->setSpacing(10);
    body->setMinimumWidth(340);

    // teste ao vivo
    StabilizerPad *pad = new StabilizerPad(body);
    root->addWidget(pad);
    QLabel *legend = nqLabel(i18n("Cinza: sua mão · Claro: com o estabilizador"), "nqSmall", body, true);
    root->addWidget(legend);

    auto addSlider = [body, root](const QString &title, int min, int max, std::function<QString(int)> fmt) {
        QHBoxLayout *t = new QHBoxLayout();
        t->addWidget(nqLabel(title, "nqItem", body), 1);
        QLabel *v = nqLabel(QString(), "nqSmall", body);
        v->setStyleSheet(QStringLiteral("font-size: 14px; color: #a8a398;"));
        t->addWidget(v);
        QSlider *s = new QSlider(Qt::Horizontal, body);
        s->setRange(min, max);
        QObject::connect(s, &QSlider::valueChanged, v, [v, fmt](int x) { v->setText(fmt(x)); });
        QVBoxLayout *box = new QVBoxLayout();
        box->setSpacing(2);
        box->addLayout(t);
        box->addWidget(s);
        root->addLayout(box);
        return s;
    };
    QSlider *sizeS = addSlider(i18n("Tamanho"), 0, 1000, [](int v) { return QStringLiteral("%1 px").arg(qRound(sliderToSize(v))); });
    QSlider *opacityS = addSlider(i18n("Opacidade"), 1, 100, [](int v) { return QStringLiteral("%1%").arg(v); });
    QSlider *flowS = addSlider(i18n("Fluxo"), 1, 100, [](int v) { return QStringLiteral("%1%").arg(v); });

    // estabilizador
    root->addWidget(nqDivider(body));
    QHBoxLayout *stabHead = new QHBoxLayout();
    QVBoxLayout *stabText = new QVBoxLayout();
    stabText->setSpacing(1);
    QLabel *stabTitle = nqLabel(i18n("Estabilizador"), "nqItem", body);
    stabTitle->setStyleSheet(QStringLiteral("font-size: 16px;"));
    stabText->addWidget(stabTitle);
    stabText->addWidget(nqLabel(i18n("Alisa as tremidas da mão"), "nqSmall", body));
    stabHead->addLayout(stabText, 1);
    NqSwitch *stabOn = new NqSwitch(body);
    stabHead->addWidget(stabOn, 0, Qt::AlignVCenter);
    root->addLayout(stabHead);
    QWidget *stabBlock = new QWidget(body);
    QVBoxLayout *sb = new QVBoxLayout(stabBlock);
    sb->setContentsMargins(0, 0, 0, 0);
    sb->setSpacing(6);
    QButtonGroup *stabPresets = new QButtonGroup(stabBlock);
    QHBoxLayout *presetRow = new QHBoxLayout();
    presetRow->setSpacing(8);
    const int presetValues[] = {20, 45, 75};
    const char *presetNames[] = {"Leve", "Médio", "Forte"};
    for (int i = 0; i < 3; i++) {
        QPushButton *chip = nqChip(i18n(presetNames[i]), stabBlock, stabPresets, presetValues[i]);
        presetRow->addWidget(chip, 1);
    }
    stabPresets->setExclusive(false);
    sb->addLayout(presetRow);
    QHBoxLayout *strHead = new QHBoxLayout();
    strHead->addWidget(nqLabel(i18n("Força"), "nqItem", stabBlock), 1);
    QLabel *strVal = nqLabel(QString(), "nqSmall", stabBlock);
    strVal->setStyleSheet(QStringLiteral("font-size: 14px; color: #a8a398;"));
    strHead->addWidget(strVal);
    sb->addLayout(strHead);
    QSlider *strength = new QSlider(Qt::Horizontal, stabBlock);
    strength->setRange(0, 100);
    sb->addWidget(strength);
    sb->addWidget(nqLabel(i18n("Mais força deixa a linha mais lisa, mas o traço demora um pouco a alcançar a caneta. "
                               "Vale para este pincel."), "nqSmall", stabBlock, true));
    root->addWidget(stabBlock);

    // curva de pressão (recolhida)
    QPushButton *curveRow = new QPushButton(i18n("Curva de pressão"), body);
    curveRow->setObjectName(QStringLiteral("nqRow"));
    curveRow->setCheckable(true);
    curveRow->setIcon(designIcon(Svg::FORWARD, 20));
    curveRow->setLayoutDirection(Qt::RightToLeft);
    root->addWidget(curveRow);
    QWidget *curveBlock = new QWidget(body);
    QVBoxLayout *cb = new QVBoxLayout(curveBlock);
    cb->setContentsMargins(0, 0, 0, 0);
    QButtonGroup *handGroup = new QButtonGroup(curveBlock);
    cb->addLayout(chipRow(curveBlock, handGroup, {i18n("Leve"), i18n("Normal"), i18n("Firme")}, currentPressureCurve()));
    cb->addWidget(nqLabel(i18n("Força da caneta. Vale para todos os pincéis."), "nqSmall", curveBlock, true));
    curveBlock->hide();
    root->addWidget(curveBlock);
    QObject::connect(curveRow, &QPushButton::toggled, curveBlock, [curveBlock, curveRow](bool on) {
        curveBlock->setVisible(on);
        curveRow->setIcon(designIcon(on ? "<path d='M6 9l6 6 6-6'/>" : Svg::FORWARD, 20));
    });

    // rodapé
    root->addWidget(nqDivider(body));
    QPushButton *restore = nqButton(i18n("Restaurar"), body);
    QPushButton *save = nqButton(i18n("Salvar pincel"), body, "nqPrimary");
    QHBoxLayout *row = new QHBoxLayout();
    row->setSpacing(12);
    row->addStretch(1);
    row->addWidget(restore);
    row->addWidget(save);
    root->addLayout(row);
    QPushButton *saveNew = nqButton(i18n("Salvar como novo"), body, "nqGhost");
    QPushButton *full = nqButton(i18n("Editor completo do Krita"), body, "nqGhost");
    QHBoxLayout *row2 = new QHBoxLayout();
    row2->addWidget(saveNew);
    row2->addStretch(1);
    row2->addWidget(full);
    root->addLayout(row2);
    root->addStretch(1);
    scroll->setWidget(body);
    dock->setWidget(scroll);

    QPointer<KisMainWindow> guard(mw);
    QSharedPointer<bool> pulling(new bool(false));
    auto refreshPad = [guard, pad, sizeS, opacityS, stabOn, strength]() {
        if (!guard) return;
        KisViewManager *vm = guard->viewManager();
        const QColor c = vm && vm->canvasResourceProvider() ? vm->canvasResourceProvider()->fgColor().toQColor() : QColor(Qt::white);
        pad->set(stabOn->isChecked(), strength->value(), sliderToSize(sizeS->value()), opacityS->value() / 100.0, c);
    };
    auto syncPresets = [stabPresets, strength, stabOn]() {
        Q_FOREACH (QAbstractButton *b, stabPresets->buttons()) {
            QSignalBlocker block(b);
            b->setChecked(stabOn->isChecked() && stabPresets->id(b) == strength->value());
        }
    };
    auto pull = [guard, dock, sizeS, opacityS, flowS, stabOn, strength, stabBlock, strVal, pulling, refreshPad, syncPresets]() {
        if (!guard) return;
        KisViewManager *vm = guard->viewManager();
        if (!vm || !vm->canvasResourceProvider()) return;
        KoCanvasResourceProvider *res = resources(guard);
        QString n = currentPresetName(guard);
        n.replace(QLatin1Char('_'), QLatin1Char(' '));
        dock->setWindowTitle(n.isEmpty() ? i18n("Ajustes do pincel") : n);
        const qreal size = res ? res->resource(KoCanvasResource::Size).toReal() : 10.0;
        const qreal opacity = res ? res->resource(KoCanvasResource::Opacity).toReal() : 1.0;
        const qreal flow = res ? res->resource(KoCanvasResource::Flow).toReal() : 1.0;
        bool on = false;
        int str = 45;
        readStabilizer(guard, &on, &str);
        // os rótulos acompanham; o pincel não é tocado enquanto lê
        *pulling = true;
        sizeS->setValue(sizeToSlider(size));
        opacityS->setValue(qRound(opacity * 100));
        flowS->setValue(qRound(flow * 100));
        stabOn->setChecked(on);
        strength->setValue(str);
        *pulling = false;
        strVal->setText(QStringLiteral("%1%").arg(str));
        stabBlock->setVisible(on);
        syncPresets();
        refreshPad();
    };
    auto stabChanged = [guard, stabOn, strength, stabBlock, strVal, pulling, refreshPad, syncPresets]() {
        strVal->setText(QStringLiteral("%1%").arg(strength->value()));
        stabBlock->setVisible(stabOn->isChecked());
        syncPresets();
        refreshPad();
        if (*pulling || !guard) return;
        storeStabilizer(guard, stabOn->isChecked(), strength->value());
        applyStabilizer(guard, stabOn->isChecked(), strength->value());
    };
    QObject::connect(stabOn, &QAbstractButton::toggled, dock, [stabChanged](bool) { stabChanged(); });
    QObject::connect(strength, &QSlider::valueChanged, dock, [stabChanged](int) { stabChanged(); });
    QObject::connect(stabPresets, &QButtonGroup::idClicked, dock, [strength, stabOn](int v) {
        stabOn->setChecked(true);
        strength->setValue(v);
    });
    QObject::connect(sizeS, &QSlider::valueChanged, mw, [mw, pulling, refreshPad](int v) {
        refreshPad();
        if (*pulling) return;
        if (KisViewManager *m = mw->viewManager()) m->canvasResourceProvider()->setSize(sliderToSize(v));
    });
    QObject::connect(opacityS, &QSlider::valueChanged, mw, [mw, pulling, refreshPad](int v) {
        refreshPad();
        if (*pulling) return;
        if (KisViewManager *m = mw->viewManager()) m->canvasResourceProvider()->setOpacity(v / 100.0);
    });
    QObject::connect(flowS, &QSlider::valueChanged, mw, [mw, pulling](int v) {
        if (*pulling) return;
        if (KisViewManager *m = mw->viewManager()) m->canvasResourceProvider()->setFlow(v / 100.0);
    });
    if (KoCanvasResourceProvider *r = resources(mw)) {
        QObject::connect(r, &KoCanvasResourceProvider::canvasResourceChanged, dock, [pull](int key, const QVariant &) {
            if (key == KoCanvasResource::Size || key == KoCanvasResource::Opacity || key == KoCanvasResource::Flow
                    || key == KoCanvasResource::CurrentPaintOpPreset || key == KoCanvasResource::ForegroundColor) {
                pull();
            }
        });
    }
    QObject::connect(handGroup, &QButtonGroup::idClicked, dock, [pull](int id) {
        setPressureCurve(id);
        pull();
    });
    QObject::connect(dock, &QDockWidget::visibilityChanged, dock, [pull](bool on) { if (on) pull(); });

    QObject::connect(full, &QPushButton::clicked, mw, [mw, dock]() {
        closePanel(mw, dock, false);
        QTimer::singleShot(150, mw, [mw]() { openBrushEditor(mw); });
    });
    QObject::connect(restore, &QPushButton::clicked, mw, [mw, pull]() {
        if (QAction *a = act(mw, "reload_preset_action")) a->trigger();
        QTimer::singleShot(50, mw, pull);
    });
    QObject::connect(saveNew, &QPushButton::clicked, mw, [mw]() {
        if (saveCurrentAsNewBrush(mw)) showToast(mw, i18n("Pincel novo guardado nos Importados"));
    });
    QObject::connect(save, &QPushButton::clicked, mw, [mw]() {
        KisViewManager *m = mw->viewManager();
        KisPaintOpPresetSP p = m && m->canvasResourceProvider() ? m->canvasResourceProvider()->currentPreset() : KisPaintOpPresetSP();
        if (p && KisResourceUserOperations::updateResourceWithUserInput(mw, p)) {
            showToast(mw, i18n("Pincel salvo"));
        }
    });
    dock->hide();
    mw->addDockWidget(panelArea(), dock);
    dock->hide();
    pull();
    return dock;
}

// cada pincel volta com o próprio estabilizador (se ele tiver um guardado)
void installStabilizerPerBrush(KisMainWindow *mw)
{
    KoCanvasResourceProvider *r = resources(mw);
    if (!r) return;
    QPointer<KisMainWindow> guard(mw);
    QObject::connect(r, &KoCanvasResourceProvider::canvasResourceChanged, mw, [guard](int key, const QVariant &) {
        if (key != KoCanvasResource::CurrentPaintOpPreset || !guard) return;
        const QString name = currentPresetName(guard);
        if (name.isEmpty() || !KSharedConfig::openConfig()->group(STAB_GROUP).hasKey(name)) return;
        bool on = false;
        int str = 45;
        readStabilizer(guard, &on, &str);
        applyStabilizer(guard, on, str);
    });
}

void showBrushSettings(KisMainWindow *mw)
{
    QDockWidget *dock = mw->findChild<QDockWidget*>(BRUSH_SETTINGS_DOCK_ID);
    if (!dock) {
        // primeira vez: abre fixado (o artista testa o traço com ele aberto)
        KConfigGroup cfg = KSharedConfig::openConfig()->group("Nanquim");
        const QByteArray key = QStringLiteral("pinned_%1").arg(BRUSH_SETTINGS_DOCK_ID).toUtf8();
        if (!cfg.hasKey(key.constData())) cfg.writeEntry(key.constData(), true);
        dock = buildBrushSettingsDock(mw);
    }
    if (!panelOpen(mw, dock)) togglePanelFrom(mw, dock, mw->findChild<QToolButton*>(QStringLiteral("nanquimBrushButton")));
}

// ---------------------------------------------------------------------------
// Nova página: formatos de quadrinho, resolução, papel e como começar
// ---------------------------------------------------------------------------
struct PageFormat { const char *name; const char *detail; double wmm; double hmm; int wpx; int hpx; };
const PageFormat PAGE_FORMATS[] = {
    {"Mangá B4", "257 × 364 mm", 257, 364, 0, 0},
    {"Mangá A4", "210 × 297 mm", 210, 297, 0, 0},
    {"Mangá B5", "182 × 257 mm · tankobon", 182, 257, 0, 0},
    {"Comic americano", "6,63 × 10,25 pol", 168.3, 260.4, 0, 0},
    {"Webtoon", "800 × 12800 px · rolagem", 0, 0, 800, 12800},
    {"Quadrado", "2160 × 2160 px · redes", 0, 0, 2160, 2160},
    {"Personalizado", "você escolhe", 0, 0, 0, 0},
};

// capa de capítulo para a janela de novo capítulo: três folhas com requadros
QPixmap chapterCoverArt(int w)
{
    const int h = qRound(w * 1.414);
    const qreal dpr = qApp->devicePixelRatio();
    QPixmap pm(QSize(w + 14, h + 14) * dpr);
    pm.setDevicePixelRatio(dpr);
    pm.fill(Qt::transparent);
    QPainter g(&pm);
    g.setRenderHint(QPainter::Antialiasing);
    const QRectF front(0, 14, w, h);
    g.setPen(Qt::NoPen);
    g.setBrush(QColor(0xbd, 0xbd, 0xb6));
    g.drawRoundedRect(front.translated(14, -14), 6, 6);
    g.setBrush(QColor(0xda, 0xda, 0xd3));
    g.drawRoundedRect(front.translated(7, -7), 6, 6);
    g.setBrush(QColor(0, 0, 0, 70));
    g.drawRoundedRect(front.translated(0, 4), 6, 6);
    g.setBrush(QColor(0xfa, 0xfa, 0xf7));
    g.drawRoundedRect(front, 6, 6);
    g.setPen(QPen(QColor(0x15, 0x15, 0x15), 1.6));
    g.setBrush(Qt::NoBrush);
    const qreal m = w * 0.08;
    const QRectF inner = front.adjusted(m, m, -m, -m);
    const qreal gap = w * 0.04;
    const qreal top = inner.height() * 0.42;
    g.drawRect(QRectF(inner.left(), inner.top(), inner.width(), top));
    g.drawRect(QRectF(inner.left(), inner.top() + top + gap, inner.width() * 0.45, inner.height() - top - gap));
    g.drawRect(QRectF(inner.left() + inner.width() * 0.45 + gap, inner.top() + top + gap,
                      inner.width() * 0.55 - gap, inner.height() - top - gap));
    return pm;
}

void showChapterSheet(KisMainWindow *mw, const QString &chapterId, std::function<void(const QString &)> done)
{
    const bool editing = !chapterId.isEmpty();
    const ChapterStore::Chapter existing = ChapterStore::instance()->chapter(chapterId);
    QDialog *d = makeSheet(mw, 720);
    d->setStyleSheet(d->styleSheet() + QStringLiteral(
        "QFrame#nqSideArt { background: #2e2d29; border: none; border-right: 1px solid #4a4842;"
        "  border-top-left-radius: 18px; border-bottom-left-radius: 18px; }"
        "QLineEdit#nqNameField { min-height: 46px; font-size: 16px; border: 1.5px solid #3e8e7e; background: #24231f; }"));
    QHBoxLayout *root = new QHBoxLayout(d);
    root->setContentsMargins(1, 1, 1, 1);
    root->setSpacing(0);

    QFrame *side = new QFrame(d);
    side->setObjectName(QStringLiteral("nqSideArt"));
    side->setFixedWidth(220);
    QVBoxLayout *sl = new QVBoxLayout(side);
    sl->setContentsMargins(16, 28, 16, 28);
    sl->setSpacing(10);
    QLabel *cover = new QLabel(side);
    cover->setPixmap(chapterCoverArt(120));
    cover->setAlignment(Qt::AlignCenter);
    QLabel *coverName = nqLabel(QString(), "nqItem", side);
    coverName->setAlignment(Qt::AlignCenter);
    QLabel *coverInfo = nqLabel(QString(), "nqSmall", side);
    coverInfo->setAlignment(Qt::AlignCenter);
    sl->addStretch(1);
    sl->addWidget(cover);
    sl->addWidget(coverName);
    sl->addWidget(coverInfo);
    sl->addStretch(1);
    root->addWidget(side);

    QWidget *right = new QWidget(d);
    QVBoxLayout *rl = new QVBoxLayout(right);
    rl->setContentsMargins(26, 22, 22, 22);
    rl->setSpacing(12);
    rl->addLayout(sheetHeader(d, editing ? i18n("Capítulo") : i18n("Novo capítulo"),
                              editing ? i18n("Renomear e escolher o formato") : i18n("Dê um nome ao capítulo")));
    rl->addWidget(nqLabel(i18n("Nome"), "nqSmall", right));
    QLineEdit *name = new QLineEdit(right);
    name->setObjectName(QStringLiteral("nqNameField"));
    name->setText(editing ? existing.name : i18n("Capítulo %1", ChapterStore::instance()->chapters().size() + 1));
    name->selectAll();
    rl->addWidget(name);
    rl->addSpacing(4);
    rl->addWidget(nqLabel(i18n("Formato das páginas novas"), "nqSmall", right));
    QButtonGroup *formatGroup = new QButtonGroup(right);
    QStringList labels;
    for (int i = 0; i < CHAPTER_FORMAT_COUNT; i++) labels << i18n(CHAPTER_FORMATS[i].name);
    rl->addLayout(chipRow(right, formatGroup, labels, editing ? ChapterStore::instance()->format(chapterId) : 0));
    rl->addWidget(nqLabel(i18n("Usado pelo botão \"+ Nova página\" de dentro do capítulo. Dá para mudar depois."),
                          "nqHint", right, true));
    rl->addStretch(1);
    QHBoxLayout *foot = new QHBoxLayout();
    foot->addStretch(1);
    QPushButton *cancel = nqButton(i18n("Cancelar"), right, "nqGhost");
    QPushButton *ok = nqButton(editing ? i18n("Salvar") : i18n("Criar capítulo"), right, "nqPrimary");
    foot->addWidget(cancel);
    foot->addWidget(ok);
    rl->addLayout(foot);
    root->addWidget(right, 1);

    auto updatePreview = [name, coverName, coverInfo, formatGroup, existing, editing]() {
        coverName->setText(name->text().trimmed().isEmpty() ? i18n("Sem nome") : name->text().trimmed());
        const int f = qMax(0, formatGroup->checkedId());
        const int n = editing ? existing.pages.size() : 0;
        coverInfo->setText(n == 1 ? i18n("1 página · %1", i18n(CHAPTER_FORMATS[f].name))
                                  : i18n("%1 páginas · %2", n, i18n(CHAPTER_FORMATS[f].name)));
    };
    updatePreview();
    QObject::connect(name, &QLineEdit::textChanged, d, [updatePreview](const QString &) { updatePreview(); });
    QObject::connect(formatGroup, &QButtonGroup::idClicked, d, [updatePreview](int) { updatePreview(); });
    QObject::connect(cancel, &QPushButton::clicked, d, &QDialog::reject);
    auto accept = [d, name, formatGroup, chapterId, editing, done]() {
        const QString n = name->text().trimmed();
        if (n.isEmpty()) {
            name->setFocus();
            return;
        }
        const int f = qMax(0, formatGroup->checkedId());
        QString id = chapterId;
        if (editing) {
            ChapterStore::instance()->rename(id, n);
        } else {
            id = ChapterStore::instance()->create(n);
        }
        ChapterStore::instance()->setFormat(id, f);
        d->accept();
        if (done) done(id);
    };
    QObject::connect(ok, &QPushButton::clicked, d, accept);
    QObject::connect(name, &QLineEdit::returnPressed, d, accept);
    d->open();
    name->setFocus();
}

void showComicsSheet(KisMainWindow *mw, int tab);

void showNewPage(KisMainWindow *mw)
{
    QDialog *d = makeSheet(mw, 1060, 660);
    QVBoxLayout *root = new QVBoxLayout(d);
    root->setContentsMargins(26, 22, 26, 20);
    root->setSpacing(14);
    root->addLayout(sheetHeader(d, i18n("Nova página"), i18n("Escolha o formato")));

    QHBoxLayout *mid = new QHBoxLayout();
    mid->setSpacing(22);
    QGridLayout *grid = new QGridLayout();
    grid->setSpacing(12);
    QButtonGroup *formats = new QButtonGroup(d);
    const int nFormats = int(sizeof(PAGE_FORMATS) / sizeof(PAGE_FORMATS[0]));
    for (int i = 0; i < nFormats; i++) {
        const PageFormat &f = PAGE_FORMATS[i];
        const double ratio = f.wmm > 0 ? f.wmm / f.hmm : (f.wpx > 0 ? double(f.wpx) / f.hpx : 0.8);
        PaperTile *card = new PaperTile(i18n(f.name), i18n(f.detail), ratio, true, d);
        card->setCheckable(true);
        formats->addButton(card, i);
        grid->addWidget(card, i / 3, i % 3);
    }
    QVBoxLayout *gridWrap = new QVBoxLayout();
    gridWrap->addLayout(grid);
    gridWrap->addStretch(1);
    mid->addLayout(gridWrap, 1);

    QFrame *side = new QFrame(d);
    side->setObjectName(QStringLiteral("nqPanel"));
    side->setFixedWidth(340);
    QVBoxLayout *sl = new QVBoxLayout(side);
    sl->setContentsMargins(18, 16, 18, 16);
    sl->setSpacing(10);
    QHBoxLayout *sizeRow = new QHBoxLayout();
    QSpinBox *wBox = new QSpinBox(side);
    QSpinBox *hBox = new QSpinBox(side);
    for (QSpinBox *s : {wBox, hBox}) {
        s->setRange(64, 30000);
        s->setSuffix(QStringLiteral(" px"));
        s->setButtonSymbols(QAbstractSpinBox::NoButtons);
    }
    QVBoxLayout *wl = new QVBoxLayout();
    wl->addWidget(nqLabel(i18n("Largura"), "nqSmall", side));
    wl->addWidget(wBox);
    QVBoxLayout *hl = new QVBoxLayout();
    hl->addWidget(nqLabel(i18n("Altura"), "nqSmall", side));
    hl->addWidget(hBox);
    sizeRow->addLayout(wl);
    sizeRow->addLayout(hl);
    sl->addLayout(sizeRow);

    sl->addWidget(nqLabel(i18n("Resolução"), "nqSmall", side));
    QButtonGroup *dpiGroup = new QButtonGroup(side);
    sl->addLayout(chipRow(side, dpiGroup, {QStringLiteral("300 dpi"), QStringLiteral("600 dpi"), i18n("Tela")}, 1));

    sl->addWidget(nqLabel(i18n("Papel"), "nqSmall", side));
    QHBoxLayout *paperRow = new QHBoxLayout();
    paperRow->setSpacing(8);
    QButtonGroup *paperGroup = new QButtonGroup(side);
    const QList<QPair<QColor, QString>> papers = {
        {QColor(Qt::white), i18n("Branco")}, {QColor(0xf3, 0xec, 0xdc), i18n("Creme")}, {QColor(0xdd, 0xdd, 0xdd), i18n("Transparente")}};
    for (int i = 0; i < papers.size(); i++) {
        QPushButton *b = swatchButton(side, papers[i].first, papers[i].second);
        b->setCheckable(true);
        b->setChecked(i == 0);
        if (i == 2) {
            b->setStyleSheet(QStringLiteral("background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #ffffff, stop:0.49 #ffffff,"
                                            " stop:0.5 #b8b8b8, stop:1 #b8b8b8);"));
        }
        paperGroup->addButton(b, i);
        paperRow->addWidget(b);
    }
    paperRow->addStretch(1);
    sl->addLayout(paperRow);

    sl->addWidget(nqLabel(i18n("Começar com"), "nqSmall", side));
    QCheckBox *draft = new QCheckBox(i18n("Camada de rascunho"), side);
    draft->setChecked(true);
    QCheckBox *guides = new QCheckBox(i18n("Guias de sangria e área segura"), side);
    guides->setChecked(true);
    QCheckBox *panels = new QCheckBox(i18n("Escolher requadros depois"), side);
    sl->addWidget(draft);
    sl->addWidget(guides);
    sl->addWidget(panels);

    sl->addWidget(nqLabel(i18n("Capítulo"), "nqSmall", side));
    QComboBox *chapter = new QComboBox(side);
    chapter->addItem(i18n("Nenhum"), QString());
    Q_FOREACH (const ChapterStore::Chapter &c, ChapterStore::instance()->chapters()) {
        chapter->addItem(i18n("%1 · página %2", c.name, c.pages.size() + 1), c.id);
        if (c.id == s_pendingChapter) chapter->setCurrentIndex(chapter->count() - 1);
    }
    sl->addWidget(chapter);
    sl->addStretch(1);
    mid->addWidget(side);
    root->addLayout(mid, 1);

    QHBoxLayout *foot = new QHBoxLayout();
    QLabel *estimate = nqLabel(QString(), "nqSmall", d, true);
    foot->addWidget(estimate, 1);
    QPushButton *cancel = nqButton(i18n("Cancelar"), d, "nqGhost");
    QPushButton *create = nqButton(i18n("Criar página"), d, "nqPrimary");
    foot->addWidget(cancel);
    foot->addWidget(create);
    root->addLayout(foot);

    auto dpiValue = [dpiGroup]() { const int id = dpiGroup->checkedId(); return id == 0 ? 300 : id == 1 ? 600 : 72; };
    auto updating = std::make_shared<bool>(false);
    auto applyFormat = [formats, wBox, hBox, dpiValue, updating]() {
        const int i = formats->checkedId();
        if (i < 0) return;
        const PageFormat &f = PAGE_FORMATS[i];
        *updating = true;
        if (f.wmm > 0) {
            wBox->setValue(qRound(f.wmm / 25.4 * dpiValue()));
            hBox->setValue(qRound(f.hmm / 25.4 * dpiValue()));
        } else if (f.wpx > 0) {
            wBox->setValue(f.wpx);
            hBox->setValue(f.hpx);
        }
        *updating = false;
    };
    auto updateEstimate = [wBox, hBox, draft, estimate]() {
        const qint64 layers = draft->isChecked() ? 3 : 2;
        const double mb = double(wBox->value()) * hBox->value() * 4 * (layers + 1) / (1024.0 * 1024.0);
        // só avisa quando pesa: a conta de sempre não ajuda a escolher
        estimate->setText(mb > 900
            ? i18n("Cerca de %1 MB abertos. É pesado para o tablet: prefira 300 dpi ou um formato menor.", qRound(mb))
            : QString());
    };
    QObject::connect(formats, &QButtonGroup::idClicked, d, [applyFormat, updateEstimate](int) { applyFormat(); updateEstimate(); });
    QObject::connect(dpiGroup, &QButtonGroup::idClicked, d, [applyFormat, updateEstimate](int) { applyFormat(); updateEstimate(); });
    for (QSpinBox *s : {wBox, hBox}) {
        QObject::connect(s, QOverload<int>::of(&QSpinBox::valueChanged), d, [formats, updating, updateEstimate, nFormats](int) {
            if (!*updating) {
                if (QAbstractButton *custom = formats->button(nFormats - 1)) custom->setChecked(true);
            }
            updateEstimate();
        });
    }
    QObject::connect(draft, &QCheckBox::toggled, d, [updateEstimate](bool) { updateEstimate(); });
    formats->button(0)->setChecked(true);
    applyFormat();
    updateEstimate();

    QObject::connect(cancel, &QPushButton::clicked, d, &QDialog::reject);
    QObject::connect(create, &QPushButton::clicked, d, [=]() {
        const int w = wBox->value(), h = hBox->value();
        const int dpi = dpiValue();
        const int paper = paperGroup->checkedId();
        const QString chapterId = chapter->currentData().toString();
        const int fmt = qMax(0, formats->checkedId());
        const bool wantDraft = draft->isChecked();
        const bool wantGuides = guides->isChecked();
        const bool wantPanels = panels->isChecked();
        d->accept();

        const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
        QColor bgColor = paper == 1 ? QColor(0xf3, 0xec, 0xdc) : QColor(Qt::white);
        KoColor bg(bgColor, cs);
        if (paper == 2) bg.setOpacity(OPACITY_TRANSPARENT_U8);
        KisDocument *doc = KisPart::instance()->createDocument();
        if (!doc->newImage(i18n(PAGE_FORMATS[fmt].name), w, h, cs, bg, KisConfig::RASTER_LAYER, 2, QString(), dpi / 72.0)) {
            delete doc;
            return;
        }
        KisImageSP image = doc->image();
        if (wantDraft && image) {
            KisPaintLayerSP layer = new KisPaintLayer(image, i18n("Rascunho"), OPACITY_OPAQUE_U8);
            image->addNode(layer, image->root());
        }
        if (wantGuides && image) {
            // sangria de 3 mm e área segura de 10 mm, em pontos (unidade do documento)
            const qreal ptW = w / qreal(dpi) * 72.0, ptH = h / qreal(dpi) * 72.0;
            const qreal bleed = 3.0 / 25.4 * 72.0, safe = 10.0 / 25.4 * 72.0;
            KisGuidesConfig g = doc->guidesConfig();
            g.setVerticalGuideLines({bleed, safe, ptW - safe, ptW - bleed});
            g.setHorizontalGuideLines({bleed, safe, ptH - safe, ptH - bleed});
            g.setShowGuides(true);
            g.setLockGuides(true);
            doc->setGuidesConfig(g);
        }
        doc->setModified(false);
        attachDocumentToChapter(doc, chapterId);
        KisPart::instance()->startCustomDocument(doc);
        if (wantPanels) {
            QTimer::singleShot(900, mw, [mw]() { if (mw->activeView()) showComicsSheet(mw, 0); });
        }
    });
    d->open();
}

// ---------------------------------------------------------------------------
// Exportar: página, capítulo ou time-lapse
// ---------------------------------------------------------------------------
QString publishExport(const QString &path, const QString &name, const QString &mime, QString *uriOut)
{
#ifdef Q_OS_ANDROID
    QAndroidJniObject result = QAndroidJniObject::callStaticObjectMethod(
        "org/krita/android/NanquimMedia", "publish",
        "(Landroid/app/Activity;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;",
        QtAndroid::androidActivity().object(),
        QAndroidJniObject::fromString(path).object<jstring>(),
        QAndroidJniObject::fromString(name).object<jstring>(),
        QAndroidJniObject::fromString(mime).object<jstring>());
    const QString r = result.isValid() ? result.toString() : QString();
    if (r.isEmpty()) return QString();
    const int nl = r.indexOf(QLatin1Char('\n'));
    if (uriOut) *uriOut = nl > 0 ? r.left(nl) : QString();
    return nl >= 0 ? r.mid(nl + 1) : r;
#else
    Q_UNUSED(mime);
    if (uriOut) uriOut->clear();
    QDir dir(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
    dir.mkpath(QStringLiteral("Nanquim"));
    const QString dst = dir.filePath(QStringLiteral("Nanquim/") + name);
    QFile::remove(dst);
    return QFile::copy(path, dst) ? dst : QString();
#endif
}

void shareExport(const QString &uri, const QString &mime)
{
#ifdef Q_OS_ANDROID
    if (uri.isEmpty()) return;
    QAndroidJniObject::callStaticMethod<void>(
        "org/krita/android/NanquimMedia", "share",
        "(Landroid/app/Activity;Ljava/lang/String;Ljava/lang/String;)V",
        QtAndroid::androidActivity().object(),
        QAndroidJniObject::fromString(uri).object<jstring>(),
        QAndroidJniObject::fromString(mime).object<jstring>());
#else
    Q_UNUSED(uri);
    Q_UNUSED(mime);
#endif
}

bool isDraftLayerName(const QString &name)
{
    const QString n = name.toLower();
    return n.contains(QLatin1String("rascunho")) || n.contains(QLatin1String("sketch")) || n.contains(QLatin1String("esboço"));
}

// imagem final da página, sem as camadas de rascunho se pedido
QImage renderPage(KisImageSP image, bool hideDraft)
{
    if (!image) return QImage();
    QList<KisNodeSP> hidden;
    if (hideDraft) {
        std::function<void(KisNodeSP)> walk = [&](KisNodeSP node) {
            for (KisNodeSP child = node->firstChild(); child; child = child->nextSibling()) {
                if (isDraftLayerName(child->name()) && child->visible()) {
                    child->setVisible(false);
                    child->setDirty();
                    hidden << child;
                } else {
                    walk(child);
                }
            }
        };
        walk(image->root());
    }
    image->waitForDone();
    QImage out = image->convertToQImage(image->bounds(), nullptr);
    Q_FOREACH (KisNodeSP node, hidden) {
        node->setVisible(true);
        node->setDirty();
    }
    return out;
}

QImage finishImage(QImage img, int sizeMode, bool blackWhite, bool opaque)
{
    if (img.isNull()) return img;
    if (sizeMode == 1) {
        img = img.scaled(img.width() / 2, img.height() / 2, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    } else if (sizeMode == 2 && qMax(img.width(), img.height()) > 2048) {
        img = img.scaled(2048, 2048, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    if (opaque || blackWhite) {
        QImage flat(img.size(), QImage::Format_RGB32);
        flat.fill(Qt::white);
        {
            QPainter g(&flat);
            g.drawImage(0, 0, img);
        }
        img = flat;
    }
    if (blackWhite) {
        img = img.convertToFormat(QImage::Format_Grayscale8)
                 .convertToFormat(QImage::Format_Mono, Qt::ThresholdDither | Qt::MonoOnly);
    }
    return img;
}

QString exportBaseName(KisDocument *doc)
{
    QString title = doc ? doc->caption().trimmed() : QString();
    if (title.endsWith(QLatin1String(".kra"), Qt::CaseInsensitive)) title.chop(4);
    title.remove(QLatin1Char('*'));
    title = title.trimmed();
    if (title.isEmpty() || title.startsWith(QLatin1Char('['))) title = QStringLiteral("nanquim");
    title.replace(QRegularExpression(QStringLiteral("[\\\\/:*?\"<>|]")), QStringLiteral("_"));
    return title;
}

QString chapterOf(const QString &path)
{
    if (path.isEmpty()) return QString();
    return ChapterStore::instance()->info(path).chapterId;
}

void showExport(KisMainWindow *mw)
{
    KisView *view = mw->activeView();
    if (!view || !view->document() || !view->image()) return;
    KisDocument *doc = view->document();
    const QString chapterId = chapterOf(doc->path());
    const ChapterStore::Chapter chapter = ChapterStore::instance()->chapter(chapterId);

    QDialog *d = makeSheet(mw, 620);
    QVBoxLayout *root = new QVBoxLayout(d);
    root->setContentsMargins(26, 22, 26, 20);
    root->setSpacing(14);
    root->addLayout(sheetHeader(d, i18n("Exportar"), QString()));

    QButtonGroup *what = new QButtonGroup(d);
    QHBoxLayout *cards = new QHBoxLayout();
    cards->setSpacing(8);
    const QStringList titles = {i18n("Esta página"), i18n("Capítulo"), i18n("Time-lapse")};
    const QStringList details = {exportBaseName(doc),
                                 chapterId.isEmpty() ? i18n("fora de capítulo") : i18n("%1 páginas", chapter.pages.size()),
                                 i18n("vídeo MP4")};
    for (int i = 0; i < 3; i++) {
        QPushButton *c = nqButton(QString(), d);
        c->setProperty("class", QStringLiteral("nqCard"));
        c->setCheckable(true);
        QVBoxLayout *cl = new QVBoxLayout(c);
        cl->setContentsMargins(8, 8, 8, 8);
        cl->setSpacing(2);
        QLabel *t = nqLabel(titles[i], "nqItem", c);
        QLabel *s = nqLabel(details[i], "nqSmall", c);
        t->setAlignment(Qt::AlignCenter);
        s->setAlignment(Qt::AlignCenter);
        t->setAttribute(Qt::WA_TransparentForMouseEvents);
        s->setAttribute(Qt::WA_TransparentForMouseEvents);
        cl->addWidget(t);
        cl->addWidget(s);
        c->setEnabled(i != 1 || !chapterId.isEmpty());
        what->addButton(c, i);
        cards->addWidget(c, 1);
    }
    what->button(0)->setChecked(true);
    root->addLayout(cards);

    QStackedWidget *pages = new QStackedWidget(d);
    // --- página e capítulo -----------------------------------------------------
    QWidget *imagePage = new QWidget(pages);
    QVBoxLayout *il = new QVBoxLayout(imagePage);
    il->setContentsMargins(0, 0, 0, 0);
    il->setSpacing(10);
    il->addWidget(nqLabel(i18n("Formato"), "nqSmall", imagePage));
    QButtonGroup *format = new QButtonGroup(imagePage);
    il->addLayout(chipRow(imagePage, format, {QStringLiteral("PNG"), QStringLiteral("JPG"), QStringLiteral("PSD"),
                                              QStringLiteral("PDF"), i18n("Cópia .kra")}, 0));
    il->addWidget(nqLabel(i18n("Tamanho"), "nqSmall", imagePage));
    QButtonGroup *size = new QButtonGroup(imagePage);
    il->addLayout(chipRow(imagePage, size, {i18n("Original"), i18n("Metade"), i18n("Para redes · 2048 px")}, 0));
    QCheckBox *hideDraft = new QCheckBox(i18n("Esconder camadas de rascunho"), imagePage);
    hideDraft->setChecked(true);
    QCheckBox *blackWhite = new QCheckBox(i18n("Só preto e branco (para gráfica)"), imagePage);
    il->addWidget(hideDraft);
    il->addWidget(blackWhite);
    pages->addWidget(imagePage);

    // --- time-lapse --------------------------------------------------------------
    QWidget *videoPage = new QWidget(pages);
    QVBoxLayout *vl = new QVBoxLayout(videoPage);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(10);
    NanquimTimelapseRecorder *recorder = s_recorder.data();
    const int frames = recorder ? recorder->frameCount() : 0;
    vl->addWidget(nqLabel(frames > 0 ? i18n("%1 quadros gravados neste desenho, sem as pausas.", frames)
                                     : i18n("Nada gravado ainda neste desenho."), "nqBody", videoPage, true));
    vl->addWidget(nqLabel(i18n("Duração"), "nqSmall", videoPage));
    QButtonGroup *duration = new QButtonGroup(videoPage);
    vl->addLayout(chipRow(videoPage, duration, {i18n("30 segundos"), i18n("1 minuto"), i18n("Completo")}, 0));
    QCheckBox *autoRecord = new QCheckBox(i18n("Gravar sozinho enquanto desenho"), videoPage);
    autoRecord->setChecked(NanquimTimelapseRecorder::enabled());
    QObject::connect(autoRecord, &QCheckBox::toggled, d, [](bool on) { NanquimTimelapseRecorder::setEnabled(on); });
    vl->addWidget(autoRecord);
    vl->addStretch(1);
    pages->addWidget(videoPage);
    root->addWidget(pages);

    QLabel *where = nqLabel(i18n("Salva em Imagens › Nanquim. PSD, PDF e .kra vão para Downloads › Nanquim."),
                            "nqSmall", d, true);
    root->addWidget(where);
    QHBoxLayout *actions = new QHBoxLayout();
    actions->setSpacing(10);
    QPushButton *share = nqButton(i18n("Compartilhar"), d);
    QPushButton *go = nqButton(i18n("Exportar"), d, "nqPrimary");
    actions->addStretch(1);
    actions->addWidget(share);
    actions->addWidget(go);
    root->addLayout(actions);

    auto refresh = [what, pages, format, size, hideDraft, blackWhite, where, share, go, frames]() {
        const int w = what->checkedId();
        pages->setCurrentIndex(w == 2 ? 1 : 0);
        const int f = format->checkedId();
        const bool chapterMode = w == 1;
        // capítulo: só imagens soltas ou um PDF
        if (QAbstractButton *psd = format->button(2)) psd->setEnabled(!chapterMode);
        if (QAbstractButton *kra = format->button(4)) kra->setEnabled(!chapterMode);
        if (chapterMode && (f == 2 || f == 4)) format->button(3)->setChecked(true);
        const bool layered = !chapterMode && (format->checkedId() == 2 || format->checkedId() == 4);
        for (QAbstractButton *b : size->buttons()) b->setEnabled(!layered);
        hideDraft->setEnabled(!layered);
        blackWhite->setEnabled(!layered);
        where->setVisible(w != 2);
        share->setVisible(w == 0);
        go->setText(w == 2 ? i18n("Exportar vídeo") : chapterMode ? i18n("Exportar capítulo") : i18n("Exportar"));
        go->setEnabled(w != 2 || frames > 0);
    };
    QObject::connect(what, &QButtonGroup::idClicked, d, [refresh](int) { refresh(); });
    QObject::connect(format, &QButtonGroup::idClicked, d, [refresh](int) { refresh(); });
    refresh();

    auto run = [=](bool thenShare) {
        const int w = what->checkedId();
        if (w == 2) {
            const int id = duration->checkedId();
            d->accept();
            if (s_recorder) exportTimelapse(mw, s_recorder.data(), id == 0 ? 30 : id == 1 ? 60 : 0);
            return;
        }
        const int f = format->checkedId();
        const int sizeMode = size->checkedId();
        const bool hide = hideDraft->isChecked();
        const bool bw = blackWhite->isChecked();
        d->accept();

        QDir cache(QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
        cache.mkpath(QStringLiteral("export"));
        const QString base = exportBaseName(doc);
        static const char *exts[] = {"png", "jpg", "psd", "pdf", "kra"};
        static const char *mimes[] = {"image/png", "image/jpeg", "image/vnd.adobe.photoshop", "application/pdf", "application/x-krita"};
        QApplication::setOverrideCursor(Qt::WaitCursor);
        QString location, uri, error;
        // janela de progresso (capítulo e PDF): barra azul e verde, "Página X de N"
        QPointer<QDialog> prog;
        QPointer<QProgressBar> progBar;
        QPointer<QLabel> progTitle, progInfo;
        QPointer<QPushButton> progButton;
        qint64 exportedBytes = 0;
        int exportedPages = 0;
        const char *barStyle =
            "QProgressBar { background: #4a4842; border: none; border-radius: 5px; }"
            "QProgressBar::chunk { border-radius: 5px;"
            "  background: #6fbfad; }";

        if (w == 0 && (f == 2 || f == 4)) {
            const QString path = cache.filePath(QStringLiteral("export/%1.%2").arg(base, QLatin1String(exts[f])));
            QFile::remove(path);
            if (doc->exportDocumentSync(path, QByteArray(mimes[f]))) {
                location = publishExport(path, QFileInfo(path).fileName(), QLatin1String(mimes[f]), &uri);
            }
        } else if (w == 0 && f != 3) {
            const QImage img = finishImage(renderPage(view->image(), hide), sizeMode, bw, f == 1);
            const QString path = cache.filePath(QStringLiteral("export/%1.%2").arg(base, QLatin1String(exts[f])));
            if (!img.isNull() && img.save(path, f == 1 ? "JPG" : "PNG", f == 1 ? 92 : -1)) {
                location = publishExport(path, QFileInfo(path).fileName(), QLatin1String(mimes[f]), &uri);
            }
        } else {
            // PDF da página, ou o capítulo inteiro (imagens soltas ou PDF)
            QStringList paths;
            if (w == 1) paths = ChapterStore::instance()->chapter(chapterId).pages;
            else paths << doc->path();
            const QString pdfPath = cache.filePath(QStringLiteral("export/%1.pdf").arg(w == 1 ? chapter.name : base));
            QScopedPointer<QPdfWriter> pdf;
            QScopedPointer<QPainter> pdfPainter;
            if (f == 3) {
                QFile::remove(pdfPath);
                pdf.reset(new QPdfWriter(pdfPath));
                pdf->setCreator(QStringLiteral("Nanquim"));
                pdf->setPageMargins(QMarginsF(0, 0, 0, 0));
            }
            auto cancelled = std::make_shared<bool>(false);
            prog = makeSheet(mw, 460);
            {
                QVBoxLayout *pl = new QVBoxLayout(prog);
                pl->setContentsMargins(26, 22, 26, 20);
                pl->setSpacing(12);
                progTitle = nqLabel(w == 1 ? i18n("Exportando o capítulo") : i18n("Exportando"), "nqTitle", prog);
                pl->addWidget(progTitle);
                progBar = new QProgressBar(prog);
                progBar->setRange(0, qMax(1, paths.size()));
                progBar->setValue(0);
                progBar->setTextVisible(false);
                progBar->setFixedHeight(10);
                progBar->setStyleSheet(QLatin1String(barStyle));
                pl->addWidget(progBar);
                progInfo = nqLabel(QString(), "nqSmall", prog);
                pl->addWidget(progInfo);
                QHBoxLayout *pa = new QHBoxLayout();
                pa->addStretch(1);
                progButton = nqButton(i18n("Cancelar"), prog);
                pa->addWidget(progButton);
                pl->addLayout(pa);
                QPointer<QDialog> pg = prog;
                QObject::connect(progButton.data(), &QPushButton::clicked, prog.data(), [cancelled, pg]() {
                    if (pg && pg->property("nanquimDone").toBool()) pg->close();
                    else *cancelled = true;
                });
                prog->show();
            }
            int done = 0;
            for (int i = 0; i < paths.size() && !*cancelled; i++) {
                if (progBar) progBar->setValue(i);
                if (progInfo) progInfo->setText(i18n("Página %1 de %2", i + 1, paths.size()));
                QApplication::processEvents();
                QImage img;
                qreal dpi = 300;
                if (paths[i] == doc->path() || paths[i].isEmpty()) {
                    img = renderPage(view->image(), hide);
                    dpi = view->image()->xRes() * 72.0;
                } else {
                    QScopedPointer<KisDocument> page(KisPart::instance()->createDocument());
                    page->setFileBatchMode(true);
                    if (page->openPath(paths[i], KisDocument::DontAddToRecent) && page->image()) {
                        img = renderPage(page->image(), hide);
                        dpi = page->image()->xRes() * 72.0;
                    }
                }
                img = finishImage(img, sizeMode, bw, f != 0);
                if (img.isNull()) continue;
                if (sizeMode == 1) dpi /= 2;
                if (f == 3) {
                    const QSizeF mm(img.width() / dpi * 25.4, img.height() / dpi * 25.4);
                    if (done == 0) {
                        pdf->setPageSize(QPageSize(mm, QPageSize::Millimeter));
                        pdf->setResolution(qRound(dpi));
                        pdfPainter.reset(new QPainter(pdf.data()));
                    } else {
                        pdf->setPageSize(QPageSize(mm, QPageSize::Millimeter));
                        pdf->newPage();
                    }
                    const QRect target = pdfPainter->viewport();
                    pdfPainter->drawImage(target, img);
                } else {
                    const QString path = cache.filePath(QStringLiteral("export/%1_%2.%3")
                                                        .arg(chapter.name).arg(i + 1, 2, 10, QLatin1Char('0'))
                                                        .arg(QLatin1String(exts[f])));
                    if (img.save(path, f == 1 ? "JPG" : "PNG", f == 1 ? 92 : -1)) {
                        exportedBytes += QFileInfo(path).size();
                        location = publishExport(path, QFileInfo(path).fileName(), QLatin1String(mimes[f]), &uri);
                    }
                }
                done++;
            }
            if (progBar) progBar->setValue(progBar->maximum());
            exportedPages = done;
            if (f == 3 && pdfPainter) {
                pdfPainter->end();
                pdfPainter.reset();
                pdf.reset();
                exportedBytes = QFileInfo(pdfPath).size();
                location = publishExport(pdfPath, QFileInfo(pdfPath).fileName(), QStringLiteral("application/pdf"), &uri);
            }
            if (done == 0) location.clear();
        }
        QApplication::restoreOverrideCursor();
        if (location.isEmpty()) {
            if (prog) prog->close();
            QMessageBox::warning(mw, QStringLiteral("Nanquim"), i18n("Não consegui exportar. Veja se há espaço livre no tablet."));
            return;
        }
        if (prog) {
            // pronto: a janela fica verde com o tamanho e fecha sozinha
            prog->setProperty("nanquimDone", true);
            if (progTitle) progTitle->setText(i18n("Exportado"));
            if (progBar) {
                progBar->setValue(progBar->maximum());
                progBar->setStyleSheet(QStringLiteral(
                    "QProgressBar { background: #4a4842; border: none; border-radius: 5px; }"
                    "QProgressBar::chunk { border-radius: 5px; background: #5ec98c; }"));
            }
            if (progInfo) {
                const QString mb = QLocale().toString(exportedBytes / 1048576.0, 'f', 1);
                progInfo->setText(exportedPages == 1 ? i18n("1 página · %1 MB · %2", mb, location)
                                                     : i18n("%1 páginas · %2 MB · %3", exportedPages, mb, location));
                progInfo->setStyleSheet(QStringLiteral("color: #5ec98c; font-weight: 600;"));
            }
            if (progButton) progButton->setText(i18n("Fechar"));
            QPointer<QDialog> pg = prog;
            QTimer::singleShot(1800, mw, [pg]() { if (pg) pg->close(); });
        }
        showToast(mw, i18n("Exportado · %1", location));
        if (thenShare) shareExport(uri, QLatin1String(mimes[f]));
    };
    QObject::connect(go, &QPushButton::clicked, d, [run]() { run(false); });
    QObject::connect(share, &QPushButton::clicked, d, [run]() { run(true); });
    d->open();
}

// ---------------------------------------------------------------------------
// Configurações do Nanquim
// ---------------------------------------------------------------------------
void switchUiMode(KisMainWindow *mw);

QWidget *settingRow(QWidget *parent, const QString &title, const QString &info, QWidget *control)
{
    QWidget *row = new QWidget(parent);
    row->setMinimumHeight(60);
    QHBoxLayout *l = new QHBoxLayout(row);
    l->setContentsMargins(0, 6, 0, 6);
    l->setSpacing(14);
    QVBoxLayout *t = new QVBoxLayout();
    t->setSpacing(2);
    t->addWidget(nqLabel(title, "nqItem", row));
    if (!info.isEmpty()) t->addWidget(nqLabel(info, "nqSmall", row, true));
    l->addLayout(t, 1);
    if (control) l->addWidget(control);
    return row;
}

void showSettings(KisMainWindow *mw)
{
    QDialog *d = makeSheet(mw, 1000, 640);
    QHBoxLayout *root = new QHBoxLayout(d);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    QWidget *nav = new QWidget(d);
    nav->setFixedWidth(250);
    nav->setAttribute(Qt::WA_StyledBackground);
    nav->setStyleSheet(QStringLiteral(
        "QWidget { background: #2e2d29; border-top-left-radius: 18px; border-bottom-left-radius: 18px; }"
        "QPushButton { text-align: left; border: none; background: transparent; color: #cfcabf; font-weight: 500; border-radius: 10px; }"
        "QPushButton:checked { background: #2a3d38; color: #cde9e1; }"));
    QVBoxLayout *nl = new QVBoxLayout(nav);
    nl->setContentsMargins(14, 22, 14, 18);
    nl->setSpacing(4);
    QLabel *navTitle = nqLabel(i18n("Configurações"), "nqItem", nav);
    navTitle->setStyleSheet(QStringLiteral("font-size: 16px; font-weight: 700; padding: 0 10px 12px 10px;"));
    nl->addWidget(navTitle);
    QButtonGroup *sections = new QButtonGroup(nav);
    const QStringList names = {i18n("Caneta e toque"), i18n("Interface"), i18n("Desempenho"), i18n("Salvamento"), i18n("Mais")};
    for (int i = 0; i < names.size(); i++) {
        QPushButton *b = nqButton(names[i], nav);
        b->setCheckable(true);
        b->setChecked(i == 0);
        sections->addButton(b, i);
        nl->addWidget(b);
    }
    nl->addStretch(1);
    root->addWidget(nav);

    QWidget *right = new QWidget(d);
    QVBoxLayout *rl = new QVBoxLayout(right);
    rl->setContentsMargins(30, 22, 24, 22);
    rl->setSpacing(10);
    QLabel *sectionLabel = nqLabel(names[0].toUpper(), "nqSection", right);
    QHBoxLayout *head = new QHBoxLayout();
    head->addWidget(sectionLabel, 1);
    QPushButton *close = nqButton(QStringLiteral("×"), right, "nqClose");
    QObject::connect(close, &QPushButton::clicked, d, &QDialog::reject);
    head->addWidget(close);
    rl->addLayout(head);
    QStackedWidget *stack = new QStackedWidget(right);
    rl->addWidget(stack, 1);
    root->addWidget(right, 1);

    auto page = [stack]() {
        QWidget *p = new QWidget(stack);
        QVBoxLayout *l = new QVBoxLayout(p);
        l->setContentsMargins(0, 0, 0, 0);
        l->setSpacing(4);
        stack->addWidget(p);
        return l;
    };

    // --- caneta e toque ----------------------------------------------------------
    {
        QVBoxLayout *l = page();
        QWidget *p = l->parentWidget();
        l->addWidget(nqLabel(i18n("Como o Nanquim entende a sua mão"), "nqTitle", p));
        l->addSpacing(8);
        QCheckBox *penOnly = new QCheckBox(p);
        penOnly->setChecked(KisConfig(true).touchPainting() == KisConfig::TOUCH_PAINTING_DISABLED);
        l->addWidget(settingRow(p, i18n("Desenhar só com a caneta"), i18n("O dedo move, gira e dá zoom, mas não pinta. Evita marcas da palma da mão."), penOnly));
        l->addWidget(nqDivider(p));
        QWidget *curveBox = new QWidget(p);
        QHBoxLayout *cb = new QHBoxLayout(curveBox);
        cb->setContentsMargins(0, 0, 0, 0);
        QButtonGroup *curve = new QButtonGroup(curveBox);
        cb->addLayout(chipRow(curveBox, curve, {i18n("Leve"), i18n("Normal"), i18n("Firme")}, currentPressureCurve()));
        l->addWidget(settingRow(p, i18n("Força da caneta"), i18n("Leve: traço grosso com pouca força. Firme: precisa apertar mais."), curveBox));
        l->addWidget(nqDivider(p));
        l->addSpacing(8);
        l->addWidget(nqLabel(i18n("Gestos com os dedos"), "nqSmall", p));
        struct Gesture { const char *fingers; const char *what; const char *action; };
        const Gesture gestures[] = {
            {"2", "Tocar", "Desfazer"}, {"3", "Tocar", "Refazer"}, {"2", "Arrastar, pinçar e girar", "Mover, zoom e rotação"},
        };
        for (const Gesture &g : gestures) {
            QWidget *row = new QWidget(p);
            QHBoxLayout *rl2 = new QHBoxLayout(row);
            rl2->setContentsMargins(0, 4, 0, 4);
            rl2->setSpacing(14);
            QLabel *f = new QLabel(QString::fromUtf8(g.fingers), row);
            f->setFixedSize(38, 38);
            f->setAlignment(Qt::AlignCenter);
            f->setStyleSheet(QStringLiteral("background: #34332e; border-radius: 10px; color: #6fbfad; font-weight: 700;"));
            rl2->addWidget(f);
            rl2->addWidget(nqLabel(i18n(g.what), "nqBody", row), 1);
            rl2->addWidget(nqLabel(i18n(g.action), "nqBody", row));
            l->addWidget(row);
        }
        l->addWidget(nqLabel(i18n("Os gestos vêm do Krita e podem ser trocados em Mais › Configurações completas › Atalhos de toque."),
                             "nqSmall", p, true));
        l->addStretch(1);
        QObject::connect(penOnly, &QCheckBox::toggled, d, [](bool on) {
            KisConfig cfg(false);
            cfg.setTouchPainting(on ? KisConfig::TOUCH_PAINTING_DISABLED : KisConfig::TOUCH_PAINTING_AUTO);
            KisConfigNotifier::instance()->notifyConfigChanged();
        });
        QObject::connect(curve, &QButtonGroup::idClicked, d, [](int id) { setPressureCurve(id); });
    }

    // --- interface ---------------------------------------------------------------
    {
        QVBoxLayout *l = page();
        QWidget *p = l->parentWidget();
        l->addWidget(nqLabel(i18n("Jeito da tela"), "nqTitle", p));
        l->addSpacing(8);
        QHBoxLayout *modes = new QHBoxLayout();
        modes->setSpacing(14);
        const bool classic = isClassicMode();
        const QStringList modeNames = {i18n("Tablet"), i18n("Clássico")};
        const QStringList modeInfo = {i18n("Barras do Nanquim e painéis que deslizam"), i18n("Menus e painéis fixos, como no computador")};
        for (int i = 0; i < 2; i++) {
            QPushButton *m = nqButton(QString(), p);
            m->setProperty("class", QStringLiteral("nqCard"));
            m->setCheckable(true);
            m->setChecked((i == 1) == classic);
            m->setMinimumHeight(110);
            QVBoxLayout *ml = new QVBoxLayout(m);
            ml->setContentsMargins(14, 12, 14, 12);
            QLabel *t = nqLabel(modeNames[i], "nqItem", m);
            QLabel *s = nqLabel(modeInfo[i], "nqSmall", m, true);
            t->setAttribute(Qt::WA_TransparentForMouseEvents);
            s->setAttribute(Qt::WA_TransparentForMouseEvents);
            ml->addStretch(1);
            ml->addWidget(t);
            ml->addWidget(s);
            const bool wantClassic = i == 1;
            QObject::connect(m, &QPushButton::clicked, d, [mw, d, wantClassic, classic]() {
                if (wantClassic != classic) {
                    d->accept();
                    switchUiMode(mw);
                }
            });
            modes->addWidget(m, 1);
        }
        l->addLayout(modes);
        l->addSpacing(10);
        if (!classic) {
            QWidget *sideBox = new QWidget(p);
            QHBoxLayout *sb = new QHBoxLayout(sideBox);
            sb->setContentsMargins(0, 0, 0, 0);
            QButtonGroup *side = new QButtonGroup(sideBox);
            sb->addLayout(chipRow(sideBox, side, {i18n("Esquerda"), i18n("Direita")}, railOnRight() ? 1 : 0));
            l->addWidget(settingRow(p, i18n("Lado da barra de ferramentas"),
                                    i18n("Do lado da mão que não desenha. Desfazer, refazer e navegar ficam junto dela, "
                                         "e os painéis abrem do outro lado."), sideBox));
            QObject::connect(side, &QButtonGroup::idClicked, d, [mw](int id) { setRailSide(mw, id == 1); });
            l->addWidget(nqDivider(p));
        }
        QPushButton *guide = nqButton(i18n("Rever boas-vindas e guia"), p);
        QObject::connect(guide, &QPushButton::clicked, d, [mw, d]() { d->accept(); showWelcome(mw, false); });
        l->addWidget(settingRow(p, i18n("Primeiros passos"), i18n("A mensagem de boas-vindas e o guia rápido."), guide));
        l->addStretch(1);
    }

    // --- desempenho --------------------------------------------------------------
    {
        QVBoxLayout *l = page();
        QWidget *p = l->parentWidget();
        l->addWidget(nqLabel(i18n("Leve e estável"), "nqTitle", p));
        l->addSpacing(8);
        KisImageConfig imageCfg(true);
        const int percent = qRound(imageCfg.memoryHardLimitPercent());
        QWidget *memBox = new QWidget(p);
        memBox->setFixedWidth(300);
        QVBoxLayout *mb = new QVBoxLayout(memBox);
        mb->setContentsMargins(0, 0, 0, 0);
        QLabel *memValue = nqLabel(i18n("%1% da memória do tablet", percent), "nqSmall", memBox);
        QSlider *mem = new QSlider(Qt::Horizontal, memBox);
        mem->setRange(30, 60);
        mem->setValue(qBound(30, percent, 60));
        mb->addWidget(memValue);
        mb->addWidget(mem);
        l->addWidget(settingRow(p, i18n("Memória para o desenho"),
                                i18n("Mais memória deixa páginas grandes mais rápidas, mas o Android pode fechar o app. "
                                     "Vale na próxima vez que abrir o Nanquim."), memBox));
        l->addWidget(nqDivider(p));
        // escala da página na tela: 3 = alta qualidade (Krita), 2 = trilinear, 1 = bilinear (mais leve)
        const int filterModes[] = {3, 2, 1};
        const int curFilter = KisConfig(true).openGLFilteringMode();
        QWidget *filterBox = new QWidget(p);
        QHBoxLayout *fb = new QHBoxLayout(filterBox);
        fb->setContentsMargins(0, 0, 0, 0);
        QButtonGroup *filterGroup = new QButtonGroup(filterBox);
        fb->addLayout(chipRow(filterBox, filterGroup, {i18n("Alta qualidade"), i18n("Trilinear"), i18n("Bilinear")},
                              curFilter == 2 ? 1 : (curFilter == 1 || curFilter == 0) ? 2 : 0));
        l->addWidget(settingRow(p, i18n("Qualidade da página na tela"),
                                i18n("Alta qualidade é a do Krita. Bilinear é a mais leve para o tablet. "
                                     "Não muda o arquivo, só como a página aparece."), filterBox));
        l->addWidget(nqDivider(p));
        const int curTexture = KisConfig(true).openGLTextureSize();
        QWidget *texBox = new QWidget(p);
        QHBoxLayout *tb = new QHBoxLayout(texBox);
        tb->setContentsMargins(0, 0, 0, 0);
        QButtonGroup *texGroup = new QButtonGroup(texBox);
        tb->addLayout(chipRow(texBox, texGroup, {i18n("256 px (como o Krita)"), i18n("512 px")}, curTexture >= 512 ? 1 : 0));
        l->addWidget(settingRow(p, i18n("Blocos de textura"),
                                i18n("256 px deixa o traço mais fluido. 512 px pode aliviar zoom e giro em páginas grandes. "
                                     "Vale ao reabrir a página."), texBox));
        l->addWidget(nqDivider(p));
        QObject::connect(filterGroup, &QButtonGroup::idClicked, d, [filterModes](int id) {
            KConfigGroup general = KSharedConfig::openConfig()->group("");
            general.writeEntry("OpenGLFilterMode", filterModes[qBound(0, id, 2)]);
            KSharedConfig::openConfig()->sync();
            KisConfigNotifier::instance()->notifyConfigChanged();
        });
        QObject::connect(texGroup, &QButtonGroup::idClicked, d, [mw](int id) {
            KConfigGroup general = KSharedConfig::openConfig()->group("");
            general.writeEntry("textureSize", id == 1 ? 512 : 256);
            KSharedConfig::openConfig()->sync();
            showToast(mw, i18n("Blocos de %1 px · vale ao reabrir a página", id == 1 ? 512 : 256));
        });
        QCheckBox *lod = new QCheckBox(p);
        lod->setChecked(KisConfig(true).levelOfDetailEnabled());
        l->addWidget(settingRow(p, i18n("Prévia rápida ao dar zoom"), i18n("Desenha numa versão reduzida enquanto o zoom está longe."), lod));
        l->addWidget(nqDivider(p));
        // diagnóstico da fase de otimização: marca os tempos de cada tela no
        // log do Krita (medir antes de mudar). Pode deixar desligado.
        QCheckBox *timing = new QCheckBox(p);
        timing->setChecked(KSharedConfig::openConfig()->group(QStringLiteral("Nanquim")).readEntry("tempos", false));
        l->addWidget(settingRow(p, i18n("Registrar tempos no log (diagnóstico)"),
                                i18n("Marca quanto leva abrir o app, trocar de tela e montar a janela. "
                                     "Manda o arquivo de log junto quando for reportar lentidão."), timing));
        l->addStretch(1);
        QObject::connect(mem, &QSlider::valueChanged, d, [memValue](int v) {
            memValue->setText(i18n("%1% da memória do tablet", v));
            KisImageConfig cfg(false);
            cfg.setMemoryHardLimitPercent(v);
        });
        QObject::connect(lod, &QCheckBox::toggled, d, [](bool on) {
            KisConfig cfg(false);
            cfg.setLevelOfDetailEnabled(on);
            KisConfigNotifier::instance()->notifyConfigChanged();
        });
        QObject::connect(timing, &QCheckBox::toggled, d, [](bool on) {
            KSharedConfig::openConfig()->group(QStringLiteral("Nanquim")).writeEntry("tempos", on);
            KSharedConfig::openConfig()->sync();
        });
    }

    // --- salvamento ----------------------------------------------------------------
    {
        QVBoxLayout *l = page();
        QWidget *p = l->parentWidget();
        l->addWidget(nqLabel(i18n("Para não perder nada"), "nqTitle", p));
        l->addSpacing(8);
        const int interval = KisConfig(true).autoSaveInterval();
        const QList<int> options = {0, 5 * 60, 10 * 60, 15 * 60};
        int checked = 2;
        for (int i = 0; i < options.size(); i++) if (options[i] == interval) checked = i;
        QWidget *box = new QWidget(p);
        QHBoxLayout *bl = new QHBoxLayout(box);
        bl->setContentsMargins(0, 0, 0, 0);
        QButtonGroup *auto_ = new QButtonGroup(box);
        bl->addLayout(chipRow(box, auto_, {i18n("Desligado"), i18n("5 min"), i18n("10 min"), i18n("15 min")}, checked));
        l->addWidget(settingRow(p, i18n("Cópia automática"), i18n("Guarda uma cópia de segurança enquanto você desenha."), box));
        l->addStretch(1);
        QObject::connect(auto_, &QButtonGroup::idClicked, d, [options](int id) {
            KisConfig cfg(false);
            cfg.setAutoSaveInterval(options.value(id, 10 * 60));
            KisConfigNotifier::instance()->notifyConfigChanged();
        });
    }

    // --- mais ----------------------------------------------------------------------
    {
        QVBoxLayout *l = page();
        QWidget *p = l->parentWidget();
        l->addWidget(nqLabel(i18n("Mais"), "nqTitle", p));
        l->addSpacing(8);
        QPushButton *full = nqButton(i18n("Abrir"), p);
        QObject::connect(full, &QPushButton::clicked, d, [mw, d]() {
            d->accept();
            if (QAction *a = act(mw, "options_configure")) a->trigger();
        });
        l->addWidget(settingRow(p, i18n("Configurações completas do Krita"), i18n("Todas as opções do motor de pintura, atalhos e cores."), full));
        l->addWidget(nqDivider(p));
        QPushButton *about = nqButton(i18n("Ver"), p);
        QObject::connect(about, &QPushButton::clicked, d, [mw, d]() { d->accept(); showAbout(mw); });
        l->addWidget(settingRow(p, i18n("Sobre e créditos"), i18n("Krita, Tyson Tan e as licenças."), about));
        l->addStretch(1);
    }

    QObject::connect(sections, &QButtonGroup::idClicked, d, [stack, sectionLabel, names](int id) {
        stack->setCurrentIndex(id);
        sectionLabel->setText(names.value(id).toUpper());
    });
    d->open();
}

// ---------------------------------------------------------------------------
// Requadros e balões, numa janela só
// ---------------------------------------------------------------------------
QString xmlEscape(const QString &s)
{
    QString out = s;
    out.replace(QLatin1Char('&'), QLatin1String("&amp;"));
    out.replace(QLatin1Char('<'), QLatin1String("&lt;"));
    out.replace(QLatin1Char('>'), QLatin1String("&gt;"));
    out.replace(QLatin1Char('"'), QLatin1String("&quot;"));
    return out;
}

// 0 fala, 1 pensamento, 2 grito, 3 narração, 4 sussurro, 5 rádio/TV
void insertBalloonWithText(KisMainWindow *mw, int kind, const QString &text, int sizeIndex)
{
    KisView *view = mw->activeView();
    if (!view || !view->image()) return;
    KisImageSP image = view->image();
    const qreal W = image->width() / image->xRes();
    const qreal H = image->height() / image->yRes();
    const qreal fontSize = W * (sizeIndex == 0 ? 0.016 : sizeIndex == 2 ? 0.028 : 0.021);
    const QStringList lines = text.trimmed().isEmpty() ? QStringList() : text.trimmed().split(QLatin1Char('\n'));
    int longest = 0;
    for (const QString &l : lines) longest = qMax(longest, l.size());
    qreal bw = qMax(W * 0.2, longest * fontSize * 0.62 + fontSize * 3.2);
    bw = qMin(bw, W * 0.7);
    const qreal lineH = fontSize * 1.25;
    qreal bh = qMax(bw * 0.5, lines.size() * lineH + fontSize * 2.6);
    if (kind == 3) bh = qMax(lines.size() * lineH + fontSize * 1.6, bw * 0.3) / 0.6;   // a caixa usa 60% da altura
    const QRectF r(W / 2 - bw / 2, H / 2 - bh / 2, bw, bh);
    const qreal unit = qMax(2.0, image->width() * 0.0011) / image->xRes();

    QString shape;
    if (kind == 4) {
        shape = balloonPath(0, r, unit);
        shape.replace(QLatin1String("stroke-linejoin=\"round\""),
                      QStringLiteral("stroke-linejoin=\"round\" stroke-dasharray=\"%1 %2\"").arg(unit * 4).arg(unit * 3));
    } else if (kind == 5) {
        const qreal x = r.x(), y = r.y() + r.height() * 0.1, w = r.width(), h = r.height() * 0.7;
        const qreal t = w * 0.12;
        const QString d = QStringLiteral("M %1 %2 L %3 %2 L %3 %4 L %5 %4 L %6 %7 L %8 %4 L %1 %4 Z")
                .arg(x).arg(y).arg(x + w).arg(y + h).arg(x + w * 0.42 + t).arg(x + w * 0.3).arg(y + h + t * 1.6).arg(x + w * 0.42);
        shape = QStringLiteral("<path d=\"%1\" fill=\"#ffffff\" stroke=\"#000000\" stroke-width=\"%2\" stroke-linejoin=\"miter\"/>")
                .arg(d).arg(unit);
    } else {
        shape = balloonPath(kind, r, unit);
    }

    QString textSvg;
    if (!lines.isEmpty()) {
        const qreal cx = r.center().x();
        const qreal cy = kind == 5 ? r.y() + r.height() * 0.45 : r.center().y();
        const qreal y0 = cy - (lines.size() - 1) * lineH / 2 + fontSize * 0.35;
        textSvg = QStringLiteral("<text text-anchor=\"middle\" font-family=\"Montserrat, sans-serif\" font-weight=\"700\" "
                                 "font-size=\"%1\" fill=\"#000000\">").arg(fontSize);
        for (int i = 0; i < lines.size(); i++) {
            textSvg += QStringLiteral("<tspan x=\"%1\" y=\"%2\">%3</tspan>").arg(cx).arg(y0 + i * lineH).arg(xmlEscape(lines[i]));
        }
        textSvg += QLatin1String("</text>");
    }
    if (!addVectorShapes(mw, i18n("Balões"), shape + textSvg, true)) {
        QMessageBox::warning(mw, QStringLiteral("Nanquim"), i18n("Não consegui criar o balão."));
        return;
    }
    if (QAction *select = act(mw, "InteractionTool")) select->trigger();
}

QPixmap balloonKindIcon(int kind)
{
    static const char *icons[] = {
        "<ellipse cx='12' cy='10' rx='9' ry='6'/><path d='M8.5 15.2L6 20l5.5-4'/>",
        "<path d='M6 12a4 4 0 0 1 2.5-6 4.5 4.5 0 0 1 7.5-.5 4 4 0 0 1 5 4 3.5 3.5 0 0 1-2.5 5.5 4 4 0 0 1-6 .5 4 4 0 0 1-6.5-3.5z'/><circle cx='6' cy='18.5' r='1.3'/><circle cx='3.5' cy='21' r='.8'/>",
        "<path d='M2.5 11l4-2-1.5-4.5 5 2 2-4 2 4 5-2-1.5 4.5 4 2-4 2 1.5 4.5-5-1.5-2 4.5-2-4.5-5 1.5 1.5-4.5z'/>",
        "<rect x='3' y='7' width='18' height='10'/>",
        "<ellipse cx='12' cy='10' rx='9' ry='6' stroke-dasharray='2.4 2'/><path d='M8.5 15.2L6 20l5.5-4' stroke-dasharray='2 2'/>",
        "<path d='M3 5h18v11h-8l-3 4-1-4H3z'/>",
    };
    return svgPixmap(QLatin1String(icons[qBound(0, kind, 5)]), C_TEXT, 30);
}

void showComicsSheet(KisMainWindow *mw, int tab)
{
    KisView *view = mw->activeView();
    if (!view || !view->image()) return;
    QDialog *d = makeSheet(mw, 720);
    QVBoxLayout *root = new QVBoxLayout(d);
    root->setContentsMargins(26, 20, 26, 20);
    root->setSpacing(14);
    root->addLayout(sheetHeader(d, i18n("Requadros e balões"), QString()));
    QButtonGroup *tabs = new QButtonGroup(d);
    root->addLayout(chipRow(d, tabs, {i18n("Requadros"), i18n("Balões")}, tab));
    QStackedWidget *stack = new QStackedWidget(d);
    root->addWidget(stack);

    // --- requadros -----------------------------------------------------------------
    QWidget *panelsPage = new QWidget(stack);
    QVBoxLayout *pl = new QVBoxLayout(panelsPage);
    pl->setContentsMargins(0, 0, 0, 0);
    pl->setSpacing(12);
    pl->addWidget(nqLabel(i18n("Os quadros entram na camada vetorial “Requadros”: dá para mover e redimensionar cada um depois."),
                          "nqSmall", panelsPage, true));
    const QList<PanelLayout> layouts = panelLayouts();
    QGridLayout *grid = new QGridLayout();
    grid->setSpacing(8);
    QButtonGroup *layoutGroup = new QButtonGroup(panelsPage);
    for (int i = 0; i < layouts.size(); i++) {
        QPushButton *b = nqButton(QString(), panelsPage);
        b->setProperty("class", QStringLiteral("nqCard"));
        b->setCheckable(true);
        b->setFixedSize(116, 128);
        QVBoxLayout *bl = new QVBoxLayout(b);
        bl->setContentsMargins(6, 8, 6, 6);
        bl->setSpacing(4);
        QLabel *icon = new QLabel(b);
        icon->setPixmap(layoutIcon(layouts[i]).pixmap(QSize(56, 78)));
        icon->setAlignment(Qt::AlignCenter);
        icon->setAttribute(Qt::WA_TransparentForMouseEvents);
        QLabel *name = nqLabel(i18n(layouts[i].name), "nqSmall", b);
        name->setAlignment(Qt::AlignCenter);
        name->setAttribute(Qt::WA_TransparentForMouseEvents);
        bl->addWidget(icon, 1);
        bl->addWidget(name);
        layoutGroup->addButton(b, i);
        grid->addWidget(b, i / 5, i % 5);
    }
    if (QAbstractButton *def = layoutGroup->button(qMin(5, layouts.size() - 1))) def->setChecked(true);
    pl->addLayout(grid);
    QHBoxLayout *numbers = new QHBoxLayout();
    numbers->setSpacing(10);
    auto spin = [panelsPage, numbers](const QString &title, double min, double max, double value, const QString &suffix) {
        QDoubleSpinBox *s = new QDoubleSpinBox(panelsPage);
        s->setRange(min, max);
        s->setValue(value);
        s->setSuffix(suffix);
        s->setButtonSymbols(QAbstractSpinBox::NoButtons);
        QVBoxLayout *col = new QVBoxLayout();
        col->setSpacing(4);
        col->addWidget(nqLabel(title, "nqSmall", panelsPage));
        col->addWidget(s);
        numbers->addLayout(col);
        return s;
    };
    QDoubleSpinBox *margin = spin(i18n("Margem"), 0, 25, 7, QStringLiteral(" %"));
    QDoubleSpinBox *gutterX = spin(i18n("Entre colunas"), 0, 10, 1.5, QStringLiteral(" %"));
    QDoubleSpinBox *gutterY = spin(i18n("Entre faixas"), 0, 10, 2.5, QStringLiteral(" %"));
    QDoubleSpinBox *border = spin(i18n("Borda"), 0.5, 40, qMax(2.0, view->image()->width() * 0.0012), QStringLiteral(" px"));
    pl->addLayout(numbers);
    stack->addWidget(panelsPage);

    // --- balões --------------------------------------------------------------------
    QWidget *balloonsPage = new QWidget(stack);
    QVBoxLayout *bpl = new QVBoxLayout(balloonsPage);
    bpl->setContentsMargins(0, 0, 0, 0);
    bpl->setSpacing(12);
    bpl->addWidget(nqLabel(i18n("Tipo de balão"), "nqSmall", balloonsPage));
    QGridLayout *kinds = new QGridLayout();
    kinds->setSpacing(8);
    QButtonGroup *kindGroup = new QButtonGroup(balloonsPage);
    const QStringList kindNames = {i18n("Fala"), i18n("Pensamento"), i18n("Grito"), i18n("Narração"), i18n("Sussurro"), i18n("Rádio / TV")};
    for (int i = 0; i < kindNames.size(); i++) {
        QPushButton *b = nqButton(QString(), balloonsPage);
        b->setProperty("class", QStringLiteral("nqCard"));
        b->setCheckable(true);
        b->setMinimumHeight(84);
        QVBoxLayout *kl = new QVBoxLayout(b);
        kl->setContentsMargins(6, 8, 6, 6);
        kl->setSpacing(4);
        QLabel *icon = new QLabel(b);
        icon->setPixmap(balloonKindIcon(i));
        icon->setAlignment(Qt::AlignCenter);
        icon->setAttribute(Qt::WA_TransparentForMouseEvents);
        QLabel *name = nqLabel(kindNames[i], "nqSmall", b);
        name->setAlignment(Qt::AlignCenter);
        name->setAttribute(Qt::WA_TransparentForMouseEvents);
        kl->addWidget(icon, 1);
        kl->addWidget(name);
        kindGroup->addButton(b, i);
        kinds->addWidget(b, i / 3, i % 3);
    }
    kindGroup->button(0)->setChecked(true);
    bpl->addLayout(kinds);
    bpl->addWidget(nqLabel(i18n("Texto (cada linha vira uma linha no balão)"), "nqSmall", balloonsPage));
    QPlainTextEdit *text = new QPlainTextEdit(balloonsPage);
    text->setFixedHeight(84);
    text->setPlaceholderText(i18n("Escreva a fala aqui, ou deixe vazio e escreva depois com a ferramenta Texto"));
    bpl->addWidget(text);
    bpl->addWidget(nqLabel(i18n("Tamanho da letra"), "nqSmall", balloonsPage));
    QButtonGroup *sizeGroup = new QButtonGroup(balloonsPage);
    bpl->addLayout(chipRow(balloonsPage, sizeGroup, {i18n("Pequena"), i18n("Média"), i18n("Grande")}, 1));
    stack->addWidget(balloonsPage);

    QHBoxLayout *actions = new QHBoxLayout();
    actions->setSpacing(10);
    QPushButton *textTool = nqButton(i18n("Ferramenta Texto"), d, "nqGhost");
    QPushButton *go = nqButton(QString(), d, "nqPrimary");
    actions->addWidget(textTool);
    actions->addStretch(1);
    actions->addWidget(go);
    root->addLayout(actions);

    auto sync = [stack, tabs, go, textTool]() {
        const int t = qMax(0, tabs->checkedId());
        stack->setCurrentIndex(t);
        go->setText(t == 0 ? i18n("Criar requadros") : i18n("Inserir balão"));
        textTool->setVisible(t == 1);
    };
    QObject::connect(tabs, &QButtonGroup::idClicked, d, [sync](int) { sync(); });
    sync();
    QObject::connect(textTool, &QPushButton::clicked, d, [mw, d]() {
        d->accept();
        if (QAction *t = act(mw, "SvgTextTool")) t->trigger();
    });
    QObject::connect(go, &QPushButton::clicked, d, [=]() {
        if (tabs->checkedId() == 1) {
            const int kind = qMax(0, kindGroup->checkedId());
            const QString t = text->toPlainText();
            const int sizeIndex = qMax(0, sizeGroup->checkedId());
            d->accept();
            insertBalloonWithText(mw, kind, t, sizeIndex);
            return;
        }
        const int chosen = qMax(0, layoutGroup->checkedId());
        const double m = margin->value(), gx = gutterX->value(), gy = gutterY->value(), bw = border->value();
        d->accept();
        KisView *v = mw->activeView();
        if (!v || !v->image()) return;
        KisImageSP image = v->image();
        const qreal xRes = image->xRes();
        const qreal W = image->width() / xRes;
        const qreal H = image->height() / image->yRes();
        const qreal mm = W * m / 100.0;
        const qreal vm = mm * (H / W > 1.2 ? 1.0 : 0.7);
        const QRectF area(mm, vm, W - 2 * mm, H - 2 * vm);
        const QList<QRectF> rects = layoutRects(layouts[chosen], area, W * gx / 100.0, W * gy / 100.0);
        const qreal stroke = bw / xRes;
        QString body;
        for (const QRectF &r : rects) {
            body += QStringLiteral("<rect x=\"%1\" y=\"%2\" width=\"%3\" height=\"%4\" fill=\"none\" stroke=\"#000000\" "
                                   "stroke-width=\"%5\" stroke-linejoin=\"miter\"/>")
                    .arg(r.x()).arg(r.y()).arg(r.width()).arg(r.height()).arg(stroke);
        }
        if (!addVectorShapes(mw, i18n("Requadros"), body, false)) {
            QMessageBox::warning(mw, QStringLiteral("Nanquim"), i18n("Não consegui criar os requadros."));
        }
    });
    d->open();
}

// ---------------------------------------------------------------------------
// Central de Quadrinhos: todas as ferramentas de mangá, quadrinhos, webtoon e
// pincéis num só lugar, em cartões com explicação. Abre pelo botão "Quadrinhos"
// da barra do topo e da barra do modo foco, e por Mais → Quadrinhos.
// ---------------------------------------------------------------------------
void showComicHubSheet(KisMainWindow *mw)
{
    if (!mw) return;
    QPointer<KisMainWindow> mwGuard(mw);
    const bool hasCanvas = mw->activeView() != nullptr;

    QDialog *d = makeSheet(mw, 760, qMin(mw->height() - 40, 680));
    QVBoxLayout *root = new QVBoxLayout(d);
    root->setContentsMargins(24, 20, 24, 20);
    root->setSpacing(12);
    root->addLayout(sheetHeader(d, i18n("Quadrinhos"), i18n("Mangá, quadrinhos, webtoon e pincéis")));

    QScrollArea *scroll = new QScrollArea(d);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setProperty("nanquimSmooth", true);
    tuneSmoothScroller(KisKineticScroller::createPreconfiguredScroller(scroll));
    QWidget *host = new QWidget(scroll);
    QVBoxLayout *hl = new QVBoxLayout(host);
    hl->setContentsMargins(0, 0, 6, 6);
    hl->setSpacing(8);
    scroll->setWidget(host);
    root->addWidget(scroll, 1);

    struct HubCard {
        const char *icon;
        QString title;
        QString info;
        bool needsCanvas;
        std::function<void()> run;
        std::function<QString()> liveTitle = nullptr;   // interruptor: não fecha a central e atualiza o título
    };

    // cada cartão fecha a central e abre a ferramenta logo depois (a central é modal)
    auto addSection = [&](const QString &name, const QVector<HubCard> &cards) {
        hl->addSpacing(10);
        QLabel *secLabel = nqLabel(name.toUpper(), "nqSection", host);
        secLabel->setStyleSheet(QStringLiteral("color: #6fbfad; font-size: 12px; font-weight: 700; letter-spacing: 1px; background: transparent;"));
        hl->addWidget(secLabel);
        hl->addWidget(nqDivider(host));
        QGridLayout *grid = new QGridLayout();
        grid->setSpacing(10);
        int i = 0;
        for (const HubCard &c : cards) {
            QPushButton *card = new QPushButton(host);
            card->setObjectName(QStringLiteral("nqHubCard"));
            card->setFocusPolicy(Qt::NoFocus);
            card->setMinimumHeight(74);
            card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
            card->setEnabled(hasCanvas || !c.needsCanvas);
            card->setStyleSheet(QStringLiteral(
                "QPushButton#nqHubCard { background: #2e2d29; border: 1px solid #4a4842; border-radius: 14px; text-align: left; }"
                "QPushButton#nqHubCard:pressed { background: #253d37; border-color: #3e8e7e; }"
                "QPushButton#nqHubCard:disabled { background: #262521; border-color: #393832; }"));
            QHBoxLayout *cl = new QHBoxLayout(card);
            cl->setContentsMargins(14, 10, 14, 10);
            cl->setSpacing(12);
            QLabel *ic = new QLabel(card);
            ic->setFixedSize(30, 30);
            ic->setAttribute(Qt::WA_TransparentForMouseEvents);
            ic->setStyleSheet(QStringLiteral("background: transparent;"));
            ic->setPixmap(svgPixmap(QString::fromUtf8(c.icon), QColor(111, 191, 173), 28));
            cl->addWidget(ic, 0, Qt::AlignTop);
            QVBoxLayout *tx = new QVBoxLayout();
            tx->setSpacing(2);
            QLabel *t = nqLabel(c.title, "nqItem", card, true);
            t->setAttribute(Qt::WA_TransparentForMouseEvents);
            t->setStyleSheet(QStringLiteral("background: transparent; color: #eeeae0; font-size: 14px; font-weight: 700;"));
            QLabel *inf = nqLabel(card->isEnabled() ? c.info : i18n("Abra uma página para usar."), "nqSmall", card, true);
            inf->setAttribute(Qt::WA_TransparentForMouseEvents);
            inf->setStyleSheet(QStringLiteral("background: transparent; color: #a8a398; font-size: 12px;"));
            tx->addWidget(t);
            tx->addWidget(inf);
            cl->addLayout(tx, 1);
            const std::function<void()> run = c.run;
            const std::function<QString()> live = c.liveTitle;
            QPointer<QLabel> titleGuard(t);
            QObject::connect(card, &QPushButton::clicked, d, [d, mwGuard, run, live, titleGuard]() {
                if (live) {
                    run();
                    if (titleGuard) titleGuard->setText(live());
                    return;
                }
                d->accept();
                QTimer::singleShot(180, mwGuard.data(), [run]() { run(); });
            });
            // cartão sozinho na última linha ocupa a largura toda
            const bool lastAlone = (i == cards.size() - 1) && (cards.size() % 2 == 1);
            grid->addWidget(card, i / 2, lastAlone ? 0 : i % 2, 1, lastAlone ? 2 : 1);
            ++i;
        }
        grid->setColumnStretch(0, 1);
        grid->setColumnStretch(1, 1);
        hl->addLayout(grid);
    };

    const std::function<QString()> bucketTitle = []() {
        return KSharedConfig::openConfig()->group("Nanquim").readEntry("comicBucketMode", false)
                ? i18n("Lata: modo quadrinho · LIGADO") : i18n("Lata: modo quadrinho · desligado");
    };
    addSection(i18n("Mangá"), {
        {Svg::TONE, i18n("Retícula"), i18n("Pontos, linhas ou grade, recortados pela seleção."), true,
         [mwGuard]() { if (mwGuard) showScreentoneSheet(mwGuard); }},
        {Svg::SPEED, i18n("Linhas de velocidade e foco"), i18n("Gera as linhas de ação e de concentração."), true,
         [mwGuard]() { if (mwGuard) showSpeedlinesSheet(mwGuard); }},
        {Svg::EXTRACT, i18n("Extrair linha"), i18n("Tira a arte-final de uma foto ou rascunho, numa camada nova."), true,
         [mwGuard]() { if (mwGuard) showExtractLineSheet(mwGuard); }},
        {Svg::OUTLINE, i18n("Contorno na camada"), i18n("Cria um contorno ao redor do que está na camada."), true,
         [mwGuard]() { if (mwGuard) showLayerOutlineSheet(mwGuard); }},
        {Svg::DRAFT, i18n("Rascunho azul"), i18n("Passa a camada para azul claro em 1 toque."), true,
         [mwGuard]() { if (mwGuard) applyBlueDraftLayer(mwGuard); }},
        {Svg::BUCKET, bucketTitle(), i18n("A lata de tinta cobre lacunas e passa um pouco da linha."), true,
         [mwGuard]() { if (mwGuard) toggleComicBucketPreset(mwGuard); }, bucketTitle},
    });

    addSection(i18n("Quadrinhos"), {
        {Svg::PANELS, i18n("Requadros"), i18n("Organiza a página em 9 arranjos de quadros."), true,
         [mwGuard]() { if (mwGuard) showComicsSheet(mwGuard, 0); }},
        {Svg::BALLOON, i18n("Balões de fala"), i18n("Balões vetoriais para colocar e ajustar."), true,
         [mwGuard]() { if (mwGuard) showComicsSheet(mwGuard, 1); }},
        {Svg::RULER, i18n("Réguas, perspectiva e ímã"), i18n("Réguas de perspectiva e paralela, grade e ímã."), true,
         [mwGuard]() {
             if (!mwGuard) return;
             QMenu *m = new QMenu(mwGuard);
             m->setProperty("nanquimNoHub", true);
             m->setAttribute(Qt::WA_DeleteOnClose);
             populateRulersMenu(mwGuard, m, mwGuard);
             m->popup(mwGuard->mapToGlobal(mwGuard->rect().center()) - QPoint(120, 120));
         }},
    });

    addSection(i18n("Webtoon"), {
        {Svg::PHONE, i18n("Prévia de celular"), i18n("Veja a tira como no celular (9:19,5), com marcas de corte."), true,
         [mwGuard]() { if (mwGuard) showWebtoonToolsSheet(mwGuard, 0); }},
        {Svg::SLICE, i18n("Fatiar e nova tira"), i18n("Corta a página em fatias de 800 × 1280 px ou cria uma tira vertical."), true,
         [mwGuard]() { if (mwGuard) showWebtoonToolsSheet(mwGuard, 1); }},
    });

    addSection(i18n("Pincéis"), {
        {Svg::IMPORT, i18n("Importar pincéis"), i18n("Abre arquivos .sut do Clip Studio, .kpp, .bundle ou .zip."), false,
         [mwGuard]() { if (mwGuard) importBrushFiles(mwGuard); }},
        {Svg::BOOK, i18n("Catálogo da comunidade"), i18n("Baixe pincéis e pacotes prontos, ou cole um link."), false,
         [mwGuard]() { if (mwGuard) showCommunityCatalogSheet(mwGuard); }},
    });
    hl->addStretch(1);

    d->open();
}

// ---------------------------------------------------------------------------
// Recursos de Mangá (patch 0120): menu rápido de Réguas + Ímã, gerador de
// Retícula recortada pela seleção e extração de linha de camada/foto.
// ---------------------------------------------------------------------------
void populateRulersMenu(KisMainWindow *mw, QMenu *menu, QWidget *anchor)
{
    if (!mw || !menu) return;
    menu->clear();
    QPointer<KisMainWindow> mwGuard(mw);
    QPointer<QWidget> anchorGuard(anchor);

    if (!menu->property("nanquimNoHub").toBool()) {
        QAction *hubAct = menu->addAction(designIcon(Svg::HUB, 18), i18n("Central de Quadrinhos…"));
        QObject::connect(hubAct, &QAction::triggered, mw, [mwGuard]() {
            if (mwGuard) showComicHubSheet(mwGuard);
        });
        menu->addSeparator();
    }
    if (QAction *assistant = act(mw, "KisAssistantTool")) {
        QAction *editRulers = menu->addAction(designIcon(Svg::RULER, 18),
                                              i18n("Editar réguas (Perspectiva / Paralela)"));
        QObject::connect(editRulers, &QAction::triggered, mw, [mwGuard, assistant, anchorGuard]() {
            if (!mwGuard) return;
            assistant->trigger();
            if (QDockWidget *opts = findDock(mwGuard, {QStringLiteral("sharedtooldocker")})) {
                if (!panelOpen(mwGuard, opts)) {
                    togglePanelFrom(mwGuard, opts, anchorGuard);
                }
            }
            showToast(mwGuard, i18n("Escolha a régua nas opções ao lado"));
        });
    }
    for (const char *name : {"toggle_assistant", "view_toggle_painting_assistants",
                             "view_toggle_assistant_previews", "KritaShape/KisToolMeasure",
                             "view_grid", "view_snap_to_grid"}) {
        if (QAction *a = act(mw, name)) {
            menu->addAction(a);
        }
    }
    menu->addSeparator();
    QAction *toneAct = menu->addAction(designIcon(Svg::TONE, 18), i18n("Aplicar retícula…"));
    QObject::connect(toneAct, &QAction::triggered, mw, [mwGuard]() {
        if (mwGuard) showScreentoneSheet(mwGuard);
    });
    QAction *speedAct = menu->addAction(designIcon(Svg::SPEED, 18), i18n("Linhas de velocidade e foco…"));
    QObject::connect(speedAct, &QAction::triggered, mw, [mwGuard]() {
        if (mwGuard) showSpeedlinesSheet(mwGuard);
    });
    QAction *outlineAct = menu->addAction(designIcon(Svg::OUTLINE, 18), i18n("Contorno na camada…"));
    QObject::connect(outlineAct, &QAction::triggered, mw, [mwGuard]() {
        if (mwGuard) showLayerOutlineSheet(mwGuard);
    });
    QAction *blueDraftAct = menu->addAction(designIcon(Svg::DRAFT, 18), i18n("Rascunho azul (1 toque)"));
    QObject::connect(blueDraftAct, &QAction::triggered, mw, [mwGuard]() {
        if (mwGuard) applyBlueDraftLayer(mwGuard);
    });
    QAction *extractAct = menu->addAction(designIcon(Svg::EXTRACT, 18), i18n("Extrair linha da camada…"));
    QObject::connect(extractAct, &QAction::triggered, mw, [mwGuard]() {
        if (mwGuard) showExtractLineSheet(mwGuard);
    });
    QAction *bucketAct = menu->addAction(designIcon(Svg::BUCKET, 18), i18n("Lata de tinta: Modo Quadrinho"));
    bucketAct->setCheckable(true);
    bucketAct->setChecked(KSharedConfig::openConfig()->group("Nanquim").readEntry("comicBucketMode", false));
    QObject::connect(bucketAct, &QAction::triggered, mw, [mwGuard]() {
        if (mwGuard) toggleComicBucketPreset(mwGuard);
    });
    QAction *panelsAct = menu->addAction(designIcon(Svg::PANELS, 18), i18n("Criar requadros e balões…"));
    QObject::connect(panelsAct, &QAction::triggered, mw, [mwGuard]() {
        if (mwGuard) showComicsSheet(mwGuard, 0);
    });
    menu->addSeparator();
    QAction *webtoonAct = menu->addAction(designIcon(Svg::PHONE, 18), i18n("Kit Webtoon (Prévia e Fatiar)…"));
    QObject::connect(webtoonAct, &QAction::triggered, mw, [mwGuard]() {
        if (mwGuard) showWebtoonToolsSheet(mwGuard, 0);
    });
    QAction *catalogAct = menu->addAction(designIcon(Svg::IMPORT, 18), i18n("Catálogo da comunidade…"));
    QObject::connect(catalogAct, &QAction::triggered, mw, [mwGuard]() {
        if (mwGuard) showCommunityCatalogSheet(mwGuard);
    });
}

// Gera retícula anti-aliased alinhada às coordenadas da página (sem moiré)
// pattern: 0 = Pontos, 1 = Linhas (hachura), 2 = Grade (hachura cruzada)
QImage renderScreentoneImage(const QSize &size, const QPoint &origin, int pattern,
                             qreal density, qreal stepPx, qreal angleDeg, const QColor &ink)
{
    QImage img(size, QImage::Format_ARGB32);
    if (img.isNull()) return img;
    const qreal clampedDensity = qBound(0.03, density, 0.90);
    const qreal step = qMax(2.5, stepPx);
    const qreal invStep = 1.0 / step;
    const qreal rad = angleDeg * (M_PI / 180.0);
    const qreal cosA = std::cos(rad) * invStep;
    const qreal sinA = std::sin(rad) * invStep;
    const qreal aa = qMax(0.08, invStep * 0.65);

    const qreal dotR = std::sqrt(clampedDensity / M_PI);
    const qreal lineHalf = clampedDensity * 0.5;
    const qreal gridHalf = (1.0 - std::sqrt(qMax(0.0, 1.0 - clampedDensity))) * 0.5;

    const int rC = ink.red();
    const int gC = ink.green();
    const int bC = ink.blue();
    const int w = size.width();
    const int h = size.height();
    const qreal ox = origin.x();
    const qreal oy = origin.y();

    for (int y = 0; y < h; ++y) {
        QRgb *row = reinterpret_cast<QRgb*>(img.scanLine(y));
        const qreal py = oy + y;
        qreal fu = ox * cosA + py * sinA;
        qreal fv = -ox * sinA + py * cosA;
        for (int x = 0; x < w; ++x) {
            const qreal du = fu - std::floor(fu) - 0.5;
            const qreal dv = fv - std::floor(fv) - 0.5;
            qreal dist = 0.0;
            qreal thresh = 0.0;
            if (pattern == 0) {
                dist = std::sqrt(du * du + dv * dv);
                thresh = dotR;
            } else if (pattern == 1) {
                dist = std::fabs(dv);
                thresh = lineHalf;
            } else {
                dist = qMin(std::fabs(du), std::fabs(dv));
                thresh = gridHalf;
            }
            const qreal cov = qBound(0.0, (thresh - dist) / aa + 0.5, 1.0);
            const int alpha = qRound(cov * 255.0);
            row[x] = alpha > 0 ? qRgba(rC, gC, bC, alpha) : 0;
            fu += cosA;
            fv -= sinA;
        }
    }
    return img;
}

void showScreentoneSheet(KisMainWindow *mw)
{
    if (!mw) return;
    KisView *view = mw->activeView();
    KisViewManager *vm = mw->viewManager();
    if (!view || !view->image() || !vm) return;
    KisImageSP image = view->image();

    KisSelectionSP sel = vm->selection();
    if (!sel) sel = image->globalSelection();
    QRect selRect;
    if (sel) selRect = sel->selectedExactRect().intersected(image->bounds());
    const bool hasSel = !selRect.isEmpty();

    QDialog *d = makeSheet(mw, 560);
    QVBoxLayout *root = new QVBoxLayout(d);
    root->setContentsMargins(24, 20, 24, 20);
    root->setSpacing(14);
    root->addLayout(sheetHeader(d, i18n("Mangá"),
                                hasSel ? i18n("Retícula na seleção") : i18n("Retícula na página")));

    QHBoxLayout *body = new QHBoxLayout();
    body->setSpacing(18);

    // Prévia ao vivo sobre papel creme
    QLabel *preview = new QLabel(d);
    preview->setFixedSize(172, 172);
    preview->setAlignment(Qt::AlignCenter);
    preview->setStyleSheet(QStringLiteral(
        "background: #f3eee3; border: 1px solid #524f48; border-radius: 14px;"));
    body->addWidget(preview, 0, Qt::AlignTop);

    QVBoxLayout *controls = new QVBoxLayout();
    controls->setSpacing(10);

    controls->addWidget(nqLabel(i18n("Padrão"), "nqSmall", d));
    QButtonGroup *patGroup = new QButtonGroup(d);
    controls->addLayout(chipRow(d, patGroup, {i18n("Pontos"), i18n("Linhas"), i18n("Grade")}, 0));

    QHBoxLayout *densHead = new QHBoxLayout();
    densHead->addWidget(nqLabel(i18n("Densidade"), "nqSmall", d));
    densHead->addStretch(1);
    QLabel *densVal = nqLabel(QStringLiteral("25%"), "nqItem", d);
    densHead->addWidget(densVal);
    controls->addLayout(densHead);

    QSlider *densSlider = new QSlider(Qt::Horizontal, d);
    densSlider->setRange(5, 80);
    densSlider->setValue(25);
    densSlider->setFocusPolicy(Qt::NoFocus);
    controls->addWidget(densSlider);

    controls->addWidget(nqLabel(i18n("Frequência (LPI)"), "nqSmall", d));
    const QList<int> lpiList = {45, 60, 75, 85};
    QButtonGroup *lpiGroup = new QButtonGroup(d);
    controls->addLayout(chipRow(d, lpiGroup, {QStringLiteral("45"), QStringLiteral("60"),
                                              QStringLiteral("75"), QStringLiteral("85")}, 1));

    controls->addWidget(nqLabel(i18n("Ângulo"), "nqSmall", d));
    const QList<int> angleList = {45, 0, 30, 60, 90};
    QButtonGroup *angleGroup = new QButtonGroup(d);
    controls->addLayout(chipRow(d, angleGroup, {QStringLiteral("45°"), QStringLiteral("0°"),
                                                QStringLiteral("30°"), QStringLiteral("60°"),
                                                QStringLiteral("90°")}, 0));

    controls->addWidget(nqLabel(i18n("Cor"), "nqSmall", d));
    QButtonGroup *colorGroup = new QButtonGroup(d);
    controls->addLayout(chipRow(d, colorGroup, {i18n("Nanquim (preto)"), i18n("Cor atual")}, 0));

    body->addLayout(controls, 1);
    root->addLayout(body);
    root->addWidget(nqDivider(d));

    QHBoxLayout *foot = new QHBoxLayout();
    foot->setSpacing(10);
    foot->addWidget(nqLabel(hasSel
                            ? i18n("Cria uma nova camada recortada pela seleção.")
                            : i18n("Sem seleção ativa: aplica na página inteira."), "nqSmall", d, true), 1);
    QPushButton *cancelBtn = nqButton(i18n("Cancelar"), d, "nqGhost");
    QPushButton *applyBtn = nqButton(i18n("Aplicar retícula"), d, "nqPrimary");
    QObject::connect(cancelBtn, &QPushButton::clicked, d, &QDialog::reject);
    foot->addWidget(cancelBtn);
    foot->addWidget(applyBtn);
    root->addLayout(foot);

    const QColor fgColor = vm->canvasResourceProvider()
            ? vm->canvasResourceProvider()->fgColor().toQColor()
            : QColor(0x14, 0x14, 0x14);

    auto refreshPreview = [preview, patGroup, densSlider, densVal, lpiGroup, lpiList,
                           angleGroup, angleList, colorGroup, fgColor]() {
        const int pat = qMax(0, patGroup->checkedId());
        const int pct = densSlider->value();
        densVal->setText(QStringLiteral("%1%").arg(pct));
        const int lpi = lpiList.value(qMax(0, lpiGroup->checkedId()), 60);
        const int ang = angleList.value(qMax(0, angleGroup->checkedId()), 45);
        const QColor ink = colorGroup->checkedId() == 1 ? fgColor : QColor(0x14, 0x14, 0x14);
        // prévia ampliada ~2,2x para enxergar nitidamente os pontos no tablet
        const qreal previewStep = qBound(4.0, 540.0 / qMax(20, lpi), 14.0);
        QImage tone = renderScreentoneImage(QSize(156, 156), QPoint(0, 0), pat, pct / 100.0, previewStep, ang, ink);
        QImage composed(QSize(156, 156), QImage::Format_ARGB32);
        composed.fill(QColor(0xf3, 0xee, 0xe3));
        QPainter p(&composed);
        p.drawImage(0, 0, tone);
        p.end();
        preview->setPixmap(QPixmap::fromImage(composed));
    };

    QObject::connect(patGroup, &QButtonGroup::idClicked, d, [refreshPreview](int) { refreshPreview(); });
    QObject::connect(lpiGroup, &QButtonGroup::idClicked, d, [refreshPreview](int) { refreshPreview(); });
    QObject::connect(angleGroup, &QButtonGroup::idClicked, d, [refreshPreview](int) { refreshPreview(); });
    QObject::connect(colorGroup, &QButtonGroup::idClicked, d, [refreshPreview](int) { refreshPreview(); });
    QObject::connect(densSlider, &QSlider::valueChanged, d, [refreshPreview](int) { refreshPreview(); });
    refreshPreview();

    QPointer<KisMainWindow> mwGuard(mw);
    QObject::connect(applyBtn, &QPushButton::clicked, d, [=]() {
        const int pat = qMax(0, patGroup->checkedId());
        const int pct = densSlider->value();
        const int lpi = lpiList.value(qMax(0, lpiGroup->checkedId()), 60);
        const int ang = angleList.value(qMax(0, angleGroup->checkedId()), 45);
        const QColor ink = colorGroup->checkedId() == 1 ? fgColor : QColor(0x14, 0x14, 0x14);
        d->accept();

        if (!mwGuard) return;
        KisView *v = mwGuard->activeView();
        KisViewManager *m = mwGuard->viewManager();
        if (!v || !v->image() || !m) return;
        KisImageSP imgSp = v->image();

        KisSelectionSP activeSel = m->selection();
        if (!activeSel) activeSel = imgSp->globalSelection();
        QRect target = imgSp->bounds();
        bool clipped = false;
        if (activeSel) {
            const QRect sr = activeSel->selectedExactRect().intersected(imgSp->bounds());
            if (!sr.isEmpty()) {
                target = sr;
                clipped = true;
            }
        }
        if (target.isEmpty()) return;

        const qreal dpi = qMax(72.0, imgSp->xRes() * 72.0);
        const qreal stepPx = qMax(3.0, dpi / qMax(10, lpi));
        const QImage tone = renderScreentoneImage(target.size(), target.topLeft(), pat,
                                                  pct / 100.0, stepPx, ang, ink);
        if (tone.isNull()) return;

        const QString name = i18n("Retícula · %1% · %2 LPI", pct, lpi);
        KisPaintLayerSP layer = new KisPaintLayer(imgSp, name, OPACITY_OPAQUE_U8);
        if (clipped) {
            KisPaintDeviceSP tmp = new KisPaintDevice(imgSp->colorSpace());
            tmp->convertFromQImage(tone, nullptr, target.x(), target.y());
            KisPainter painter(layer->paintDevice(), activeSel);
            painter.bitBlt(target.x(), target.y(), tmp, target.x(), target.y(), target.width(), target.height());
            painter.end();
        } else {
            layer->paintDevice()->convertFromQImage(tone, nullptr, target.x(), target.y());
        }

        KisNodeSP active = m->activeNode();
        KisNodeSP parent = (active && active->parent()) ? active->parent() : KisNodeSP(imgSp->root());
        KisNodeSP above = (active && active->parent()) ? active : parent->lastChild();
        KisNodeCommandsAdapter adapter(m);
        adapter.addNode(layer, parent, above);
        showToast(mwGuard, i18n("Retícula aplicada · %1% · %2 LPI", pct, lpi));
    });
    d->open();
}

// Extrai contornos limpos (Sobel + preservação opcional de traços escuros) com fundo transparente
QImage extractLineImage(const QImage &src, int sensitivity, int cleanup, bool keepDark)
{
    if (src.isNull() || src.width() < 2 || src.height() < 2) return QImage();
    const QImage in = (src.format() == QImage::Format_ARGB32) ? src : src.convertToFormat(QImage::Format_ARGB32);
    const int w = in.width();
    const int h = in.height();

    QVector<quint8> lum(w * h, 255);
    QVector<quint8> alphaMask(w * h, 0);
    for (int y = 0; y < h; ++y) {
        const QRgb *row = reinterpret_cast<const QRgb*>(in.constScanLine(y));
        const int off = y * w;
        for (int x = 0; x < w; ++x) {
            const QRgb px = row[x];
            const int a = qAlpha(px);
            alphaMask[off + x] = static_cast<quint8>(a);
            if (a == 0) {
                lum[off + x] = 255;
            } else {
                const int g = (qRed(px) * 77 + qGreen(px) * 150 + qBlue(px) * 29) >> 8;
                // compõe sobre papel branco para evitar borda falsa no recorte da transparência
                lum[off + x] = static_cast<quint8>((g * a + 255 * (255 - a)) / 255);
            }
        }
    }

    QImage out(w, h, QImage::Format_ARGB32);
    out.fill(0);
    const qreal gain = 0.25 + (sensitivity / 100.0) * 1.85;
    const int floorCut = qRound(cleanup * 1.45);
    const int darkCut = qBound(20, 145 - cleanup, 145);

    for (int y = 1; y < h - 1; ++y) {
        QRgb *dst = reinterpret_cast<QRgb*>(out.scanLine(y));
        const int r0 = (y - 1) * w;
        const int r1 = y * w;
        const int r2 = (y + 1) * w;
        for (int x = 1; x < w - 1; ++x) {
            const int a = alphaMask[r1 + x];
            if (a < 8) continue;
            const int tl = lum[r0 + x - 1], tc = lum[r0 + x], tr = lum[r0 + x + 1];
            const int ml = lum[r1 + x - 1], mc = lum[r1 + x], mr = lum[r1 + x + 1];
            const int bl = lum[r2 + x - 1], bc = lum[r2 + x], br = lum[r2 + x + 1];

            const int gx = -tl + tr - 2 * ml + 2 * mr - bl + br;
            const int gy = -tl - 2 * tc - tr + bl + 2 * bc + br;
            const int mag = qRound((std::abs(gx) + std::abs(gy)) * 0.25 * gain);

            int ink = 0;
            if (mag > floorCut) {
                ink = qBound(0, qRound((mag - floorCut) * 2.2), 255);
            }
            if (keepDark && mc < darkCut) {
                const int darkInk = qBound(0, qRound((darkCut - mc) * (255.0 / qMax(1, darkCut)) * 1.35), 255);
                if (darkInk > ink) ink = darkInk;
            }
            ink = (ink * a) / 255;
            if (ink > 4) {
                dst[x] = qRgba(0x11, 0x11, 0x11, ink);
            }
        }
    }
    return out;
}

void showExtractLineSheet(KisMainWindow *mw)
{
    if (!mw) return;
    KisView *view = mw->activeView();
    KisViewManager *vm = mw->viewManager();
    if (!view || !view->image() || !vm) return;
    KisImageSP image = view->image();

    KisNodeSP active = vm->activeNode();
    KisPaintDeviceSP srcDev = (active && active->paintDevice()) ? active->paintDevice() : image->projection();
    const QRect bounds = srcDev ? srcDev->exactBounds().intersected(image->bounds()) : QRect();
    if (bounds.isEmpty()) {
        showToast(mw, i18n("A camada ativa está vazia"));
        return;
    }

    const QImage fullSrc = srcDev->convertToQImage(nullptr, bounds).convertToFormat(QImage::Format_ARGB32);
    if (fullSrc.isNull()) {
        showToast(mw, i18n("Não foi possível ler a camada ativa"));
        return;
    }
    const QImage smallSrc = (fullSrc.width() > 360 || fullSrc.height() > 240)
            ? fullSrc.scaled(360, 240, Qt::KeepAspectRatio, Qt::SmoothTransformation)
            : fullSrc;

    QDialog *d = makeSheet(mw, 560);
    QVBoxLayout *root = new QVBoxLayout(d);
    root->setContentsMargins(24, 20, 24, 20);
    root->setSpacing(14);
    root->addLayout(sheetHeader(d, i18n("Mangá"), i18n("Extrair linha da camada")));

    QLabel *preview = new QLabel(d);
    preview->setFixedHeight(230);
    preview->setAlignment(Qt::AlignCenter);
    preview->setStyleSheet(QStringLiteral(
        "background: #f3eee3; border: 1px solid #524f48; border-radius: 14px;"));
    root->addWidget(preview);

    QHBoxLayout *sensHead = new QHBoxLayout();
    sensHead->addWidget(nqLabel(i18n("Sensibilidade de borda"), "nqSmall", d));
    sensHead->addStretch(1);
    QLabel *sensVal = nqLabel(QStringLiteral("55%"), "nqItem", d);
    sensHead->addWidget(sensVal);
    root->addLayout(sensHead);

    QSlider *sensSlider = new QSlider(Qt::Horizontal, d);
    sensSlider->setRange(10, 100);
    sensSlider->setValue(55);
    sensSlider->setFocusPolicy(Qt::NoFocus);
    root->addWidget(sensSlider);

    QHBoxLayout *cleanHead = new QHBoxLayout();
    cleanHead->addWidget(nqLabel(i18n("Limpeza / Contraste"), "nqSmall", d));
    cleanHead->addStretch(1);
    QLabel *cleanVal = nqLabel(QStringLiteral("45%"), "nqItem", d);
    cleanHead->addWidget(cleanVal);
    root->addLayout(cleanHead);

    QSlider *cleanSlider = new QSlider(Qt::Horizontal, d);
    cleanSlider->setRange(0, 100);
    cleanSlider->setValue(45);
    cleanSlider->setFocusPolicy(Qt::NoFocus);
    root->addWidget(cleanSlider);

    QCheckBox *keepDark = new QCheckBox(i18n("Preservar traços escuros (ideal para fotos 3D e rascunhos)"), d);
    keepDark->setChecked(true);
    keepDark->setFocusPolicy(Qt::NoFocus);
    root->addWidget(keepDark);

    root->addWidget(nqDivider(d));

    QHBoxLayout *foot = new QHBoxLayout();
    foot->setSpacing(10);
    foot->addWidget(nqLabel(i18n("Gera uma nova camada transparente pronta para arte-final."), "nqSmall", d, true), 1);
    QPushButton *cancelBtn = nqButton(i18n("Cancelar"), d, "nqGhost");
    QPushButton *applyBtn = nqButton(i18n("Extrair linha"), d, "nqPrimary");
    QObject::connect(cancelBtn, &QPushButton::clicked, d, &QDialog::reject);
    foot->addWidget(cancelBtn);
    foot->addWidget(applyBtn);
    root->addLayout(foot);

    auto refreshPreview = [preview, smallSrc, sensSlider, sensVal, cleanSlider, cleanVal, keepDark]() {
        sensVal->setText(QStringLiteral("%1%").arg(sensSlider->value()));
        cleanVal->setText(QStringLiteral("%1%").arg(cleanSlider->value()));
        const QImage linesImg = extractLineImage(smallSrc, sensSlider->value(), cleanSlider->value(), keepDark->isChecked());
        QImage composed(smallSrc.size(), QImage::Format_ARGB32);
        composed.fill(QColor(0xf3, 0xee, 0xe3));
        QPainter p(&composed);
        p.drawImage(0, 0, linesImg);
        p.end();
        preview->setPixmap(QPixmap::fromImage(composed.scaled(preview->size() - QSize(12, 12),
                                                              Qt::KeepAspectRatio, Qt::SmoothTransformation)));
    };
    QObject::connect(sensSlider, &QSlider::valueChanged, d, [refreshPreview](int) { refreshPreview(); });
    QObject::connect(cleanSlider, &QSlider::valueChanged, d, [refreshPreview](int) { refreshPreview(); });
    QObject::connect(keepDark, &QCheckBox::toggled, d, [refreshPreview](bool) { refreshPreview(); });
    QTimer::singleShot(0, d, refreshPreview);

    QPointer<KisMainWindow> mwGuard(mw);
    QObject::connect(applyBtn, &QPushButton::clicked, d, [=]() {
        const int sens = sensSlider->value();
        const int clean = cleanSlider->value();
        const bool dark = keepDark->isChecked();
        d->accept();

        if (!mwGuard) return;
        KisView *v = mwGuard->activeView();
        KisViewManager *m = mwGuard->viewManager();
        if (!v || !v->image() || !m) return;
        KisImageSP imgSp = v->image();

        const QImage out = extractLineImage(fullSrc, sens, clean, dark);
        if (out.isNull()) return;

        KisPaintLayerSP layer = new KisPaintLayer(imgSp, i18n("Linha extraída"), OPACITY_OPAQUE_U8);
        layer->paintDevice()->convertFromQImage(out, nullptr, bounds.x(), bounds.y());

        KisNodeSP cur = m->activeNode();
        KisNodeSP parent = (cur && cur->parent()) ? cur->parent() : KisNodeSP(imgSp->root());
        KisNodeSP above = (cur && cur->parent()) ? cur : parent->lastChild();
        KisNodeCommandsAdapter adapter(m);
        adapter.addNode(layer, parent, above);
        showToast(mwGuard, i18n("Linha extraída em nova camada"));
    });
    d->open();
}

// ---------------------------------------------------------------------------
// Kit Quadrinhos e Webtoon (patch 0121):
// 1. Gerador de Linhas de Velocidade e Foco (radial interativo + paralelas)
// 2. Contorno automático na camada (transformada de distância euclidiana 2D)
// 3. Rascunho azul em 1 toque (Non-Photo Blue ignorado ao exportar)
// 4. Lata de tinta "Modo Quadrinho" (expande 2 px, fecha lacunas, todas camadas)
// 5. Kit Webtoon (prévia de celular com marcas de corte, fatiador e nova tira)
// ---------------------------------------------------------------------------

// Renderiza linhas de foco (mode == 0) ou velocidade paralela (mode == 1)
QImage renderSpeedlinesImage(const QSize &size, int mode, const QPointF &centerNorm,
                             qreal innerRadiusPct, int count, qreal thicknessPx,
                             qreal angleDeg, int jitterPct, const QColor &ink, quint32 seed)
{
    QImage img(size, QImage::Format_ARGB32);
    if (img.isNull() || size.width() < 2 || size.height() < 2) return img;
    img.fill(0);

    QPainter p(&img);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setPen(Qt::NoPen);
    p.setBrush(ink);

    QRandomGenerator rng(seed);
    const qreal w = size.width();
    const qreal h = size.height();
    const qreal diag = std::hypot(w, h);
    const qreal jitter = qBound(0.0, jitterPct / 100.0, 1.0);
    const int n = qBound(12, count, 360);

    if (mode == 0) {
        // Linhas de foco radiais (triângulos afunilados apontando para o centro)
        const QPointF c(qBound(0.05, centerNorm.x(), 0.95) * w,
                        qBound(0.05, centerNorm.y(), 0.95) * h);
        const qreal baseInner = qMin(w, h) * 0.5 * qBound(0.06, innerRadiusPct / 100.0, 0.85);
        const qreal outerR = diag * 1.15;
        const qreal stepAngle = (2.0 * M_PI) / n;

        for (int i = 0; i < n; ++i) {
            const qreal aJitter = (rng.generateDouble() - 0.5) * stepAngle * 0.85 * jitter;
            const qreal ang = i * stepAngle + aJitter;
            const qreal rJitter = 1.0 + (rng.generateDouble() - 0.35) * 0.55 * jitter;
            const qreal rIn = qMax(6.0, baseInner * rJitter);
            const qreal wJitter = 0.55 + rng.generateDouble() * 0.95 * jitter + (1.0 - jitter) * 0.45;
            const qreal halfBase = qMax(0.6, thicknessPx * wJitter * 0.5);

            const qreal ca = std::cos(ang);
            const qreal sa = std::sin(ang);
            const QPointF tip(c.x() + ca * rIn, c.y() + sa * rIn);
            const QPointF outC(c.x() + ca * outerR, c.y() + sa * outerR);
            const QPointF perp(-sa * halfBase, ca * halfBase);

            QPolygonF poly;
            poly << tip << (outC + perp) << (outC - perp);
            p.drawPolygon(poly);
        }
    } else {
        // Linhas de velocidade paralelas com pontas afuniladas
        p.translate(w * 0.5, h * 0.5);
        p.rotate(angleDeg);
        const qreal span = diag * 0.65;
        const qreal band = diag * 0.65;
        const qreal step = (2.0 * band) / n;

        for (int i = 0; i < n; ++i) {
            const qreal yBase = -band + (i + 0.5) * step;
            const qreal y = yBase + (rng.generateDouble() - 0.5) * step * 0.85 * jitter;
            const qreal lenFactor = 0.45 + rng.generateDouble() * 0.55;
            const qreal halfLen = span * (jitter > 0.05 ? lenFactor : 1.0);
            const qreal xShift = (rng.generateDouble() - 0.5) * span * 0.7 * jitter;
            const qreal halfW = qMax(0.5, thicknessPx * (0.45 + rng.generateDouble() * 0.85 * jitter) * 0.5);

            const qreal x1 = xShift - halfLen;
            const qreal x2 = xShift + halfLen;
            const qreal xm = (x1 + x2) * 0.5;
            QPolygonF poly;
            poly << QPointF(x1, y)
                 << QPointF(xm, y - halfW)
                 << QPointF(x2, y)
                 << QPointF(xm, y + halfW);
            p.drawPolygon(poly);
        }
    }
    p.end();
    return img;
}

class SpeedlinesPreview : public QWidget
{
public:
    explicit SpeedlinesPreview(QWidget *parent) : QWidget(parent)
    {
        setFixedSize(196, 232);
        setCursor(Qt::CrossCursor);
    }
    void setImage(const QImage &img, bool showCenter, const QPointF &centerNorm)
    {
        m_img = img;
        m_showCenter = showCenter;
        m_center = centerNorm;
        update();
    }
    std::function<void(const QPointF &)> onCenterChanged;
protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter g(this);
        g.setRenderHint(QPainter::Antialiasing, true);
        const QRectF box = QRectF(rect()).adjusted(1.0, 1.0, -1.0, -1.0);
        QPainterPath clip;
        clip.addRoundedRect(box, 12.0, 12.0);
        g.save();
        g.setClipPath(clip);
        g.fillRect(box, QColor(0xf3, 0xee, 0xe3));
        if (!m_img.isNull()) {
            g.drawImage(box, m_img, QRectF(m_img.rect()));
        }
        if (m_showCenter) {
            const QPointF c(box.left() + m_center.x() * box.width(),
                            box.top() + m_center.y() * box.height());
            g.setPen(QPen(QColor(0x2f, 0x75, 0x68), 2.0));
            g.setBrush(QColor(0x6f, 0xbf, 0xad, 180));
            g.drawEllipse(c, 6.0, 6.0);
            g.drawLine(c + QPointF(-11, 0), c + QPointF(11, 0));
            g.drawLine(c + QPointF(0, -11), c + QPointF(0, 11));
        }
        g.restore();
        g.setPen(QPen(QColor(0x52, 0x4f, 0x48), 1.0));
        g.setBrush(Qt::NoBrush);
        g.drawRoundedRect(box, 12.0, 12.0);
    }
    void mousePressEvent(QMouseEvent *e) override { updateCenter(e->pos()); }
    void mouseMoveEvent(QMouseEvent *e) override
    {
        if (e->buttons() & Qt::LeftButton) updateCenter(e->pos());
    }
private:
    void updateCenter(const QPoint &pt)
    {
        if (!m_showCenter || width() <= 0 || height() <= 0) return;
        m_center = QPointF(qBound(0.08, pt.x() / qreal(width()), 0.92),
                           qBound(0.08, pt.y() / qreal(height()), 0.92));
        if (onCenterChanged) onCenterChanged(m_center);
        update();
    }
    QImage m_img;
    bool m_showCenter = true;
    QPointF m_center = QPointF(0.5, 0.5);
};

void showSpeedlinesSheet(KisMainWindow *mw)
{
    if (!mw) return;
    KisView *view = mw->activeView();
    KisViewManager *vm = mw->viewManager();
    if (!view || !view->image() || !vm) return;
    KisImageSP image = view->image();

    KisSelectionSP sel = vm->selection();
    if (!sel) sel = image->globalSelection();
    const bool hasSel = sel && !sel->selectedExactRect().intersected(image->bounds()).isEmpty();

    QDialog *d = makeSheet(mw, 620);
    QVBoxLayout *root = new QVBoxLayout(d);
    root->setContentsMargins(24, 20, 24, 20);
    root->setSpacing(14);
    root->addLayout(sheetHeader(d, i18n("Mangá"), i18n("Linhas de velocidade e foco")));

    QHBoxLayout *body = new QHBoxLayout();
    body->setSpacing(18);

    QVBoxLayout *leftCol = new QVBoxLayout();
    leftCol->setSpacing(6);
    SpeedlinesPreview *preview = new SpeedlinesPreview(d);
    leftCol->addWidget(preview, 0, Qt::AlignTop);
    QLabel *hint = nqLabel(i18n("Arraste o ponto verde para mover o centro."), "nqSmall", d, true);
    hint->setFixedWidth(196);
    leftCol->addWidget(hint);
    leftCol->addStretch(1);
    body->addLayout(leftCol);

    QVBoxLayout *controls = new QVBoxLayout();
    controls->setSpacing(9);

    controls->addWidget(nqLabel(i18n("Tipo"), "nqSmall", d));
    QButtonGroup *modeGroup = new QButtonGroup(d);
    controls->addLayout(chipRow(d, modeGroup, {i18n("Foco (radial)"), i18n("Velocidade (paralelas)")}, 0));

    QHBoxLayout *countHead = new QHBoxLayout();
    countHead->addWidget(nqLabel(i18n("Quantidade de linhas"), "nqSmall", d));
    countHead->addStretch(1);
    QLabel *countVal = nqLabel(QStringLiteral("110"), "nqItem", d);
    countHead->addWidget(countVal);
    controls->addLayout(countHead);
    QSlider *countSlider = new QSlider(Qt::Horizontal, d);
    countSlider->setRange(24, 240);
    countSlider->setValue(110);
    countSlider->setFocusPolicy(Qt::NoFocus);
    controls->addWidget(countSlider);

    QHBoxLayout *thickHead = new QHBoxLayout();
    thickHead->addWidget(nqLabel(i18n("Espessura"), "nqSmall", d));
    thickHead->addStretch(1);
    QLabel *thickVal = nqLabel(QStringLiteral("4 px"), "nqItem", d);
    thickHead->addWidget(thickVal);
    controls->addLayout(thickHead);
    QSlider *thickSlider = new QSlider(Qt::Horizontal, d);
    thickSlider->setRange(1, 16);
    thickSlider->setValue(4);
    thickSlider->setFocusPolicy(Qt::NoFocus);
    controls->addWidget(thickSlider);

    QWidget *radialBox = new QWidget(d);
    QVBoxLayout *rbl = new QVBoxLayout(radialBox);
    rbl->setContentsMargins(0, 0, 0, 0);
    rbl->setSpacing(4);
    QHBoxLayout *radHead = new QHBoxLayout();
    radHead->addWidget(nqLabel(i18n("Área livre no centro"), "nqSmall", radialBox));
    radHead->addStretch(1);
    QLabel *radVal = nqLabel(QStringLiteral("42%"), "nqItem", radialBox);
    radHead->addWidget(radVal);
    rbl->addLayout(radHead);
    QSlider *radSlider = new QSlider(Qt::Horizontal, radialBox);
    radSlider->setRange(10, 82);
    radSlider->setValue(42);
    radSlider->setFocusPolicy(Qt::NoFocus);
    rbl->addWidget(radSlider);
    controls->addWidget(radialBox);

    QWidget *angleBox = new QWidget(d);
    QVBoxLayout *abl = new QVBoxLayout(angleBox);
    abl->setContentsMargins(0, 0, 0, 0);
    abl->setSpacing(4);
    abl->addWidget(nqLabel(i18n("Direção"), "nqSmall", angleBox));
    const QList<int> angles = {0, 30, 45, 90, 135};
    QButtonGroup *angleGroup = new QButtonGroup(angleBox);
    abl->addLayout(chipRow(angleBox, angleGroup, {QStringLiteral("0°"), QStringLiteral("30°"),
                                                  QStringLiteral("45°"), QStringLiteral("90°"),
                                                  QStringLiteral("135°")}, 0));
    angleBox->hide();
    controls->addWidget(angleBox);

    QHBoxLayout *jitHead = new QHBoxLayout();
    jitHead->addWidget(nqLabel(i18n("Variação orgânica"), "nqSmall", d));
    jitHead->addStretch(1);
    QLabel *jitVal = nqLabel(QStringLiteral("60%"), "nqItem", d);
    jitHead->addWidget(jitVal);
    controls->addLayout(jitHead);
    QSlider *jitSlider = new QSlider(Qt::Horizontal, d);
    jitSlider->setRange(0, 100);
    jitSlider->setValue(60);
    jitSlider->setFocusPolicy(Qt::NoFocus);
    controls->addWidget(jitSlider);

    controls->addWidget(nqLabel(i18n("Cor"), "nqSmall", d));
    QButtonGroup *colorGroup = new QButtonGroup(d);
    controls->addLayout(chipRow(d, colorGroup, {i18n("Nanquim"), i18n("Branco"), i18n("Cor atual")}, 0));

    body->addLayout(controls, 1);
    root->addLayout(body);
    root->addWidget(nqDivider(d));

    QHBoxLayout *foot = new QHBoxLayout();
    foot->setSpacing(10);
    foot->addWidget(nqLabel(hasSel
                            ? i18n("Recortado automaticamente pela seleção ativa.")
                            : i18n("Gera em uma nova camada transparente com desfazer."), "nqSmall", d, true), 1);
    QPushButton *cancelBtn = nqButton(i18n("Cancelar"), d, "nqGhost");
    QPushButton *applyBtn = nqButton(i18n("Gerar linhas"), d, "nqPrimary");
    QObject::connect(cancelBtn, &QPushButton::clicked, d, &QDialog::reject);
    foot->addWidget(cancelBtn);
    foot->addWidget(applyBtn);
    root->addLayout(foot);

    const QColor fgColor = vm->canvasResourceProvider()
            ? vm->canvasResourceProvider()->fgColor().toQColor()
            : QColor(0x11, 0x11, 0x11);
    auto centerState = std::make_shared<QPointF>(0.5, 0.5);
    const quint32 seed = QRandomGenerator::global()->generate();

    auto pickInk = [colorGroup, fgColor]() {
        const int cid = colorGroup->checkedId();
        if (cid == 1) return QColor(0xff, 0xff, 0xff);
        if (cid == 2) return fgColor;
        return QColor(0x11, 0x11, 0x11);
    };

    auto refreshPreview = [=]() {
        const int mode = qMax(0, modeGroup->checkedId());
        const bool isRadial = (mode == 0);
        radialBox->setVisible(isRadial);
        angleBox->setVisible(!isRadial);
        hint->setText(isRadial ? i18n("Arraste o ponto verde para mover o centro.")
                               : i18n("Linhas paralelas afuniladas para ação."));
        countVal->setText(QString::number(countSlider->value()));
        thickVal->setText(QStringLiteral("%1 px").arg(thickSlider->value()));
        radVal->setText(QStringLiteral("%1%").arg(radSlider->value()));
        jitVal->setText(QStringLiteral("%1%").arg(jitSlider->value()));

        const int ang = angles.value(qMax(0, angleGroup->checkedId()), 0);
        const qreal previewThick = qMax(1.0, thickSlider->value() * 0.42);
        const QImage img = renderSpeedlinesImage(QSize(194, 230), mode, *centerState,
                                                 radSlider->value(), countSlider->value(),
                                                 previewThick, ang, jitSlider->value(),
                                                 pickInk(), seed);
        preview->setImage(img, isRadial, *centerState);
    };

    preview->onCenterChanged = [centerState, refreshPreview](const QPointF &c) {
        *centerState = c;
        refreshPreview();
    };
    QObject::connect(modeGroup, &QButtonGroup::idClicked, d, [refreshPreview](int) { refreshPreview(); });
    QObject::connect(angleGroup, &QButtonGroup::idClicked, d, [refreshPreview](int) { refreshPreview(); });
    QObject::connect(colorGroup, &QButtonGroup::idClicked, d, [refreshPreview](int) { refreshPreview(); });
    QObject::connect(countSlider, &QSlider::valueChanged, d, [refreshPreview](int) { refreshPreview(); });
    QObject::connect(thickSlider, &QSlider::valueChanged, d, [refreshPreview](int) { refreshPreview(); });
    QObject::connect(radSlider, &QSlider::valueChanged, d, [refreshPreview](int) { refreshPreview(); });
    QObject::connect(jitSlider, &QSlider::valueChanged, d, [refreshPreview](int) { refreshPreview(); });
    refreshPreview();

    QPointer<KisMainWindow> mwGuard(mw);
    QObject::connect(applyBtn, &QPushButton::clicked, d, [=]() {
        const int mode = qMax(0, modeGroup->checkedId());
        const int cnt = countSlider->value();
        const qreal thick = thickSlider->value();
        const qreal radPct = radSlider->value();
        const int ang = angles.value(qMax(0, angleGroup->checkedId()), 0);
        const int jit = jitSlider->value();
        const QColor ink = pickInk();
        const QPointF center = *centerState;
        d->accept();

        if (!mwGuard) return;
        KisView *v = mwGuard->activeView();
        KisViewManager *m = mwGuard->viewManager();
        if (!v || !v->image() || !m) return;
        KisImageSP imgSp = v->image();

        KisSelectionSP activeSel = m->selection();
        if (!activeSel) activeSel = imgSp->globalSelection();
        QRect target = imgSp->bounds();
        bool clipped = false;
        if (activeSel) {
            const QRect sr = activeSel->selectedExactRect().intersected(imgSp->bounds());
            if (!sr.isEmpty()) {
                target = sr;
                clipped = true;
            }
        }
        if (target.isEmpty()) return;

        // escala proporcional para manter a espessura coerente em páginas de alta resolução
        const qreal scale = qMax(1.0, qMin(target.width(), target.height()) / 1200.0);
        const QImage linesImg = renderSpeedlinesImage(target.size(), mode, center, radPct, cnt,
                                                      thick * scale, ang, jit, ink, seed);
        if (linesImg.isNull()) return;

        const QString name = mode == 0 ? i18n("Linhas de foco") : i18n("Linhas de velocidade");
        KisPaintLayerSP layer = new KisPaintLayer(imgSp, name, OPACITY_OPAQUE_U8);
        if (clipped) {
            KisPaintDeviceSP tmp = new KisPaintDevice(imgSp->colorSpace());
            tmp->convertFromQImage(linesImg, nullptr, target.x(), target.y());
            KisPainter painter(layer->paintDevice(), activeSel);
            painter.bitBlt(target.x(), target.y(), tmp, target.x(), target.y(), target.width(), target.height());
            painter.end();
        } else {
            layer->paintDevice()->convertFromQImage(linesImg, nullptr, target.x(), target.y());
        }

        KisNodeSP active = m->activeNode();
        KisNodeSP parent = (active && active->parent()) ? active->parent() : KisNodeSP(imgSp->root());
        KisNodeSP above = (active && active->parent()) ? active : parent->lastChild();
        KisNodeCommandsAdapter adapter(m);
        adapter.addNode(layer, parent, above);
        showToast(mwGuard, mode == 0 ? i18n("Linhas de foco criadas") : i18n("Linhas de velocidade criadas"));
    });
    d->open();
}

// Transformada de Distância Euclidiana 1D (Felzenszwalb & Huttenlocher) — O(N) exata
void edt1D(const float *f, float *d, int n, int *v, double *z, float inf)
{
    int q0 = -1;
    for (int i = 0; i < n; ++i) {
        if (f[i] < inf) {
            q0 = i;
            break;
        }
    }
    if (q0 < 0) {
        for (int i = 0; i < n; ++i) d[i] = inf;
        return;
    }
    int k = 0;
    v[0] = q0;
    z[0] = -1e30;
    z[1] = 1e30;
    for (int q = q0 + 1; q < n; ++q) {
        if (f[q] >= inf) continue;
        double s = 0.0;
        for (;;) {
            const int vk = v[k];
            s = ((double(f[q]) + double(q) * double(q)) - (double(f[vk]) + double(vk) * double(vk)))
                / double(2 * (q - vk));
            if (s <= z[k]) {
                --k;
            } else {
                break;
            }
        }
        ++k;
        v[k] = q;
        z[k] = s;
        z[k + 1] = 1e30;
    }
    k = 0;
    for (int q = 0; q < n; ++q) {
        while (z[k + 1] < double(q)) ++k;
        const double dx = double(q - v[k]);
        d[q] = float(dx * dx + double(f[v[k]]));
    }
}

// Gera contorno circular anti-aliased ao redor da silhueta alfa da camada (O(W*H))
QImage renderLayerOutlineImage(const QImage &src, int radiusPx, const QColor &color)
{
    if (src.isNull() || radiusPx < 1) return QImage();
    const QImage in = (src.format() == QImage::Format_ARGB32) ? src : src.convertToFormat(QImage::Format_ARGB32);
    const int pad = radiusPx + 2;
    const int sw = in.width();
    const int sh = in.height();
    const int w = sw + 2 * pad;
    const int h = sh + 2 * pad;
    const float INF = 1e15f;

    QVector<float> grid(w * h, INF);
    for (int y = 0; y < sh; ++y) {
        const QRgb *row = reinterpret_cast<const QRgb*>(in.constScanLine(y));
        float *dst = grid.data() + (y + pad) * w + pad;
        for (int x = 0; x < sw; ++x) {
            if (qAlpha(row[x]) >= 40) dst[x] = 0.0f;
        }
    }

    const int maxDim = qMax(w, h);
    QVector<float> fBuf(maxDim), dBuf(maxDim);
    QVector<double> zBuf(maxDim + 1);
    QVector<int> vBuf(maxDim);

    // Passo 1: colunas
    for (int x = 0; x < w; ++x) {
        for (int y = 0; y < h; ++y) fBuf[y] = grid[y * w + x];
        edt1D(fBuf.constData(), dBuf.data(), h, vBuf.data(), zBuf.data(), INF);
        for (int y = 0; y < h; ++y) grid[y * w + x] = dBuf[y];
    }
    // Passo 2: linhas
    for (int y = 0; y < h; ++y) {
        float *row = grid.data() + y * w;
        edt1D(row, dBuf.data(), w, vBuf.data(), zBuf.data(), INF);
        for (int x = 0; x < w; ++x) row[x] = dBuf[x];
    }

    QImage out(w, h, QImage::Format_ARGB32);
    out.fill(0);
    const int cr = color.red(), cg = color.green(), cb = color.blue();
    const float rLimit = float(radiusPx);
    const float maxSq = (rLimit + 1.2f) * (rLimit + 1.2f);

    for (int y = 0; y < h; ++y) {
        QRgb *dst = reinterpret_cast<QRgb*>(out.scanLine(y));
        const float *row = grid.constData() + y * w;
        for (int x = 0; x < w; ++x) {
            const float d2 = row[x];
            if (d2 > maxSq) continue;
            const float dist = std::sqrt(d2);
            const float cov = qBound(0.0f, (rLimit + 0.5f) - dist, 1.0f);
            const int a = qRound(cov * 255.0f);
            if (a > 0) dst[x] = qRgba(cr, cg, cb, a);
        }
    }
    return out;
}

void showLayerOutlineSheet(KisMainWindow *mw)
{
    if (!mw) return;
    KisView *view = mw->activeView();
    KisViewManager *vm = mw->viewManager();
    if (!view || !view->image() || !vm) return;
    KisImageSP image = view->image();

    KisNodeSP active = vm->activeNode();
    if (!active || !active->paintDevice()) {
        showToast(mw, i18n("Selecione uma camada de desenho primeiro"));
        return;
    }
    const QRect bounds = active->paintDevice()->exactBounds().intersected(image->bounds());
    if (bounds.isEmpty()) {
        showToast(mw, i18n("A camada ativa está vazia"));
        return;
    }

    const QImage fullSrc = active->paintDevice()->convertToQImage(nullptr, bounds).convertToFormat(QImage::Format_ARGB32);
    if (fullSrc.isNull()) return;
    const qreal previewScale = (fullSrc.width() > 240 || fullSrc.height() > 180)
            ? qMin(240.0 / fullSrc.width(), 180.0 / fullSrc.height())
            : 1.0;
    const QImage smallSrc = previewScale < 1.0
            ? fullSrc.scaled(qMax(2, qRound(fullSrc.width() * previewScale)),
                             qMax(2, qRound(fullSrc.height() * previewScale)),
                             Qt::KeepAspectRatio, Qt::SmoothTransformation)
            : fullSrc;

    QDialog *d = makeSheet(mw, 540);
    QVBoxLayout *root = new QVBoxLayout(d);
    root->setContentsMargins(24, 20, 24, 20);
    root->setSpacing(14);
    root->addLayout(sheetHeader(d, i18n("Mangá"), i18n("Contorno na camada")));

    QLabel *preview = new QLabel(d);
    preview->setFixedHeight(200);
    preview->setAlignment(Qt::AlignCenter);
    preview->setStyleSheet(QStringLiteral(
        "background: #3a3934; border: 1px solid #524f48; border-radius: 14px;"));
    root->addWidget(preview);

    QHBoxLayout *thickHead = new QHBoxLayout();
    thickHead->addWidget(nqLabel(i18n("Espessura da borda"), "nqSmall", d));
    thickHead->addStretch(1);
    QLabel *thickVal = nqLabel(QStringLiteral("8 px"), "nqItem", d);
    thickHead->addWidget(thickVal);
    root->addLayout(thickHead);

    QSlider *thickSlider = new QSlider(Qt::Horizontal, d);
    thickSlider->setRange(2, 36);
    thickSlider->setValue(8);
    thickSlider->setFocusPolicy(Qt::NoFocus);
    root->addWidget(thickSlider);

    root->addWidget(nqLabel(i18n("Cor do contorno"), "nqSmall", d));
    QButtonGroup *colorGroup = new QButtonGroup(d);
    root->addLayout(chipRow(d, colorGroup, {i18n("Branco (destaque)"), i18n("Nanquim (preto)"), i18n("Cor atual")}, 0));

    root->addWidget(nqDivider(d));

    QHBoxLayout *foot = new QHBoxLayout();
    foot->setSpacing(10);
    foot->addWidget(nqLabel(i18n("Cria uma camada de contorno logo abaixo da camada ativa."), "nqSmall", d, true), 1);
    QPushButton *cancelBtn = nqButton(i18n("Cancelar"), d, "nqGhost");
    QPushButton *applyBtn = nqButton(i18n("Criar contorno"), d, "nqPrimary");
    QObject::connect(cancelBtn, &QPushButton::clicked, d, &QDialog::reject);
    foot->addWidget(cancelBtn);
    foot->addWidget(applyBtn);
    root->addLayout(foot);

    const QColor fgColor = vm->canvasResourceProvider()
            ? vm->canvasResourceProvider()->fgColor().toQColor()
            : QColor(0x11, 0x11, 0x11);

    auto pickColor = [colorGroup, fgColor]() {
        const int cid = colorGroup->checkedId();
        if (cid == 1) return QColor(0x11, 0x11, 0x11);
        if (cid == 2) return fgColor;
        return QColor(0xff, 0xff, 0xff);
    };

    auto refreshPreview = [=]() {
        const int rad = thickSlider->value();
        thickVal->setText(QStringLiteral("%1 px").arg(rad));
        const int prevRad = qMax(1, qRound(rad * previewScale));
        const QImage outline = renderLayerOutlineImage(smallSrc, prevRad, pickColor());
        if (outline.isNull()) return;
        QImage composed(outline.size(), QImage::Format_ARGB32);
        composed.fill(QColor(0x3a, 0x39, 0x34));
        QPainter p(&composed);
        p.drawImage(0, 0, outline);
        const int pad = prevRad + 2;
        p.drawImage(pad, pad, smallSrc);
        p.end();
        preview->setPixmap(QPixmap::fromImage(composed.scaled(preview->size() - QSize(12, 12),
                                                              Qt::KeepAspectRatio, Qt::SmoothTransformation)));
    };
    QObject::connect(thickSlider, &QSlider::valueChanged, d, [refreshPreview](int) { refreshPreview(); });
    QObject::connect(colorGroup, &QButtonGroup::idClicked, d, [refreshPreview](int) { refreshPreview(); });
    QTimer::singleShot(0, d, refreshPreview);

    QPointer<KisMainWindow> mwGuard(mw);
    QObject::connect(applyBtn, &QPushButton::clicked, d, [=]() {
        const int rad = thickSlider->value();
        const QColor col = pickColor();
        d->accept();

        if (!mwGuard) return;
        KisView *v = mwGuard->activeView();
        KisViewManager *m = mwGuard->viewManager();
        if (!v || !v->image() || !m) return;
        KisImageSP imgSp = v->image();

        const QImage outline = renderLayerOutlineImage(fullSrc, rad, col);
        if (outline.isNull()) return;
        const int pad = rad + 2;

        KisPaintLayerSP layer = new KisPaintLayer(imgSp, i18n("Contorno · %1 px", rad), OPACITY_OPAQUE_U8);
        layer->paintDevice()->convertFromQImage(outline, nullptr, bounds.x() - pad, bounds.y() - pad);

        KisNodeSP cur = m->activeNode();
        KisNodeSP parent = (cur && cur->parent()) ? cur->parent() : KisNodeSP(imgSp->root());
        // insere logo abaixo da camada ativa (prevSibling) para ficar atrás do traço
        KisNodeSP below = (cur && cur->parent()) ? cur->prevSibling() : parent->lastChild();
        KisNodeCommandsAdapter adapter(m);
        adapter.addNode(layer, parent, below);
        showToast(mwGuard, i18n("Contorno de %1 px criado abaixo da camada", rad));
    });
    d->open();
}

// Converte a camada ativa para Rascunho azul (Non-Photo Blue), preservando a original oculta
void applyBlueDraftLayer(KisMainWindow *mw)
{
    if (!mw) return;
    KisView *view = mw->activeView();
    KisViewManager *vm = mw->viewManager();
    if (!view || !view->image() || !vm) return;
    KisImageSP image = view->image();

    KisNodeSP active = vm->activeNode();
    if (!active || !active->paintDevice()) {
        showToast(mw, i18n("Selecione uma camada de desenho para converter"));
        return;
    }
    const QRect bounds = active->paintDevice()->exactBounds().intersected(image->bounds());
    if (bounds.isEmpty()) {
        showToast(mw, i18n("A camada ativa está vazia"));
        return;
    }

    const QImage src = active->paintDevice()->convertToQImage(nullptr, bounds).convertToFormat(QImage::Format_ARGB32);
    if (src.isNull()) return;
    const int w = src.width();
    const int h = src.height();

    // detecta se a camada tem fundo opaco claro (ex.: papel escaneado)
    qint64 opaqueCount = 0;
    qint64 brightCount = 0;
    const int stepY = qMax(1, h / 64);
    const int stepX = qMax(1, w / 64);
    qint64 samples = 0;
    for (int y = 0; y < h; y += stepY) {
        const QRgb *row = reinterpret_cast<const QRgb*>(src.constScanLine(y));
        for (int x = 0; x < w; x += stepX) {
            ++samples;
            const QRgb px = row[x];
            if (qAlpha(px) > 235) {
                ++opaqueCount;
                const int lum = (qRed(px) * 77 + qGreen(px) * 150 + qBlue(px) * 29) >> 8;
                if (lum > 225) ++brightCount;
            }
        }
    }
    const bool whitePaper = samples > 0 && (opaqueCount * 100 / samples) > 85 && (brightCount * 100 / samples) > 55;

    QImage out(w, h, QImage::Format_ARGB32);
    for (int y = 0; y < h; ++y) {
        const QRgb *inRow = reinterpret_cast<const QRgb*>(src.constScanLine(y));
        QRgb *outRow = reinterpret_cast<QRgb*>(out.scanLine(y));
        for (int x = 0; x < w; ++x) {
            const QRgb px = inRow[x];
            const int a = qAlpha(px);
            if (a == 0) {
                outRow[x] = 0;
                continue;
            }
            const int lum = (qRed(px) * 77 + qGreen(px) * 150 + qBlue(px) * 29) >> 8;
            if (whitePaper) {
                const int inkA = qBound(0, qRound((255 - lum) * 1.15), 255);
                outRow[x] = inkA > 3 ? qRgba(0x3a, 0x84, 0xd8, inkA) : 0;
            } else {
                const qreal darkness = qBound(0.0, (255 - lum) / 255.0, 1.0);
                const int r = qRound(0x3a + (1.0 - darkness) * 95.0);
                const int g = qRound(0x84 + (1.0 - darkness) * 72.0);
                const int b = qRound(0xd8 + (1.0 - darkness) * 26.0);
                outRow[x] = qRgba(qMin(255, r), qMin(255, g), qMin(255, b), a);
            }
        }
    }

    const QString origName = active->name();
    const QString draftName = origName.contains(QLatin1String("rascunho"), Qt::CaseInsensitive)
            ? i18n("%1 (azul)", origName)
            : i18n("Rascunho azul · %1", origName);

    KisPaintLayerSP layer = new KisPaintLayer(image, draftName, OPACITY_OPAQUE_U8);
    layer->paintDevice()->convertFromQImage(out, nullptr, bounds.x(), bounds.y());

    KisNodeSP parent = active->parent() ? active->parent() : KisNodeSP(image->root());
    KisNodeCommandsAdapter adapter(vm);
    adapter.addNode(layer, parent, active);
    active->setVisible(false);
    active->setDirty();
    showToast(mw, i18n("Rascunho azul criado (ignorado ao exportar)"));
}

// Alterna a Lata de Tinta entre "Modo Quadrinho" (expande 2 px, fecha lacunas,
// usa todas as camadas como referência) e o padrão do Krita.
void toggleComicBucketPreset(KisMainWindow *mw)
{
    if (!mw) return;
    KConfigGroup nq = KSharedConfig::openConfig()->group("Nanquim");
    const bool enable = !nq.readEntry("comicBucketMode", false);
    nq.writeEntry("comicBucketMode", enable);

    KConfigGroup fill = KSharedConfig::openConfig()->group("KritaFill/KisToolFill");
    if (enable) {
        fill.writeEntry("fillSelection", false);
        fill.writeEntry("useFastMode", false);
        fill.writeEntry("sampleMerged", true);
        fill.writeEntry("reference", QStringLiteral("allLayers"));
        fill.writeEntry("threshold", 18);
        fill.writeEntry("growSelection", 2);
        fill.writeEntry("feather", 0);
        fill.writeEntry("antiAlias", true);
        fill.writeEntry("closeGapEnabled", true);
        fill.writeEntry("closeGap", 2);
        fill.writeEntry("useSelectionAsBoundary", true);
    } else {
        fill.writeEntry("growSelection", 0);
        fill.writeEntry("threshold", 8);
        fill.writeEntry("closeGapEnabled", false);
        fill.writeEntry("reference", QStringLiteral("currentLayer"));
        fill.writeEntry("sampleMerged", false);
    }
    KSharedConfig::openConfig()->sync();
    KisConfigNotifier::instance()->notifyConfigChanged();

    if (enable) {
        if (QAction *bucket = act(mw, "KritaFill/KisToolFill")) bucket->trigger();
        showToast(mw, i18n("Lata de tinta: Modo Quadrinho (expande 2 px, todas as camadas)"));
    } else {
        showToast(mw, i18n("Lata de tinta: padrão do Krita restaurado"));
    }
}

// Prévia de celular + Fatiador 800x1280 + Nova tira Webtoon
void showWebtoonToolsSheet(KisMainWindow *mw, int initialTab)
{
    if (!mw) return;
    KisView *view = mw->activeView();
    KisImageSP image;
    if (view) image = view->image();

    QDialog *d = makeSheet(mw, 620, 640);
    QVBoxLayout *root = new QVBoxLayout(d);
    root->setContentsMargins(24, 20, 24, 20);
    root->setSpacing(12);
    root->addLayout(sheetHeader(d, i18n("Webtoon"), i18n("Prévia de celular e fatiador")));

    QButtonGroup *tabs = new QButtonGroup(d);
    root->addLayout(chipRow(d, tabs, {i18n("Prévia de celular"), i18n("Fatiar e Nova tira")}, qBound(0, initialTab, 1)));

    QStackedWidget *stack = new QStackedWidget(d);
    root->addWidget(stack, 1);

    // --- Aba 0: Prévia de celular (9:19,5 com rolagem suave e marcas de corte) ---
    QWidget *previewPage = new QWidget(stack);
    QVBoxLayout *pvl = new QVBoxLayout(previewPage);
    pvl->setContentsMargins(0, 4, 0, 0);
    pvl->setSpacing(10);

    QCheckBox *showCuts = new QCheckBox(i18n("Mostrar linhas de corte a cada 1280 px"), previewPage);
    showCuts->setChecked(true);
    showCuts->setFocusPolicy(Qt::NoFocus);
    pvl->addWidget(showCuts);

    QFrame *phone = new QFrame(previewPage);
    phone->setFixedWidth(292);
    phone->setMinimumHeight(240);
    phone->setStyleSheet(QStringLiteral(
        "QFrame { background: #191815; border: 3px solid #524f48; border-radius: 26px; }"));
    QVBoxLayout *phl = new QVBoxLayout(phone);
    phl->setContentsMargins(8, 12, 8, 12);
    phl->setSpacing(0);

    QScrollArea *scroll = new QScrollArea(phone);
    scroll->setWidgetResizable(false);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setStyleSheet(QStringLiteral("QScrollArea { background: #ffffff; border: none; border-radius: 16px; }"));
    scroll->setProperty("nanquimSmooth", true);
    tuneSmoothScroller(KisKineticScroller::createPreconfiguredScroller(scroll));

    QLabel *stripLabel = new QLabel(scroll);
    stripLabel->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    scroll->setWidget(stripLabel);
    phl->addWidget(scroll);
    pvl->addWidget(phone, 1, Qt::AlignHCenter);

    QLabel *previewMeta = nqLabel(QString(), "nqSmall", previewPage, true);
    previewMeta->setAlignment(Qt::AlignCenter);
    pvl->addWidget(previewMeta);

    QImage basePhoneImg;
    int sliceCountPreview = 1;
    int sliceStepPhone = 432;
    if (image) {
        const QRect b = image->bounds();
        const QImage proj = image->convertToQImage(b, nullptr);
        if (!proj.isNull() && proj.width() > 0 && proj.height() > 0) {
            const int phoneW = 270;
            const int phoneH = qMax(1, qRound(qreal(proj.height()) * phoneW / proj.width()));
            basePhoneImg = proj.scaled(phoneW, phoneH, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)
                               .convertToFormat(QImage::Format_ARGB32);
            const qreal scaled800H = qreal(proj.height()) * 800.0 / proj.width();
            sliceCountPreview = qMax(1, int(std::ceil(scaled800H / 1280.0)));
            sliceStepPhone = qMax(10, qRound(1280.0 * phoneW / 800.0));
            previewMeta->setText(i18n("Página atual: %1 × %2 px · equivale a %3 fatia(s) de 800 × 1280 px",
                                      proj.width(), proj.height(), sliceCountPreview));
        }
    } else {
        previewMeta->setText(i18n("Abra uma página para visualizar na proporção de celular."));
    }

    auto refreshPhone = [stripLabel, basePhoneImg, showCuts, sliceStepPhone]() {
        if (basePhoneImg.isNull()) {
            stripLabel->setText(i18n("Sem página aberta"));
            stripLabel->resize(270, 320);
            return;
        }
        QImage drawn = basePhoneImg.copy();
        if (showCuts->isChecked() && sliceStepPhone > 0) {
            QPainter p(&drawn);
            QPen dash(QColor(0x3e, 0x8e, 0x7e, 220), 1.5, Qt::DashLine);
            p.setPen(dash);
            QFont f = p.font();
            f.setPixelSize(10);
            f.setWeight(QFont::Bold);
            p.setFont(f);
            int idx = 1;
            for (int y = sliceStepPhone; y < drawn.height(); y += sliceStepPhone, ++idx) {
                p.drawLine(0, y, drawn.width(), y);
                const QRect badge(drawn.width() - 62, y - 16, 56, 15);
                p.fillRect(badge, QColor(36, 35, 31, 210));
                p.setPen(QColor(0x6f, 0xbf, 0xad));
                p.drawText(badge, Qt::AlignCenter, i18n("Corte %1", idx));
                p.setPen(dash);
            }
            p.end();
        }
        stripLabel->setPixmap(QPixmap::fromImage(drawn));
        stripLabel->resize(drawn.size());
    };
    QObject::connect(showCuts, &QCheckBox::toggled, d, [refreshPhone](bool) { refreshPhone(); });
    refreshPhone();
    stack->addWidget(previewPage);

    // --- Aba 1: Fatiar página atual + Criar nova tira Webtoon ---
    QWidget *toolsPage = new QWidget(stack);
    QVBoxLayout *tvl = new QVBoxLayout(toolsPage);
    tvl->setContentsMargins(0, 6, 0, 0);
    tvl->setSpacing(12);

    tvl->addWidget(nqLabel(i18n("Fatiar página atual para publicação"), "nqItem", toolsPage));
    tvl->addWidget(nqLabel(i18n("Largura final da fatia"), "nqSmall", toolsPage));
    const QList<int> widths = {800, 1280};
    QButtonGroup *widthGroup = new QButtonGroup(toolsPage);
    tvl->addLayout(chipRow(toolsPage, widthGroup, {i18n("800 px (WEBTOON / Tapas)"), i18n("1280 px (HD)")}, 0));

    tvl->addWidget(nqLabel(i18n("Altura máxima de cada fatia"), "nqSmall", toolsPage));
    const QList<int> heights = {1280, 1600};
    QButtonGroup *heightGroup = new QButtonGroup(toolsPage);
    tvl->addLayout(chipRow(toolsPage, heightGroup, {QStringLiteral("1280 px"), QStringLiteral("1600 px")}, 0));

    tvl->addWidget(nqLabel(i18n("Formato"), "nqSmall", toolsPage));
    QButtonGroup *fmtGroup = new QButtonGroup(toolsPage);
    tvl->addLayout(chipRow(toolsPage, fmtGroup, {QStringLiteral("PNG"), QStringLiteral("JPG")}, 0));

    QLabel *sliceSummary = nqLabel(QString(), "nqSmall", toolsPage, true);
    tvl->addWidget(sliceSummary);

    QPushButton *exportSlicesBtn = nqButton(i18n("Exportar fatias agora"), toolsPage, "nqPrimary");
    exportSlicesBtn->setEnabled(image != nullptr);
    tvl->addWidget(exportSlicesBtn, 0, Qt::AlignLeft);

    tvl->addWidget(nqDivider(toolsPage));
    tvl->addWidget(nqLabel(i18n("Criar nova tira Webtoon"), "nqItem", toolsPage));
    tvl->addWidget(nqLabel(i18n("Já abre com camada Rascunho pronta para desenhar na vertical."), "nqSmall", toolsPage, true));

    QHBoxLayout *newStripsRow = new QHBoxLayout();
    newStripsRow->setSpacing(8);
    struct StripSpec { const char *label; int w; int h; };
    const StripSpec specs[] = {
        {"Curta (800×5120)", 800, 5120},
        {"Média (800×7680)", 800, 7680},
        {"HD 2× (1600×10240)", 1600, 10240},
    };
    QPointer<KisMainWindow> mwGuard(mw);
    for (const StripSpec &sp : specs) {
        QPushButton *b = nqButton(i18n(sp.label), toolsPage);
        QObject::connect(b, &QPushButton::clicked, d, [d, mwGuard, sp]() {
            d->accept();
            const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
            KoColor bg(Qt::white, cs);
            KisDocument *doc = KisPart::instance()->createDocument();
            if (!doc->newImage(i18n("Tira Webtoon"), sp.w, sp.h, cs, bg,
                               KisConfig::RASTER_LAYER, 2, QString(), 300.0 / 72.0)) {
                delete doc;
                return;
            }
            if (KisImageSP imgSp = doc->image()) {
                KisPaintLayerSP draft = new KisPaintLayer(imgSp, i18n("Rascunho"), OPACITY_OPAQUE_U8);
                imgSp->addNode(draft, imgSp->root());
            }
            doc->setModified(false);
            KisPart::instance()->startCustomDocument(doc);
            if (mwGuard) showToast(mwGuard, i18n("Tira Webtoon criada (%1 × %2 px)", sp.w, sp.h));
        });
        newStripsRow->addWidget(b);
    }
    tvl->addLayout(newStripsRow);
    tvl->addStretch(1);
    stack->addWidget(toolsPage);

    auto updateSummary = [sliceSummary, image, widthGroup, widths, heightGroup, heights]() {
        if (!image) {
            sliceSummary->setText(i18n("Nenhuma página aberta para fatiar."));
            return;
        }
        const int targetW = widths.value(qMax(0, widthGroup->checkedId()), 800);
        const int sliceH = heights.value(qMax(0, heightGroup->checkedId()), 1280);
        const int scaledH = qMax(1, qRound(qreal(image->height()) * targetW / qMax(1, image->width())));
        const int count = qMax(1, (scaledH + sliceH - 1) / sliceH);
        sliceSummary->setText(i18n("Vai gerar %1 arquivo(s) de até %2 × %3 px na pasta de exportação.",
                                   count, targetW, sliceH));
    };
    QObject::connect(widthGroup, &QButtonGroup::idClicked, d, [updateSummary](int) { updateSummary(); });
    QObject::connect(heightGroup, &QButtonGroup::idClicked, d, [updateSummary](int) { updateSummary(); });
    updateSummary();

    QObject::connect(exportSlicesBtn, &QPushButton::clicked, d, [=]() {
        const int targetW = widths.value(qMax(0, widthGroup->checkedId()), 800);
        const int sliceH = heights.value(qMax(0, heightGroup->checkedId()), 1280);
        const bool isJpg = (fmtGroup->checkedId() == 1);
        d->accept();

        if (!mwGuard) return;
        KisView *v = mwGuard->activeView();
        if (!v || !v->image()) return;
        KisImageSP imgSp = v->image();
        const QImage proj = imgSp->convertToQImage(imgSp->bounds(), nullptr);
        if (proj.isNull() || proj.width() <= 0 || proj.height() <= 0) return;

        // garante fundo branco caso haja áreas transparentes
        QImage flat(proj.size(), QImage::Format_RGB32);
        flat.fill(Qt::white);
        QPainter fp(&flat);
        fp.drawImage(0, 0, proj);
        fp.end();

        const int scaledH = qMax(1, qRound(qreal(flat.height()) * targetW / flat.width()));
        const QImage scaled = (flat.width() == targetW)
                ? flat
                : flat.scaled(targetW, scaledH, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        QString base = (v->document() && !v->document()->path().isEmpty())
                ? QFileInfo(v->document()->path()).completeBaseName()
                : QStringLiteral("webtoon");
        if (base.isEmpty()) base = QStringLiteral("webtoon");

        const QString ext = isJpg ? QStringLiteral("jpg") : QStringLiteral("png");
        const QString mime = isJpg ? QStringLiteral("image/jpeg") : QStringLiteral("image/png");
        const int total = qMax(1, (scaled.height() + sliceH - 1) / sliceH);
        int saved = 0;
        QString lastLoc;

        for (int i = 0; i < total; ++i) {
            const int y = i * sliceH;
            const int h = qMin(sliceH, scaled.height() - y);
            if (h <= 0) break;
            const QImage slice = scaled.copy(0, y, scaled.width(), h);
            const QString fileName = QStringLiteral("%1_f%2.%3")
                    .arg(base)
                    .arg(i + 1, 2, 10, QLatin1Char('0'))
                    .arg(ext);
            const QString tmpPath = QDir::tempPath() + QLatin1Char('/') + fileName;
            const bool ok = isJpg ? slice.save(tmpPath, "JPG", 92) : slice.save(tmpPath, "PNG");
            if (ok) {
                const QString loc = publishExport(tmpPath, fileName, mime, nullptr);
                if (!loc.isEmpty()) lastLoc = loc;
                QFile::remove(tmpPath);
                ++saved;
            }
        }
        if (saved > 0) {
            showToast(mwGuard, lastLoc.isEmpty()
                      ? i18n("%1 fatias Webtoon exportadas", saved)
                      : i18n("%1 fatias Webtoon salvas · %2", saved, lastLoc));
        } else {
            showToast(mwGuard, i18n("Não foi possível exportar as fatias"));
        }
    });

    QObject::connect(tabs, &QButtonGroup::idClicked, d, [stack](int id) { stack->setCurrentIndex(id); });
    stack->setCurrentIndex(qBound(0, initialTab, 1));
    d->open();
}

// ---------------------------------------------------------------------------
// Catálogo da Comunidade (patch 0122): lê um JSON curado hospedado no GitHub
// do Nanquim (com cache local e catálogo inicial embutido), permite filtrar,
// baixar e instalar pacotes .bundle / .kpp / .sut em 1 toque ou por link direto.
// ---------------------------------------------------------------------------
struct CommunityCatalogItem
{
    QString id;
    QString name;
    QString author;
    QString category;
    QString format;
    QString license;
    QString description;
    QStringList tags;
    QString downloadUrl;
    QString pageUrl;
    QString previewUrl;
};

QString defaultCommunityCatalogUrl()
{
    return QStringLiteral("https://raw.githubusercontent.com/isaiasdaniel290308-bot/Nanquim/main/catalogo-comunidade.json");
}

QString communityCatalogCachePath()
{
    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(base);
    return base + QStringLiteral("/nanquim-catalogo-cache.json");
}

QString communityDownloadsDir()
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
            + QStringLiteral("/nanquim-downloads");
    QDir().mkpath(dir);
    return dir;
}

QVector<CommunityCatalogItem> builtinCommunityCatalog()
{
    return {
        {
            QStringLiteral("portal-krita-artists-brushes"),
            i18n("Krita-Artists · Pincéis e Pacotes"),
            i18n("Comunidade Krita"),
            QStringLiteral("Portais"),
            QStringLiteral("Portal"),
            QStringLiteral("Livre / CC"),
            i18n("Acervo oficial da comunidade com pacotes .bundle e .kpp de nanquim, lápis, pintura e mangá. Baixe ou copie o link direto do arquivo para instalar aqui."),
            {QStringLiteral("bundle"), QStringLiteral("kpp"), QStringLiteral("comunidade"), QStringLiteral("pincéis")},
            QString(),
            QStringLiteral("https://krita-artists.org/c/resources/brushes-and-bundles/32")
        },
        {
            QStringLiteral("portal-krita-artists-manga"),
            i18n("Krita-Artists · Mangá e Nanquim"),
            i18n("Comunidade Krita"),
            QStringLiteral("Nanquim"),
            QStringLiteral("Portal"),
            QStringLiteral("Livre / CC"),
            i18n("Busca direta de bicos-de-pena, g-pen, pincéis de nanquim texturizados e kits de quadrinhos feitos pela comunidade."),
            {QStringLiteral("mangá"), QStringLiteral("nanquim"), QStringLiteral("g-pen"), QStringLiteral("ink")},
            QString(),
            QStringLiteral("https://krita-artists.org/search?q=manga%20%23resources%3Abrushes-and-bundles")
        },
        {
            QStringLiteral("portal-krita-artists-pencils"),
            i18n("Krita-Artists · Lápis Texturizados"),
            i18n("Comunidade Krita"),
            QStringLiteral("Lápis"),
            QStringLiteral("Portal"),
            QStringLiteral("Livre / CC"),
            i18n("Lápis grafite, carvão e rascunho com textura orgânica de papel para esboços e arte-final."),
            {QStringLiteral("lápis"), QStringLiteral("grafite"), QStringLiteral("textura"), QStringLiteral("rascunho")},
            QString(),
            QStringLiteral("https://krita-artists.org/search?q=pencil%20%23resources%3Abrushes-and-bundles")
        },
        {
            QStringLiteral("portal-krita-artists-screentones"),
            i18n("Krita-Artists · Retículas e Padrões"),
            i18n("Comunidade Krita"),
            QStringLiteral("Retícula"),
            QStringLiteral("Portal"),
            QStringLiteral("Livre / CC"),
            i18n("Retículas de mangá, hachuras e texturas de papel prontas para importar no Nanquim."),
            {QStringLiteral("retícula"), QStringLiteral("screentone"), QStringLiteral("hachura"), QStringLiteral("padrões")},
            QString(),
            QStringLiteral("https://krita-artists.org/search?q=screentone%20%23resources")
        },
        {
            QStringLiteral("portal-krita-docs-resources"),
            i18n("Página Oficial de Recursos do Krita"),
            QStringLiteral("KDE / Krita"),
            QStringLiteral("Portais"),
            QStringLiteral("Portal"),
            QStringLiteral("GPL / CC0"),
            i18n("Lista curada pela equipe do Krita com os pacotes clássicos de pintura, tinta e quadrinhos mais testados."),
            {QStringLiteral("oficial"), QStringLiteral("kde"), QStringLiteral("bundle"), QStringLiteral("pintura")},
            QString(),
            QStringLiteral("https://docs.krita.org/en/resources_page.html")
        }
    };
}

QVector<CommunityCatalogItem> parseCommunityCatalogJson(const QByteArray &raw, bool *okOut = nullptr)
{
    QVector<CommunityCatalogItem> items;
    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(raw, &err);
    if (err.error != QJsonParseError::NoError) {
        if (okOut) *okOut = false;
        return items;
    }
    QJsonArray arr;
    if (doc.isObject()) arr = doc.object().value(QStringLiteral("items")).toArray();
    else if (doc.isArray()) arr = doc.array();

    for (const QJsonValue &val : arr) {
        if (!val.isObject()) continue;
        const QJsonObject o = val.toObject();
        CommunityCatalogItem it;
        it.id = o.value(QStringLiteral("id")).toString().trimmed();
        it.name = o.value(QStringLiteral("name")).toString().trimmed();
        if (it.name.isEmpty()) continue;
        if (it.id.isEmpty()) it.id = it.name.toLower().replace(QLatin1Char(' '), QLatin1Char('-'));
        it.author = o.value(QStringLiteral("author")).toString().trimmed();
        it.category = o.value(QStringLiteral("category")).toString().trimmed();
        if (it.category.isEmpty()) it.category = QStringLiteral("Nanquim");
        it.format = o.value(QStringLiteral("format")).toString().trimmed();
        it.license = o.value(QStringLiteral("license")).toString().trimmed();
        it.description = o.value(QStringLiteral("description")).toString().trimmed();
        it.downloadUrl = o.value(QStringLiteral("download_url")).toString().trimmed();
        it.pageUrl = o.value(QStringLiteral("page_url")).toString().trimmed();
        it.previewUrl = o.value(QStringLiteral("preview_url")).toString().trimmed();
        const QJsonArray tagsArr = o.value(QStringLiteral("tags")).toArray();
        for (const QJsonValue &tv : tagsArr) {
            const QString s = tv.toString().trimmed();
            if (!s.isEmpty()) it.tags << s;
        }
        if (it.format.isEmpty()) {
            if (!it.downloadUrl.isEmpty()) {
                const QString ext = QFileInfo(QUrl(it.downloadUrl).path()).suffix().toLower();
                it.format = ext.isEmpty() ? QStringLiteral(".bundle") : (QLatin1Char('.') + ext);
            } else {
                it.format = QStringLiteral("Portal");
            }
        }
        items << it;
    }
    if (okOut) *okOut = !items.isEmpty();
    return items;
}

QString resolveCatalogDownloadName(const QNetworkReply *reply, const QByteArray &data,
                                   const QString &fallbackBase, const QString &fallbackFormat)
{
    QString name;
    if (reply) {
        const QString cd = QString::fromUtf8(reply->rawHeader("Content-Disposition"));
        if (!cd.isEmpty()) {
            const QRegularExpression re(QStringLiteral("filename\\s*=\\s*\"?([^\";\\r\\n]+)\"?"),
                                        QRegularExpression::CaseInsensitiveOption);
            const QRegularExpressionMatch m = re.match(cd);
            if (m.hasMatch()) name = QFileInfo(m.captured(1).trimmed()).fileName();
        }
        if (name.isEmpty()) {
            name = QFileInfo(reply->url().path()).fileName();
        }
    }
    name.replace(QRegularExpression(QStringLiteral("[^A-Za-z0-9._-]")), QStringLiteral("_"));
    if (name.isEmpty() || name == QStringLiteral("_")) {
        name = fallbackBase.isEmpty() ? QStringLiteral("pacote_comunidade") : fallbackBase;
        name.replace(QRegularExpression(QStringLiteral("[^A-Za-z0-9._-]")), QStringLiteral("_"));
    }

    // pacote do Krita publicado como .zip (ex.: Deevad): o conteúdo manda, vira .bundle
    if (name.toLower().endsWith(QLatin1String(".zip")) && data.startsWith("PK\x03\x04")
            && data.left(512).contains("x-krita-resourcebundle")) {
        name = name.left(name.size() - 4) + QStringLiteral(".bundle");
    }
    const QString lower = name.toLower();
    const bool hasKnownExt = lower.endsWith(QLatin1String(".bundle"))
            || lower.endsWith(QLatin1String(".kpp"))
            || lower.endsWith(QLatin1String(".sut"))
            || lower.endsWith(QLatin1String(".zip"));
    if (!hasKnownExt) {
        QString ext;
        if (data.size() >= 16 && data.startsWith("SQLite format 3")) ext = QStringLiteral(".sut");
        else if (data.size() >= 8 && data.startsWith("\x89PNG")) ext = QStringLiteral(".kpp");
        else if (data.size() >= 4 && data.startsWith("PK\x03\x04")) {
            ext = data.left(256).contains("x-krita-resourcebundle") ? QStringLiteral(".bundle") : QStringLiteral(".zip");
        } else {
            const QString f = fallbackFormat.toLower().trimmed();
            if (f.contains(QLatin1String("sut"))) ext = QStringLiteral(".sut");
            else if (f.contains(QLatin1String("kpp"))) ext = QStringLiteral(".kpp");
            else if (f.contains(QLatin1String("zip"))) ext = QStringLiteral(".zip");
            else ext = QStringLiteral(".bundle");
        }
        name += ext;
    }
    return name;
}

// Links de serviços conhecidos viram link de download direto
// (Dropbox dl=0 -> dl=1, Google Drive "view" -> download, GitHub "blob" -> "raw").
QUrl normalizeCatalogDownloadUrl(const QUrl &in)
{
    QUrl url = in;
    const QString host = url.host().toLower();
    if (host.endsWith(QLatin1String("dropbox.com"))) {
        QUrlQuery q(url);
        q.removeAllQueryItems(QStringLiteral("dl"));
        q.addQueryItem(QStringLiteral("dl"), QStringLiteral("1"));
        url.setQuery(q);
    } else if (host == QLatin1String("drive.google.com")) {
        const QRegularExpressionMatch m = QRegularExpression(QStringLiteral("/file/d/([A-Za-z0-9_-]+)")).match(url.path());
        const QString id = m.hasMatch() ? m.captured(1) : QUrlQuery(url).queryItemValue(QStringLiteral("id"));
        if (!id.isEmpty()) {
            url = QUrl(QStringLiteral("https://drive.usercontent.google.com/download?id=%1&export=download&confirm=t").arg(id));
        }
    } else if (host == QLatin1String("github.com") && url.path().contains(QLatin1String("/blob/"))) {
        QString path = url.path();
        path.replace(QLatin1String("/blob/"), QLatin1String("/raw/"));
        url.setPath(path);
    }
    return url;
}

// O arquivo baixado é mesmo um pacote? (.bundle/.zip = PK, .kpp = PNG, .sut = SQLite)
bool looksLikeBrushPayload(const QByteArray &data)
{
    return data.startsWith("PK\x03\x04") || data.startsWith("\x89PNG") || data.startsWith("SQLite format 3");
}

// Miniatura com cantos arredondados, cortada para preencher o tamanho
QPixmap catalogPreviewPixmap(const QImage &img, const QSize &size, qreal dpr)
{
    const QSize px = size * dpr;
    QImage scaled = img.scaled(px, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    scaled = scaled.copy((scaled.width() - px.width()) / 2, (scaled.height() - px.height()) / 2, px.width(), px.height());
    QPixmap out(px);
    out.fill(Qt::transparent);
    QPainter g(&out);
    g.setRenderHint(QPainter::Antialiasing);
    g.setRenderHint(QPainter::SmoothPixmapTransform);
    QPainterPath clip;
    clip.addRoundedRect(QRectF(QPointF(0, 0), QSizeF(px)), 10 * dpr, 10 * dpr);
    g.setClipPath(clip);
    g.drawImage(0, 0, scaled);
    g.end();
    out.setDevicePixelRatio(dpr);
    return out;
}

QString catalogPreviewCacheFile(const QString &url)
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
            + QStringLiteral("/nanquim-catalogo-previas");
    QDir().mkpath(dir);
    return dir + QLatin1Char('/')
            + QString::fromLatin1(QCryptographicHash::hash(url.toUtf8(), QCryptographicHash::Md5).toHex())
            + QStringLiteral(".jpg");
}

bool looksLikeHtmlResponse(const QNetworkReply *reply, const QByteArray &data)
{
    if (reply) {
        const QString ct = reply->header(QNetworkRequest::ContentTypeHeader).toString().toLower();
        if (ct.contains(QLatin1String("text/html"))) return true;
    }
    const QByteArray head = data.left(256).trimmed().toLower();
    return head.startsWith("<!doctype html") || head.startsWith("<html");
}

// Quando o usuário cola uma página HTML (ex.: tópico do Krita-Artists ou site de artista),
// procura no HTML o primeiro link direto de .bundle, .kpp, .sut, .zip ou anexo Discourse.
QString extractDownloadUrlFromHtml(const QByteArray &htmlBytes, const QUrl &baseUrl)
{
    const QString text = QString::fromUtf8(htmlBytes);
    const QRegularExpression anchorRe(
                QStringLiteral("<a\\b([^>]*)>(.*?)</a>"),
                QRegularExpression::CaseInsensitiveOption | QRegularExpression::DotMatchesEverythingOption);
    const QRegularExpression hrefRe(
                QStringLiteral("href\\s*=\\s*[\"']([^\"']+)[\"']"),
                QRegularExpression::CaseInsensitiveOption);
    const QRegularExpression extRe(
                QStringLiteral("\\.(bundle|kpp|sut|zip)(?:$|[?#\\s\"')\\]])"),
                QRegularExpression::CaseInsensitiveOption);

    auto scoreExt = [](const QString &ext) {
        const QString e = ext.toLower();
        if (e == QLatin1String("bundle")) return 0;
        if (e == QLatin1String("kpp")) return 1;
        if (e == QLatin1String("sut")) return 2;
        return 3;
    };

    int bestScore = 999;
    QString bestUrl;
    QRegularExpressionMatchIterator it = anchorRe.globalMatch(text);
    while (it.hasNext()) {
        const QRegularExpressionMatch m = it.next();
        const QString attrs = m.captured(1);
        const QString inner = m.captured(2);
        const QRegularExpressionMatch hm = hrefRe.match(attrs);
        if (!hm.hasMatch()) continue;

        QString rawHref = hm.captured(1).trimmed();
        rawHref.replace(QLatin1String("&amp;"), QLatin1String("&"));
        const QUrl resolved = baseUrl.resolved(QUrl(rawHref));
        if (!resolved.isValid()) continue;
        if (resolved.scheme() != QLatin1String("https") && resolved.scheme() != QLatin1String("http")) continue;

        const bool isAttachment = attrs.contains(QLatin1String("attachment"), Qt::CaseInsensitive)
                || resolved.path().contains(QLatin1String("/uploads/short-url/"), Qt::CaseInsensitive)
                || resolved.path().contains(QLatin1String("/uploads/default/original/"), Qt::CaseInsensitive);
        const QRegularExpressionMatch emUrl = extRe.match(resolved.path());
        const QRegularExpressionMatch emInner = extRe.match(inner);
        if (!emUrl.hasMatch() && !emInner.hasMatch()) continue;

        const QString ext = emUrl.hasMatch() ? emUrl.captured(1) : emInner.captured(1);
        const int score = scoreExt(ext) + (isAttachment ? 0 : 10);
        if (score < bestScore) {
            bestScore = score;
            bestUrl = resolved.toString();
        }
    }
    return bestUrl;
}

// ---------------------------------------------------------------------------
// Baixa e instala um item do catálogo. Usado pela aba Comunidade da tela inicial.
// onProgress recebe 0..100; onDone recebe se deu certo e o texto para o usuário.
// ---------------------------------------------------------------------------
void installCatalogItem(KisMainWindow *mw, QNetworkAccessManager *net, QObject *ctx,
                        const QString &urlStr, const QString &itemId, const QString &displayName,
                        const QString &formatHint, const std::function<void(int)> &onProgress,
                        const std::function<void(bool, const QString &)> &onDone, int depth = 0)
{
    const QUrl url = QUrl::fromUserInput(urlStr.trimmed());
    if (!url.isValid() || (url.scheme() != QLatin1String("https") && url.scheme() != QLatin1String("http"))) {
        onDone(false, i18n("Link inválido para download."));
        return;
    }
    QNetworkRequest req(normalizeCatalogDownloadUrl(url));
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::UserVerifiedRedirectPolicy);
    req.setMaximumRedirectsAllowed(10);
    req.setHeader(QNetworkRequest::UserAgentHeader,
                  QStringLiteral("Mozilla/5.0 (Linux; Android 16; Tablet) AppleWebKit/537.36 (KHTML, like Gecko) Nanquim/1.0"));
    req.setRawHeader("Accept", "application/octet-stream,application/zip,*/*;q=0.8");
    QNetworkReply *reply = net->get(req);
    QObject::connect(reply, &QNetworkReply::redirected, reply, &QNetworkReply::redirectAllowed);
    QObject::connect(reply, &QNetworkReply::downloadProgress, ctx, [onProgress](qint64 got, qint64 tot) {
        if (tot > 0) onProgress(qBound(0, int((got * 100) / tot), 99));
    });
    QPointer<KisMainWindow> mwGuard(mw);
    QPointer<QNetworkAccessManager> netGuard(net);
    QPointer<QObject> ctxGuard(ctx);
    QObject::connect(reply, &QNetworkReply::finished, ctx, [=]() {
        const auto netErr = reply->error();
        const QString errMsg = reply->errorString();
        const QUrl finalUrl = reply->url();
        const QByteArray payload = (netErr == QNetworkReply::NoError) ? reply->readAll() : QByteArray();
        const bool isHtml = (netErr == QNetworkReply::NoError) && looksLikeHtmlResponse(reply, payload);
        const QString fileName = resolveCatalogDownloadName(reply, payload, itemId, formatHint);
        reply->deleteLater();

        if (netErr != QNetworkReply::NoError) {
            onDone(false, i18n("Falha ao baixar: %1", errMsg));
            return;
        }
        if (isHtml && depth == 0 && netGuard && ctxGuard) {
            const QString found = extractDownloadUrlFromHtml(payload, finalUrl);
            if (!found.isEmpty() && found != finalUrl.toString()) {
                installCatalogItem(mwGuard, netGuard, ctxGuard, found, itemId, displayName, formatHint,
                                   onProgress, onDone, 1);
                return;
            }
        }
        if (payload.isEmpty() || isHtml) {
            onDone(false, i18n("O site mostrou uma página em vez do arquivo (ele pode pedir login). Use \"Ver no site\"."));
            return;
        }
        if (!looksLikeBrushPayload(payload)) {
            onDone(false, i18n("O arquivo baixado não é um pacote de pincel."));
            return;
        }
        const QString outPath = communityDownloadsDir() + QLatin1Char('/') + fileName;
        QFile f(outPath);
        if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            onDone(false, i18n("Não foi possível salvar o arquivo no tablet."));
            return;
        }
        f.write(payload);
        f.close();
        if (!mwGuard) return;
        QString importMsg;
        if (!importBrushFileQuietly(mwGuard, outPath, &importMsg)) {
            onDone(false, importMsg.isEmpty() ? i18n("O Nanquim não reconheceu \"%1\" como pacote de pincel.", fileName)
                                              : importMsg);
            return;
        }
        markBrushPanelDirty(mwGuard);
        refreshFavoriteIds();
        refreshStudioBrushes(mwGuard);
        if (!itemId.isEmpty()) {
            KConfigGroup cfg = KSharedConfig::openConfig()->group("Nanquim");
            QStringList saved = cfg.readEntry("installedCatalogIds", QStringList());
            if (!saved.contains(itemId)) saved << itemId;
            cfg.writeEntry("installedCatalogIds", saved);
            cfg.sync();
        }
        onDone(true, i18n("Instalado · %1 (Pincéis → Importados)", displayName.isEmpty() ? fileName : displayName));
    });
}

// Chama uma função quando o widget muda de tamanho (a grade refaz as colunas).
class NanquimResizeWatcher : public QObject
{
public:
    NanquimResizeWatcher(QWidget *target, std::function<void()> cb)
        : QObject(target), m_cb(std::move(cb)) { target->installEventFilter(this); }
protected:
    bool eventFilter(QObject *o, QEvent *e) override
    {
        if (e->type() == QEvent::Resize && m_cb) m_cb();
        return QObject::eventFilter(o, e);
    }
private:
    std::function<void()> m_cb;
};

// ---------------------------------------------------------------------------
// Aba Comunidade da tela inicial (como os Assets do Clip Studio): destaque,
// grade de pacotes com prévia, painel de detalhes e instalação no próprio app.
// ---------------------------------------------------------------------------
QWidget *buildCommunityPage(KisMainWindow *mw, QWidget *parent)
{
    QWidget *page = new QWidget(parent);
    page->setObjectName(QStringLiteral("nanquimCommunityPage"));
    page->setAttribute(Qt::WA_StyledBackground);
    page->setStyleSheet(QStringLiteral(
        "QWidget#nanquimCommunityPage { background: #24231f; }"
        "QWidget#nqComHost { background: transparent; }"
        "QScrollArea#nqComScroll { background: transparent; border: none; }"
        "QFrame#nqComCard { background: #2e2d29; border: 1px solid #4a4842; border-radius: 16px; }"
        "QFrame#nqComFeatured { background: #2a2925; border: 1px solid #3a3934; border-radius: 22px; }"
        "QFrame#nqComPanel { background: #2a2925; border-left: 1px solid #4a4842; }"
        "QLabel#nqComKicker { color: #6fbfad; font-size: 12px; font-weight: 800; letter-spacing: 2px; background: transparent; }"
        "QLabel#nqComTitle { color: #eeeae0; font-size: 15px; font-weight: 800; background: transparent; }"
        "QLabel#nqComBig { color: #eeeae0; font-size: 24px; font-weight: 800; background: transparent; }"
        "QLabel#nqComMeta { color: #a8a398; font-size: 12px; background: transparent; }"
        "QLabel#nqComDesc { color: #cfcabf; font-size: 14px; background: transparent; }"
        "QLabel#nqComStatus { color: #a8a398; font-size: 13px; background: transparent; }"
        "QLabel#nqComBadge { background: rgba(29, 28, 25, 215); color: #eeeae0; border-radius: 8px;"
        "  padding: 2px 8px; font-size: 12px; font-weight: 700; }"
        "QLabel#nqComPreview { background: #3a3934; border-radius: 12px; }"
        "QLabel#nqComEmpty { color: #a8a398; font-size: 14px; border: 2px dashed #4a4842; border-radius: 16px; background: transparent; }"
        "QPushButton#nqComInstall { min-height: 44px; border-radius: 11px; border: none; background: #2f7568;"
        "  color: #ffffff; font-size: 14px; font-weight: 800; padding: 0 18px; }"
        "QPushButton#nqComInstall:pressed { background: #276459; }"
        "QPushButton#nqComInstall:disabled { background: #253d37; color: #6fbfad; border: 1px solid #3e8e7e; }"
        "QPushButton#nqComGhost { min-height: 44px; border-radius: 11px; border: 1px solid #4a4842; background: transparent;"
        "  color: #eeeae0; font-size: 14px; font-weight: 700; padding: 0 18px; }"
        "QPushButton#nqComGhost:pressed { background: #3a3934; }"
        "QPushButton#nqComOpen { border: none; background: transparent; padding: 0; text-align: left; }"
        "QPushButton#nqComScrim { border: none; background: rgba(12, 12, 10, 160); }"
        "QCheckBox#nqComOnly { color: #cfcabf; font-size: 14px; font-weight: 700; spacing: 10px; background: transparent; }"
        "QCheckBox#nqComOnly::indicator { width: 22px; height: 22px; border-radius: 7px; border: 1px solid #6b675d; background: #2e2d29; }"
        "QCheckBox#nqComOnly::indicator:checked { background: #2f7568; border-color: #6fbfad; }"));

    QVBoxLayout *pl = new QVBoxLayout(page);
    pl->setContentsMargins(32, 14, 26, 0);
    pl->setSpacing(12);

    // busca, filtro e atualizar
    QHBoxLayout *row1 = new QHBoxLayout();
    row1->setSpacing(12);
    QLineEdit *search = new QLineEdit(page);
    search->setObjectName(QStringLiteral("nanquimSearch"));
    search->setPlaceholderText(i18n("Buscar pincel, autor ou estilo"));
    search->setClearButtonEnabled(true);
    search->addAction(designIcon(Svg::SEARCH, 18), QLineEdit::LeadingPosition);
    search->setMaximumWidth(460);
    row1->addWidget(search, 1);
    row1->addStretch(0);
    QCheckBox *onlyHere = new QCheckBox(i18n("Só o que instala aqui"), page);
    onlyHere->setObjectName(QStringLiteral("nqComOnly"));
    onlyHere->setFocusPolicy(Qt::NoFocus);
    onlyHere->setChecked(true);
    row1->addWidget(onlyHere);
    QPushButton *refresh = galleryButton(i18n("Atualizar"), nullptr, page);
    row1->addWidget(refresh);
    pl->addLayout(row1);

    // categorias + contagem
    const QStringList cats = {i18n("Tudo"), QStringLiteral("Nanquim"), QStringLiteral("Lápis"),
                              QStringLiteral("Pintura"), QStringLiteral("Retícula"), QStringLiteral("Portais")};
    QHBoxLayout *chips = new QHBoxLayout();
    chips->setSpacing(8);
    QButtonGroup *catGroup = new QButtonGroup(page);
    catGroup->setExclusive(true);
    for (int i = 0; i < cats.size(); ++i) {
        QPushButton *c = new QPushButton(cats[i], page);
        c->setProperty("class", QStringLiteral("nanquimChip"));
        c->setCheckable(true);
        c->setChecked(i == 0);
        c->setFocusPolicy(Qt::NoFocus);
        catGroup->addButton(c, i);
        chips->addWidget(c);
    }
    chips->addStretch(1);
    QLabel *status = new QLabel(page);
    status->setObjectName(QStringLiteral("nqComStatus"));
    chips->addWidget(status);
    pl->addLayout(chips);

    QScrollArea *scroll = new QScrollArea(page);
    scroll->setObjectName(QStringLiteral("nqComScroll"));
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->viewport()->setAutoFillBackground(false);
    scroll->setProperty("nanquimSmooth", true);
    tuneSmoothScroller(KisKineticScroller::createPreconfiguredScroller(scroll));
    QWidget *host = new QWidget();
    host->setObjectName(QStringLiteral("nqComHost"));
    QVBoxLayout *hostLayout = new QVBoxLayout(host);
    hostLayout->setContentsMargins(0, 6, 6, 32);
    hostLayout->setSpacing(20);
    scroll->setWidget(host);
    pl->addWidget(scroll, 1);

    struct ComState {
        QVector<CommunityCatalogItem> items;
        QHash<QString, int> progress;
        QSet<QString> installed;
        QSet<QString> failed;
        QHash<QString, QImage> previews;
        QSet<QString> pending;
        int cols = 0;
    };
    auto st = std::make_shared<ComState>();
    Q_FOREACH (const QString &id, KSharedConfig::openConfig()->group("Nanquim").readEntry("installedCatalogIds", QStringList())) {
        st->installed.insert(id);
    }
    QNetworkAccessManager *net = new QNetworkAccessManager(page);
    QPointer<KisMainWindow> mwGuard(mw);
    auto rebuild = std::make_shared<std::function<void()>>();

    // --- prévias: memória -> disco -> rede ---
    auto applyPreview = [page, st](const QString &url) {
        const QImage img = st->previews.value(url);
        if (img.isNull()) return;
        Q_FOREACH (QLabel *lbl, page->findChildren<QLabel*>(QStringLiteral("nqComPreview"))) {
            if (lbl->property("nqPreviewUrl").toString() == url) {
                lbl->setPixmap(catalogPreviewPixmap(img, lbl->size(), lbl->devicePixelRatioF()));
            }
        }
    };
    auto requestPreview = [=](const QString &url) {
        if (url.isEmpty()) return;
        if (st->previews.contains(url)) {
            applyPreview(url);
            return;
        }
        const QString cacheFile = catalogPreviewCacheFile(url);
        const QImage disk(cacheFile);
        if (!disk.isNull()) {
            st->previews.insert(url, disk);
            applyPreview(url);
            return;
        }
        if (st->pending.contains(url)) return;
        st->pending.insert(url);
        QNetworkRequest req{QUrl(url)};
        req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
        req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("Mozilla/5.0 (Linux; Android 16) Nanquim/1.0"));
        QNetworkReply *reply = net->get(req);
        QObject::connect(reply, &QNetworkReply::finished, page, [=]() {
            const QByteArray data = reply->error() == QNetworkReply::NoError ? reply->readAll() : QByteArray();
            reply->deleteLater();
            st->pending.remove(url);
            QImage img = QImage::fromData(data);
            if (img.isNull()) return;
            if (img.width() > 640 || img.height() > 400) {
                img = img.scaled(640, 400, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            }
            img = img.convertToFormat(QImage::Format_RGB32);
            img.save(cacheFile, "JPG", 85);
            st->previews.insert(url, img);
            applyPreview(url);
        });
    };
    auto makePreview = [=](QWidget *parentW, const CommunityCatalogItem &it, const QSize &size) {
        QLabel *p = new QLabel(parentW);
        p->setObjectName(QStringLiteral("nqComPreview"));
        p->setFixedSize(size);
        p->setAlignment(Qt::AlignCenter);
        p->setAttribute(Qt::WA_TransparentForMouseEvents);
        p->setProperty("nqPreviewUrl", it.previewUrl);
        p->setPixmap(svgPixmap(QString::fromUtf8(it.downloadUrl.isEmpty() ? Svg::BOOK : Svg::BRUSH),
                               QColor(122, 118, 109), 34));
        requestPreview(it.previewUrl);
        return p;
    };

    // --- botões de instalar: o texto acompanha o estado ---
    auto paintButton = [st](QPushButton *b) {
        const QString id = b->property("nqItemId").toString();
        if (st->progress.contains(id)) {
            b->setEnabled(false);
            b->setText(i18n("Instalando %1%", st->progress.value(id)));
        } else if (st->installed.contains(id)) {
            b->setEnabled(false);
            b->setText(i18n("Instalado ✓"));
        } else {
            b->setEnabled(true);
            b->setText(st->failed.contains(id) ? i18n("Tentar de novo") : i18n("Instalar"));
        }
    };
    auto refreshButtons = [page, paintButton](const QString &id) {
        Q_FOREACH (QPushButton *b, page->findChildren<QPushButton*>(QStringLiteral("nqComInstall"))) {
            if (b->property("nqItemId").toString() == id) paintButton(b);
        }
    };
    auto startInstall = [=](const CommunityCatalogItem &it) {
        if (st->progress.contains(it.id) || it.downloadUrl.isEmpty()) return;
        const QString id = it.id;
        st->failed.remove(id);
        st->progress.insert(id, 0);
        refreshButtons(id);
        installCatalogItem(mw, net, page, it.downloadUrl, id, it.name, it.format,
                           [st, refreshButtons, id](int pct) {
                               st->progress[id] = pct;
                               refreshButtons(id);
                           },
                           [st, refreshButtons, id, mwGuard, status](bool ok, const QString &msg) {
                               st->progress.remove(id);
                               if (ok) st->installed.insert(id);
                               else st->failed.insert(id);
                               refreshButtons(id);
                               status->setText(msg);
                               if (mwGuard) showToast(mwGuard, msg);
                           });
    };
    auto makeInstall = [=](QWidget *parentW, const CommunityCatalogItem &it) {
        QPushButton *b = new QPushButton(parentW);
        b->setObjectName(QStringLiteral("nqComInstall"));
        b->setFocusPolicy(Qt::NoFocus);
        b->setProperty("nqItemId", it.id);
        paintButton(b);
        const CommunityCatalogItem copy = it;
        QObject::connect(b, &QPushButton::clicked, page, [startInstall, copy]() { startInstall(copy); });
        return b;
    };
    auto makePageButton = [page](QWidget *parentW, const QString &text, const QString &url) {
        QPushButton *b = new QPushButton(text, parentW);
        b->setObjectName(QStringLiteral("nqComGhost"));
        b->setFocusPolicy(Qt::NoFocus);
        QObject::connect(b, &QPushButton::clicked, page, [url]() { QDesktopServices::openUrl(QUrl::fromUserInput(url)); });
        return b;
    };
    auto label = [](const QString &text, const char *name, QWidget *parentW, bool wrap) {
        QLabel *l = new QLabel(text, parentW);
        l->setObjectName(QLatin1String(name));
        l->setWordWrap(wrap);
        l->setAttribute(Qt::WA_TransparentForMouseEvents);
        return l;
    };
    auto metaOf = [](const CommunityCatalogItem &it) {
        QStringList parts;
        if (!it.author.isEmpty()) parts << it.author;
        if (!it.license.isEmpty()) parts << it.license;
        return parts.join(QStringLiteral(" · "));
    };

    // --- painel de detalhes, entra pela direita ---
    auto openDetail = [=](const CommunityCatalogItem &it) {
        if (QWidget *old = page->findChild<QWidget*>(QStringLiteral("nqComOverlay"))) old->deleteLater();
        QWidget *overlay = new QWidget(page);
        overlay->setObjectName(QStringLiteral("nqComOverlay"));
        overlay->setGeometry(page->rect());
        QPushButton *scrim = new QPushButton(overlay);
        scrim->setObjectName(QStringLiteral("nqComScrim"));
        scrim->setFocusPolicy(Qt::NoFocus);
        scrim->setGeometry(overlay->rect());
        const int pw = qMin(480, page->width() - 40);
        QFrame *panel = new QFrame(overlay);
        panel->setObjectName(QStringLiteral("nqComPanel"));
        panel->setGeometry(page->width() - pw, 0, pw, page->height());
        QVBoxLayout *v = new QVBoxLayout(panel);
        v->setContentsMargins(24, 20, 24, 24);
        v->setSpacing(14);
        QHBoxLayout *top = new QHBoxLayout();
        top->addWidget(label(it.category.toUpper(), "nqComKicker", panel, false));
        top->addStretch(1);
        QPushButton *close = new QPushButton(QStringLiteral("×"), panel);
        close->setObjectName(QStringLiteral("nqComGhost"));
        close->setFocusPolicy(Qt::NoFocus);
        close->setFixedSize(44, 44);
        close->setToolTip(i18n("Fechar"));
        top->addWidget(close);
        v->addLayout(top);
        const int pvW = pw - 48;
        v->addWidget(makePreview(panel, it, QSize(pvW, pvW * 9 / 16)));
        v->addWidget(label(it.name, "nqComBig", panel, true));
        QString meta = metaOf(it);
        if (!it.format.isEmpty()) meta += (meta.isEmpty() ? QString() : QStringLiteral(" · ")) + it.format;
        v->addWidget(label(meta, "nqComMeta", panel, true));
        if (!it.description.isEmpty()) v->addWidget(label(it.description, "nqComDesc", panel, true));
        v->addWidget(label(i18n("Depois de instalar, os pincéis ficam em Pincéis → Importados, dentro de um desenho. "
                                "Dá para remover na lixeira de cada um."), "nqComMeta", panel, true));
        v->addStretch(1);
        if (!it.downloadUrl.isEmpty()) {
            QPushButton *inst = makeInstall(panel, it);
            inst->setMinimumHeight(52);
            v->addWidget(inst);
        }
        if (!it.pageUrl.isEmpty()) v->addWidget(makePageButton(panel, i18n("Ver no site"), it.pageUrl));
        QPointer<QWidget> overlayGuard(overlay);
        auto closeOverlay = [overlayGuard]() { if (overlayGuard) overlayGuard->deleteLater(); };
        QObject::connect(scrim, &QPushButton::clicked, overlay, closeOverlay);
        QObject::connect(close, &QPushButton::clicked, overlay, closeOverlay);
        overlay->show();
        overlay->raise();
        QPropertyAnimation *slide = new QPropertyAnimation(panel, "pos", panel);
        slide->setDuration(220);
        slide->setStartValue(QPoint(page->width(), 0));
        slide->setEndValue(QPoint(page->width() - pw, 0));
        slide->setEasingCurve(QEasingCurve::OutCubic);
        slide->start(QAbstractAnimation::DeleteWhenStopped);
    };

    // --- monta destaque e grade ---
    *rebuild = [=]() {
        while (hostLayout->count() > 0) {
            QLayoutItem *li = hostLayout->takeAt(0);
            if (QWidget *w = li->widget()) w->deleteLater();
            else if (QLayout *l = li->layout()) {
                while (l->count() > 0) {
                    QLayoutItem *ci = l->takeAt(0);
                    if (QWidget *cw = ci->widget()) cw->deleteLater();
                    delete ci;
                }
            }
            delete li;
        }

        const int catIdx = qMax(0, catGroup->checkedId());
        const bool portals = catIdx == cats.size() - 1;
        const QString query = search->text().trimmed().toLower();
        QVector<CommunityCatalogItem> filtered;
        for (const CommunityCatalogItem &it : st->items) {
            if (onlyHere->isChecked() && it.downloadUrl.isEmpty() && !portals) continue;
            if (catIdx > 0 && it.category.compare(cats[catIdx], Qt::CaseInsensitive) != 0) continue;
            if (!query.isEmpty()) {
                const QString hay = (it.name + QLatin1Char(' ') + it.author + QLatin1Char(' ')
                                     + it.description + QLatin1Char(' ') + it.tags.join(QLatin1Char(' '))).toLower();
                if (!hay.contains(query)) continue;
            }
            filtered << it;
        }
        std::stable_sort(filtered.begin(), filtered.end(), [](const CommunityCatalogItem &a, const CommunityCatalogItem &b) {
            return !a.downloadUrl.isEmpty() && b.downloadUrl.isEmpty();
        });
        const int total = filtered.size();

        // destaque: o mais curtido que instala aqui (só em "Tudo", sem busca), fora da grade
        if (catIdx == 0 && query.isEmpty() && !filtered.isEmpty() && !filtered.first().downloadUrl.isEmpty()) {
            const CommunityCatalogItem feat = filtered.takeFirst();
            QFrame *f = new QFrame(host);
            f->setObjectName(QStringLiteral("nqComFeatured"));
            QHBoxLayout *fl = new QHBoxLayout(f);
            fl->setContentsMargins(18, 18, 22, 18);
            fl->setSpacing(24);
            fl->addWidget(makePreview(f, feat, QSize(380, 214)), 0, Qt::AlignTop);
            QVBoxLayout *fr = new QVBoxLayout();
            fr->setSpacing(8);
            fr->addWidget(label(i18n("DESTAQUE · O MAIS CURTIDO"), "nqComKicker", f, false));
            fr->addWidget(label(feat.name, "nqComBig", f, true));
            fr->addWidget(label(metaOf(feat), "nqComMeta", f, true));
            if (!feat.description.isEmpty()) fr->addWidget(label(feat.description, "nqComDesc", f, true));
            fr->addStretch(1);
            QHBoxLayout *fb = new QHBoxLayout();
            fb->setSpacing(10);
            fb->addWidget(makeInstall(f, feat));
            QPushButton *more = new QPushButton(i18n("Ver detalhes"), f);
            more->setObjectName(QStringLiteral("nqComGhost"));
            more->setFocusPolicy(Qt::NoFocus);
            QObject::connect(more, &QPushButton::clicked, page, [openDetail, feat]() { openDetail(feat); });
            fb->addWidget(more);
            fb->addStretch(1);
            fr->addLayout(fb);
            fl->addLayout(fr, 1);
            hostLayout->addWidget(f);
        }

        status->setText(total == 1 ? i18n("1 pacote") : i18n("%1 pacotes", total));

        if (total == 0) {
            QLabel *none = new QLabel(onlyHere->isChecked()
                                      ? i18n("Nada por aqui. Desligue \"Só o que instala aqui\" ou troque a categoria.")
                                      : i18n("Nada encontrado com esse filtro."), host);
            none->setObjectName(QStringLiteral("nqComEmpty"));
            none->setAlignment(Qt::AlignCenter);
            none->setWordWrap(true);
            none->setMinimumHeight(160);
            hostLayout->addWidget(none);
        }

        const int avail = qMax(320, scroll->viewport()->width() - 6);
        const int gap = 16;
        const int cols = qBound(2, (avail + gap) / (260 + gap), 5);
        st->cols = cols;
        const int cardW = (avail - gap * (cols - 1)) / cols;
        QGridLayout *grid = new QGridLayout();
        grid->setHorizontalSpacing(gap);
        grid->setVerticalSpacing(gap);
        for (int i = 0; i < filtered.size(); ++i) {
            const CommunityCatalogItem it = filtered[i];
            QFrame *card = new QFrame(host);
            card->setObjectName(QStringLiteral("nqComCard"));
            card->setFixedWidth(cardW);
            QVBoxLayout *cv = new QVBoxLayout(card);
            cv->setContentsMargins(0, 0, 0, 0);
            cv->setSpacing(0);

            QPushButton *openBtn = new QPushButton(card);
            openBtn->setObjectName(QStringLiteral("nqComOpen"));
            openBtn->setFocusPolicy(Qt::NoFocus);
            openBtn->setToolTip(i18n("Ver detalhes"));
            QVBoxLayout *ob = new QVBoxLayout(openBtn);
            ob->setContentsMargins(8, 8, 8, 0);
            ob->setSpacing(8);
            QLabel *prev = makePreview(openBtn, it, QSize(cardW - 18, 132));
            if (!it.format.isEmpty()) {
                QLabel *badge = new QLabel(it.format, prev);
                badge->setObjectName(QStringLiteral("nqComBadge"));
                badge->adjustSize();
                badge->move(10, 10);
            }
            ob->addWidget(prev);
            QVBoxLayout *tb = new QVBoxLayout();
            tb->setContentsMargins(6, 2, 6, 0);
            tb->setSpacing(4);
            QLabel *title = label(it.name, "nqComTitle", openBtn, true);
            title->setMaximumHeight(44);
            tb->addWidget(title);
            tb->addWidget(label(metaOf(it), "nqComMeta", openBtn, false));
            ob->addLayout(tb);
            openBtn->setMinimumHeight(8 + 132 + 8 + 44 + 4 + 18 + 4);
            QObject::connect(openBtn, &QPushButton::clicked, page, [openDetail, it]() { openDetail(it); });
            cv->addWidget(openBtn);

            QHBoxLayout *act = new QHBoxLayout();
            act->setContentsMargins(14, 8, 14, 14);
            if (!it.downloadUrl.isEmpty()) act->addWidget(makeInstall(card, it), 1);
            else if (!it.pageUrl.isEmpty()) act->addWidget(makePageButton(card, i18n("Ver no site"), it.pageUrl), 1);
            cv->addLayout(act);
            grid->addWidget(card, i / cols, i % cols, Qt::AlignTop | Qt::AlignLeft);
        }
        hostLayout->addLayout(grid);
        hostLayout->addStretch(1);
    };

    // a grade refaz as colunas quando a largura muda (girar o tablet, abrir a tela)
    new NanquimResizeWatcher(scroll->viewport(), [=]() {
        const int avail = qMax(320, scroll->viewport()->width() - 6);
        const int cols = qBound(2, (avail + 16) / (260 + 16), 5);
        if (cols != st->cols) (*rebuild)();
        if (QWidget *ov = page->findChild<QWidget*>(QStringLiteral("nqComOverlay"))) ov->deleteLater();
    });

    // catálogo: o salvo aparece na hora; o online chega depois
    {
        bool okCache = false;
        QFile cf(communityCatalogCachePath());
        if (cf.open(QIODevice::ReadOnly)) {
            const auto cached = parseCommunityCatalogJson(cf.readAll(), &okCache);
            if (okCache) st->items = cached;
        }
        if (!okCache) st->items = builtinCommunityCatalog();
    }
    auto fetch = [=]() {
        const QString catalogUrl = KSharedConfig::openConfig()->group("Nanquim")
                .readEntry("catalogUrl", defaultCommunityCatalogUrl()).trimmed();
        QNetworkRequest req(QUrl::fromUserInput(catalogUrl.isEmpty() ? defaultCommunityCatalogUrl() : catalogUrl));
        req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
        req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("Nanquim/1.0 (Krita)"));
        status->setText(i18n("Atualizando catálogo…"));
        QNetworkReply *reply = net->get(req);
        QObject::connect(reply, &QNetworkReply::finished, page, [=]() {
            const auto err = reply->error();
            const QByteArray raw = err == QNetworkReply::NoError ? reply->readAll() : QByteArray();
            reply->deleteLater();
            bool ok = false;
            const auto parsed = raw.isEmpty() ? QVector<CommunityCatalogItem>() : parseCommunityCatalogJson(raw, &ok);
            if (ok) {
                st->items = parsed;
                QFile cf(communityCatalogCachePath());
                if (cf.open(QIODevice::WriteOnly | QIODevice::Truncate)) cf.write(raw);
                (*rebuild)();
            } else {
                (*rebuild)();
                status->setText(i18n("Sem internet: mostrando o catálogo salvo."));
            }
        });
    };

    QObject::connect(catGroup, &QButtonGroup::idClicked, page, [rebuild](int) { (*rebuild)(); });
    QObject::connect(onlyHere, &QCheckBox::toggled, page, [rebuild](bool) { (*rebuild)(); });
    QObject::connect(search, &QLineEdit::textChanged, page, [rebuild](const QString &) { (*rebuild)(); });
    QObject::connect(refresh, &QPushButton::clicked, page, [fetch]() { fetch(); });

    (*rebuild)();
    QTimer::singleShot(60, page, [fetch]() { fetch(); });
    return page;
}

void showCommunityCatalogSheet(KisMainWindow *mw)
{
    if (!mw) return;

    QDialog *d = makeSheet(mw, 720, 660);
    QVBoxLayout *root = new QVBoxLayout(d);
    root->setContentsMargins(24, 20, 24, 20);
    root->setSpacing(12);
    root->addLayout(sheetHeader(d, i18n("Comunidade"), i18n("Catálogo de pincéis e materiais")));

    // Linha de busca + atualizar + fonte JSON
    QHBoxLayout *topRow = new QHBoxLayout();
    topRow->setSpacing(8);
    QLineEdit *searchEdit = new QLineEdit(d);
    searchEdit->setPlaceholderText(i18n("Buscar por nome, autor ou tag…"));
    searchEdit->setClearButtonEnabled(true);
    topRow->addWidget(searchEdit, 1);
    QPushButton *refreshBtn = nqButton(i18n("Atualizar"), d, "nqGhost");
    QPushButton *sourceBtn = nqButton(i18n("Fonte JSON"), d, "nqGhost");
    topRow->addWidget(refreshBtn);
    topRow->addWidget(sourceBtn);
    root->addLayout(topRow);

    // Filtros de categoria + botão "Só 1 toque"
    const QStringList categories = {
        i18n("Tudo"), QStringLiteral("Nanquim"), QStringLiteral("Lápis"),
        QStringLiteral("Pintura"), QStringLiteral("Retícula"), QStringLiteral("Portais")
    };
    QButtonGroup *catGroup = new QButtonGroup(d);
    QHBoxLayout *filterRow = chipRow(d, catGroup, categories, 0);
    QCheckBox *onlyDirectCheck = new QCheckBox(i18n("Só instalação direta"), d);
    onlyDirectCheck->setChecked(true);
    onlyDirectCheck->setFocusPolicy(Qt::NoFocus);
    onlyDirectCheck->setStyleSheet(QStringLiteral(
        "QCheckBox { color: #cfcabf; font-size: 12px; font-weight: 600; spacing: 8px; background: transparent; }"
        "QCheckBox::indicator { width: 22px; height: 22px; border-radius: 7px; border: 1px solid #6b675d; background: #2e2d29; }"
        "QCheckBox::indicator:checked { background: #2f7568; border-color: #6fbfad; }"));
    filterRow->addWidget(onlyDirectCheck);
    root->addLayout(filterRow);

    // Barra de estado e progresso de download
    QLabel *statusLabel = nqLabel(i18n("Verificando catálogo online…"), "nqSmall", d, true);
    root->addWidget(statusLabel);

    QProgressBar *progress = new QProgressBar(d);
    progress->setFixedHeight(8);
    progress->setTextVisible(false);
    progress->setStyleSheet(QStringLiteral(
        "QProgressBar { background: #3a3934; border: none; border-radius: 4px; }"
        "QProgressBar::chunk { background: #6fbfad; border-radius: 4px; }"));
    progress->hide();
    root->addWidget(progress);

    // Área rolável de cartões do catálogo
    QScrollArea *scroll = new QScrollArea(d);
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setProperty("nanquimSmooth", true);
    tuneSmoothScroller(KisKineticScroller::createPreconfiguredScroller(scroll));

    QWidget *cardsHost = new QWidget(scroll);
    QVBoxLayout *cardsLayout = new QVBoxLayout(cardsHost);
    cardsLayout->setContentsMargins(0, 2, 6, 6);
    cardsLayout->setSpacing(10);
    cardsLayout->addStretch(1);
    scroll->setWidget(cardsHost);
    root->addWidget(scroll, 1);

    root->addWidget(nqDivider(d));

    // Instalação por link direto (.bundle / .kpp / .sut)
    QHBoxLayout *directRow = new QHBoxLayout();
    directRow->setSpacing(8);
    QLineEdit *urlEdit = new QLineEdit(d);
    urlEdit->setPlaceholderText(i18n("Cole um link direto (.bundle, .kpp, .sut ou .zip) para instalar em 1 toque…"));
    QPushButton *directBtn = nqButton(i18n("Instalar link"), d, "nqPrimary");
    directRow->addWidget(urlEdit, 1);
    directRow->addWidget(directBtn);
    root->addLayout(directRow);

    QNetworkAccessManager *net = new QNetworkAccessManager(d);
    QPointer<KisMainWindow> mwGuard(mw);
    QPointer<QDialog> dGuard(d);

    auto itemsState = std::make_shared<QVector<CommunityCatalogItem>>();
    auto installedIds = std::make_shared<QSet<QString>>();
    {
        const QStringList saved = KSharedConfig::openConfig()->group("Nanquim")
                .readEntry("installedCatalogIds", QStringList());
        for (const QString &id : saved) installedIds->insert(id);
    }
    auto busyDownload = std::make_shared<bool>(false);
    auto rebuildCards = std::make_shared<std::function<void()>>();

    // Prévias dos pacotes: memória -> disco -> rede (uma requisição por endereço)
    const QSize previewSize(128, 96);
    auto previewCache = std::make_shared<QHash<QString, QImage>>();
    auto previewPending = std::make_shared<QSet<QString>>();
    auto applyPreview = [cardsHost, previewSize](const QString &url, const QImage &img) {
        if (img.isNull()) return;
        Q_FOREACH (QLabel *lbl, cardsHost->findChildren<QLabel*>(QStringLiteral("nqCatalogPreview"))) {
            if (lbl->property("nqPreviewUrl").toString() == url) {
                lbl->setPixmap(catalogPreviewPixmap(img, previewSize, lbl->devicePixelRatioF()));
            }
        }
    };
    auto requestPreview = [=](const QString &url) {
        if (url.isEmpty()) return;
        if (previewCache->contains(url)) {
            applyPreview(url, previewCache->value(url));
            return;
        }
        const QString cacheFile = catalogPreviewCacheFile(url);
        const QImage disk(cacheFile);
        if (!disk.isNull()) {
            previewCache->insert(url, disk);
            applyPreview(url, disk);
            return;
        }
        if (previewPending->contains(url)) return;
        previewPending->insert(url);
        QNetworkRequest req{QUrl(url)};
        req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
        req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("Mozilla/5.0 (Linux; Android 16) Nanquim/1.0"));
        QNetworkReply *reply = net->get(req);
        QObject::connect(reply, &QNetworkReply::finished, d, [=]() {
            const QByteArray data = reply->error() == QNetworkReply::NoError ? reply->readAll() : QByteArray();
            reply->deleteLater();
            previewPending->remove(url);
            if (!dGuard) return;
            QImage img = QImage::fromData(data);
            if (img.isNull()) return;
            if (img.width() > 480 || img.height() > 360) {
                img = img.scaled(480, 360, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            }
            img = img.convertToFormat(QImage::Format_RGB32);
            img.save(cacheFile, "JPG", 85);
            previewCache->insert(url, img);
            applyPreview(url, img);
        });
    };

    auto startInstallUrl = std::make_shared<std::function<void(const QString &, const QString &, const QString &, const QString &, int)>>();
    *startInstallUrl = [=](const QString &urlStr, const QString &itemId,
                           const QString &displayName, const QString &formatHint, int depth) {
        if (*busyDownload && depth == 0) {
            statusLabel->setText(i18n("Aguarde o download atual terminar."));
            return;
        }
        const QUrl url = QUrl::fromUserInput(urlStr.trimmed());
        if (!url.isValid() || (url.scheme() != QLatin1String("https") && url.scheme() != QLatin1String("http"))) {
            statusLabel->setText(i18n("Link inválido. Use um endereço http:// ou https://."));
            return;
        }
        *busyDownload = true;
        directBtn->setEnabled(false);
        progress->setRange(0, 0);
        progress->show();
        statusLabel->setText(depth > 0
                             ? i18n("Link encontrado na página! Baixando %1…", url.fileName())
                             : i18n("Baixando %1…", displayName.isEmpty() ? url.fileName() : displayName));

        // files.kde.org manda para espelhos http://; o conteúdo é conferido depois (looksLikeBrushPayload)
        QNetworkRequest req(normalizeCatalogDownloadUrl(url));
        req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::UserVerifiedRedirectPolicy);
        req.setMaximumRedirectsAllowed(10);
        req.setHeader(QNetworkRequest::UserAgentHeader,
                      QStringLiteral("Mozilla/5.0 (Linux; Android 16; Tablet) AppleWebKit/537.36 (KHTML, like Gecko) Nanquim/1.0"));
        req.setRawHeader("Accept", "application/octet-stream,application/zip,*/*;q=0.8");
        QNetworkReply *reply = net->get(req);
        QObject::connect(reply, &QNetworkReply::redirected, reply, &QNetworkReply::redirectAllowed);

        QObject::connect(reply, &QNetworkReply::downloadProgress, d, [dGuard, progress](qint64 got, qint64 tot) {
            if (!dGuard) return;
            if (tot > 0) {
                progress->setRange(0, 100);
                progress->setValue(qBound(0, int((got * 100) / tot), 100));
            }
        });
        QObject::connect(reply, &QNetworkReply::finished, d, [=]() {
            const auto netErr = reply->error();
            const QString errMsg = reply->errorString();
            const QUrl finalUrl = reply->url();
            const QByteArray payload = (netErr == QNetworkReply::NoError) ? reply->readAll() : QByteArray();
            const bool isHtml = (netErr == QNetworkReply::NoError) && looksLikeHtmlResponse(reply, payload);
            const QString fileName = resolveCatalogDownloadName(reply, payload, itemId, formatHint);
            reply->deleteLater();

            if (!dGuard) return;

            if (netErr != QNetworkReply::NoError) {
                *busyDownload = false;
                directBtn->setEnabled(true);
                progress->hide();
                statusLabel->setText(i18n("Falha ao baixar: %1", errMsg));
                return;
            }

            if (isHtml && depth == 0) {
                const QString foundUrl = extractDownloadUrlFromHtml(payload, finalUrl);
                if (!foundUrl.isEmpty() && foundUrl != finalUrl.toString()) {
                    (*startInstallUrl)(foundUrl, itemId, displayName, formatHint, 1);
                    return;
                }
            }

            *busyDownload = false;
            directBtn->setEnabled(true);
            progress->hide();

            if (payload.isEmpty() || isHtml) {
                statusLabel->setText(i18n("O site mostrou uma página em vez do arquivo (ele pode pedir login). "
                                          "Toque em \"Ver página\", baixe por lá e abra o arquivo com o Nanquim."));
                return;
            }
            if (!looksLikeBrushPayload(payload)) {
                statusLabel->setText(i18n("O arquivo baixado não é um pacote de pincel (.bundle, .kpp, .sut ou .zip)."));
                return;
            }

            const QString outPath = communityDownloadsDir() + QLatin1Char('/') + fileName;
            QFile f(outPath);
            if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                statusLabel->setText(i18n("Não foi possível salvar o arquivo temporário no tablet."));
                return;
            }
            f.write(payload);
            f.close();

            if (!mwGuard) return;
            if (importBrushFileQuietly(mwGuard, outPath, nullptr)) {
                markBrushPanelDirty(mwGuard);
                refreshFavoriteIds();
                refreshStudioBrushes(mwGuard);
                if (!itemId.isEmpty()) {
                    installedIds->insert(itemId);
                    QStringList savedList;
                    for (const QString &sid : *installedIds) savedList << sid;
                    KSharedConfig::openConfig()->group("Nanquim").writeEntry("installedCatalogIds", savedList);
                    KSharedConfig::openConfig()->sync();
                }
                const QString shown = displayName.isEmpty() ? fileName : displayName;
                statusLabel->setText(i18n("Instalado com sucesso: %1 (abra Pincéis → Importados)", shown));
                showToast(mwGuard, i18n("Instalado · %1", shown));
                if (*rebuildCards) (*rebuildCards)();
            } else {
                statusLabel->setText(i18n("O Nanquim não reconheceu \"%1\" como pacote de pincel (.bundle, .kpp, .sut ou .zip).", fileName));
            }
        });
    };

    *rebuildCards = [=]() {
        while (cardsLayout->count() > 0) {
            QLayoutItem *li = cardsLayout->takeAt(0);
            if (QWidget *w = li->widget()) w->deleteLater();
            delete li;
        }

        const int catIdx = qMax(0, catGroup->checkedId());
        const QString chosenCat = categories.value(catIdx, categories.first());
        const QString query = searchEdit->text().trimmed().toLower();
        const bool directOnly = onlyDirectCheck->isChecked();

        QVector<CommunityCatalogItem> filtered;
        filtered.reserve(itemsState->size());
        for (const CommunityCatalogItem &it : *itemsState) {
            if (directOnly && it.downloadUrl.isEmpty() && catIdx != categories.size() - 1) continue;
            if (catIdx > 0 && it.category.compare(chosenCat, Qt::CaseInsensitive) != 0) continue;
            if (!query.isEmpty()) {
                const QString hay = (it.name + QLatin1Char(' ') + it.author + QLatin1Char(' ')
                                     + it.description + QLatin1Char(' ') + it.tags.join(QLatin1Char(' '))).toLower();
                if (!hay.contains(query)) continue;
            }
            filtered << it;
        }
        std::stable_sort(filtered.begin(), filtered.end(), [](const CommunityCatalogItem &a, const CommunityCatalogItem &b) {
            return !a.downloadUrl.isEmpty() && b.downloadUrl.isEmpty();
        });

        cardsHost->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::MinimumExpanding);

        for (const CommunityCatalogItem &it : filtered) {
            QFrame *card = new QFrame(cardsHost);
            card->setObjectName(QStringLiteral("nqPanel"));
            card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
            QHBoxLayout *outer = new QHBoxLayout(card);
            outer->setContentsMargins(12, 12, 14, 12);
            outer->setSpacing(14);
            QLabel *thumb = new QLabel(card);
            thumb->setObjectName(QStringLiteral("nqCatalogPreview"));
            thumb->setFixedSize(previewSize);
            thumb->setAlignment(Qt::AlignCenter);
            thumb->setProperty("nqPreviewUrl", it.previewUrl);
            thumb->setStyleSheet(QStringLiteral("background: #3a3934; border-radius: 10px;"));
            thumb->setPixmap(svgPixmap(QString::fromUtf8(it.downloadUrl.isEmpty() ? Svg::BOOK : Svg::BRUSH),
                                       QColor(122, 118, 109), 30));
            outer->addWidget(thumb, 0, Qt::AlignTop);
            QVBoxLayout *cl = new QVBoxLayout();
            cl->setContentsMargins(0, 0, 0, 0);
            cl->setSpacing(6);
            outer->addLayout(cl, 1);

            QHBoxLayout *head = new QHBoxLayout();
            head->setSpacing(8);
            QLabel *nameLbl = nqLabel(it.name, "nqItem", card, true);
            nameLbl->setMinimumWidth(0);
            nameLbl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
            nameLbl->setStyleSheet(QStringLiteral("color: #eeeae0; font-size: 14px; font-weight: 700; background: transparent;"));
            head->addWidget(nameLbl, 1);

            const bool isInstalled = installedIds->contains(it.id);
            if (isInstalled) {
                QLabel *doneBadge = new QLabel(i18n("Instalado"), card);
                doneBadge->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
                doneBadge->setStyleSheet(QStringLiteral(
                    "background: #253d37; color: #6fbfad; border: 1px solid #3e8e7e;"
                    " border-radius: 8px; padding: 2px 8px; font-size: 11px; font-weight: 700;"));
                head->addWidget(doneBadge, 0, Qt::AlignTop);
            }

            QLabel *fmtBadge = new QLabel(it.format, card);
            fmtBadge->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
            fmtBadge->setStyleSheet(QStringLiteral(
                "background: #3a3934; color: #cfcabf; border: 1px solid #524f48;"
                " border-radius: 8px; padding: 2px 8px; font-size: 11px; font-weight: 600;"));
            head->addWidget(fmtBadge, 0, Qt::AlignTop);
            cl->addLayout(head);

            QString meta = it.category;
            if (!it.author.isEmpty()) meta += QStringLiteral(" · ") + it.author;
            if (!it.license.isEmpty()) meta += QStringLiteral(" · ") + it.license;
            QLabel *metaLbl = nqLabel(meta, "nqSmall", card, true);
            metaLbl->setMinimumWidth(0);
            cl->addWidget(metaLbl);

            if (!it.description.isEmpty()) {
                QLabel *descLbl = nqLabel(it.description, "nqSmall", card, true);
                descLbl->setMinimumWidth(0);
                cl->addWidget(descLbl);
            }

            QHBoxLayout *actRow = new QHBoxLayout();
            actRow->setSpacing(8);
            actRow->addStretch(1);
            if (!it.pageUrl.isEmpty()) {
                QPushButton *pageBtn = nqButton(it.downloadUrl.isEmpty() ? i18n("Abrir página") : i18n("Ver página"),
                                                card, "nqGhost");
                const QString pageUrl = it.pageUrl;
                QObject::connect(pageBtn, &QPushButton::clicked, d, [pageUrl]() {
                    QDesktopServices::openUrl(QUrl::fromUserInput(pageUrl));
                });
                actRow->addWidget(pageBtn);
            }
            if (!it.downloadUrl.isEmpty()) {
                QPushButton *instBtn = nqButton(isInstalled ? i18n("Reinstalar") : i18n("Instalar"), card, "nqPrimary");
                const QString dl = it.downloadUrl;
                const QString id = it.id;
                const QString nm = it.name;
                const QString fm = it.format;
                QObject::connect(instBtn, &QPushButton::clicked, d, [startInstallUrl, dl, id, nm, fm]() {
                    (*startInstallUrl)(dl, id, nm, fm, 0);
                });
                actRow->addWidget(instBtn);
            }
            cl->addLayout(actRow);
            cardsLayout->addWidget(card);
            if (!it.previewUrl.isEmpty()) requestPreview(it.previewUrl);
        }

        if (filtered.isEmpty()) {
            QLabel *none = nqLabel(i18n("Nenhum material encontrado com esse filtro."), "nqSmall", cardsHost, true);
            none->setAlignment(Qt::AlignCenter);
            none->setMinimumHeight(80);
            cardsLayout->addWidget(none);
        }
        cardsLayout->addStretch(1);
    };

    // Carrega cache local ou catálogo inicial imediatamente (zero espera)
    {
        bool okCache = false;
        QFile cf(communityCatalogCachePath());
        if (cf.open(QIODevice::ReadOnly)) {
            const auto cached = parseCommunityCatalogJson(cf.readAll(), &okCache);
            if (okCache) *itemsState = cached;
        }
        if (!okCache) *itemsState = builtinCommunityCatalog();
        (*rebuildCards)();
    }

    auto fetchRemoteCatalog = [=]() {
        const QString catalogUrl = KSharedConfig::openConfig()->group("Nanquim")
                .readEntry("catalogUrl", defaultCommunityCatalogUrl()).trimmed();
        const QUrl url = QUrl::fromUserInput(catalogUrl.isEmpty() ? defaultCommunityCatalogUrl() : catalogUrl);
        if (!url.isValid()) {
            statusLabel->setText(i18n("Catálogo local ativo (URL do catálogo inválida)."));
            return;
        }
        statusLabel->setText(i18n("Atualizando catálogo online…"));
        QNetworkRequest req(url);
        req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
        req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("Nanquim/1.0 (Krita)"));
        QNetworkReply *reply = net->get(req);
        QObject::connect(reply, &QNetworkReply::finished, d, [=]() {
            const auto err = reply->error();
            const QByteArray raw = (err == QNetworkReply::NoError) ? reply->readAll() : QByteArray();
            reply->deleteLater();
            if (!dGuard) return;

            if (err == QNetworkReply::NoError && !raw.isEmpty()) {
                bool ok = false;
                const auto parsed = parseCommunityCatalogJson(raw, &ok);
                if (ok) {
                    *itemsState = parsed;
                    QFile cf(communityCatalogCachePath());
                    if (cf.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                        cf.write(raw);
                    }
                    statusLabel->setText(i18n("Catálogo online atualizado · %1 item(ns) disponíveis.", parsed.size()));
                    (*rebuildCards)();
                    return;
                }
            }
            statusLabel->setText(i18n("Usando catálogo inicial/cache · suba catalogo-comunidade.json no GitHub para sincronizar pacotes diretos."));
        });
    };

    QObject::connect(catGroup, &QButtonGroup::idClicked, d, [rebuildCards](int) { (*rebuildCards)(); });
    QObject::connect(onlyDirectCheck, &QCheckBox::toggled, d, [rebuildCards](bool) { (*rebuildCards)(); });
    QObject::connect(searchEdit, &QLineEdit::textChanged, d, [rebuildCards](const QString &) { (*rebuildCards)(); });
    QObject::connect(refreshBtn, &QPushButton::clicked, d, [fetchRemoteCatalog]() { fetchRemoteCatalog(); });
    QObject::connect(sourceBtn, &QPushButton::clicked, d, [d, fetchRemoteCatalog]() {
        const QString cur = KSharedConfig::openConfig()->group("Nanquim")
                .readEntry("catalogUrl", defaultCommunityCatalogUrl());
        bool ok = false;
        const QString next = QInputDialog::getText(
                    d, i18n("Fonte do catálogo JSON"),
                    i18n("Endereço HTTPS do arquivo catalogo-comunidade.json (deixe em branco para restaurar o padrão):"),
                    QLineEdit::Normal, cur, &ok).trimmed();
        if (!ok) return;
        const QString finalUrl = next.isEmpty() ? defaultCommunityCatalogUrl() : next;
        KSharedConfig::openConfig()->group("Nanquim").writeEntry("catalogUrl", finalUrl);
        KSharedConfig::openConfig()->sync();
        fetchRemoteCatalog();
    });
    QObject::connect(directBtn, &QPushButton::clicked, d, [urlEdit, startInstallUrl]() {
        const QString u = urlEdit->text().trimmed();
        if (u.isEmpty()) return;
        (*startInstallUrl)(u, QString(), QFileInfo(QUrl::fromUserInput(u).path()).fileName(), QString(), 0);
    });

    QTimer::singleShot(50, d, [fetchRemoteCatalog]() { fetchRemoteCatalog(); });
    d->open();
}

// ---------------------------------------------------------------------------
// Tela de desenho do redesenho ("estúdio"): a página ocupa a tela inteira e as
// ferramentas flutuam por cima, em peças pequenas. Uma coluna só de um lado
// (cor, 5 pincéis, tamanho, opacidade, desfazer e refazer), as camadas do
// outro, o título e as ferramentas em cápsulas no topo e as páginas do
// capítulo embaixo. As barras do Krita (topo, lateral, abas e status) somem.
// ---------------------------------------------------------------------------
const QString STUDIO_TITLE_ID = QStringLiteral("nanquimStudioTitle");
const QString STUDIO_TOOLS_ID = QStringLiteral("nanquimStudioTools");
const QString STUDIO_RAIL_ID = QStringLiteral("nanquimStudioRail");
const QString STUDIO_LAYERS_ID = QStringLiteral("nanquimStudioLayers");
const QString STUDIO_PAGES_ID = QStringLiteral("nanquimStudioPages");
const int STUDIO_SLOTS = 5;      // pincéis na coluna
const int STUDIO_LAYER_MAX = 6;  // miniaturas de camada na faixa

const char *STUDIO_STYLE =
    "QFrame#nanquimStudioTitle, QFrame#nanquimStudioTools, QFrame#nanquimStudioPages {"
    "  background: rgba(36,35,31,0.94); border: 1px solid rgba(238,234,224,0.08); border-radius: 18px; }"
    "QFrame#nanquimStudioRail, QFrame#nanquimStudioLayers {"
    "  background: rgba(46,45,41,0.96); border: 1px solid rgba(238,234,224,0.08); border-radius: 22px; }"
    "QToolButton { border: none; border-radius: 12px; background: transparent; color: #eeeae0; padding: 0px; }"
    "QToolButton:pressed { background: #3a3934; }"
    "QToolButton:checked { background: #2f7568; }"
    "QToolButton:disabled { background: transparent; }"
    "QToolButton::menu-indicator { image: none; width: 0px; }"
    "QLabel { background: transparent; }"
    "QLabel#nanquimStudioName { color: #eeeae0; font-size: 15px; font-weight: 600; }"
    "QLabel#nanquimStudioSub { color: #a8a398; font-size: 12px; }"
    "QLabel#nanquimStudioValue { color: #a8a398; font-size: 12px; }"
    "QLabel#nanquimStudioPage { color: #eeeae0; font-size: 14px; padding: 0px 8px; }"
    "QFrame#nanquimStudioDivider { background: #4a4842; border: none; }"
    "QSlider:vertical { width: 32px; background: transparent; }"
    "QSlider::groove:vertical { width: 6px; border-radius: 3px; background: #4a4842; }"
    "QSlider::add-page:vertical { width: 6px; border-radius: 3px; background: #6fbfad; }"
    "QSlider::sub-page:vertical { width: 6px; border-radius: 3px; background: #4a4842; }"
    "QSlider::handle:vertical { height: 24px; width: 24px; margin: 0px -9px; border-radius: 12px; background: #eeeae0; }";

// ícone das peças do estúdio: claro quando solto, branco sobre o verdete quando ativo
QIcon studioIcon(const char *body, int size = 22)
{
    const QString b = QString::fromUtf8(body);
    QIcon icon;
    icon.addPixmap(svgPixmap(b, C_TEXT, size), QIcon::Normal, QIcon::Off);
    icon.addPixmap(svgPixmap(b, QColor(Qt::white), size), QIcon::Normal, QIcon::On);
    icon.addPixmap(svgPixmap(b, C_DISABLED, size), QIcon::Disabled, QIcon::Off);
    icon.addPixmap(svgPixmap(b, C_DISABLED, size), QIcon::Disabled, QIcon::On);
    return icon;
}

QToolButton *studioButton(QWidget *parent, const QString &tip, const char *svg, bool checkable = false, int side = 48)
{
    QToolButton *b = iconButton(parent, tip, svg ? studioIcon(svg) : QIcon(), checkable);
    b->setAccessibleName(tip);
    b->setFixedSize(side, side);
    return b;
}

QFrame *studioDivider(QWidget *parent, bool vertical)
{
    QFrame *d = new QFrame(parent);
    d->setObjectName(QStringLiteral("nanquimStudioDivider"));
    if (vertical) d->setFixedSize(1, 28);
    else d->setFixedSize(40, 1);
    return d;
}

QFrame *studioPiece(KisMainWindow *mw, const QString &id)
{
    QFrame *f = new QFrame(mw);
    f->setObjectName(id);
    f->setAttribute(Qt::WA_StyledBackground);
    f->setStyleSheet(QLatin1String(STUDIO_STYLE));
    f->hide();
    return f;
}

// respiro da barra de status e da barra de navegação do Android
QMargins studioInsets(KisMainWindow *mw)
{
    QScreen *screen = mw->screen();
    if (!screen) return QMargins();
    const QRect avail = screen->availableGeometry();
    const int winTop = mw->mapToGlobal(QPoint(0, 0)).y();
    const int winBottom = mw->mapToGlobal(QPoint(0, mw->height())).y();
    return QMargins(0, qMax(0, avail.top() - winTop), 0, qBound(0, winBottom - (avail.bottom() + 1), 160));
}

// --- cor atual: a cor de frente grande, a de fundo pequena atrás ------------
class StudioColorButton : public QAbstractButton
{
public:
    StudioColorButton(KisMainWindow *mw, QWidget *parent) : QAbstractButton(parent), m_mw(mw)
    {
        setFocusPolicy(Qt::NoFocus);
        setFixedSize(56, 56);
        setToolTip(i18n("Cor"));
        setAccessibleName(i18n("Cor atual. Toque para trocar"));
    }
protected:
    void paintEvent(QPaintEvent *) override
    {
        QColor fg(0x1b, 0x1a, 0x18), bg(Qt::white);
        KisViewManager *vm = m_mw ? m_mw->viewManager() : nullptr;
        if (vm && vm->canvasResourceProvider()) {
            fg = vm->canvasResourceProvider()->fgColor().toQColor();
            bg = vm->canvasResourceProvider()->bgColor().toQColor();
        }
        QPainter g(this);
        g.setRenderHint(QPainter::Antialiasing);
        const QColor ring(0x2e, 0x2d, 0x29);
        g.setPen(QPen(ring, 2));
        g.setBrush(bg);
        g.drawEllipse(QRectF(width() - 27, height() - 27, 22, 22));
        if (isDown()) g.setOpacity(0.85);
        g.setPen(QPen(ring, 2));
        g.setBrush(fg);
        const QRectF big(5, 5, 38, 38);
        g.drawEllipse(big);
        g.setPen(QPen(C_TEXT, 2));
        g.setBrush(Qt::NoBrush);
        g.drawEllipse(big.adjusted(-2, -2, 2, 2));
    }
private:
    QPointer<KisMainWindow> m_mw;
};

// --- pincel da coluna: o traço de amostra do próprio pincel -----------------
// versão sem ler as configurações do pincel (paintOpSize derrubava o app ao abrir páginas)
BrushItem itemForPresetSafe(const KisPaintOpPresetSP &preset)
{
    BrushItem item;
    item.preset = preset;
    item.id = preset->resourceId();
    const QString filename = preset->filename();
    for (const NanquimBrush &b : NANQUIM_BRUSHES) {
        const QString file = QString::fromUtf8(b.file);
        if (file.endsWith(QLatin1Char('*')) ? filename.startsWith(file.left(file.size() - 1)) : filename == file) {
            item.name = QString::fromUtf8(b.name);
            item.shortName = QString::fromUtf8(b.shortName);
            item.style = b.style;
            item.weight = b.weight;
            return item;
        }
    }
    item.name = filename.section(QLatin1Char('.'), 0, 0).replace(QLatin1Char('_'), QLatin1Char(' '));
    item.shortName = item.name;
    item.weight = 0.5;
    return item;
}

class StudioBrushSlot : public QAbstractButton
{
public:
    explicit StudioBrushSlot(QWidget *parent) : QAbstractButton(parent)
    {
        setFocusPolicy(Qt::NoFocus);
        setFixedSize(56, 48);
        setCheckable(true);
    }
    void setPreset(const KisPaintOpPresetSP &preset)
    {
        m_preset = preset;
        if (preset) {
            m_item = itemForPresetSafe(preset);
            setToolTip(m_item.name);
            setAccessibleName(m_item.name);
        }
        setVisible(bool(preset));
        update();
    }
    int presetId() const { return m_preset ? m_preset->resourceId() : -1; }
    KisPaintOpPresetSP preset() const { return m_preset; }
protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter g(this);
        g.setRenderHint(QPainter::Antialiasing);
        const QRectF r = QRectF(rect()).adjusted(1, 1, -1, -1);
        if (isChecked()) {
            g.setPen(QPen(QColor(0x6f, 0xbf, 0xad), 2));
            g.setBrush(QColor(0x3a, 0x39, 0x34));
            g.drawRoundedRect(r.adjusted(1, 1, -1, -1), 12, 12);
        } else if (isDown()) {
            g.setPen(Qt::NoPen);
            g.setBrush(QColor(0x3a, 0x39, 0x34));
            g.drawRoundedRect(r, 12, 12);
        }
        if (!m_preset) return;
        const QRectF strokeRect = r.adjusted(4, 6, -4, -6);
        const QImage real = strokePreview(m_preset, strokeRect.size(), C_TEXT, this);
        if (!real.isNull()) g.drawImage(strokeRect, real);
        else paintStrokeSample(g, r.adjusted(8, 10, -8, -10), m_item.style, qMax<qreal>(0.12, m_item.weight), C_TEXT);
    }
private:
    KisPaintOpPresetSP m_preset;
    BrushItem m_item;
};

// os 5 pincéis da coluna: favoritos primeiro, depois os usados há pouco
QVector<KisPaintOpPresetSP> studioBrushes()
{
    QVector<KisPaintOpPresetSP> out;
    QSet<int> seen;
    // favoritos desligados na coluna por enquanto: investigando a queda ao abrir páginas
    const bool STUDIO_USA_FAVORITOS = false;
    if (STUDIO_USA_FAVORITOS) {
        Q_FOREACH (const KisPaintOpPresetSP &p, favoritePresets()) {
            if (out.size() >= STUDIO_SLOTS) break;
            if (p && !seen.contains(p->resourceId())) { out << p; seen.insert(p->resourceId()); }
        }
    }
    KisUsageLogger::log(QStringLiteral("[Nanquim] pincéis: recentes"));
    for (int i = 0; i < s_recentBrushes.size() && out.size() < STUDIO_SLOTS; i++) {
        KisPaintOpPresetSP p = presetForId(s_recentBrushes[i]);
        if (p && !seen.contains(p->resourceId())) { out << p; seen.insert(p->resourceId()); }
    }
    if (out.size() < STUDIO_SLOTS) {
        KisUsageLogger::log(QStringLiteral("[Nanquim] pincéis: do pacote"));
        Q_FOREACH (const BrushItem &b, nanquimBrushItems(QString(), true)) {
            if (out.size() >= STUDIO_SLOTS) break;
            if (b.preset && !seen.contains(b.id)) { out << b.preset; seen.insert(b.id); }
        }
    }
    return out;
}

void refreshStudioBrushes(KisMainWindow *mw)
{
    QFrame *rail = mw->findChild<QFrame*>(STUDIO_RAIL_ID);
    if (!rail) return;
    const QList<StudioBrushSlot*> slots = rail->findChildren<StudioBrushSlot*>();
    const QVector<KisPaintOpPresetSP> brushes = studioBrushes();
    KisUsageLogger::log(QStringLiteral("[Nanquim] pincéis: %1 achados").arg(brushes.size()));
    KisViewManager *vm = mw->viewManager();
    KisPaintOpPresetSP cur = vm && vm->canvasResourceProvider() ? vm->canvasResourceProvider()->currentPreset() : KisPaintOpPresetSP();
    KisUsageLogger::log(QStringLiteral("[Nanquim] pincéis: pincel atual lido"));
    for (int i = 0; i < slots.size(); i++) {
        KisUsageLogger::log(QStringLiteral("[Nanquim] pincéis: espaço %1").arg(i));
        slots[i]->setPreset(i < brushes.size() ? brushes[i] : KisPaintOpPresetSP());
        KisUsageLogger::log(QStringLiteral("[Nanquim] pincéis: espaço %1 pronto").arg(i));
        slots[i]->setChecked(cur && slots[i]->preset() && slots[i]->preset()->resourceId() == cur->resourceId());
    }
}

// --- camadas: miniaturas das de cima, a ativa com anel ----------------------
class StudioLayerThumb : public QAbstractButton
{
public:
    explicit StudioLayerThumb(QWidget *parent) : QAbstractButton(parent)
    {
        setFocusPolicy(Qt::NoFocus);
        setFixedSize(52, 64);
        setCheckable(true);
    }
    KisNodeSP node;
    QImage thumb;
protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter g(this);
        g.setRenderHint(QPainter::Antialiasing);
        g.setRenderHint(QPainter::SmoothPixmapTransform);
        const QRectF r = QRectF(rect()).adjusted(3, 3, -3, -3);
        QPainterPath clip;
        clip.addRoundedRect(r, 5, 5);
        g.fillPath(clip, QColor(0xf3, 0xee, 0xe3));
        if (!thumb.isNull()) {
            g.save();
            g.setClipPath(clip);
            const QSizeF s = QSizeF(thumb.size()).scaled(r.size(), Qt::KeepAspectRatio);
            g.drawImage(QRectF(r.center().x() - s.width() / 2, r.center().y() - s.height() / 2, s.width(), s.height()), thumb);
            g.restore();
        }
        if (node && !node->visible()) {
            g.fillPath(clip, QColor(36, 35, 31, 150));
        }
        g.setBrush(Qt::NoBrush);
        if (isChecked()) {
            g.setPen(QPen(QColor(0x6f, 0xbf, 0xad), 2.5));
            g.drawRoundedRect(r.adjusted(-1.5, -1.5, 1.5, 1.5), 6, 6);
        } else {
            g.setPen(QPen(QColor(0x4a, 0x48, 0x42), 1));
            g.drawRoundedRect(r, 5, 5);
        }
    }
};

// thumbs: refaz as miniaturas; onlyActive: só a da camada ativa (depois de um traço)
void refreshStudioLayers(KisMainWindow *mw, bool thumbs, bool onlyActive = false)
{
    QFrame *strip = mw->findChild<QFrame*>(STUDIO_LAYERS_ID);
    if (!strip || !strip->isVisible()) return;
    KisView *view = mw->activeView();
    KisImageSP image;
    if (view) image = view->image();
    const QList<StudioLayerThumb*> items = strip->findChildren<StudioLayerThumb*>();
    QVector<KisNodeSP> nodes;
    if (image && image->root()) {
        // de cima para baixo, como o painel de camadas mostra
        for (KisNodeSP n = image->root()->lastChild(); n && nodes.size() < items.size(); n = n->prevSibling()) {
            if (n->inherits("KisSelectionMask")) continue;
            nodes << n;
        }
    }
    KisNodeSP active = mw->viewManager() ? mw->viewManager()->activeNode() : KisNodeSP();
    for (int i = 0; i < items.size(); i++) {
        StudioLayerThumb *t = items[i];
        KisNodeSP n = i < nodes.size() ? nodes[i] : KisNodeSP();
        const bool changed = t->node != n;
        t->node = n;
        t->setVisible(bool(n));
        if (!n) continue;
        t->setToolTip(n->name());
        t->setAccessibleName(i18n("Camada %1", n->name()));
        t->setChecked(active && (active == n || active->parent() == n));
        const bool wanted = thumbs && (!onlyActive || t->isChecked());
        if (wanted || changed || t->thumb.isNull()) {
            // limites aproximados: em página grande, a miniatura sai sem varrer a camada inteira
            const qreal dpr = qApp->devicePixelRatio();
            t->thumb = n->createThumbnail(qRound(46 * dpr), qRound(58 * dpr), Qt::KeepAspectRatio,
                                          KisThumbnailBoundsMode::Coarse);
        }
        t->update();
    }
}

// --- título: nome da página e, se ela for de um capítulo, onde ela fica -----
void refreshStudioTitle(KisMainWindow *mw)
{
    QFrame *pill = mw->findChild<QFrame*>(STUDIO_TITLE_ID);
    if (!pill || !pill->isVisible()) return;
    QLabel *name = pill->findChild<QLabel*>(QStringLiteral("nanquimStudioName"));
    QLabel *sub = pill->findChild<QLabel*>(QStringLiteral("nanquimStudioSub"));
    KisView *view = mw->activeView();
    if (!name || !sub || !view || !view->document() || !view->image()) return;
    const QString path = view->document()->path();
    QString title = path.isEmpty() ? view->document()->caption().trimmed() : displayNameFor(path);
    if (title.endsWith(QLatin1String(".kra"), Qt::CaseInsensitive)) title.chop(4);
    if (title.isEmpty()) title = i18n("Página nova");
    const ChapterStore::PageInfo info = ChapterStore::instance()->info(path);
    const QString detail = info.chapterId.isEmpty()
            ? QStringLiteral("%1 × %2 px").arg(view->image()->width()).arg(view->image()->height())
            : i18n("%1, página %2", info.chapterName, info.index + 1);
    const QFontMetrics fm(name->font());
    const QString shown = fm.elidedText(title, Qt::ElideRight, 260);
    if (name->text() != shown) name->setText(shown);
    if (sub->text() != detail) sub->setText(detail);
}

// --- páginas do capítulo, embaixo ------------------------------------------
void refreshStudioPages(KisMainWindow *mw)
{
    QFrame *nav = mw->findChild<QFrame*>(STUDIO_PAGES_ID);
    if (!nav) return;
    KisView *view = mw->activeView();
    const QString path = view && view->document() ? view->document()->path() : QString();
    const ChapterStore::PageInfo info = ChapterStore::instance()->info(path);
    const bool onCanvas = mw->property("nanquimStudioOn").toBool();
    if (info.chapterId.isEmpty() || !onCanvas) {
        nav->hide();
        return;
    }
    const QStringList pages = ChapterStore::instance()->chapter(info.chapterId).pages;
    const int i = pages.indexOf(path);
    nav->setProperty("nanquimChapter", info.chapterId);
    nav->setProperty("nanquimPrev", i > 0 ? pages.at(i - 1) : QString());
    nav->setProperty("nanquimNext", i >= 0 && i + 1 < pages.size() ? pages.at(i + 1) : QString());
    if (QLabel *label = nav->findChild<QLabel*>(QStringLiteral("nanquimStudioPage"))) {
        label->setText(i18n("Página %1 de %2", qMax(1, i + 1), qMax(1, pages.size())));
    }
    if (QToolButton *prev = nav->findChild<QToolButton*>(QStringLiteral("nanquimStudioPrev"))) prev->setEnabled(i > 0);
    if (QToolButton *next = nav->findChild<QToolButton*>(QStringLiteral("nanquimStudioNext"))) next->setEnabled(i >= 0 && i + 1 < pages.size());
    nav->adjustSize();
    nav->show();
    nav->raise();
}

void placeStudio(KisMainWindow *mw)
{
    QWidget *central = mw->centralWidget();
    if (!central || !mw->property("nanquimStudioOn").toBool()) return;
    const QMargins inset = studioInsets(mw);
    const QRect area = QRect(central->mapTo(mw, QPoint(0, 0)), central->size()).marginsRemoved(inset);
    const int m = 12;
    const bool right = railOnRight();
    int top = area.top() + m;
    int below = top;
    if (QFrame *title = mw->findChild<QFrame*>(STUDIO_TITLE_ID)) {
        title->adjustSize();
        title->move(area.left() + m, top);
        below = qMax(below, title->geometry().bottom() + 1);
    }
    if (QFrame *tools = mw->findChild<QFrame*>(STUDIO_TOOLS_ID)) {
        tools->adjustSize();
        tools->move(area.right() - m - tools->width() + 1, top);
        below = qMax(below, tools->geometry().bottom() + 1);
    }
    below += m;
    const int maxH = area.bottom() - m - below + 1;
    if (QFrame *rail = mw->findChild<QFrame*>(STUDIO_RAIL_ID)) {
        rail->setFixedWidth(76);
        rail->setFixedHeight(qMax(200, qMin(rail->layout() ? rail->layout()->sizeHint().height() : 700, maxH)));
        rail->move(right ? area.right() - m - rail->width() + 1 : area.left() + m, below);
    }
    if (QFrame *layers = mw->findChild<QFrame*>(STUDIO_LAYERS_ID)) {
        layers->setFixedWidth(76);
        layers->adjustSize();
        layers->setFixedHeight(qMin(layers->sizeHint().height(), maxH));
        layers->move(right ? area.left() + m : area.right() - m - layers->width() + 1, below);
    }
    if (QFrame *pages = mw->findChild<QFrame*>(STUDIO_PAGES_ID)) {
        pages->adjustSize();
        pages->move(area.center().x() - pages->width() / 2, area.bottom() - m - pages->height() + 1);
    }
    for (const QString &id : {STUDIO_TITLE_ID, STUDIO_TOOLS_ID, STUDIO_RAIL_ID, STUDIO_LAYERS_ID, STUDIO_PAGES_ID}) {
        if (QWidget *w = mw->findChild<QWidget*>(id)) if (w->isVisible()) w->raise();
    }
}

// entrada das peças ao chegar na tela de desenho: cada uma desliza alguns
// pixels da própria borda e aparece, uma depois da outra (40/30/30 ms)
void animateStudioIn(KisMainWindow *mw)
{
    struct Piece { QString id; QPoint offset; int delay; };
    const int railDx = railOnRight() ? 12 : -12;
    const QList<Piece> pieces = {
        {STUDIO_RAIL_ID, QPoint(railDx, 0), 0},
        {STUDIO_TITLE_ID, QPoint(0, -8), 40},
        {STUDIO_TOOLS_ID, QPoint(0, -8), 40},
        {STUDIO_LAYERS_ID, QPoint(-railDx, 0), 70},
        {STUDIO_PAGES_ID, QPoint(0, 8), 100},
    };
    for (const Piece &pc : pieces) {
        QWidget *w = mw->findChild<QWidget*>(pc.id);
        if (!w || !w->isVisible()) continue;
        const QPoint target = w->pos();
        QGraphicsOpacityEffect *fx = new QGraphicsOpacityEffect(w);
        fx->setOpacity(0.0);
        w->setGraphicsEffect(fx);
        w->move(target + pc.offset);
        QPointer<QWidget> wg(w);
        QPointer<QGraphicsOpacityEffect> fg(fx);
        const QPoint offset = pc.offset;
        QTimer::singleShot(pc.delay, w, [wg, fg, target, offset]() {
            if (!wg || !fg) return;
            QVariantAnimation *a = new QVariantAnimation(wg.data());
            a->setStartValue(0.0);
            a->setEndValue(1.0);
            a->setDuration(200);
            a->setEasingCurve(QEasingCurve::OutQuint);
            QObject::connect(a, &QVariantAnimation::valueChanged, wg.data(), [wg, fg, target, offset](const QVariant &v) {
                const qreal t = v.toReal();
                if (fg) fg->setOpacity(t);
                if (wg) wg->move(target + QPoint(qRound(offset.x() * (1 - t)), qRound(offset.y() * (1 - t))));
            });
            QObject::connect(a, &QVariantAnimation::finished, wg.data(), [wg, fg, target]() {
                QTimer::singleShot(0, wg.data(), [wg, fg, target]() {
                    if (!wg) return;
                    wg->move(target);
                    if (fg && wg->graphicsEffect() == fg.data()) wg->setGraphicsEffect(nullptr);
                });
            });
            a->start(QAbstractAnimation::DeleteWhenStopped);
        });
    }
}

void setStudioVisible(KisMainWindow *mw, bool on)
{
    if (!mw || !mw->findChild<QFrame*>(STUDIO_RAIL_ID)) return;
    const bool was = mw->property("nanquimStudioOn").toBool();
    mw->setProperty("nanquimStudioOn", on);
    for (const QString &id : {STUDIO_TITLE_ID, STUDIO_TOOLS_ID, STUDIO_RAIL_ID, STUDIO_LAYERS_ID}) {
        if (QWidget *w = mw->findChild<QWidget*>(id)) w->setVisible(on);
    }
    if (!on) {
        if (QWidget *pages = mw->findChild<QWidget*>(STUDIO_PAGES_ID)) pages->hide();
        return;
    }
    // rastro no log de uso: se o app cair aqui, a última linha diz em que passo
    KisUsageLogger::log(QStringLiteral("[Nanquim] estúdio: título"));
    refreshStudioTitle(mw);
    KisUsageLogger::log(QStringLiteral("[Nanquim] estúdio: pincéis"));
    refreshStudioBrushes(mw);
    KisUsageLogger::log(QStringLiteral("[Nanquim] estúdio: páginas"));
    refreshStudioPages(mw);
    KisUsageLogger::log(QStringLiteral("[Nanquim] estúdio: posição"));
    flushLayouts();
    placeStudio(mw);
    KisUsageLogger::log(QStringLiteral("[Nanquim] estúdio: camadas"));
    refreshStudioLayers(mw, !was);
    KisUsageLogger::log(QStringLiteral("[Nanquim] estúdio: entrada"));
    if (!was) animateStudioIn(mw);
    KisUsageLogger::log(QStringLiteral("[Nanquim] estúdio: pronto"));
}

// as barras do Krita (abas dos documentos e barra de status): somem no estúdio
// e também no modo foco para deixar a página 100% limpa de borda a borda
void setKritaChromeHidden(KisMainWindow *mw, bool hidden)
{
    if (!mw) return;
    mw->setProperty("nanquimHideChrome", hidden);
    QPointer<KisMainWindow> mwGuard(mw);
    auto watchAndSet = [mwGuard, hidden](QWidget *w) {
        if (!w) return;
        if (!w->property("nanquimChromeWatched").toBool()) {
            w->setProperty("nanquimChromeWatched", true);
            w->installEventFilter(new EventFn(w, [mwGuard](QObject *o, QEvent *e) {
                if (e->type() == QEvent::Show && !isClassicMode() && mwGuard
                        && (NANQUIM_ESTUDIO || mwGuard->property("nanquimHideChrome").toBool())) {
                    QPointer<QWidget> wg(static_cast<QWidget*>(o));
                    QTimer::singleShot(0, wg.data(), [wg]() { if (wg) wg->hide(); });
                }
                return false;
            }));
        }
        w->setVisible(!hidden);
    };
    watchAndSet(mw->statusBar());
    if (QWidget *central = mw->centralWidget()) {
        Q_FOREACH (QTabBar *tabs, central->findChildren<QTabBar*>()) {
            watchAndSet(tabs);
        }
    }
}

void hideKritaChrome(KisMainWindow *mw)
{
    setKritaChromeHidden(mw, true);
}

// mantém as peças no lugar quando a área de desenho muda (rotação, janela dividida)
class StudioWatcher : public QObject
{
public:
    explicit StudioWatcher(KisMainWindow *mw) : QObject(mw), m_mw(mw) {}
protected:
    bool eventFilter(QObject *o, QEvent *e) override
    {
        if (e->type() == QEvent::Resize || e->type() == QEvent::Move || e->type() == QEvent::LayoutRequest) {
            if (m_mw && (o == m_mw || o == m_mw->centralWidget())) {
                QPointer<KisMainWindow> guard(m_mw);
                QTimer::singleShot(0, m_mw, [guard]() { if (guard) placeStudio(guard); });
            }
        }
        return false;
    }
private:
    QPointer<KisMainWindow> m_mw;
};

void buildStudio(KisMainWindow *mw)
{
    QPointer<KisMainWindow> guard(mw);
    KisViewManager *vm = mw->viewManager();

    // --- título ---------------------------------------------------------------
    QFrame *title = studioPiece(mw, STUDIO_TITLE_ID);
    {
        QHBoxLayout *l = new QHBoxLayout(title);
        l->setContentsMargins(4, 4, 16, 4);
        l->setSpacing(4);
        QToolButton *back = studioButton(title, i18n("Voltar para a página inicial"), Svg::BACK);
        QObject::connect(back, &QToolButton::clicked, mw, [guard]() { if (guard) showGalleryAnimated(guard); });
        l->addWidget(back);
        // o título abre a faixa com as páginas do capítulo
        QToolButton *box = new QToolButton(title);
        box->setFocusPolicy(Qt::NoFocus);
        box->setAutoRaise(true);
        box->setAccessibleName(i18n("Páginas do capítulo"));
        box->setMinimumHeight(48);
        QVBoxLayout *tl = new QVBoxLayout(box);
        tl->setContentsMargins(6, 4, 6, 4);
        tl->setSpacing(1);
        QLabel *name = new QLabel(box);
        name->setObjectName(QStringLiteral("nanquimStudioName"));
        name->setAttribute(Qt::WA_TransparentForMouseEvents);
        QLabel *sub = new QLabel(box);
        sub->setObjectName(QStringLiteral("nanquimStudioSub"));
        sub->setAttribute(Qt::WA_TransparentForMouseEvents);
        tl->addWidget(name);
        tl->addWidget(sub);
        QObject::connect(box, &QToolButton::clicked, mw, [guard]() { if (guard) toggleFilmstripDrawer(guard); });
        l->addWidget(box);
        QTimer *timer = new QTimer(title);
        timer->setInterval(800);
        QObject::connect(timer, &QTimer::timeout, title, [guard]() { if (guard) refreshStudioTitle(guard); });
        timer->start();
    }

    // --- ferramentas ----------------------------------------------------------
    QFrame *tools = studioPiece(mw, STUDIO_TOOLS_ID);
    {
        QHBoxLayout *l = new QHBoxLayout(tools);
        l->setContentsMargins(4, 4, 4, 4);
        l->setSpacing(2);
        const QList<ToolEntry> entries = {
            {"Pincel", Svg::PEN, "KritaShape/KisToolBrush", false},
            {"Borracha", Svg::ERASER, "KritaShape/KisToolBrush", true},
            {"Balde", Svg::BUCKET, "KritaFill/KisToolFill", false},
            {"Seleção", Svg::SELECT, "KisToolSelectRectangular", false},
            {"Transformar", Svg::TRANSFORM, "KisToolTransform", false},
        };
        QAction *erase = act(mw, "erase_action");
        KoToolManager *toolManager = KoToolManager::instance();
        QList<QPair<QToolButton*, ToolEntry>> buttons;
        Q_FOREACH (const ToolEntry &t, entries) {
            QAction *toolAction = act(mw, t.toolId);
            if (!toolAction || (t.eraser && !erase)) continue;
            QToolButton *b = studioButton(tools, i18n(t.tip), t.svg, true);
            QObject::connect(b, &QToolButton::clicked, mw, [toolAction, erase, t]() {
                toolAction->trigger();
                if (erase && QLatin1String(t.toolId) == QLatin1String("KritaShape/KisToolBrush")
                        && erase->isChecked() != t.eraser) {
                    erase->trigger();
                }
            });
            l->addWidget(b);
            buttons << qMakePair(b, t);
        }
        auto sync = [buttons, erase, toolManager]() {
            const QString active = toolManager->activeToolId();
            const bool erasing = erase && erase->isChecked();
            for (const auto &pair : buttons) {
                const bool isBrush = QLatin1String(pair.second.toolId) == QLatin1String("KritaShape/KisToolBrush");
                pair.first->setChecked(active == QLatin1String(pair.second.toolId) && (!isBrush || pair.second.eraser == erasing));
            }
        };
        QObject::connect(toolManager, &KoToolManager::changedTool, tools, [sync](KoCanvasController *) { sync(); });
        if (erase) QObject::connect(erase, &QAction::toggled, tools, [sync](bool) { sync(); });
        QTimer::singleShot(0, tools, sync);

        QToolButton *balloon = studioButton(tools, i18n("Balões de fala"), Svg::BALLOON);
        QObject::connect(balloon, &QToolButton::clicked, mw, [guard]() { if (guard) showComicsSheet(guard, 1); });
        l->addWidget(balloon);
        QToolButton *frames = studioButton(tools, i18n("Requadros"), Svg::PANELS);
        QObject::connect(frames, &QToolButton::clicked, mw, [guard]() { if (guard) showComicsSheet(guard, 0); });
        l->addWidget(frames);
        l->addSpacing(4);
        l->addWidget(studioDivider(tools, true), 0, Qt::AlignVCenter);
        l->addSpacing(4);

        if (QDockWidget *layersDock = findDock(mw, {QStringLiteral("KisLayerBox")})) {
            QToolButton *b = studioButton(tools, i18n("Camadas"), Svg::LAYERS, true);
            b->setObjectName(QStringLiteral("nanquimStudioLayersButton"));
            b->setProperty("nanquimPopup", QStringLiteral("below"));
            QObject::connect(b, &QToolButton::clicked, mw, [guard, layersDock, b]() {
                if (!guard) return;
                togglePanelFrom(guard, layersDock, b);
                b->setChecked(panelOpen(guard, layersDock));
            });
            QObject::connect(layersDock->toggleViewAction(), &QAction::toggled, b, [b](bool on) { if (!on) b->setChecked(false); });
            l->addWidget(b);
        }

        // Mais: o que não cabe na cápsula, mais o menu completo do Krita
        QToolButton *more = studioButton(tools, i18n("Mais opções"), Svg::MORE);
        more->setProperty("nanquimPopup", QStringLiteral("below"));
        QMenu *menu = new QMenu(more);
        QObject::connect(menu, &QMenu::aboutToShow, mw, [guard, menu, more]() {
            if (!guard) return;
            KisMainWindow *w = guard;
            menu->clear();
            menu->addAction(designIcon(Svg::ALL_TOOLS, 18), i18n("Todas as ferramentas"), w, [w]() { setFlyoutVisible(w, true); });
            if (QAction *text = act(w, "SvgTextTool")) {
                menu->addAction(designIcon(Svg::TEXT, 18), i18n("Texto"), w, [text]() { text->trigger(); });
            }
            if (QDockWidget *options = findDock(w, {QStringLiteral("sharedtooldocker")})) {
                menu->addAction(designIcon(Svg::OPTIONS, 18), i18n("Opções da ferramenta"), w, [w, options, more]() {
                    togglePanelFrom(w, options, more);
                });
            }
            menu->addAction(designIcon(Svg::BRUSH, 18), i18n("Ajustes do pincel"), w, [w]() { showBrushSettings(w); });
            QMenu *rulers = menu->addMenu(designIcon(Svg::RULER, 18), i18n("Quadrinhos e réguas"));
            populateRulersMenu(w, rulers, more);
            if (act(w, "import_layer_as_paint_layer")) {
                menu->addAction(designIcon(Svg::INSERT_IMAGE, 18), i18n("Inserir imagem como camada"), w, [w]() { insertImageAsLayer(w); });
            }
            if (QAction *ref = act(w, "ToolReferenceImages")) {
                menu->addAction(designIcon(Svg::REFERENCE, 18), i18n("Imagem de referência"), w, [ref]() { ref->trigger(); });
            }
            menu->addAction(designIcon(Svg::EXPORT, 18), i18n("Exportar"), w, [w]() { showExport(w); });
            if (QAction *full = act(w, "view_show_canvas_only")) {
                menu->addAction(i18n("Só a página (quatro dedos)"), w, [full]() { full->trigger(); });
            }
            menu->addSeparator();
            QMenu *krita = menu->addMenu(i18n("Menu do Krita"));
            Q_FOREACH (QAction *a, w->menuBar()->actions()) krita->addAction(a);
        });
        more->setMenu(menu);
        more->setPopupMode(QToolButton::InstantPopup);
        l->addWidget(more);
    }

    // --- coluna: cor, pincéis, tamanho, opacidade, desfazer -----------------
    QFrame *rail = studioPiece(mw, STUDIO_RAIL_ID);
    {
        QVBoxLayout *l = new QVBoxLayout(rail);
        l->setContentsMargins(10, 12, 10, 12);
        l->setSpacing(8);
        StudioColorButton *color = new StudioColorButton(mw, rail);
        color->setObjectName(QStringLiteral("nanquimStudioColor"));
        color->setProperty("nanquimPopup", QStringLiteral("rail"));
        QObject::connect(color, &QAbstractButton::clicked, mw, [guard, color]() {
            if (!guard) return;
            if (QDockWidget *dw = findDock(guard, {COLOR_DOCK_ID, QStringLiteral("ColorSelectorNg")})) togglePanelFrom(guard, dw, color);
        });
        if (vm && vm->canvasResourceProvider()) {
            QObject::connect(vm->canvasResourceProvider(), &KisCanvasResourceProvider::sigFGColorChanged, color,
                             [color](const KoColor &) { color->update(); });
            QObject::connect(vm->canvasResourceProvider(), &KisCanvasResourceProvider::sigBGColorChanged, color,
                             [color](const KoColor &) { color->update(); });
        }
        l->addWidget(color, 0, Qt::AlignHCenter);
        l->addWidget(studioDivider(rail, false), 0, Qt::AlignHCenter);

        for (int i = 0; i < STUDIO_SLOTS; i++) {
            StudioBrushSlot *slot = new StudioBrushSlot(rail);
            slot->setProperty("nanquimPopup", QStringLiteral("rail"));
            QObject::connect(slot, &QAbstractButton::clicked, mw, [guard, slot]() {
                if (!guard) return;
                KisViewManager *v = guard->viewManager();
                KisPaintOpPresetSP cur = v && v->canvasResourceProvider() ? v->canvasResourceProvider()->currentPreset() : KisPaintOpPresetSP();
                const bool active = cur && slot->preset() && slot->preset()->resourceId() == cur->resourceId();
                if (active) {
                    // tocar no pincel ativo abre a lista de pincéis ao lado da coluna
                    QFrame *panel = guard->findChild<QFrame*>(BRUSH_PANEL_ID);
                    guard->setProperty("nanquimBrushAnchorY", slot->mapTo(guard, QPoint(0, 0)).y());
                    setBrushPanelVisible(guard, !(panel && panel->isVisible()));
                    slot->setChecked(true);
                    return;
                }
                if (slot->preset() && v && v->canvasResourceProvider()) {
                    v->canvasResourceProvider()->setPaintOpPreset(slot->preset());
                }
                refreshStudioBrushes(guard);
            });
            l->addWidget(slot, 0, Qt::AlignHCenter);
        }
        l->addWidget(studioDivider(rail, false), 0, Qt::AlignHCenter);

        auto sliderBox = [rail](QSlider *s, QLabel *label, int minH, int stretch, QVBoxLayout *into) {
            label->setObjectName(QStringLiteral("nanquimStudioValue"));
            label->setAlignment(Qt::AlignHCenter);
            s->setMinimumHeight(minH);
            s->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
            into->addWidget(label, 0, Qt::AlignHCenter);
            into->addWidget(s, stretch, Qt::AlignHCenter);
            Q_UNUSED(rail);
        };
        QSlider *size = new QSlider(Qt::Vertical, rail);
        size->setRange(0, 1000);
        size->setAccessibleName(i18n("Tamanho do pincel"));
        QLabel *sizeLabel = new QLabel(QStringLiteral("—"), rail);
        QSlider *opacity = new QSlider(Qt::Vertical, rail);
        opacity->setRange(0, 100);
        opacity->setAccessibleName(i18n("Opacidade do pincel"));
        QLabel *opacityLabel = new QLabel(QStringLiteral("100%"), rail);
        sliderBox(size, sizeLabel, 64, 3, l);
        sliderBox(opacity, opacityLabel, 48, 2, l);
        QObject::connect(size, &QSlider::valueChanged, mw, [guard, sizeLabel](int v) {
            if (!guard) return;
            const qreal px = sliderToSize(v);
            sizeLabel->setText(QString::number(qRound(px)));
            if (KoCanvasResourceProvider *r = resources(guard)) r->setResource(KoCanvasResource::Size, px);
        });
        QObject::connect(opacity, &QSlider::valueChanged, mw, [guard, opacityLabel](int v) {
            if (!guard) return;
            opacityLabel->setText(QStringLiteral("%1%").arg(v));
            if (KoCanvasResourceProvider *r = resources(guard)) r->setResource(KoCanvasResource::Opacity, v / 100.0);
        });
        auto pull = [guard, size, sizeLabel, opacity, opacityLabel]() {
            KoCanvasResourceProvider *r = guard ? resources(guard) : nullptr;
            if (!r) return;
            const QVariant s = r->resource(KoCanvasResource::Size);
            if (s.isValid()) {
                QSignalBlocker block(size);
                size->setValue(sizeToSlider(s.toReal()));
                sizeLabel->setText(QString::number(qRound(s.toReal())));
            }
            const QVariant o = r->resource(KoCanvasResource::Opacity);
            if (o.isValid()) {
                QSignalBlocker block(opacity);
                opacity->setValue(qRound(o.toReal() * 100));
                opacityLabel->setText(QStringLiteral("%1%").arg(qRound(o.toReal() * 100)));
            }
        };
        if (KoCanvasResourceProvider *r = resources(mw)) {
            QObject::connect(r, &KoCanvasResourceProvider::canvasResourceChanged, rail, [guard, pull](int key, const QVariant &) {
                if (key == KoCanvasResource::Size || key == KoCanvasResource::Opacity
                        || key == KoCanvasResource::CurrentPaintOpPreset) {
                    pull();
                }
                if (key == KoCanvasResource::CurrentPaintOpPreset && guard) refreshStudioBrushes(guard);
            });
        }
        QObject::connect(KoToolManager::instance(), &KoToolManager::changedTool, rail, [pull](KoCanvasController *) { pull(); });
        QTimer::singleShot(0, rail, pull);

        l->addWidget(studioDivider(rail, false), 0, Qt::AlignHCenter);
        for (const char *name : {"edit_undo", "edit_redo"}) {
            QAction *a = act(mw, name);
            const bool undo = QLatin1String(name) == QLatin1String("edit_undo");
            QToolButton *b = studioButton(rail, undo ? i18n("Desfazer") : i18n("Refazer"), undo ? Svg::UNDO : Svg::REDO);
            if (a) {
                QObject::connect(b, &QToolButton::clicked, a, &QAction::trigger);
                b->setEnabled(a->isEnabled());
                QObject::connect(a, &QAction::changed, b, [a, b]() { b->setEnabled(a->isEnabled()); });
            }
            l->addWidget(b, 0, Qt::AlignHCenter);
        }
    }

    // --- camadas -------------------------------------------------------------
    QFrame *layers = studioPiece(mw, STUDIO_LAYERS_ID);
    {
        QVBoxLayout *l = new QVBoxLayout(layers);
        l->setContentsMargins(12, 14, 12, 12);
        l->setSpacing(10);
        QDockWidget *layersDock = findDock(mw, {QStringLiteral("KisLayerBox")});
        for (int i = 0; i < STUDIO_LAYER_MAX; i++) {
            StudioLayerThumb *t = new StudioLayerThumb(layers);
            t->setProperty("nanquimPopup", QStringLiteral("strip"));
            QObject::connect(t, &QAbstractButton::clicked, mw, [guard, t, layersDock]() {
                if (!guard || !t->node) return;
                KisViewManager *v = guard->viewManager();
                const bool active = v && v->activeNode() == t->node;
                if (active && layersDock) {
                    // a camada ativa abre o painel de camadas, ao lado da faixa
                    togglePanelFrom(guard, layersDock, t);
                } else if (v && v->nodeManager()) {
                    v->nodeManager()->slotNonUiActivatedNode(t->node);
                }
                refreshStudioLayers(guard, false);
            });
            l->addWidget(t, 0, Qt::AlignHCenter);
        }
        QToolButton *add = studioButton(layers, i18n("Nova camada"), "<path d='M12 5v14M5 12h14'/>", false, 44);
        if (QAction *a = act(mw, "add_new_paint_layer")) {
            QObject::connect(add, &QToolButton::clicked, a, &QAction::trigger);
        }
        l->addWidget(add, 0, Qt::AlignHCenter);
        l->addStretch(1);

        // miniaturas: na troca de camada na hora; o desenho, um pouco depois do traço
        QTimer *later = new QTimer(layers);
        later->setSingleShot(true);
        later->setInterval(900);
        // depois de um traço só a camada ativa mudou: só ela é refeita
        QObject::connect(later, &QTimer::timeout, layers, [guard]() { if (guard) refreshStudioLayers(guard, true, true); });
        auto connectImage = [guard, later, layers]() {
            if (!guard) return;
            KisView *view = guard->activeView();
            if (!view || !view->image()) return;
            KisImageSP image = view->image();
            if (image->property("nanquimStudioHooked").toBool()) return;
            image->setProperty("nanquimStudioHooked", true);
            // espera a mão parar (0,9 s sem mudança) para não pesar durante o desenho
            QObject::connect(image.data(), &KisImage::sigImageUpdated, layers, [later](const QRect &) {
                later->start();
            });
            QObject::connect(image.data(), &KisImage::sigLayersChangedAsync, layers, [guard]() {
                if (guard) refreshStudioLayers(guard, true);
            });
        };
        QObject::connect(mw, &KisMainWindow::activeViewChanged, layers, [guard, connectImage]() {
            connectImage();
            if (guard) {
                refreshStudioLayers(guard, true);
                refreshStudioTitle(guard);
                refreshStudioPages(guard);
            }
        });
        if (vm->nodeManager()) {
            QObject::connect(vm->nodeManager(), &KisNodeManager::sigNodeActivated, layers, [guard](KisNodeSP) {
                if (guard) refreshStudioLayers(guard, false);
            });
        }
    }

    // --- páginas do capítulo -------------------------------------------------
    QFrame *pages = studioPiece(mw, STUDIO_PAGES_ID);
    {
        QHBoxLayout *l = new QHBoxLayout(pages);
        l->setContentsMargins(4, 4, 4, 4);
        l->setSpacing(4);
        QToolButton *prev = studioButton(pages, i18n("Página anterior"), Svg::BACK);
        prev->setObjectName(QStringLiteral("nanquimStudioPrev"));
        QLabel *label = new QLabel(pages);
        label->setObjectName(QStringLiteral("nanquimStudioPage"));
        QToolButton *next = studioButton(pages, i18n("Próxima página"), Svg::FORWARD);
        next->setObjectName(QStringLiteral("nanquimStudioNext"));
        QToolButton *add = studioButton(pages, i18n("Adicionar página ao capítulo"), "<path d='M12 5v14M5 12h14'/>");
        l->addWidget(prev);
        l->addWidget(label);
        l->addWidget(next);
        l->addSpacing(2);
        l->addWidget(studioDivider(pages, true), 0, Qt::AlignVCenter);
        l->addSpacing(2);
        l->addWidget(add);
        QObject::connect(prev, &QToolButton::clicked, mw, [guard, pages]() {
            const QString p = pages->property("nanquimPrev").toString();
            if (guard && !p.isEmpty()) openPageWithFeedback(guard, p);
        });
        QObject::connect(next, &QToolButton::clicked, mw, [guard, pages]() {
            const QString p = pages->property("nanquimNext").toString();
            if (guard && !p.isEmpty()) openPageWithFeedback(guard, p);
        });
        QObject::connect(add, &QToolButton::clicked, mw, [pages]() {
            const QString id = pages->property("nanquimChapter").toString();
            if (id.isEmpty()) return;
            s_pendingChapter = id;
            createPage(CHAPTER_FORMATS[ChapterStore::instance()->format(id)]);
        });
        ChapterStore::instance()->listeners << [guard]() { if (guard) refreshStudioPages(guard); };
    }

    mw->installEventFilter(new StudioWatcher(mw));
    if (QWidget *central = mw->centralWidget()) central->installEventFilter(new StudioWatcher(mw));
}

} // namespace

void NanquimTabletUi::install(KisMainWindow *mw)
{
    s_classic = isClassicMode();
    s_recorder = new NanquimTimelapseRecorder(mw);
    {
        QPointer<KisMainWindow> areaGuard(mw);
        QObject::connect(KisPart::instance(), &KisPart::sigViewAdded, mw, [areaGuard](KisView *) {
            QTimer::singleShot(600, areaGuard.data(), [areaGuard]() { rememberCanvasArea(areaGuard); });
        });
    }
    // blocos de textura de 512 px (patch 0075) pesavam no traço fino com estabilizador;
    // quem ficou com 512 guardado volta para o padrão (256), uma vez só
    {
        KConfigGroup nq = KSharedConfig::openConfig()->group("Nanquim");
        if (!nq.readEntry("textureSize256", false)) {
            KConfigGroup general = KSharedConfig::openConfig()->group("");
            if (general.readEntry("textureSize", 0) == 512) general.deleteEntry("textureSize");
            nq.writeEntry("textureSize256", true);
            KSharedConfig::openConfig()->sync();
        }
    }
    // 256 px travou a abertura do canvas OpenGL no Android desde a build 95;
    // garante volta ao padrão 512 px para quem ficou com 256 gravado no kritarc
    {
        KConfigGroup nq = KSharedConfig::openConfig()->group("Nanquim");
        if (!nq.readEntry("textureSize512Restore", false)) {
            KConfigGroup general = KSharedConfig::openConfig()->group("");
            if (general.readEntry("textureSize", 0) == 256) general.deleteEntry("textureSize");
            nq.writeEntry("textureSize512Restore", true);
            KSharedConfig::openConfig()->sync();
        }
    }
    // tela como no Krita estável: blocos de 256 px e escala em alta qualidade. Os vídeos de
    // 27/11 mostraram o traço com estabilizador chegando aos trancos com 512 px + trilinear.
    // Uma vez só; depois vale o que a pessoa escolher em Configurações > Desempenho.
    {
        KConfigGroup nq = KSharedConfig::openConfig()->group("Nanquim");
        if (!nq.readEntry("displayLikeStable", false)) {
            KConfigGroup general = KSharedConfig::openConfig()->group("");
            general.writeEntry("textureSize", 256);
            general.writeEntry("OpenGLFilterMode", 3);
            nq.writeEntry("displayLikeStable", true);
            KSharedConfig::openConfig()->sync();
        }
    }
    if (!s_classic) {
        buildColorDock(mw);   // antes da barra: o botão de cor abre este painel
        buildTopBar(mw);
        buildToolNav(mw);
        buildToolsFlyout(mw);
        // redesenho: o estúdio (coluna, cápsulas, camadas e páginas) substitui o
        // dock do polegar, as 3 cores e a cápsula da tela cheia
        if (NANQUIM_ESTUDIO) buildStudio(mw);
        buildFocusWidgets(mw);
        buildSelectionBar(mw);
        if (QAction *canvasOnly = act(mw, "view_show_canvas_only")) {
            QPointer<KisMainWindow> guard(mw);
            // quatro dedos ou botão de foco: guarda as barras atuais e traz a barra
            // unificada do modo foco deslizando pela borda (ou devolve ao sair)
            QObject::connect(canvasOnly, &QAction::toggled, mw, [guard](bool on) {
                if (!guard || !guard->property("nanquimOnCanvas").toBool()) return;
                setCanvasDim(guard, on);
                if (!NANQUIM_ESTUDIO) {
                    if (on) {
                        if (QDockWidget *dw = hostedDock(guard)) closePanel(guard, dw, false);
                        setBrushPanelVisible(guard, false);
                        if (QFrame *flyout = guard->findChild<QFrame*>(FLYOUT_ID)) flyout->hide();
                        if (QFrame *strip = guard->findChild<QFrame*>(FILMSTRIP_ID)) strip->hide();
                    }
                    setKritaChromeHidden(guard, on);
                    animateChrome(guard, on);
                    showFocusBarAnimated(guard, on, true);
                    for (int ms : {120, 400}) {   // acompanha os ajustes de janela do Krita
                        QTimer::singleShot(ms, guard, [guard, on]() {
                            if (!guard || !guard->property("nanquimOnCanvas").toBool()) return;
                            if (canvasOnlyActive(guard) == on) {
                                setKritaChromeHidden(guard, on);
                                placeFocusWidgets(guard);
                                if (on) refreshFocusBarInfo(guard);
                            }
                        });
                    }
                    return;
                }
                for (int ms : {0, 120, 400}) {   // o Krita reorganiza a janela em etapas
                    QTimer::singleShot(ms, guard, [guard, on]() {
                        if (!guard || !guard->property("nanquimOnCanvas").toBool()) return;
                        if (!NANQUIM_ESTUDIO) return;
                        hideKritaChrome(guard);
                        setStudioVisible(guard, !on);
                    });
                }
            });
        }
        // pílula de salvamento: "Salvando…" ao pedir para salvar
        for (const char *name : {"file_save", "file_save_as"}) {
            if (QAction *save = act(mw, name)) {
                QPointer<KisMainWindow> guard(mw);
                QObject::connect(save, &QAction::triggered, mw, [guard]() {
                    if (!guard || !guard->activeView() || !guard->activeView()->document()) return;
                    const QString path = guard->activeView()->document()->path();
                    showSaveState(guard, 0, i18n("Salvando…"),
                                  path.isEmpty() ? i18n("Página nova") : QFileInfo(path).completeBaseName());
                });
            }
        }
        installPanelBehavior(mw);
        installStabilizerPerBrush(mw);
        qApp->installEventFilter(new WheelCloser(mw));
        trackRecentBrushes(mw);
    } else {
        buildBrushDock(mw);
    }
    // a janela de pincéis existe nos dois modos (no Clássico, abre pelo "+ Pincéis" da coluna)
    buildBrushPanel(mw);
    if (s_classic) qApp->installEventFilter(new BrushPanelCloser(mw));   // no Tablet, o PanelCloser cuida
    installGallery(mw);
    buildNanquimMenu(mw);
    qApp->installEventFilter(new PopupAnimator(mw));
    qApp->installEventFilter(new MessageStyler(mw));
#ifdef Q_OS_ANDROID
    // perguntas e avisos desenhados pelo Qt, com o estilo do Nanquim; os seletores
    // de arquivo continuam os do Android (o KoFileDialog libera só para eles)
    QCoreApplication::setAttribute(Qt::AA_DontUseNativeDialogs, true);
    // o carregador Java do Qt liga QT_USE_ANDROID_NATIVE_DIALOGS=1, e é essa
    // variável que o tema do Android consulta antes de cada QMessageBox
    qputenv("QT_USE_ANDROID_NATIVE_DIALOGS", "0");
#endif
    {
        // dicas (o quadro amarelo de memória, por exemplo) no escuro do Nanquim
        QPalette tip = QToolTip::palette();
        tip.setColor(QPalette::ToolTipBase, QColor(0x2e, 0x2d, 0x29));
        tip.setColor(QPalette::ToolTipText, QColor(0xee, 0xea, 0xe0));
        tip.setColor(QPalette::Window, QColor(0x2e, 0x2d, 0x29));
        tip.setColor(QPalette::WindowText, QColor(0xee, 0xea, 0xe0));
        QToolTip::setPalette(tip);
    }

    // a cada desenho aberto: aviso de memória (páginas enormes) e "página salva"
    QPointer<KisMainWindow> mwGuard(mw);
    QObject::connect(KisPart::instance(), &KisPart::sigViewAdded, mw, [mwGuard](KisView *view) {
        if (!mwGuard || !view) return;
        QPointer<KisView> viewGuard(view);
        QTimer::singleShot(1500, mwGuard, [mwGuard, viewGuard]() {
            if (mwGuard && viewGuard && mwGuard->activeView() == viewGuard) showMemoryWarning(mwGuard, viewGuard);
        });
        KisDocument *doc = view->document();
        if (!doc || doc->property("nanquimToast").toBool()) return;
        doc->setProperty("nanquimToast", true);
        QObject::connect(doc, &KisDocument::sigSavingFinished, mwGuard, [mwGuard](const QString &path) {
            if (!mwGuard || path.contains(QLatin1String("autosave")) || path.contains(QLatin1String("krita-backup"))) return;
            if (path.contains(QLatin1String("/export/"))) return;   // cópia da exportação, não a página
            const QString name = QFileInfo(path).completeBaseName();
            showSaveState(mwGuard, 1, i18n("Salvo"), name);
        });
        // falha no salvamento em segundo plano (o salvar normal do Krita)
        QObject::connect(doc, &KisDocument::sigCompleteBackgroundSaving, mwGuard,
                         [mwGuard](const KritaUtils::ExportFileJob &job, KisImportExportErrorCode status,
                                   const QString &errorMessage, const QString &) {
            if (!mwGuard || (job.flags & KritaUtils::SaveInAutosaveMode)) return;
            if (status.isOk() || status.isCancelled()) return;
            QString why = errorMessage.isEmpty() ? status.errorMessage() : errorMessage;
            why = why.section(QLatin1Char('\n'), 0, 0).trimmed();
            if (why.isEmpty()) why = i18n("veja se há espaço livre no tablet");
            showSaveState(mwGuard, 2, i18n("Não salvou"), why);
        });
    });

#ifdef Q_OS_ANDROID
    // pendência 4: o dedo não desenha por padrão (só move, gira e dá zoom).
    // Vale uma vez; quem ligar o dedo depois em Configurações continua com ele.
    {
        KConfigGroup c = KSharedConfig::openConfig()->group("Nanquim");
        if (!c.readEntry("penOnlyDefaultV1", false)) {
            KisConfig cfg(false);
            if (cfg.touchPainting() == KisConfig::TOUCH_PAINTING_AUTO) {
                cfg.setTouchPainting(KisConfig::TOUCH_PAINTING_DISABLED);
            }
            c.writeEntry("penOnlyDefaultV1", true);
        }
    }
#endif

    // paleta do redesenho: quem tinha o cinza-azulado antigo em volta da página
    // (gravado pela tela cheia) passa para o grafite novo. Uma vez só.
    {
        KConfigGroup c = KSharedConfig::openConfig()->group("Nanquim");
        if (!c.readEntry("paletteVerdeteV1", false)) {
            KisConfig cfg(false);
            if (cfg.canvasBorderColor() == QColor(43, 43, 49)) cfg.setCanvasBorderColor(QColor(36, 35, 31));
            c.writeEntry("paletteVerdeteV1", true);
        }
    }

    // camadas com miniaturas maiores (uma vez; o tamanho vale ao abrir de novo)
    {
        KConfigGroup c = KSharedConfig::openConfig()->group("Nanquim");
        if (!c.readEntry("layerThumbsV1", false)) {
            KisConfig cfg(false);
            if (cfg.layerThumbnailSize() <= 20) cfg.setLayerThumbnailSize(36);
            c.writeEntry("layerThumbsV1", true);
        }
    }

    // depois que tudo carregou: etiqueta de pincéis (uma vez) e boas-vindas (primeira abertura)
    QPointer<KisMainWindow> guard(mw);
    QTimer::singleShot(2500, mw, [guard]() {
        if (!guard) return;
        setupNanquimBrushes(guard);
        seedNanquimFavorites(guard);
        if (!KSharedConfig::openConfig()->group("Nanquim").readEntry("welcomeShownV1", false)) {
            showWelcome(guard, false);
        }
    });
    KisUsageLogger::log(QStringLiteral("[Nanquim] interface instalada (modo %1)")
                        .arg(s_classic ? QStringLiteral("Clássico") : QStringLiteral("Tablet")));
}

bool NanquimTabletUi::toggleWheel(QWidget *canvasWidget, const QPoint &pos)
{
    if (!canvasWidget) return false;
    KisMainWindow *mw = qobject_cast<KisMainWindow*>(canvasWidget->window());
    if (!mw) return false;
    return openWheel(mw, canvasWidget->mapTo(mw, pos));
}

void NanquimTabletUi::applyLayout(KisMainWindow *mw, bool canvasMode)
{
    const bool hasGallery = mw->findChild<QWidget*>(GALLERY_ID) != nullptr;
    QToolBar *top = mw->findChild<QToolBar*>(TOP_ID);
    QToolBar *nav = mw->findChild<QToolBar*>(NAV_ID);
    if (!hasGallery && !top) {
        return;   // install() não rodou: deixa o Krita como está
    }

    static int s_lastMode = -1;
    const int mode = canvasMode ? 1 : 0;
    const bool screenChanged = s_lastMode != -1 && s_lastMode != mode;
    s_lastMode = mode;

    const bool hasDrawing = mw->activeView() != nullptr;
    updateSafeTop(mw);
    if (QWidget *back = mw->findChild<QWidget*>(QStringLiteral("nanquimBackToDrawing"))) {
        back->setVisible(hasDrawing);
    }

    if (s_classic) {
        // sem a barra de menus não haveria como voltar ao modo Tablet
        mw->menuBar()->setVisible(true);
        if (!canvasMode) {
            if (QFrame *brushPanel = mw->findChild<QFrame*>(BRUSH_PANEL_ID)) brushPanel->hide();
        }
        KConfigGroup cfg = KSharedConfig::openConfig()->group("Nanquim");
        if (canvasMode) {
            // O Krita restaura por cima o estado salvo no modo Tablet (tudo escondido).
            // Por isso o arranjo é aplicado depois que a troca de tela termina, e de
            // novo sempre que a tela de desenho aparece sem nenhum painel principal.
            QPointer<KisMainWindow> guard(mw);
            QTimer::singleShot(150, mw, [guard]() {
                if (!guard) return;
                KConfigGroup c = KSharedConfig::openConfig()->group("Nanquim");
                bool anyVisible = false;
                for (const QString &id : {TOOLBOX_ID, QStringLiteral("KisLayerBox"),
                                          QStringLiteral("ColorSelectorNg"), QStringLiteral("sharedtooldocker")}) {
                    if (QDockWidget *dw = guard->dockWidget(id)) anyVisible |= dw->isVisible();
                }
                bool anyBar = false;
                Q_FOREACH (KisToolBar *tb, guard->toolBars()) anyBar |= tb->isVisible();
                if (!c.readEntry("classicArrangedV4", false) || !anyVisible || !anyBar) {
                    arrangeClassic(guard);
                    c.writeEntry("classicArrangedV4", true);
                } else {
                    styleClassicDocks(guard);
                }
            });
        }
        if (screenChanged) {
            flushLayouts();
            animateScreenIn(mw);
        }
        return;
    }
    if (!top || !nav) {
        return;
    }

    // menus e barras padrão do Krita somem; o Nanquim tem as suas
    mw->menuBar()->setVisible(false);
    Q_FOREACH (KisToolBar *tb, mw->toolBars()) {
        tb->setVisible(false);
    }

    // na galeria, o cabeçalho dela faz o papel da barra do topo; na tela de
    // desenho, o estúdio (as peças flutuantes) substitui a barra
    top->setVisible(canvasMode ? !NANQUIM_ESTUDIO : !hasGallery);
    mw->setProperty("nanquimOnCanvas", canvasMode);
    Q_FOREACH (QWidget *w, top->findChildren<QWidget*>()) {
        if (w->property(WELCOME_ONLY).toBool()) {
            // o botão fica dentro de uma QWidgetAction da barra: mostra a ação
            Q_FOREACH (QAction *a, top->actions()) {
                if (top->widgetForAction(a) == w) {
                    a->setVisible(!canvasMode && hasDrawing);
                }
            }
        } else if (w->property(NEEDS_CANVAS).toBool()) {
            Q_FOREACH (QAction *a, top->actions()) {
                if (top->widgetForAction(a) == w) {
                    a->setVisible(canvasMode);
                }
            }
        }
    }
    if (QAction *paintops = mw->actionCollection()->action(QStringLiteral("paintops"))) {
        paintops->setVisible(canvasMode);
    }
    if (!canvasMode) {
        if (QLabel *title = top->findChild<QLabel*>(QStringLiteral("nanquimTitle"))) {
            title->setText(i18n("Galeria"));
        }
        if (QLabel *info = top->findChild<QLabel*>(QStringLiteral("nanquimInfo"))) {
            info->setText(hasDrawing ? i18n("Seu desenho continua aberto") : QString());
        }
    }

    if (!canvasMode) {
        if (QWidget *wheel = mw->findChild<QWidget*>(QStringLiteral("nanquimWheel"))) wheel->hide();
        // painel solto volta para a janela antes de o Krita salvar o arranjo
        if (QDockWidget *hosted = hostedDock(mw)) {
            closePanel(mw, hosted, false);
        }
        if (QFrame *flyout = mw->findChild<QFrame*>(FLYOUT_ID)) {
            flyout->hide();
        }
        if (QFrame *brushPanel = mw->findChild<QFrame*>(BRUSH_PANEL_ID)) {
            brushPanel->hide();
        }
        if (QWidget *hud = mw->findChild<QWidget*>(QStringLiteral("nanquimBrushHud"))) {
            hud->hide();
        }
        if (QWidget *strip = mw->findChild<QWidget*>(FILMSTRIP_ID)) {
            strip->hide();
        }
    }
    if (!canvasMode) {
        s_focusMode = false;
        setKritaChromeHidden(mw, NANQUIM_ESTUDIO);
        showFocusBarAnimated(mw, false, false);
        updateSelectionBar(mw, false);
    } else {
        updateSelectionBar(mw, false);
    }
    setThumbDockVisible(mw, canvasMode);
    // a barra lateral antiga fica guardada (o estúdio tem a coluna dele)
    nav->setVisible(canvasMode && !NANQUIM_ESTUDIO);
    if (QToolButton *tab = mw->findChild<QToolButton*>(QStringLiteral("nanquimExpandTab"))) tab->hide();
    if (canvasMode && NANQUIM_ESTUDIO) hideKritaChrome(mw);
    if (canvasMode && screenChanged && canvasOnlyActive(mw)) {
        // "só a página" ficou ligado de outra vez: ao voltar para o desenho, as
        // peças voltam também (senão não haveria botão nenhum na tela)
        if (QAction *a = act(mw, "view_show_canvas_only")) a->setChecked(false);
    }
    // build 79: esta chamada vem de dentro do carregamento da página, quando a
    // vista ainda não está pronta (a build 78 caía aqui). As peças esperam o
    // Krita terminar, e o estado é lido na hora de mostrar.
    if (NANQUIM_ESTUDIO) {
        QPointer<KisMainWindow> guard(mw);
        QTimer::singleShot(250, mw, [guard, canvasMode]() {
            if (!guard) return;
            if (guard->property("nanquimOnCanvas").toBool() != canvasMode) return;   // a tela já mudou
            setStudioVisible(guard, canvasMode && !canvasOnlyActive(guard));
        });
    }

    if (canvasMode && s_focusMode) applyFocusVisibility(mw);

    // A primeira tela de desenho da sessão começa limpa (a barra da esquerda
    // substitui a caixa de ferramentas do Krita). Painel fechado não se redesenha.
    if (canvasMode && !s_canvasCleaned) {
        styleClassicDocks(mw);   // painéis do Krita com a cara do Nanquim também no modo Tablet
        s_canvasCleaned = true;
        Q_FOREACH (QDockWidget *dw, mw->dockWidgets()) {
            dw->hide();
        }
        QPointer<KisMainWindow> warmGuard(mw);
        QTimer::singleShot(450, mw, [warmGuard]() {
            if (warmGuard && warmGuard->property("nanquimOnCanvas").toBool()) {
                warmUpPopups(warmGuard);
            }
        });
    }

    if (screenChanged) {
        flushLayouts();
        animateScreenIn(mw);
    }
}

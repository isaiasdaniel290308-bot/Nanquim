/*
 *  SPDX-FileCopyrightText: 2026 Nanquim
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef NANQUIM_THEME_H
#define NANQUIM_THEME_H

#include <QColor>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QImageReader>
#include <QLinearGradient>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QRadialGradient>
#include <QRandomGenerator>
#include <QStandardPaths>
#include <QString>
#include <QStringList>
#include <QtMath>
#include <cmath>

#include <kconfiggroup.h>
#include <ksharedconfig.h>

/**
 * Motor de tema, texturas e imagem de fundo dos painéis do Nanquim.
 *
 * Inspirado na proposta de personalização ("Color Intensity", retícula e imagem
 * no canto do painel de camadas) do redesign de u/JuhoSprite (r/krita).
 *
 * Tudo é pré-renderizado em PNG leve em cache somente quando o usuário altera
 * uma opção: custo zero durante o desenho na página OpenGL.
 */
namespace NanquimTheme
{

struct Settings {
    QString accent = QStringLiteral("#3e8e7e");      // Verdete padrão do Nanquim
    int intensity = 0;                               // 0..100% (tinge fundos e painéis)
    QString texture = QStringLiteral("perspectiva"); // perspectiva, nenhuma, pontos, favo, reticula, hachura, grao
    int textureStrength = 55;                        // 0..100%
    QString panelArtMode = QStringLiteral("canto");  // canto, inteira, xadrez, nenhuma
    int panelArtOpacity = 85;                        // 0..100%
    bool panelArtTint = false;                       // tinge a imagem com a cor do tema
};

struct Palette {
    QColor bg;          // #24231f (grafite)
    QColor surface;     // #2e2d29 (superfície)
    QColor elevated;    // #34332e (botões/cartões)
    QColor surface2;    // #3a3934 (pressionado)
    QColor line;        // #4a4842 (bordas/divisórias)
    QColor border2;     // #524f48 (borda relevo 2)
    QColor border3;     // #5a574f (borda relevo 3)
    QColor accent;      // #3e8e7e (destaque)
    QColor accentDark;  // #2f7568 (botão principal)
    QColor accentLight; // #6fbfad (toque/ativo)
    QColor accentTint;  // #2a3d38 (seleção suave)
    QColor accentTint2; // #2f4a43 (checked barra)
    QColor accentText;  // #cde9e1 (texto claro sobre seleção)
};

inline QColor mixColor(const QColor &a, const QColor &b, qreal w)
{
    const qreal t = qBound(0.0, w, 1.0);
    return QColor(
        qBound(0, qRound(a.red()   + (b.red()   - a.red())   * t), 255),
        qBound(0, qRound(a.green() + (b.green() - a.green()) * t), 255),
        qBound(0, qRound(a.blue()  + (b.blue()  - a.blue())  * t), 255));
}

struct State {
    Settings s;
    Palette pal;
    bool loaded = false;
    int generation = 0;
    QString texPath;
    QString layerBgPath;
    QPixmap texPixmap;
};

inline State &state()
{
    static State st;
    return st;
}

inline QString themeDataDir()
{
    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(base);
    dir.mkpath(QStringLiteral("nanquim-theme"));
    return dir.filePath(QStringLiteral("nanquim-theme"));
}

inline QString themeCacheDir()
{
    const QString base = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    QDir dir(base);
    dir.mkpath(QStringLiteral("nanquim-theme"));
    return dir.filePath(QStringLiteral("nanquim-theme"));
}

inline QString customArtFilePath()
{
    return QDir(themeDataDir()).filePath(QStringLiteral("custom_panel.png"));
}

inline bool hasCustomArt()
{
    return QFileInfo::exists(customArtFilePath());
}

inline bool installCustomArt(const QString &sourcePath)
{
    QImageReader reader(sourcePath);
    reader.setAutoTransform(true);
    QImage img = reader.read();
    if (img.isNull()) return false;
    // Reduz para no máximo 512 px para economizar RAM no tablet
    if (img.width() > 512 || img.height() > 512) {
        img = img.scaled(512, 512, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    return img.save(customArtFilePath(), "PNG");
}

inline void removeCustomArt()
{
    QFile::remove(customArtFilePath());
}

inline Palette computePalette(const Settings &s)
{
    QColor acc(s.accent);
    if (!acc.isValid()) acc = QColor(0x3e, 0x8e, 0x7e);
    const qreal t = qBound(0, s.intensity, 100) / 100.0;

    Palette p;
    p.bg          = mixColor(QColor(0x24, 0x23, 0x1f), acc, t * 0.30);
    p.surface     = mixColor(QColor(0x2e, 0x2d, 0x29), acc, t * 0.42);
    p.elevated    = mixColor(QColor(0x34, 0x33, 0x2e), acc, t * 0.48);
    p.surface2    = mixColor(QColor(0x3a, 0x39, 0x34), acc, t * 0.55);
    p.line        = mixColor(QColor(0x4a, 0x48, 0x42), acc, t * 0.55);
    p.border2     = mixColor(QColor(0x52, 0x4f, 0x48), acc, t * 0.55);
    p.border3     = mixColor(QColor(0x5a, 0x57, 0x4f), acc, t * 0.55);
    p.accent      = acc;
    p.accentDark  = mixColor(acc, QColor(0, 0, 0), 0.18);
    p.accentLight = mixColor(acc, QColor(255, 255, 255), 0.35);
    p.accentTint  = mixColor(p.surface, acc, 0.32);
    p.accentTint2 = mixColor(p.surface, acc, 0.42);
    p.accentText  = mixColor(acc, QColor(255, 255, 255), 0.72);
    return p;
}

inline QImage renderTextureImage(const Settings &s)
{
    const int strength = qBound(0, s.textureStrength, 100);
    if (s.texture == QLatin1String("nenhuma") || strength <= 0) {
        return QImage();
    }

    if (s.texture == QLatin1String("perspectiva")) {
        // Retícula de mangá em perspectiva com pontos escuros só no canto inferior direito
        const int W = 300;
        const int H = 300;
        const qreal R0 = 270.0;
        QImage img(W, H, QImage::Format_ARGB32_Premultiplied);
        img.fill(Qt::transparent);
        QPainter g(&img);
        g.setRenderHint(QPainter::Antialiasing, true);
        g.setPen(Qt::NoPen);
        const int alpha = qBound(0, qRound((strength / 100.0) * 0.50 * 255.0), 255);
        g.setBrush(QColor(0, 0, 0, alpha));

        qreal y = H;
        qreal sp = 18.0;
        int row = 0;
        while (sp > 3.0 && y > 0.0) {
            for (qreal x = (row % 2) * sp * 0.5; x <= W; x += sp) {
                const qreal d = std::hypot(W - x, H - y) / R0;
                const qreal fade = qMax(0.0, 1.0 - d);
                const qreal r = sp * 0.42 * std::pow(fade, 1.3);
                if (r > 0.45) {
                    g.drawEllipse(QPointF(x, y), r, r);
                }
            }
            y -= sp * 0.87;
            sp *= 0.94;
            ++row;
        }
        g.end();
        return img;
    }

    const int alpha = qBound(0, qRound((strength / 100.0) * 0.28 * 255.0), 255);
    const QColor dark(0, 0, 0, alpha);

    if (s.texture == QLatin1String("pontos")) {
        QImage img(8, 8, QImage::Format_ARGB32_Premultiplied);
        img.fill(Qt::transparent);
        QPainter g(&img);
        g.setRenderHint(QPainter::Antialiasing, true);
        g.setPen(Qt::NoPen);
        g.setBrush(dark);
        g.drawEllipse(QPointF(4.0, 4.0), 1.1, 1.1);
        g.end();
        return img;
    }
    if (s.texture == QLatin1String("favo")) {
        QImage img(28, 49, QImage::Format_ARGB32_Premultiplied);
        img.fill(Qt::transparent);
        QPainter g(&img);
        g.setRenderHint(QPainter::Antialiasing, true);
        g.setPen(QPen(dark, 1.2));
        g.setBrush(Qt::NoBrush);
        QPainterPath p;
        p.moveTo(14, 0);
        p.lineTo(28, 8);
        p.lineTo(28, 25);
        p.lineTo(14, 33);
        p.lineTo(0, 25);
        p.lineTo(0, 8);
        p.closeSubpath();
        p.moveTo(14, 33);
        p.lineTo(14, 49);
        g.drawPath(p);
        g.end();
        return img;
    }
    if (s.texture == QLatin1String("reticula")) {
        QImage img(10, 10, QImage::Format_ARGB32_Premultiplied);
        img.fill(Qt::transparent);
        QPainter g(&img);
        g.setRenderHint(QPainter::Antialiasing, true);
        g.setPen(Qt::NoPen);
        g.setBrush(dark);
        const QPointF pts[] = {{0, 0}, {10, 0}, {0, 10}, {10, 10}, {5, 5}};
        for (const QPointF &pt : pts) g.drawEllipse(pt, 1.6, 1.6);
        g.end();
        return img;
    }
    if (s.texture == QLatin1String("hachura")) {
        QImage img(8, 8, QImage::Format_ARGB32_Premultiplied);
        img.fill(Qt::transparent);
        QPainter g(&img);
        g.setRenderHint(QPainter::Antialiasing, true);
        g.setPen(QPen(dark, 1.0));
        g.drawLine(QPointF(-2, 10), QPointF(10, -2));
        g.drawLine(QPointF(-2, 2), QPointF(2, -2));
        g.drawLine(QPointF(6, 10), QPointF(10, 6));
        g.end();
        return img;
    }
    if (s.texture == QLatin1String("grao")) {
        QImage img(96, 96, QImage::Format_ARGB32_Premultiplied);
        img.fill(Qt::transparent);
        QRandomGenerator rng(1337);
        const int maxA = qBound(0, qRound((strength / 100.0) * 0.22 * 255.0), 255);
        for (int y = 0; y < 96; ++y) {
            QRgb *row = reinterpret_cast<QRgb*>(img.scanLine(y));
            for (int x = 0; x < 96; ++x) {
                const int a = rng.bounded(maxA + 1);
                row[x] = qRgba(0, 0, 0, a);
            }
        }
        return img;
    }
    return QImage();
}

inline QImage renderDefaultNibArt(int size, const Palette &pal)
{
    QImage img(size, size, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);
    QPainter g(&img);
    g.setRenderHint(QPainter::Antialiasing, true);
    const qreal s = size / 200.0;
    g.scale(s, s);

    // Círculo colorido no tema do Nanquim
    QLinearGradient grad(28, 28, 196, 196);
    grad.setColorAt(0.0, mixColor(pal.accent, QColor(255, 255, 255), 0.45));
    grad.setColorAt(0.55, pal.accent);
    grad.setColorAt(1.0, QColor(0xe6, 0xb4, 0x5a));
    g.setPen(Qt::NoPen);
    g.setBrush(grad);
    g.drawEllipse(QPointF(112, 112), 84, 84);

    // Pena de nanquim clássica
    QPainterPath nib;
    nib.moveTo(30, 22);
    nib.lineTo(128, 104);
    nib.cubicTo(140, 116, 138, 136, 124, 146);
    nib.cubicTo(110, 156, 94, 154, 86, 140);
    nib.lineTo(22, 32);
    nib.closeSubpath();
    g.setBrush(QColor(0x1b, 0x1a, 0x17));
    g.drawPath(nib);

    // Fenda e furo da pena
    const QColor slitColor = mixColor(pal.accent, QColor(255, 255, 255), 0.45);
    g.setPen(QPen(slitColor, 3.0, Qt::SolidLine, Qt::RoundCap));
    g.drawLine(QPointF(30, 22), QPointF(100, 120));
    g.setPen(Qt::NoPen);
    g.setBrush(slitColor);
    g.drawEllipse(QPointF(100, 120), 6.0, 6.0);

    // Gota saindo da ponta
    QPainterPath drop;
    drop.moveTo(150, 150);
    drop.cubicTo(160, 160, 158, 172, 148, 176);
    drop.cubicTo(152, 168, 148, 160, 150, 150);
    g.setBrush(QColor(0x1b, 0x1a, 0x17));
    g.drawPath(drop);

    g.end();
    return img;
}

inline void tintImagePreserveLuminance(QImage &img, const QColor &accent)
{
    img = img.convertToFormat(QImage::Format_ARGB32);
    qreal ah = 0, as = 0, al = 0;
    accent.getHslF(&ah, &as, &al);
    if (ah < 0.0) ah = 0.0;
    const qreal sat = qBound(0.25, as * 0.85, 0.90);
    for (int y = 0; y < img.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb*>(img.scanLine(y));
        for (int x = 0; x < img.width(); ++x) {
            const int a = qAlpha(line[x]);
            if (a == 0) continue;
            const qreal lum = (0.299 * qRed(line[x]) + 0.587 * qGreen(line[x]) + 0.114 * qBlue(line[x])) / 255.0;
            const QColor c = QColor::fromHslF(ah, sat, qBound(0.05, lum, 0.95));
            line[x] = qRgba(c.red(), c.green(), c.blue(), a);
        }
    }
    img = img.convertToFormat(QImage::Format_ARGB32_Premultiplied);
}

inline QImage renderLayerBackgroundImage(const Settings &s, const Palette &pal, const QImage &texImg)
{
    const bool hasCornerTex = (s.texture == QLatin1String("perspectiva") && !texImg.isNull());
    const bool hasArt = (s.panelArtMode != QLatin1String("nenhuma") && s.panelArtOpacity > 0);
    if (!hasCornerTex && !hasArt) {
        return QImage();
    }

    const int W = 320;
    const int H = 320;
    QImage out(W, H, QImage::Format_ARGB32_Premultiplied);
    out.fill(Qt::transparent);

    QPainter g(&out);
    g.setRenderHint(QPainter::Antialiasing, true);
    g.setRenderHint(QPainter::SmoothPixmapTransform, true);

    if (hasCornerTex) {
        g.drawImage(W - texImg.width(), H - texImg.height(), texImg);
    }

    if (hasArt) {
        QImage artLayer(W, H, QImage::Format_ARGB32_Premultiplied);
        artLayer.fill(Qt::transparent);

        if (s.panelArtMode == QLatin1String("xadrez")) {
            QPainter ag(&artLayer);
            const int cell = 20;
            const QColor c1(0x5a, 0x58, 0x52);
            const QColor c2(0x3a, 0x39, 0x34);
            for (int y = 0; y < H; y += cell) {
                for (int x = 0; x < W; x += cell) {
                    ag.fillRect(x, y, cell, cell, (((x / cell) + (y / cell)) % 2 == 0) ? c1 : c2);
                }
            }
            // Suaviza de baixo para cima
            ag.setCompositionMode(QPainter::CompositionMode_DestinationIn);
            QLinearGradient mask(0, H, 0, H * 0.40);
            mask.setColorAt(0.0, QColor(0, 0, 0, 255));
            mask.setColorAt(1.0, QColor(0, 0, 0, 0));
            ag.fillRect(0, 0, W, H, mask);
            ag.end();
        } else {
            QImage src;
            if (hasCustomArt()) {
                QImageReader reader(customArtFilePath());
                reader.setAutoTransform(true);
                src = reader.read();
            }
            if (src.isNull()) {
                src = renderDefaultNibArt(240, pal);
            }

            if (s.panelArtMode == QLatin1String("canto")) {
                const int box = 210;
                QImage scaled = src.scaled(box, box, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
                const int cx = (scaled.width() - box) / 2;
                const int cy = (scaled.height() - box) / 2;
                QImage cropped = scaled.copy(cx, cy, box, box).convertToFormat(QImage::Format_ARGB32_Premultiplied);

                QPainter mg(&cropped);
                mg.setCompositionMode(QPainter::CompositionMode_DestinationIn);
                QRadialGradient rad(QPointF(box * 0.60, box * 0.60), box * 0.65);
                rad.setColorAt(0.0, QColor(0, 0, 0, 255));
                rad.setColorAt(0.55, QColor(0, 0, 0, 255));
                rad.setColorAt(1.0, QColor(0, 0, 0, 0));
                mg.fillRect(0, 0, box, box, rad);
                mg.end();

                QPainter ag(&artLayer);
                ag.setRenderHint(QPainter::SmoothPixmapTransform, true);
                ag.drawImage(W - box + 24, H - box + 24, cropped);
                ag.end();
            } else {
                // "inteira": preenche e suaviza em direção ao topo
                QImage scaled = src.scaled(W, H, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
                const int cx = (scaled.width() - W) / 2;
                const int cy = (scaled.height() - H) / 2;
                artLayer = scaled.copy(cx, cy, W, H).convertToFormat(QImage::Format_ARGB32_Premultiplied);

                QPainter mg(&artLayer);
                mg.setCompositionMode(QPainter::CompositionMode_DestinationIn);
                QLinearGradient mask(0, H, 0, H * 0.22);
                mask.setColorAt(0.0, QColor(0, 0, 0, 255));
                mask.setColorAt(0.45, QColor(0, 0, 0, 155));
                mask.setColorAt(1.0, QColor(0, 0, 0, 0));
                mg.fillRect(0, 0, W, H, mask);
                mg.end();
            }
        }

        if (s.panelArtTint) {
            tintImagePreserveLuminance(artLayer, pal.accent);
        }

        g.setOpacity(qBound(0, s.panelArtOpacity, 100) / 100.0);
        g.drawImage(0, 0, artLayer);
    }

    g.end();
    return out;
}

inline void rebuildAssets()
{
    State &st = state();
    st.pal = computePalette(st.s);
    ++st.generation;

    const QString dirPath = themeCacheDir();
    QDir dir(dirPath);
    const QStringList oldFiles = dir.entryList({QStringLiteral("tex_*.png"), QStringLiteral("layer_*.png")}, QDir::Files);
    for (const QString &f : oldFiles) {
        dir.remove(f);
    }

    const QImage texImg = renderTextureImage(st.s);
    if (!texImg.isNull()) {
        st.texPath = dir.filePath(QStringLiteral("tex_%1.png").arg(st.generation));
        texImg.save(st.texPath, "PNG");
        st.texPixmap = QPixmap::fromImage(texImg);
    } else {
        st.texPath.clear();
        st.texPixmap = QPixmap();
    }

    const QImage layerImg = renderLayerBackgroundImage(st.s, st.pal, texImg);
    if (!layerImg.isNull()) {
        st.layerBgPath = dir.filePath(QStringLiteral("layer_%1.png").arg(st.generation));
        layerImg.save(st.layerBgPath, "PNG");
    } else {
        st.layerBgPath.clear();
    }
}

inline void ensureLoaded()
{
    State &st = state();
    if (st.loaded) return;
    st.loaded = true;
    KConfigGroup cfg = KSharedConfig::openConfig()->group(QStringLiteral("Nanquim"));
    st.s.accent          = cfg.readEntry("themeAccent", QStringLiteral("#3e8e7e"));
    st.s.intensity       = qBound(0, cfg.readEntry("themeIntensity", 0), 100);
    st.s.texture         = cfg.readEntry("themeTexture", QStringLiteral("perspectiva"));
    st.s.textureStrength = qBound(0, cfg.readEntry("themeTextureStrength", 55), 100);
    st.s.panelArtMode    = cfg.readEntry("themePanelArtMode", QStringLiteral("canto"));
    st.s.panelArtOpacity = qBound(0, cfg.readEntry("themePanelArtOpacity", 85), 100);
    st.s.panelArtTint    = cfg.readEntry("themePanelArtTint", false);
    rebuildAssets();
}

inline const Settings &settings()
{
    ensureLoaded();
    return state().s;
}

inline const Palette &palette()
{
    ensureLoaded();
    return state().pal;
}

inline const QPixmap &texturePixmap()
{
    ensureLoaded();
    return state().texPixmap;
}

inline bool isPerspectiveTexture()
{
    ensureLoaded();
    return state().s.texture == QLatin1String("perspectiva");
}

inline void saveAndRebuild(const Settings &next)
{
    ensureLoaded();
    State &st = state();
    st.s = next;
    KConfigGroup cfg = KSharedConfig::openConfig()->group(QStringLiteral("Nanquim"));
    cfg.writeEntry("themeAccent", st.s.accent);
    cfg.writeEntry("themeIntensity", st.s.intensity);
    cfg.writeEntry("themeTexture", st.s.texture);
    cfg.writeEntry("themeTextureStrength", st.s.textureStrength);
    cfg.writeEntry("themePanelArtMode", st.s.panelArtMode);
    cfg.writeEntry("themePanelArtOpacity", st.s.panelArtOpacity);
    cfg.writeEntry("themePanelArtTint", st.s.panelArtTint);
    cfg.sync();
    rebuildAssets();
}

inline QString recolorStyle(const QString &qss)
{
    ensureLoaded();
    const Palette &p = state().pal;
    // Substituição em duas passagens (marcador -> cor final) evita colisão entre cores
    struct Map { const char *from; const char *tok; QString to; };
    const QString surfRgba = QStringLiteral("rgba(%1,%2,%3,0.96)")
        .arg(p.surface.red()).arg(p.surface.green()).arg(p.surface.blue());
    const Map maps[] = {
        {"rgba(46,45,41,0.96)", "__NQ_SRGBA__", surfRgba},
        {"#24231f", "__NQ_BG__",    p.bg.name()},
        {"#2e2d29", "__NQ_SURF__",  p.surface.name()},
        {"#34332e", "__NQ_ELEV__",  p.elevated.name()},
        {"#3a3934", "__NQ_SURF2__", p.surface2.name()},
        {"#4a4842", "__NQ_LINE__",  p.line.name()},
        {"#524f48", "__NQ_B2__",    p.border2.name()},
        {"#5a574f", "__NQ_B3__",    p.border3.name()},
        {"#3e8e7e", "__NQ_ACC__",   p.accent.name()},
        {"#2f7568", "__NQ_ACCD__",  p.accentDark.name()},
        {"#6fbfad", "__NQ_ACCL__",  p.accentLight.name()},
        {"#2a3d38", "__NQ_TINT__",  p.accentTint.name()},
        {"#2f4a43", "__NQ_TINT2__", p.accentTint2.name()},
        {"#253d37", "__NQ_TINT3__", p.accentTint.name()},
        {"#253430", "__NQ_TINT4__", p.accentTint.name()},
        {"#cde9e1", "__NQ_ACCT__",  p.accentText.name()},
        {"#d8f2eb", "__NQ_ACCT2__", p.accentText.name()},
    };
    QString out = qss;
    for (const Map &m : maps) {
        out.replace(QLatin1String(m.from), QLatin1String(m.tok), Qt::CaseInsensitive);
    }
    for (const Map &m : maps) {
        out.replace(QLatin1String(m.tok), m.to);
    }
    return out;
}

inline QString panelTextureRule(const QString &selector)
{
    ensureLoaded();
    const State &st = state();
    if (st.texPath.isEmpty()) return QString();
    QString url = st.texPath;
    url.replace(QLatin1Char('\\'), QLatin1Char('/'));
    if (st.s.texture == QLatin1String("perspectiva")) {
        return QStringLiteral("%1 { background-image: url(\"%2\"); background-position: bottom right; background-repeat: no-repeat; }")
            .arg(selector, url);
    }
    return QStringLiteral("%1 { background-image: url(\"%2\"); background-repeat: repeat; }")
        .arg(selector, url);
}

inline QString layerBackgroundRule()
{
    ensureLoaded();
    const State &st = state();
    if (st.layerBgPath.isEmpty()) {
        // Sem imagem no canto, aplica textura repetida se houver
        if (!st.texPath.isEmpty() && st.s.texture != QLatin1String("perspectiva")) {
            return panelTextureRule(QStringLiteral("QAbstractItemView"));
        }
        return QString();
    }
    QString url = st.layerBgPath;
    url.replace(QLatin1Char('\\'), QLatin1Char('/'));
    return QStringLiteral(
        "QAbstractItemView { background-image: url(\"%1\"); background-position: bottom right;"
        "  background-repeat: no-repeat; background-attachment: fixed; }")
        .arg(url);
}

} // namespace NanquimTheme

#endif

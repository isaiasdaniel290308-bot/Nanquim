/*
 *  SPDX-FileCopyrightText: 2026 Nanquim
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "NanquimSutImport.h"

#include <QBuffer>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QPainter>
#include <QPointF>
#include <QScopedPointer>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QStandardPaths>
#include <QTransform>
#include <QUuid>
#include <QVariant>
#include <QVector>
#include <QtEndian>
#include <QtMath>
#include <cstring>

#include <KoResource.h>
#include <KoStore.h>
#include <KisResourceModel.h>
#include <KisResourceTypes.h>
#include <KisResourceUserOperations.h>
#include <KisStorageModel.h>
#include <KisTagModel.h>
#include <KisTagResourceModel.h>
#include <kis_paintop_preset.h>
#include <kis_paintop_settings.h>

#include "KisResourceServerProvider.h"
#include "KisUsageLogger.h"

namespace
{

struct SutEffector {
    bool valid = false;
    quint32 onMask = 0;       // 0x10 pressure, 0x20 tilt, 0x40 speed, 0x80 random
    double pressureMin = 0.0; // 0..1
    double tiltMin = 0.0;     // 0..1
    double speedMin = 0.0;    // 0..1
    double randomMin = 0.0;   // 0..1
    QVector<QVector<QPointF>> curves;
};

struct SutBrush {
    QString name;
    double size = 20.0;          // px
    double opacity = 100.0;      // %
    double flow = 100.0;         // %
    double hardness = 100.0;     // %
    double interval = 10.0;      // % do tamanho (só quando o espaçamento é fixo)
    int intervalType = 1;        // 0 = fixo; 1 = largo; 2 = normal; 3 = estreito
    double thickness = 100.0;    // % (achata a ponta)
    bool verticalThickness = false;
    double rotation = 0.0;       // graus
    int rotationEffector = 3;    // 3 = fixo; 0x04 = direção do traço; 0x08 = inclinação; 0x80 = aleatório
    double rotationRandomScale = 100.0;
    bool usePattern = false;     // ponta por imagem
    bool reverseH = false;
    bool reverseV = false;
    bool ribbon = false;         // "fita": a imagem acompanha o traço
    bool antiAlias = true;

    // Textura de papel
    bool hasTexture = false;
    double textureScale = 100.0;     // %
    double textureDensity = 50.0;    // %
    double textureRotate = 0.0;      // graus
    int textureBrightness = 0;       // -100..100
    int textureContrast = 0;         // -100..100
    int textureCompositeMode = 0;    // 0/1 = Multiply, 2/4 = Subtract
    bool textureInvert = false;

    SutEffector sizeEff;
    SutEffector opacityEff;
    SutEffector flowEff;
    SutEffector thicknessEff;
    SutEffector textureEff;

    QByteArray tipPng;
    QByteArray texturePng;
};

// Extrai um arquivo de dentro de um tar guardado em memória.
QByteArray tarEntry(const QByteArray &tar, const QString &wanted)
{
    int pos = 0;
    while (pos + 512 <= tar.size()) {
        const char *header = tar.constData() + pos;
        if (header[0] == '\0') break;
        const QString name = QString::fromUtf8(header, int(qstrnlen(header, 100)));
        qint64 size = 0;
        for (int i = 124; i < 136; i++) {
            const char c = header[i];
            if (c < '0' || c > '7') {
                if (c == ' ' && size == 0) continue;
                break;
            }
            size = size * 8 + (c - '0');
        }
        if (size < 0 || pos + 512 + size > tar.size()) break;
        if (name == wanted) {
            return tar.mid(pos + 512, int(size));
        }
        pos += 512 + int((size + 511) / 512) * 512;
    }
    return QByteArray();
}

// Verifica se um BLOB de referência do Clip Studio (UTF-16LE/BE ou ASCII) cita um UUID.
bool blobContainsUuid(const QByteArray &blob, const QString &uuid)
{
    if (blob.isEmpty() || uuid.isEmpty()) return false;
    if (blob.contains(uuid.toUtf8())) return true;
    const QByteArray le(reinterpret_cast<const char*>(uuid.utf16()), uuid.size() * 2);
    if (blob.contains(le)) return true;
    QByteArray be(uuid.size() * 2, '\0');
    for (int i = 0; i < uuid.size(); ++i) {
        const ushort u = uuid.at(i).unicode();
        be[i * 2] = char((u >> 8) & 0xff);
        be[i * 2 + 1] = char(u & 0xff);
    }
    return blob.contains(be);
}

QByteArray extractMaterialPng(QSqlDatabase &db, const QByteArray &refBlob)
{
    if (refBlob.size() < 16) return QByteArray();
    QSqlQuery material(db);
    if (!material.exec(QStringLiteral("SELECT CatalogPath, OriginalPath, FileData FROM MaterialFile"))) {
        return QByteArray();
    }
    while (material.next()) {
        const QString cat = material.value(0).toString();
        const QString orig = material.value(1).toString();
        const QString uuid = cat.section(QLatin1Char(':'), -1);
        const QString origUuid = orig.section(QLatin1Char(':'), -3, -3);
        const bool matched = (!uuid.isEmpty() && blobContainsUuid(refBlob, uuid))
                || (!origUuid.isEmpty() && blobContainsUuid(refBlob, origUuid));
        if (!matched) continue;
        const QByteArray png = tarEntry(material.value(2).toByteArray(),
                                        QStringLiteral("thumbnail/thumbnail.png"));
        if (!png.isEmpty()) return png;
    }
    return QByteArray();
}

SutEffector parseEffector(const QByteArray &blob)
{
    SutEffector eff;
    if (blob.size() < 48) return eff;
    const uchar *raw = reinterpret_cast<const uchar*>(blob.constData());
    auto u32 = [raw](int off) { return qFromBigEndian<quint32>(raw + off); };

    const quint32 hdrSize = u32(0);
    if (hdrSize < 48 || int(hdrSize) > blob.size()) return eff;

    eff.valid = true;
    eff.onMask = u32(8);
    eff.pressureMin = qBound(0.0, double(u32(12)) / 100.0, 1.0);
    eff.tiltMin = qBound(0.0, double(u32(16)) / 100.0, 1.0);
    eff.speedMin = qBound(0.0, double(u32(20)) / 100.0, 1.0);
    eff.randomMin = qBound(0.0, double(u32(24)) / 100.0, 1.0);

    int pos = int(hdrSize);
    while (pos + 12 <= blob.size()) {
        const quint32 chdr = u32(pos);
        const quint32 npts = u32(pos + 4);
        const quint32 ptsz = u32(pos + 8);
        if (chdr != 12 || ptsz != 16 || npts > 64 || pos + 12 + int(npts) * 16 > blob.size()) {
            break;
        }
        QVector<QPointF> pts;
        pts.reserve(int(npts));
        int p = pos + 12;
        for (quint32 i = 0; i < npts; ++i) {
            const quint64 xb = qFromBigEndian<quint64>(raw + p);
            const quint64 yb = qFromBigEndian<quint64>(raw + p + 8);
            double x = 0.0, y = 0.0;
            std::memcpy(&x, &xb, sizeof(double));
            std::memcpy(&y, &yb, sizeof(double));
            pts << QPointF(qBound(0.0, x, 1.0), qBound(0.0, y, 1.0));
            p += 16;
        }
        eff.curves << pts;
        pos = p;
    }
    return eff;
}

bool readSut(const QString &path, SutBrush *brush, QString *error, int variantIndex = 0, int *variantCount = nullptr)
{
    const QString connection = QStringLiteral("nanquim-sut-") + QUuid::createUuid().toString(QUuid::Id128);
    bool ok = false;
    {
        QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connection);
        db.setDatabaseName(path);
        db.setConnectOptions(QStringLiteral("QSQLITE_OPEN_READONLY"));
        if (!db.open()) {
            *error = QStringLiteral("não consegui abrir o arquivo (%1)").arg(db.lastError().text());
        } else {
            do {
                int variantId = 0;
                {
                    if (variantCount) {
                        QSqlQuery count(db);
                        *variantCount = (count.exec(QStringLiteral("SELECT COUNT(*) FROM Node WHERE NodeVariantID > 0"))
                                         && count.next()) ? count.value(0).toInt() : 1;
                    }
                    QSqlQuery node(db);
                    if (!node.exec(QStringLiteral("SELECT NodeName, NodeVariantID FROM Node WHERE NodeVariantID > 0 "
                                                  "LIMIT 1 OFFSET %1").arg(qMax(0, variantIndex)))
                            || !node.next()) {
                        *error = QStringLiteral("o arquivo não tem nenhuma subferramenta");
                        break;
                    }
                    brush->name = node.value(0).toString().trimmed();
                    variantId = node.value(1).toInt();
                }
                QByteArray patternRef;
                QByteArray textureRef;
                {
                    QSqlQuery variant(db);
                    variant.prepare(QStringLiteral("SELECT * FROM Variant WHERE VariantID = ?"));
                    variant.addBindValue(variantId);
                    if (!variant.exec() || !variant.next()) {
                        *error = QStringLiteral("não encontrei os ajustes do pincel");
                        break;
                    }
                    const QSqlRecord rec = variant.record();
                    auto number = [&rec](const char *key, double fallback) {
                        const int i = rec.indexOf(QLatin1String(key));
                        return (i >= 0 && !rec.isNull(i)) ? rec.value(i).toDouble() : fallback;
                    };
                    auto blobOf = [&rec](const char *key) {
                        const int i = rec.indexOf(QLatin1String(key));
                        return (i >= 0 && !rec.isNull(i)) ? rec.value(i).toByteArray() : QByteArray();
                    };

                    brush->size = qBound(1.0, number("BrushSize", 20.0), 1000.0);
                    brush->opacity = qBound(1.0, number("Opacity", 100.0), 100.0);
                    brush->flow = qBound(1.0, number("BrushFlow", 100.0), 100.0);
                    brush->hardness = qBound(1.0, number("BrushHardness", 100.0), 100.0);
                    brush->interval = number("BrushInterval", 10.0);
                    brush->intervalType = int(number("BrushAutoIntervalType", 1));
                    brush->thickness = qBound(1.0, number("BrushThickness", 100.0), 100.0);
                    brush->verticalThickness = number("BrushVerticalThicknes", 0) != 0;
                    brush->rotation = number("BrushRotation", 0.0);
                    brush->rotationEffector = int(number("BrushRotationEffector", 3));
                    brush->rotationRandomScale = qBound(0.0, number("BrushRotationRandomScale", 100.0), 100.0);
                    brush->usePattern = number("BrushUsePatternImage", 0) != 0;
                    brush->reverseH = number("BrushPatternReverseHorizontal", 0) != 0;
                    brush->reverseV = number("BrushPatternReverseVertical", 0) != 0;
                    brush->ribbon = number("BrushRibbon", 0) != 0;
                    brush->antiAlias = number("AntiAlias", 1) != 0;

                    brush->textureScale = qBound(5.0, number("TextureScale2", 100.0), 400.0);
                    brush->textureDensity = qBound(0.0, number("TextureDensity", 50.0), 100.0);
                    brush->textureRotate = number("TextureRotate", 0.0);
                    brush->textureBrightness = qBound(-100, int(number("TextureBrightness", 0)), 100);
                    brush->textureContrast = qBound(-100, int(number("TextureContrast", 0)), 100);
                    brush->textureCompositeMode = int(number("TextureCompositeMode", 0));
                    brush->textureInvert = number("TextureReverseDensity", 0) != 0;

                    brush->sizeEff = parseEffector(blobOf("BrushSizeEffector"));
                    brush->opacityEff = parseEffector(blobOf("BrushOpacityEffector"));
                    brush->flowEff = parseEffector(blobOf("BrushFlowEffector"));
                    brush->thicknessEff = parseEffector(blobOf("BrushThicknessEffector"));
                    brush->textureEff = parseEffector(blobOf("TextureDensityEffector"));

                    patternRef = blobOf("BrushPatternImageArray");
                    textureRef = blobOf("TextureImage");
                }
                if (brush->usePattern && !patternRef.isEmpty()) {
                    brush->tipPng = extractMaterialPng(db, patternRef);
                }
                if (!textureRef.isEmpty() && brush->textureDensity > 0.5) {
                    brush->texturePng = extractMaterialPng(db, textureRef);
                    brush->hasTexture = !brush->texturePng.isEmpty();
                }
                ok = true;
            } while (false);
            db.close();
        }
    }
    QSqlDatabase::removeDatabase(connection);
    if (ok && brush->name.isEmpty()) {
        brush->name = QFileInfo(path).completeBaseName();
    }
    return ok;
}

// Detecta se a ponta do Clip Studio usa preenchimento branco opaco (ex.: double line pen,
// canetas de contorno/sobreposição onde o branco cobre o traço que está atrás).
bool hasOpaqueWhiteFill(const QImage &src)
{
    if (src.isNull() || src.width() < 2 || src.height() < 2) return false;
    qint64 total = 0;
    qint64 opaqueWhite = 0;
    qint64 opaqueDark = 0;
    for (int y = 0; y < src.height(); ++y) {
        const QRgb *row = reinterpret_cast<const QRgb*>(src.constScanLine(y));
        for (int x = 0; x < src.width(); ++x) {
            ++total;
            const int a = qAlpha(row[x]);
            if (a < 200) continue;
            const int lum = qGray(row[x]);
            if (lum >= 235) ++opaqueWhite;
            else if (lum <= 90) ++opaqueDark;
        }
    }
    if (total <= 0) return false;
    return (opaqueWhite * 100 >= total * 3) && (opaqueDark * 100 >= total * 1);
}

// Converte a ponta do .sut para o Krita:
// - Se tiver preenchimento branco opaco -> preserva RGBA (IMAGESTAMP) para cobrir sobreposições.
// - Caso contrário -> gera máscara em tons de cinza (ALPHAMASK: preto = tinta, branco = vazio).
// - Corrige o eixo de BrushVerticalThicknes (0 = achata largura/horizontal, 1 = achata altura/vertical).
QImage makeTip(const SutBrush &brush, int *baseSize, bool *isColorStamp)
{
    *isColorStamp = false;
    QImage source = QImage::fromData(brush.tipPng, "PNG").convertToFormat(QImage::Format_ARGB32);
    if (source.isNull()) return QImage();

    const bool colorStamp = hasOpaqueWhiteFill(source);
    *isColorStamp = colorStamp;

    QImage tip;
    if (colorStamp) {
        tip = source;
        for (int y = 0; y < tip.height(); ++y) {
            QRgb *row = reinterpret_cast<QRgb*>(tip.scanLine(y));
            for (int x = 0; x < tip.width(); ++x) {
                if (qAlpha(row[x]) < 16) row[x] = 0;
            }
        }
    } else {
        tip = QImage(source.size(), QImage::Format_Grayscale8);
        for (int y = 0; y < source.height(); y++) {
            const QRgb *src = reinterpret_cast<const QRgb*>(source.constScanLine(y));
            uchar *dst = tip.scanLine(y);
            for (int x = 0; x < source.width(); x++) {
                const int alpha = qAlpha(src[x]);
                dst[x] = uchar(255 - alpha * (255 - qGray(src[x])) / 255);
            }
        }
    }

    if (brush.reverseH || brush.reverseV) {
        tip = tip.mirrored(brush.reverseH, brush.reverseV);
    }

    *baseSize = brush.ribbon ? qMin(tip.width(), tip.height()) : qMax(tip.width(), tip.height());

    if (!brush.ribbon) {
        if (brush.thickness < 99.0) {
            const double ratio = qBound(0.02, brush.thickness / 100.0, 1.0);
            if (brush.verticalThickness) {
                const int h = qMax(2, qRound(tip.height() * ratio));
                tip = tip.scaled(tip.width(), h, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            } else {
                const int w = qMax(2, qRound(tip.width() * ratio));
                tip = tip.scaled(w, tip.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            }
            *baseSize = qMax(tip.width(), tip.height());
        }
        if (!qFuzzyIsNull(std::fmod(brush.rotation, 360.0))) {
            QTransform t;
            t.rotate(-brush.rotation);
            const QRect bounds = t.mapRect(tip.rect());
            QImage rotated(bounds.size(), colorStamp ? QImage::Format_ARGB32 : QImage::Format_Grayscale8);
            rotated.fill(colorStamp ? Qt::transparent : Qt::white);
            QPainter g(&rotated);
            g.setRenderHint(QPainter::SmoothPixmapTransform);
            g.translate(-bounds.topLeft());
            g.setTransform(t, true);
            g.drawImage(0, 0, tip);
            g.end();
            tip = rotated;
        }
    }
    return tip;
}

// Textura de papel em tons de cinza (lê tanto textura em luminância quanto no canal alfa)
QImage makeTexturePattern(const SutBrush &brush)
{
    QImage source = QImage::fromData(brush.texturePng, "PNG").convertToFormat(QImage::Format_ARGB32);
    if (source.isNull()) return QImage();

    QImage pat(source.size(), QImage::Format_Grayscale8);
    for (int y = 0; y < source.height(); ++y) {
        const QRgb *src = reinterpret_cast<const QRgb*>(source.constScanLine(y));
        uchar *dst = pat.scanLine(y);
        for (int x = 0; x < source.width(); ++x) {
            const int alpha = qAlpha(src[x]);
            dst[x] = uchar(255 - alpha * (255 - qGray(src[x])) / 255);
        }
    }
    if (!qFuzzyIsNull(std::fmod(brush.textureRotate, 360.0))) {
        QImage rotated(pat.size(), QImage::Format_Grayscale8);
        rotated.fill(Qt::white);
        QPainter g(&rotated);
        g.setRenderHint(QPainter::SmoothPixmapTransform);
        g.translate(pat.width() * 0.5, pat.height() * 0.5);
        g.rotate(-brush.textureRotate);
        g.drawImage(-pat.width() * 0.5, -pat.height() * 0.5, pat);
        g.end();
        pat = rotated;
    }
    return pat;
}

QString asciiSlug(const QString &name)
{
    QString slug;
    for (const QChar c : name) {
        if (c.unicode() < 128 && (c.isLetterOrNumber() || c == QLatin1Char('-') || c == QLatin1Char('_'))) {
            slug.append(c);
        } else if (c.isSpace()) {
            slug.append(QLatin1Char('_'));
        }
    }
    const QString hash = QString::fromLatin1(
                QCryptographicHash::hash(name.toUtf8(), QCryptographicHash::Md5).toHex().left(8));
    return (slug.isEmpty() ? QStringLiteral("pincel") : slug.left(40)) + QLatin1Char('_') + hash;
}

QString cdata(double v)
{
    return QString::number(v, 'g', 6);
}

QString formatKritaCurve(const QVector<QPointF> &pts, double minVal, bool invertY = false)
{
    QVector<QPointF> src = pts;
    if (src.size() < 2) {
        src = {QPointF(0.0, 0.0), QPointF(1.0, 1.0)};
    }
    QStringList parts;
    double prevX = -1.0;
    for (const QPointF &pt : src) {
        double x = qBound(0.0, pt.x(), 1.0);
        if (x <= prevX) x = qMin(1.0, prevX + 0.001);
        prevX = x;
        const double rawY = invertY ? (1.0 - qBound(0.0, pt.y(), 1.0)) : qBound(0.0, pt.y(), 1.0);
        const double y = qBound(0.0, minVal + (1.0 - minVal) * rawY, 1.0);
        parts << QStringLiteral("%1,%2").arg(QString::number(x, 'g', 4), QString::number(y, 'g', 4));
    }
    return parts.join(QLatin1Char(';')) + QLatin1Char(';');
}

// Traduz um SutEffector do Clip Studio para o XML de sensores do Krita.
bool applyCurveOption(KisPaintOpSettingsSP settings, const QString &optName,
                      const SutEffector &eff, bool fallbackPressure, double fallbackMin)
{
    QStringList sensors;
    int curveIdx = 0;
    auto nextCurve = [&eff, &curveIdx]() -> QVector<QPointF> {
        if (curveIdx < eff.curves.size()) return eff.curves.at(curveIdx++);
        return QVector<QPointF>();
    };

    if (eff.valid && eff.onMask != 0) {
        if (eff.onMask & 0x10) {
            const QVector<QPointF> pts = nextCurve();
            if (eff.pressureMin < 0.99) {
                sensors << QStringLiteral("<ChildSensor id=\"pressure\"><curve>%1</curve></ChildSensor>")
                               .arg(formatKritaCurve(pts, eff.pressureMin));
            }
        }
        if (eff.onMask & 0x20) {
            const QVector<QPointF> pts = nextCurve();
            if (eff.tiltMin < 0.99) {
                // API não verificada no cabeçalho do KRITA_REF: conferir TiltElevationId ("declination") em kis_dynamic_sensor.h
                sensors << QStringLiteral("<ChildSensor id=\"declination\"><curve>%1</curve></ChildSensor>")
                               .arg(formatKritaCurve(pts, eff.tiltMin));
            }
        }
        if (eff.onMask & 0x40) {
            const QVector<QPointF> pts = nextCurve();
            if (eff.speedMin < 0.99) {
                sensors << QStringLiteral("<ChildSensor id=\"speed\"><curve>%1</curve></ChildSensor>")
                               .arg(formatKritaCurve(pts, eff.speedMin));
            }
        }
        if (eff.onMask & 0x80) {
            if (eff.randomMin < 0.99) {
                sensors << QStringLiteral("<ChildSensor id=\"fuzzy\"><curve>0,%1;1,1;</curve></ChildSensor>")
                               .arg(QString::number(eff.randomMin, 'g', 4));
            }
        }
    }

    if (sensors.isEmpty() && fallbackPressure) {
        sensors << QStringLiteral("<ChildSensor id=\"pressure\"><curve>0,%1;1,1;</curve></ChildSensor>")
                       .arg(QString::number(fallbackMin, 'g', 4));
    }

    const QString pressureKey = QStringLiteral("Pressure") + optName;
    const QString useCurveKey = optName + QStringLiteral("UseCurve");
    const QString sameCurveKey = optName + QStringLiteral("UseSameCurve");
    const QString sensorKey = optName + QStringLiteral("Sensor");

    if (sensors.isEmpty()) {
        settings->setProperty(pressureKey, "false");
        return false;
    }

    settings->setProperty(pressureKey, "true");
    settings->setProperty(useCurveKey, "true");
    settings->setProperty(sameCurveKey, sensors.size() > 1 ? "false" : "true");
    settings->setProperty(sensorKey,
                          QStringLiteral("<!DOCTYPE params> <params id=\"sensorslist\"> %1 </params> ")
                              .arg(sensors.join(QLatin1Char(' '))));
    return true;
}

// Procura um recurso já existente pelo nome de arquivo para evitar avisos de "Substituir?" ao reimportar.
KoResourceSP findExistingResourceByFilename(const QString &resourceType, const QString &filename)
{
    if (filename.isEmpty()) return KoResourceSP();
    KisResourceModel model(resourceType);
    for (int row = 0; row < model.rowCount(); ++row) {
        const QString fn = model.index(row, KisAbstractResourceModel::Filename).data().toString();
        if (fn.compare(filename, Qt::CaseInsensitive) == 0) {
            return model.resourceForIndex(model.index(row, 0));
        }
    }
    return KoResourceSP();
}

bool tagPresetAsNanquim(const KoResourceSP &resource, const QString &fallbackFilename = QString())
{
    int id = resource ? resource->resourceId() : -1;
    if (id < 0 && !fallbackFilename.isEmpty()) {
        KisResourceModel model(ResourceType::PaintOpPresets);
        for (int row = 0; row < model.rowCount(); ++row) {
            const QString fn = model.index(row, KisAbstractResourceModel::Filename).data().toString();
            if (fn.compare(fallbackFilename, Qt::CaseInsensitive) == 0) {
                id = model.index(row, KisAbstractResourceModel::Id).data().toInt();
                break;
            }
        }
    }
    if (id < 0) return false;
    KisTagModel tagModel(ResourceType::PaintOpPresets);
    KisTagSP tag = tagModel.tagForUrl(QStringLiteral("Nanquim"));
    if (!tag) tag = tagModel.addTag(QStringLiteral("Nanquim"), false, QVector<KoResourceSP>());
    if (!tag) return false;
    KisTagResourceModel tagResourceModel(ResourceType::PaintOpPresets);
    return tagResourceModel.tagResources(tag, QVector<int>() << id);
}

// Lê os nomes das entradas do diretório central de um arquivo ZIP (Qt puro).
QStringList listZipEntries(const QString &zipPath)
{
    QStringList names;
    QFile f(zipPath);
    if (!f.open(QIODevice::ReadOnly)) return names;
    const qint64 fileSize = f.size();
    if (fileSize < 22) return names;

    const qint64 tailSize = qMin<qint64>(fileSize, 65557);
    if (!f.seek(fileSize - tailSize)) return names;
    const QByteArray tail = f.read(tailSize);

    const QByteArray eocdSig("PK\x05\x06", 4);
    const int eocdRel = tail.lastIndexOf(eocdSig);
    if (eocdRel < 0 || eocdRel + 22 > tail.size()) return names;

    const uchar *eocd = reinterpret_cast<const uchar*>(tail.constData() + eocdRel);
    const quint32 cdSize = qFromLittleEndian<quint32>(eocd + 12);
    const quint32 cdOffset = qFromLittleEndian<quint32>(eocd + 16);
    if (cdSize == 0 || qint64(cdOffset) + qint64(cdSize) > fileSize) return names;

    if (!f.seek(cdOffset)) return names;
    const QByteArray cd = f.read(cdSize);
    int pos = 0;
    while (pos + 46 <= cd.size()) {
        const uchar *hdr = reinterpret_cast<const uchar*>(cd.constData() + pos);
        if (hdr[0] != 'P' || hdr[1] != 'K' || hdr[2] != 1 || hdr[3] != 2) break;
        const quint16 flags = qFromLittleEndian<quint16>(hdr + 8);
        const quint16 nameLen = qFromLittleEndian<quint16>(hdr + 28);
        const quint16 extraLen = qFromLittleEndian<quint16>(hdr + 30);
        const quint16 cmtLen = qFromLittleEndian<quint16>(hdr + 32);
        if (pos + 46 + nameLen > cd.size()) break;
        const QByteArray rawName = cd.mid(pos + 46, nameLen);
        const QString entry = (flags & 0x0800) ? QString::fromUtf8(rawName)
                                               : QString::fromLocal8Bit(rawName);
        names << entry;
        pos += 46 + nameLen + extraLen + cmtLen;
    }
    return names;
}

} // namespace

// Importa uma subferramenta (variante) do .sut; variantCount recebe quantas existem no arquivo.
static bool importVariant(QWidget *parent, const QString &path, int variantIndex, QString *message, int *variantCount)
{
    SutBrush brush;
    QString error;
    if (!readSut(path, &brush, &error, variantIndex, variantCount)) {
        *message = QStringLiteral("Não consegui importar %1: %2.").arg(QFileInfo(path).fileName(), error);
        KisUsageLogger::log(QStringLiteral("[Nanquim] .sut falhou: %1").arg(error));
        return false;
    }

    // Diagnóstico: tudo o que foi lido do .sut vai para o KritaUsageLog (Downloads)
    KisUsageLogger::log(QStringLiteral("[Nanquim] .sut lido: \"%1\" tam=%2 opac=%3 fluxo=%4 dureza=%5 intervalo=%6/tipo%7 "
                                       "espessura=%8 vertical=%9 rot=%10 efetorRot=0x%11 imagem=%12 fita=%13 "
                                       "textura=%14 modoTex=%15 dens=%16 curvas(tam=%17 opac=%18 fluxo=%19)")
                        .arg(brush.name).arg(brush.size).arg(brush.opacity).arg(brush.flow).arg(brush.hardness)
                        .arg(brush.interval).arg(brush.intervalType).arg(brush.thickness).arg(brush.verticalThickness)
                        .arg(brush.rotation).arg(QString::number(brush.rotationEffector, 16)).arg(brush.usePattern)
                        .arg(brush.ribbon).arg(brush.hasTexture).arg(brush.textureCompositeMode)
                        .arg(brush.textureDensity).arg(brush.sizeEff.valid).arg(brush.opacityEff.valid)
                        .arg(brush.flowEff.valid));

    KisPaintOpPresetResourceServer *server = KisResourceServerProvider::instance()->paintOpPresetServer();
    KisPaintOpPresetSP base = server->resource(QString(), QStringLiteral("d)_Ink-2_Fineliner.kpp"), QString());
    if (!base) {
        base = server->resource(QString(), QStringLiteral("b)_Basic-5_Size_default.kpp"), QString());
    }
    if (!base) {
        *message = QStringLiteral("Não encontrei um pincel base do Krita para converter o .sut.");
        return false;
    }

    const QString slug = asciiSlug(brush.name) + (variantIndex > 0 ? QStringLiteral("_v%1").arg(variantIndex) : QString());
    const QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
            + QStringLiteral("/nanquim-sut");
    QDir().mkpath(cacheDir);

    QString definition;
    QStringList details;
    QImage thumbnailSource;

    // Espaçamento: fixo quando o Clip Studio diz; senão mapeia Largo (1), Normal (2), Estreito (3)
    double spacing = 0.08;
    if (brush.intervalType == 0) {
        spacing = qBound(0.02, brush.interval / 100.0, 3.0);
    } else if (brush.intervalType == 1) {
        spacing = qBound(0.04, brush.thickness / 100.0 * 0.25, 0.14);
    } else if (brush.intervalType == 3) {
        spacing = 0.03;
    } else {
        spacing = qBound(0.03, brush.thickness / 100.0 * 0.12, 0.08);
    }

    bool tipDone = false;
    bool isColorStamp = false;
    if (brush.usePattern && !brush.tipPng.isEmpty()) {
        int baseSize = 1;
        const QImage tip = makeTip(brush, &baseSize, &isColorStamp);
        if (!tip.isNull()) {
            const double minSide = qMin(tip.width(), tip.height());
            const double maxSide = qMax(1, qMax(tip.width(), tip.height()));
            const double aspect = minSide / maxSide;
            if (brush.ribbon) {
                spacing = qBound(0.04, aspect * 0.65, 0.22);
            } else if (aspect < 0.25) {
                spacing = qBound(0.015, aspect * 0.45, spacing);
            }

            QByteArray pngBytes;
            {
                QBuffer buf(&pngBytes);
                buf.open(QIODevice::WriteOnly);
                tip.save(&buf, "PNG");
            }
            const QString tipHash = QString::fromLatin1(
                        QCryptographicHash::hash(pngBytes, QCryptographicHash::Md5).toHex().left(8));
            const QString tipBaseName = QStringLiteral("sut_") + slug + QLatin1Char('_') + tipHash + QStringLiteral(".png");
            const QString tipFile = cacheDir + QLatin1Char('/') + tipBaseName;
            KoResourceSP tipResource = findExistingResourceByFilename(ResourceType::Brushes, tipBaseName);
            if (!tipResource) {
                QFile tf(tipFile);
                if (tf.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                    tf.write(pngBytes);
                    tf.close();
                    tipResource = KisResourceUserOperations::importResourceFileWithUserInput(
                                parent, QString(), ResourceType::Brushes, tipFile);
                }
            }
            {
                if (tipResource) {
                    const double scale = brush.size / qMax(1, baseSize);
                    const int appMode = isColorStamp ? 1 : 0;
                    definition = QStringLiteral(
                                "<Brush type=\"png_brush\" BrushVersion=\"2\" filename=\"%1\" md5sum=\"%2\" "
                                "spacing=\"%3\" useAutoSpacing=\"0\" autoSpacingCoeff=\"1\" angle=\"0\" "
                                "scale=\"%4\" brushApplication=\"%5\"/>")
                            .arg(tipResource->filename().toHtmlEscaped(), tipResource->md5Sum(),
                                 cdata(spacing), cdata(scale))
                            .arg(appMode);
                    thumbnailSource = tip;
                    if (isColorStamp) details << QStringLiteral("ponta com preenchimento");
                    else details << (brush.ribbon ? QStringLiteral("ponta em fita") : QStringLiteral("ponta por imagem"));
                    tipDone = true;
                }
            }
        }
    }
    if (!tipDone) {
        const double fade = qBound(0.05, brush.hardness / 100.0, 1.0);
        const double ratio = qBound(0.05, brush.thickness / 100.0, 1.0);
        const double angleDeg = brush.verticalThickness ? brush.rotation : (brush.rotation + 90.0);
        definition = QStringLiteral(
                    "<Brush type=\"auto_brush\" BrushVersion=\"2\" spacing=\"%1\" useAutoSpacing=\"0\" "
                    "autoSpacingCoeff=\"1\" angle=\"%2\" randomness=\"0\" density=\"1\">"
                    "<MaskGenerator type=\"circle\" id=\"default\" diameter=\"%3\" ratio=\"%4\" "
                    "hfade=\"%5\" vfade=\"%5\" spikes=\"2\" antialiasEdges=\"%6\"/></Brush>")
                .arg(cdata(qMax(spacing, 0.04)), cdata(qDegreesToRadians(angleDeg)), cdata(brush.size),
                     cdata(ratio), cdata(fade))
                .arg(brush.antiAlias ? 1 : 0);
        details << QStringLiteral("ponta elíptica");
    }

    const QString presetFilename = QStringLiteral("sut_") + slug + QStringLiteral(".kpp");
    KisPaintOpPresetSP existing = server->resource(QString(), presetFilename, QString());
    if (!existing) {
        existing = findExistingResourceByFilename(ResourceType::PaintOpPresets, presetFilename).dynamicCast<KisPaintOpPreset>();
    }
    KisPaintOpPresetSP preset = base->clone().dynamicCast<KisPaintOpPreset>();
    if (!preset || !preset->settings()) {
        *message = QStringLiteral("Não consegui criar o pincel a partir do pincel base.");
        return false;
    }
    preset->setResourceId(existing ? existing->resourceId() : -1);
    preset->setName(brush.name);
    preset->setFilename(presetFilename);
    preset->setStorageLocation(existing ? existing->storageLocation() : QString());
    preset->setValid(true);

    KisPaintOpSettingsSP settings = preset->settings();
    settings->setProperty("brush_definition", definition);
    settings->setProperty("OpacityValue", cdata(brush.opacity / 100.0));
    settings->setProperty("FlowValue", cdata(brush.flow / 100.0));
    settings->setProperty("Spacing/Isotropic", "true");
    settings->setProperty("PressureSpacing", "false");
    // G-Pens clássicas do Clip Studio usam AntiAlias=0 (borda crocante de nanquim)
    if (!brush.antiAlias && !isColorStamp) {
        settings->setProperty("Sharpness/useSharpness", "true");
        settings->setProperty("Sharpness/threshold", "0.5");
        settings->setProperty("Sharpness/alignOutlinePixels", "true");
        details << QStringLiteral("borda nítida");
    } else {
        settings->setProperty("Sharpness/useSharpness", "false");
    }

    const bool sizeDyn = applyCurveOption(settings, QStringLiteral("Size"), brush.sizeEff, true, 0.15);
    applyCurveOption(settings, QStringLiteral("Opacity"), brush.opacityEff, false, 0.0);
    applyCurveOption(settings, QStringLiteral("Flow"), brush.flowEff, false, 0.0);
    if (!brush.ribbon && !isColorStamp) {
        applyCurveOption(settings, QStringLiteral("Ratio"), brush.thicknessEff, false, 0.0);
    }
    if (sizeDyn && brush.sizeEff.valid && brush.sizeEff.onMask != 0) {
        details << QStringLiteral("curva real");
    }

    // BrushRotationEffector é um conjunto de bits: 3 = fixo (padrão, sem rotação dinâmica),
    // 0x04 = direção do traço, 0x08 = inclinação, 0x80 = aleatório.
    const int eff = brush.rotationEffector;
    const bool followStroke = brush.ribbon || ((eff & 0x04) != 0);
    const bool tiltRot = !brush.ribbon && !followStroke && ((eff & 0x08) != 0);
    const bool randomRot = !brush.ribbon && ((eff & 0x80) != 0) && brush.rotationRandomScale > 1.0;

    QStringList rotSensors;
    if (followStroke) {
        rotSensors << QStringLiteral("<ChildSensor id=\"drawingangle\"/>");
        details << QStringLiteral("direção do traço");
    } else if (tiltRot) {
        // API não verificada no cabeçalho do KRITA_REF: conferir TiltDirectionId ("ascension") em kis_dynamic_sensor.h
        rotSensors << QStringLiteral("<ChildSensor id=\"ascension\"/>");
    }
    if (randomRot) {
        rotSensors << QStringLiteral("<ChildSensor id=\"fuzzy\"/>");
        details << QStringLiteral("rotação aleatória");
    }

    if (!rotSensors.isEmpty()) {
        settings->setProperty("PressureRotation", "true");
        // com direção/inclinação o valor é 1 (volta inteira); só o aleatório puro usa a escala
        settings->setProperty("RotationValue", (randomRot && !followStroke && !tiltRot)
                              ? cdata(qBound(0.05, brush.rotationRandomScale / 100.0, 1.0))
                              : QStringLiteral("1"));
        settings->setProperty("RotationUseCurve", "false");
        settings->setProperty("RotationUseSameCurve", rotSensors.size() > 1 ? "false" : "true");
        settings->setProperty("RotationSensor",
                              QStringLiteral("<!DOCTYPE params> <params id=\"sensorslist\"> %1 </params> ")
                                  .arg(rotSensors.join(QLatin1Char(' '))));
    } else {
        settings->setProperty("PressureRotation", "false");
    }

    if (brush.hasTexture && !brush.texturePng.isEmpty()) {
        const QImage patImg = makeTexturePattern(brush);
        if (!patImg.isNull()) {
            QByteArray patBytes;
            {
                QBuffer buf(&patBytes);
                buf.open(QIODevice::WriteOnly);
                patImg.save(&buf, "PNG");
            }
            const QString patHash = QString::fromLatin1(
                        QCryptographicHash::hash(patBytes, QCryptographicHash::Md5).toHex().left(8));
            const QString patBaseName = QStringLiteral("sut_tex_") + slug + QLatin1Char('_') + patHash + QStringLiteral(".png");
            const QString patFile = cacheDir + QLatin1Char('/') + patBaseName;
            KoResourceSP patRes = findExistingResourceByFilename(ResourceType::Patterns, patBaseName);
            if (!patRes) {
                QFile pf(patFile);
                if (pf.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                    pf.write(patBytes);
                    pf.close();
                    patRes = KisResourceUserOperations::importResourceFileWithUserInput(
                                parent, QString(), ResourceType::Patterns, patFile);
                }
            }
            {
                if (patRes) {
                    const QString md5Hex = patRes->md5Sum();
                    const QString md5B64 = QString::fromLatin1(
                                QByteArray::fromHex(md5Hex.toLatin1()).toBase64());
                    const int texMode = (brush.textureCompositeMode == 2 || brush.textureCompositeMode == 4) ? 1 : 0;
                    const double bright = qBound(-1.0, brush.textureBrightness / 100.0, 1.0);
                    const double contr = qBound(0.05, 1.0 + brush.textureContrast / 100.0, 2.0);
                    const double strength = qBound(0.10, brush.textureDensity / 100.0, 1.0);

                    settings->setProperty("Texture/Pattern/Enabled", "true");
                    settings->setProperty("Texture/Pattern/PatternMD5Sum", md5Hex);
                    settings->setProperty("Texture/Pattern/PatternMD5", md5B64);
                    settings->setProperty("Texture/Pattern/PatternFileName", patRes->filename());
                    settings->setProperty("Texture/Pattern/Name", patRes->name());
                    settings->setProperty("Texture/Pattern/Scale", cdata(brush.textureScale / 100.0));
                    settings->setProperty("Texture/Pattern/Brightness", cdata(bright));
                    settings->setProperty("Texture/Pattern/Contrast", cdata(contr));
                    settings->setProperty("Texture/Pattern/NeutralPoint", "0.5");
                    settings->setProperty("Texture/Pattern/Invert", brush.textureInvert ? "true" : "false");
                    settings->setProperty("Texture/Pattern/CutoffLeft", "0");
                    settings->setProperty("Texture/Pattern/CutoffRight", "255");
                    settings->setProperty("Texture/Pattern/CutoffPolicy", "0");
                    settings->setProperty("Texture/Pattern/TexturingMode", QString::number(texMode));
                    settings->setProperty("Texture/Pattern/MaximumOffsetX", QString::number(qMax(1, patImg.width() / 2)));
                    settings->setProperty("Texture/Pattern/MaximumOffsetY", QString::number(qMax(1, patImg.height() / 2)));
                    settings->setProperty("Texture/Pattern/OffsetX", "0");
                    settings->setProperty("Texture/Pattern/OffsetY", "0");
                    settings->setProperty("Texture/Pattern/isRandomOffsetX", "false");
                    settings->setProperty("Texture/Pattern/isRandomOffsetY", "false");
                    settings->setProperty("StrengthValue", cdata(strength));
                    applyCurveOption(settings, QStringLiteral("Strength"), brush.textureEff, false, 0.0);
                    details << QStringLiteral("textura de papel");
                }
            }
        }
    }

    QImage thumbnail(200, 200, QImage::Format_ARGB32);
    thumbnail.fill(Qt::white);
    {
        QPainter g(&thumbnail);
        g.setRenderHint(QPainter::SmoothPixmapTransform);
        g.setRenderHint(QPainter::Antialiasing);
        if (!thumbnailSource.isNull()) {
            const QImage fit = thumbnailSource.scaled(150, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            g.drawImage((200 - fit.width()) / 2, (200 - fit.height()) / 2, fit);
        } else {
            g.setPen(Qt::NoPen);
            g.setBrush(Qt::black);
            g.translate(100, 100);
            g.rotate(-brush.rotation);
            const double ratio = qBound(0.05, brush.thickness / 100.0, 1.0);
            if (brush.verticalThickness) g.drawEllipse(QPointF(0, 0), 60.0, 60.0 * ratio);
            else g.drawEllipse(QPointF(0, 0), 60.0 * ratio, 60.0);
        }
    }
    preset->setImage(thumbnail);

    const bool saved = existing
            ? KisResourceUserOperations::updateResourceWithUserInput(parent, preset)
            : KisResourceUserOperations::addResourceWithUserInput(parent, preset);
    if (!saved) {
        *message = QStringLiteral("O pincel \"%1\" não foi salvo.").arg(brush.name);
        return false;
    }

    const bool tagged = tagPresetAsNanquim(preset, presetFilename);
    const QString summary = details.join(QStringLiteral(", "));
    *message = QStringLiteral("Pincel do Clip Studio importado: %1\n%2 · %3 px.\n%4")
            .arg(brush.name, summary, QString::number(qRound(brush.size)),
                 tagged ? QStringLiteral("Ele já está em Pincéis → Importados / Nanquim.")
                        : QStringLiteral("Ele está em \"Tudo\", na janela de pincéis."));
    KisUsageLogger::log(QStringLiteral("[Nanquim] .sut importado: %1 (%2, %3 px, tex=%4)")
                        .arg(brush.name, summary).arg(brush.size).arg(brush.hasTexture));
    return true;
}

bool NanquimSutImport::importFile(QWidget *parent, const QString &path, QString *message)
{
    int count = 1;
    QString firstMessage;
    if (!importVariant(parent, path, 0, &firstMessage, &count)) {
        *message = firstMessage;
        return false;
    }
    if (count <= 1) {
        *message = firstMessage;
        return true;
    }
    // vários pincéis no mesmo .sut: importa todos
    int ok = 1;
    int failed = 0;
    for (int i = 1; i < qMin(count, 40); ++i) {
        QString msg;
        int ignored = 0;
        if (importVariant(parent, path, i, &msg, &ignored)) ++ok;
        else ++failed;
    }
    *message = QStringLiteral("%1 pincéis importados de %2%3.\nEles estão em Pincéis → Importados / Nanquim.\n\nPrimeiro:\n%4")
            .arg(ok).arg(QFileInfo(path).fileName(),
                         failed ? QStringLiteral(" (%1 falharam)").arg(failed) : QString(), firstMessage);
    return true;
}

bool NanquimSutImport::importZipArchive(QWidget *parent, const QString &zipPath, QString *message)
{
    const QString zipName = QFileInfo(zipPath).fileName();
    const QStringList entries = listZipEntries(zipPath);
    if (entries.isEmpty()) {
        *message = QStringLiteral("Não consegui ler o conteúdo de %1 (arquivo .zip inválido ou vazio).").arg(zipName);
        return false;
    }

    QStringList supported;
    for (const QString &entry : entries) {
        if (entry.endsWith(QLatin1Char('/')) || entry.startsWith(QLatin1String("__MACOSX/"))) continue;
        const QString base = QFileInfo(entry).fileName();
        if (base.isEmpty() || base.startsWith(QLatin1String("._"))) continue;
        const QString ext = QFileInfo(base).suffix().toLower();
        if (ext == QLatin1String("sut") || ext == QLatin1String("kpp") || ext == QLatin1String("bundle")) {
            supported << entry;
        }
    }

    if (supported.isEmpty()) {
        *message = QStringLiteral("Nenhum pincel (.sut, .kpp ou .bundle) foi encontrado dentro de %1.").arg(zipName);
        return false;
    }

    QScopedPointer<KoStore> store(KoStore::createStore(zipPath, KoStore::Read, QByteArray(), KoStore::Zip));
    if (!store || store->bad()) {
        *message = QStringLiteral("Não consegui abrir o arquivo compactado %1.").arg(zipName);
        return false;
    }

    const QString tmpDirPath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
            + QStringLiteral("/nanquim-zip-") + QUuid::createUuid().toString(QUuid::Id128);
    QDir().mkpath(tmpDirPath);

    int okSut = 0;
    int okKpp = 0;
    int okBundle = 0;
    int failed = 0;
    QStringList namesImported;

    for (const QString &entry : supported) {
        if (!store->open(entry)) {
            ++failed;
            continue;
        }
        const QByteArray payload = store->device() ? store->device()->readAll() : QByteArray();
        store->close();
        if (payload.isEmpty()) {
            ++failed;
            continue;
        }

        const QString baseName = QFileInfo(entry).fileName();
        const QString ext = QFileInfo(baseName).suffix().toLower();
        const QString outFile = tmpDirPath + QLatin1Char('/') + baseName;
        QFile out(outFile);
        if (!out.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            ++failed;
            continue;
        }
        out.write(payload);
        out.close();

        if (ext == QLatin1String("sut")) {
            QString singleMsg;
            if (importFile(parent, outFile, &singleMsg)) {
                ++okSut;
                namesImported << QFileInfo(baseName).completeBaseName();
            } else {
                ++failed;
            }
        } else if (ext == QLatin1String("kpp")) {
            KoResourceSP res = KisResourceUserOperations::importResourceFileWithUserInput(
                        parent, QString(), ResourceType::PaintOpPresets, outFile);
            if (res) {
                tagPresetAsNanquim(res);
                ++okKpp;
                namesImported << res->name();
            } else {
                ++failed;
            }
        } else if (ext == QLatin1String("bundle")) {
            if (KisStorageModel::instance()->importStorage(outFile, KisStorageModel::None)) {
                ++okBundle;
                namesImported << baseName;
            } else {
                ++failed;
            }
        }
    }

    QDir(tmpDirPath).removeRecursively();

    const int totalOk = okSut + okKpp + okBundle;
    if (totalOk == 0) {
        *message = QStringLiteral("Encontrei %1 arquivo(s) em %2, mas nenhum pôde ser importado.")
                .arg(supported.size()).arg(zipName);
        return false;
    }

    QStringList counts;
    if (okSut > 0) counts << QStringLiteral("%1 .sut").arg(okSut);
    if (okKpp > 0) counts << QStringLiteral("%1 .kpp").arg(okKpp);
    if (okBundle > 0) counts << QStringLiteral("%1 .bundle").arg(okBundle);

    QString previewNames;
    if (!namesImported.isEmpty()) {
        const QStringList first = namesImported.mid(0, 5);
        previewNames = QStringLiteral("\n") + first.join(QStringLiteral(", "));
        if (namesImported.size() > 5) {
            previewNames += QStringLiteral(" (+%1)").arg(namesImported.size() - 5);
        }
    }
    const QString failNote = failed > 0
            ? QStringLiteral("\n(%1 arquivo(s) ignorado(s) ou já instalados.)").arg(failed)
            : QString();

    *message = QStringLiteral("Pacote .zip importado (%1): %2.%3%4\nOs pincéis já estão na janela de Pincéis.")
            .arg(zipName, counts.join(QStringLiteral(" · ")), previewNames, failNote);
    KisUsageLogger::log(QStringLiteral("[Nanquim] .zip importado: %1 (ok=%2, falhas=%3)")
                        .arg(zipName).arg(totalOk).arg(failed));
    return true;
}

package org.krita.android;

import android.app.Activity;
import android.content.ContentValues;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.RectF;
import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;
import android.media.MediaMuxer;
import android.net.Uri;
import android.os.Build;
import android.os.Environment;
import android.provider.MediaStore;
import android.util.Log;
import android.widget.Toast;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.text.SimpleDateFormat;
import java.util.Arrays;
import java.util.Date;
import java.util.Locale;

/**
 * Nanquim: monta o vídeo de time-lapse (MP4/H.264) a partir dos quadros que o
 * app grava enquanto o artista desenha. Usa o codificador de vídeo do próprio
 * aparelho (MediaCodec), já que o FFmpeg do Krita não existe no Android.
 */
public final class NanquimTimelapse {
    private static final String TAG = "nanquim.Timelapse";
    private static final int FPS = 30;
    private static boolean sBusy;

    private NanquimTimelapse() {}

    /** targetSeconds <= 0: usa todos os quadros. */
    public static void export(final Activity a, final String dir, final String title, final int targetSeconds) {
        if (a == null) {
            return;
        }
        synchronized (NanquimTimelapse.class) {
            if (sBusy) {
                toast(a, "Já existe um time-lapse sendo gerado");
                return;
            }
            sBusy = true;
        }
        toast(a, "Gerando o time-lapse… pode continuar desenhando");
        new Thread(() -> {
            try {
                String where = run(a, dir, title, targetSeconds);
                toast(a, where == null ? "Ainda não há quadros gravados neste desenho"
                                       : "Time-lapse salvo em " + where);
            } catch (Throwable t) {
                Log.e(TAG, "falha ao gerar o time-lapse", t);
                toast(a, "Não consegui gerar o time-lapse: " + t.getMessage());
            } finally {
                synchronized (NanquimTimelapse.class) {
                    sBusy = false;
                }
            }
        }, "nanquim-timelapse").start();
    }

    private static void toast(final Activity a, final String text) {
        a.runOnUiThread(() -> Toast.makeText(a, text, Toast.LENGTH_LONG).show());
    }

    private static String run(Activity a, String dir, String title, int targetSeconds) throws Exception {
        File[] frames = new File(dir).listFiles((d, name) -> name.startsWith("frame_") && name.endsWith(".jpg"));
        if (frames == null || frames.length == 0) {
            return null;
        }
        Arrays.sort(frames);
        final int n = frames.length;

        // tamanho do vídeo: o do último quadro, em múltiplos de 16 (o que todo codificador aceita)
        BitmapFactory.Options bounds = new BitmapFactory.Options();
        bounds.inJustDecodeBounds = true;
        BitmapFactory.decodeFile(frames[n - 1].getPath(), bounds);
        final int w = Math.max(64, bounds.outWidth & ~15);
        final int h = Math.max(64, bounds.outHeight & ~15);

        // quais quadros entram, e quantas vezes cada um aparece
        final int count;      // quadros de origem usados
        final int repeat;     // quadros de vídeo por quadro de origem
        if (targetSeconds <= 0) {
            count = n;
            repeat = 3;       // completo: 10 traços por segundo
        } else {
            final int wanted = FPS * targetSeconds;
            if (n >= wanted) {
                count = wanted;
                repeat = 1;
            } else {
                count = n;
                repeat = Math.max(1, Math.min(8, (int) Math.ceil(wanted / (double) n)));
            }
        }
        final int tail = FPS * 2;   // segura o desenho final por 2 s

        File tmp = new File(a.getCacheDir(), "nanquim-timelapse.mp4");
        if (tmp.exists()) {
            tmp.delete();
        }

        MediaFormat format = MediaFormat.createVideoFormat(MediaFormat.MIMETYPE_VIDEO_AVC, w, h);
        format.setInteger(MediaFormat.KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420Flexible);
        format.setInteger(MediaFormat.KEY_BIT_RATE, Math.max(4_000_000, Math.min(16_000_000, w * h * 5)));
        format.setInteger(MediaFormat.KEY_FRAME_RATE, FPS);
        format.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, 1);

        MediaCodec encoder = MediaCodec.createEncoderByType(MediaFormat.MIMETYPE_VIDEO_AVC);
        MediaMuxer muxer = null;
        long videoFrames = 0;
        try {
            encoder.configure(format, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);
            encoder.start();
            muxer = new MediaMuxer(tmp.getPath(), MediaMuxer.OutputFormat.MUXER_OUTPUT_MPEG_4);
            State st = new State(encoder, muxer);

            Yuv yuv = new Yuv(w, h);
            Bitmap canvasBitmap = Bitmap.createBitmap(w, h, Bitmap.Config.ARGB_8888);
            Canvas canvas = new Canvas(canvasBitmap);
            Paint paint = new Paint(Paint.FILTER_BITMAP_FLAG);
            for (int i = 0; i < count; i++) {
                int src = (int) ((long) i * n / count);
                if (i == count - 1) {
                    src = n - 1;   // o vídeo sempre termina no desenho pronto
                }
                Bitmap bitmap = BitmapFactory.decodeFile(frames[src].getPath());
                if (bitmap == null) {
                    continue;
                }
                canvas.drawColor(Color.WHITE);
                float scale = Math.min(w / (float) bitmap.getWidth(), h / (float) bitmap.getHeight());
                float bw = bitmap.getWidth() * scale;
                float bh = bitmap.getHeight() * scale;
                float left = (w - bw) / 2f;
                float top = (h - bh) / 2f;
                canvas.drawBitmap(bitmap, null, new RectF(left, top, left + bw, top + bh), paint);
                bitmap.recycle();
                yuv.fromBitmap(canvasBitmap);

                int times = repeat + (i == count - 1 ? tail : 0);
                for (int k = 0; k < times; k++) {
                    // cada quadro entra com o seu tempo exato: quadro N = N/30 s
                    queueFrame(st, yuv, videoFrames * 1_000_000L / FPS);
                    videoFrames++;
                }
            }
            canvasBitmap.recycle();
            // fim do vídeo
            while (true) {
                int in = encoder.dequeueInputBuffer(10000);
                if (in >= 0) {
                    encoder.queueInputBuffer(in, 0, 0, videoFrames * 1_000_000L / FPS,
                            MediaCodec.BUFFER_FLAG_END_OF_STREAM);
                    break;
                }
                drain(st, false);
            }
            drain(st, true);
            Log.i(TAG, "time-lapse: " + n + " quadros gravados, " + videoFrames + " quadros de vídeo, "
                    + st.outFrames + " codificados");
        } finally {
            try { encoder.stop(); } catch (Exception ignored) { }
            encoder.release();
            if (muxer != null) {
                try { muxer.stop(); } catch (Exception ignored) { }
                muxer.release();
            }
        }

        String safe = (title == null || title.trim().isEmpty() ? "desenho" : title.trim())
                .replaceAll("[^\\p{L}\\p{N} _-]", "_");
        String name = "Nanquim " + safe + " "
                + new SimpleDateFormat("yyyy-MM-dd HH.mm.ss", Locale.ROOT).format(new Date()) + ".mp4";
        String where = publish(a, tmp, name);
        tmp.delete();
        return where + " (" + (videoFrames / FPS) + " s, " + n + " quadros)";
    }

    /** Um quadro em YUV 4:2:0 (BT.601), pronto para copiar para o codificador. */
    private static final class Yuv {
        final int w, h;
        final byte[] y, u, v;
        final int[] argb;
        Yuv(int w, int h) {
            this.w = w;
            this.h = h;
            y = new byte[w * h];
            u = new byte[(w / 2) * (h / 2)];
            v = new byte[(w / 2) * (h / 2)];
            argb = new int[w * h];
        }
        void fromBitmap(Bitmap b) {
            b.getPixels(argb, 0, w, 0, 0, w, h);
            for (int i = 0; i < argb.length; i++) {
                int c = argb[i];
                int r = (c >> 16) & 0xff, g = (c >> 8) & 0xff, bl = c & 0xff;
                y[i] = (byte) (((66 * r + 129 * g + 25 * bl + 128) >> 8) + 16);
            }
            int ci = 0;
            for (int row = 0; row < h - 1; row += 2) {
                for (int col = 0; col < w - 1; col += 2) {
                    int c = argb[row * w + col];
                    int r = (c >> 16) & 0xff, g = (c >> 8) & 0xff, bl = c & 0xff;
                    u[ci] = (byte) (((-38 * r - 74 * g + 112 * bl + 128) >> 8) + 128);
                    v[ci] = (byte) (((112 * r - 94 * g - 18 * bl + 128) >> 8) + 128);
                    ci++;
                }
            }
        }
    }

    private static void queueFrame(State st, Yuv yuv, long ptsUs) {
        int in;
        while ((in = st.encoder.dequeueInputBuffer(10000)) < 0) {
            drain(st, false);   // libera a saída enquanto espera espaço na entrada
        }
        android.media.Image image = st.encoder.getInputImage(in);
        if (image == null) {
            throw new IllegalStateException("codificador sem acesso ao quadro de entrada");
        }
        android.media.Image.Plane[] planes = image.getPlanes();
        // luminância: linha por linha
        ByteBuffer yb = planes[0].getBuffer();
        int yStride = planes[0].getRowStride();
        for (int row = 0; row < yuv.h; row++) {
            yb.position(row * yStride);
            yb.put(yuv.y, row * yuv.w, yuv.w);
        }
        // cor: respeita o formato do aparelho (planos separados ou intercalados)
        int cw = yuv.w / 2, ch = yuv.h / 2;
        for (int p = 1; p <= 2; p++) {
            byte[] src = p == 1 ? yuv.u : yuv.v;
            ByteBuffer cb = planes[p].getBuffer();
            int stride = planes[p].getRowStride();
            int step = planes[p].getPixelStride();
            if (step == 1) {
                for (int row = 0; row < ch; row++) {
                    cb.position(row * stride);
                    cb.put(src, row * cw, cw);
                }
            } else {
                for (int row = 0; row < ch; row++) {
                    int base = row * stride;
                    int s = row * cw;
                    for (int col = 0; col < cw; col++) {
                        cb.put(base + col * step, src[s + col]);
                    }
                }
            }
        }
        st.encoder.queueInputBuffer(in, 0, yuv.w * yuv.h * 3 / 2, ptsUs, 0);
        drain(st, false);
    }

    private static final class State {
        final MediaCodec encoder;
        final MediaMuxer muxer;
        final MediaCodec.BufferInfo info = new MediaCodec.BufferInfo();
        int track = -1;
        boolean started;
        long outFrames;
        State(MediaCodec e, MediaMuxer m) { encoder = e; muxer = m; }
    }

    /** Passa os quadros codificados para o arquivo MP4. */
    private static void drain(State st, boolean end) {
        int idle = 0;
        while (true) {
            int index = st.encoder.dequeueOutputBuffer(st.info, end ? 10000 : 0);
            if (index == MediaCodec.INFO_TRY_AGAIN_LATER) {
                if (!end || ++idle > 1000) {
                    return;
                }
            } else if (index == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED) {
                if (!st.started) {
                    st.track = st.muxer.addTrack(st.encoder.getOutputFormat());
                    st.muxer.start();
                    st.started = true;
                }
            } else if (index >= 0) {
                idle = 0;
                ByteBuffer data = st.encoder.getOutputBuffer(index);
                boolean config = (st.info.flags & MediaCodec.BUFFER_FLAG_CODEC_CONFIG) != 0;
                if (data != null && !config && st.info.size > 0 && st.started) {
                    st.outFrames++;
                    data.position(st.info.offset);
                    data.limit(st.info.offset + st.info.size);
                    st.muxer.writeSampleData(st.track, data, st.info);
                }
                st.encoder.releaseOutputBuffer(index, false);
                if ((st.info.flags & MediaCodec.BUFFER_FLAG_END_OF_STREAM) != 0) {
                    return;
                }
            }
        }
    }

    private static String publish(Activity a, File video, String name) throws Exception {
        if (Build.VERSION.SDK_INT >= 29) {
            ContentValues values = new ContentValues();
            values.put(MediaStore.Video.Media.DISPLAY_NAME, name);
            values.put(MediaStore.Video.Media.MIME_TYPE, "video/mp4");
            values.put(MediaStore.Video.Media.RELATIVE_PATH, Environment.DIRECTORY_MOVIES + "/Nanquim");
            values.put(MediaStore.Video.Media.IS_PENDING, 1);
            Uri uri = a.getContentResolver().insert(MediaStore.Video.Media.EXTERNAL_CONTENT_URI, values);
            if (uri == null) {
                throw new IllegalStateException("sem acesso à pasta de vídeos");
            }
            try (InputStream in = new FileInputStream(video);
                 OutputStream out = a.getContentResolver().openOutputStream(uri)) {
                copy(in, out);
            }
            values.clear();
            values.put(MediaStore.Video.Media.IS_PENDING, 0);
            a.getContentResolver().update(uri, values, null, null);
            return "Vídeos › Nanquim (aparece na Galeria)";
        }
        File dir = a.getExternalFilesDir(Environment.DIRECTORY_MOVIES);
        File out = new File(dir, name);
        try (InputStream in = new FileInputStream(video); OutputStream os = new FileOutputStream(out)) {
            copy(in, os);
        }
        return out.getPath();
    }

    private static void copy(InputStream in, OutputStream out) throws Exception {
        byte[] buffer = new byte[1 << 16];
        int read;
        while ((read = in.read(buffer)) > 0) {
            out.write(buffer, 0, read);
        }
        out.flush();
    }
}

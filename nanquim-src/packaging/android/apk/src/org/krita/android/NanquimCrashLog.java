package org.krita.android;

import android.annotation.TargetApi;
import android.app.Activity;
import android.app.ActivityManager;
import android.app.ApplicationExitInfo;
import android.content.ContentValues;
import android.content.Context;
import android.content.SharedPreferences;
import android.net.Uri;
import android.os.Build;
import android.provider.MediaStore;
import android.util.Log;

import java.io.ByteArrayOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.List;

/**
 * Nanquim: guarda o motivo da última queda do app.
 *
 * No Android 11 ou mais novo, o sistema lembra por que o processo anterior
 * morreu (queda do código nativo, exceção do Java, travamento). Na abertura
 * seguinte, se houver uma queda nova, o relatório vai para a pasta Downloads
 * do tablet como nanquim-erro-AAAAMMDD-HHMMSS.txt, para ser mandado sem
 * precisar de computador.
 */
public final class NanquimCrashLog {
    private static final String TAG = "nanquim.CrashLog";
    private static final String PREFS = "nanquim_crashlog";
    private static final String LAST = "last";

    private NanquimCrashLog() {}

    public static void saveLastCrash(Activity a) {
        try {
            if (Build.VERSION.SDK_INT < Build.VERSION_CODES.R) {
                return;
            }
            save(a);
        } catch (Throwable t) {
            Log.e(TAG, "não consegui guardar o relatório", t);
        }
    }

    @TargetApi(Build.VERSION_CODES.R)
    private static void save(Activity a) throws Exception {
        ActivityManager am = (ActivityManager) a.getSystemService(Context.ACTIVITY_SERVICE);
        if (am == null) {
            return;
        }
        List<ApplicationExitInfo> list = am.getHistoricalProcessExitReasons(a.getPackageName(), 0, 5);
        if (list == null || list.isEmpty()) {
            return;
        }
        SharedPreferences prefs = a.getSharedPreferences(PREFS, Context.MODE_PRIVATE);
        final long seen = prefs.getLong(LAST, 0L);
        long newest = seen;
        StringBuilder text = new StringBuilder();
        for (ApplicationExitInfo info : list) {
            final int reason = info.getReason();
            final boolean crash = reason == ApplicationExitInfo.REASON_CRASH
                    || reason == ApplicationExitInfo.REASON_CRASH_NATIVE
                    || reason == ApplicationExitInfo.REASON_ANR
                    || reason == ApplicationExitInfo.REASON_INITIALIZATION_FAILURE;
            if (!crash || info.getTimestamp() <= seen) {
                continue;
            }
            newest = Math.max(newest, info.getTimestamp());
            text.append("== queda em ").append(new java.util.Date(info.getTimestamp())).append(" ==\n");
            text.append("motivo: ").append(reason).append(" (4 = Java, 5 = código nativo, 6 = travou, 7 = não iniciou)\n");
            text.append("descrição: ").append(info.getDescription()).append("\n");
            text.append("versão do Android: ").append(Build.VERSION.RELEASE).append("  aparelho: ").append(Build.MODEL).append("\n");
            try (InputStream in = info.getTraceInputStream()) {
                if (in != null) {
                    ByteArrayOutputStream raw = new ByteArrayOutputStream();
                    byte[] buf = new byte[8192];
                    int n;
                    while ((n = in.read(buf)) > 0 && raw.size() < 400000) {
                        raw.write(buf, 0, n);
                    }
                    text.append("\n-- rastro --\n").append(printable(raw.toByteArray())).append("\n");
                }
            } catch (Throwable t) {
                text.append("(sem rastro: ").append(t).append(")\n");
            }
            text.append("\n");
        }
        if (text.length() == 0) {
            return;
        }
        final String stamp = new java.text.SimpleDateFormat("yyyyMMdd-HHmmss", java.util.Locale.US).format(new java.util.Date());
        ContentValues v = new ContentValues();
        v.put(MediaStore.Downloads.DISPLAY_NAME, "nanquim-erro-" + stamp + ".txt");
        v.put(MediaStore.Downloads.MIME_TYPE, "text/plain");
        v.put(MediaStore.Downloads.RELATIVE_PATH, "Download");
        Uri uri = a.getContentResolver().insert(MediaStore.Downloads.EXTERNAL_CONTENT_URI, v);
        if (uri == null) {
            return;
        }
        try (OutputStream out = a.getContentResolver().openOutputStream(uri)) {
            if (out != null) {
                out.write(text.toString().getBytes("UTF-8"));
            }
        }
        prefs.edit().putLong(LAST, newest).apply();
    }

    // o rastro de queda nativa vem em protobuf: só o texto legível interessa
    private static String printable(byte[] data) {
        StringBuilder out = new StringBuilder();
        StringBuilder run = new StringBuilder();
        for (byte b : data) {
            final int c = b & 0xFF;
            if (c >= 32 && c < 127) {
                run.append((char) c);
            } else {
                if (run.length() >= 4) {
                    out.append(run).append('\n');
                }
                run.setLength(0);
            }
        }
        if (run.length() >= 4) {
            out.append(run).append('\n');
        }
        return out.toString();
    }
}

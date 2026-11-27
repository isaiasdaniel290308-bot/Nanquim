package org.krita.android;

import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.provider.OpenableColumns;
import android.util.Log;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;

/**
 * Nanquim: pincéis (.kpp) e pacotes (.bundle) chegam do "Abrir com" como
 * content:// sem extensão. Copiamos para uma pasta do app com o nome
 * original, para o lado C++ reconhecer a extensão e importar o recurso.
 */
public final class NanquimImport {
    private static final String TAG = "nanquim.Import";
    public static final String EXTRA_PATH = "nanquim_import_path";

    private NanquimImport() {}

    public static String copyResourceFileIfNeeded(Context ctx, Uri uri) {
        String name = null;
        try (Cursor c = ctx.getContentResolver().query(uri, new String[] {OpenableColumns.DISPLAY_NAME}, null, null, null)) {
            if (c != null && c.moveToFirst()) {
                name = c.getString(0);
            }
        } catch (Exception e) {
            Log.w(TAG, "não consegui ler o nome do arquivo", e);
        }
        if (name == null) {
            name = uri.getLastPathSegment();
        }
        if (name == null) {
            return null;
        }
        String lower = name.toLowerCase();
        if (!lower.endsWith(".kpp") && !lower.endsWith(".bundle") && !lower.endsWith(".sut") && !lower.endsWith(".zip")) {
            return null;
        }
        name = new File(name).getName();
        File dir = new File(ctx.getCacheDir(), "nanquim-import");
        if (!dir.exists() && !dir.mkdirs()) {
            return null;
        }
        File out = new File(dir, name);
        try (InputStream in = ctx.getContentResolver().openInputStream(uri);
             OutputStream os = new FileOutputStream(out)) {
            if (in == null) {
                return null;
            }
            byte[] buf = new byte[1 << 16];
            int n;
            while ((n = in.read(buf)) > 0) {
                os.write(buf, 0, n);
            }
        } catch (Exception e) {
            Log.w(TAG, "falha ao copiar " + name, e);
            return null;
        }
        return out.getAbsolutePath();
    }
}

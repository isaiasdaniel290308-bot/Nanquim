/*
 *  SPDX-FileCopyrightText: 2026 Nanquim
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

package org.krita.android;

import android.app.Activity;
import android.content.ContentValues;
import android.content.Intent;
import android.net.Uri;
import android.os.Build;
import android.os.Environment;
import android.provider.MediaStore;
import android.util.Log;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;

/**
 * Nanquim: leva o arquivo exportado para a pasta pública do tablet.
 * Imagens vão para Imagens › Nanquim (aparecem na Galeria); PDF, PSD e .kra
 * vão para Downloads › Nanquim.
 */
public final class NanquimMedia {
    private static final String TAG = "nanquim.Media";

    private NanquimMedia() {}

    /** Devolve "uri\nonde ficou" ou null se não conseguiu. */
    public static String publish(Activity a, String path, String name, String mime) {
        try {
            File src = new File(path);
            boolean image = mime != null && mime.startsWith("image/") && !mime.contains("photoshop");
            if (Build.VERSION.SDK_INT >= 29) {
                ContentValues values = new ContentValues();
                values.put(MediaStore.MediaColumns.DISPLAY_NAME, name);
                values.put(MediaStore.MediaColumns.MIME_TYPE, mime);
                values.put(MediaStore.MediaColumns.RELATIVE_PATH,
                        (image ? Environment.DIRECTORY_PICTURES : Environment.DIRECTORY_DOWNLOADS) + "/Nanquim");
                values.put(MediaStore.MediaColumns.IS_PENDING, 1);
                Uri collection = image ? MediaStore.Images.Media.EXTERNAL_CONTENT_URI
                                       : MediaStore.Downloads.EXTERNAL_CONTENT_URI;
                Uri uri = a.getContentResolver().insert(collection, values);
                if (uri == null) {
                    return null;
                }
                try (InputStream in = new FileInputStream(src);
                     OutputStream out = a.getContentResolver().openOutputStream(uri)) {
                    copy(in, out);
                }
                values.clear();
                values.put(MediaStore.MediaColumns.IS_PENDING, 0);
                a.getContentResolver().update(uri, values, null, null);
                return uri.toString() + "\n" + (image ? "Imagens › Nanquim" : "Downloads › Nanquim");
            }
            File dir = new File(a.getExternalFilesDir(image ? Environment.DIRECTORY_PICTURES : Environment.DIRECTORY_DOWNLOADS), "Nanquim");
            dir.mkdirs();
            File out = new File(dir, name);
            try (InputStream in = new FileInputStream(src); OutputStream os = new FileOutputStream(out)) {
                copy(in, os);
            }
            return "\n" + out.getPath();
        } catch (Throwable t) {
            Log.e(TAG, "não consegui publicar " + path, t);
            return null;
        }
    }

    /** Abre o "Compartilhar" do Android com o arquivo já publicado. */
    public static void share(final Activity a, final String uri, final String mime) {
        a.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                try {
                    Intent send = new Intent(Intent.ACTION_SEND);
                    send.setType(mime);
                    send.putExtra(Intent.EXTRA_STREAM, Uri.parse(uri));
                    send.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
                    Intent chooser = Intent.createChooser(send, "Compartilhar");
                    chooser.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
                    a.startActivity(chooser);
                } catch (Throwable t) {
                    Log.e(TAG, "não consegui compartilhar", t);
                }
            }
        });
    }

    private static void copy(InputStream in, OutputStream out) throws Exception {
        byte[] buffer = new byte[1 << 16];
        int read;
        while ((read = in.read(buffer)) > 0) {
            out.write(buffer, 0, read);
        }
    }
}

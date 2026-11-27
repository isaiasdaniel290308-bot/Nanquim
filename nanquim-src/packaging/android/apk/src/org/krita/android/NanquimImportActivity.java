package org.krita.android;

import android.app.Activity;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.widget.Toast;

/**
 * Nanquim: recebe o "Abrir com" de pincéis e pacotes sem abrir uma segunda
 * janela do app. Copia o arquivo e entrega o caminho para a MainActivity
 * (singleTask), que já existe ou é aberta normalmente.
 */
public class NanquimImportActivity extends Activity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Intent in = getIntent();
        Uri uri = (in != null) ? in.getData() : null;
        if (uri != null) {
            String local = NanquimImport.copyResourceFileIfNeeded(this, uri);
            if (local != null) {
                Intent out = new Intent(this, MainActivity.class);
                out.putExtra(NanquimImport.EXTRA_PATH, local);
                out.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TOP);
                startActivity(out);
            } else {
                Toast.makeText(this, "O Nanquim importa pincéis .kpp, pacotes .bundle, pincéis .sut e arquivos .zip por aqui.", Toast.LENGTH_LONG).show();
            }
        }
        finish();
    }
}

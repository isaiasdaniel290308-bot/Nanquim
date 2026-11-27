package org.krita.android;

import android.animation.AnimatorListenerAdapter;
import android.animation.ValueAnimator;
import android.app.Activity;
import android.graphics.Rect;
import android.graphics.Typeface;
import android.os.Build;
import android.os.Handler;
import android.os.Looper;
import android.os.SystemClock;
import android.util.DisplayMetrics;
import android.util.Log;
import android.util.TypedValue;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.view.animation.PathInterpolator;
import android.widget.FrameLayout;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import org.krita.R;

/**
 * Nanquim: tela de abertura nativa e animada (redesenho).
 *
 * Aparece por cima de tudo assim que a MainActivity é criada, enquanto o
 * Krita carrega, e some quando o lado C++ avisa que terminou (setLoaded).
 *
 * Sobre o grafite, a gota-pena se enche de tinta a partir da ponta e fica
 * no centro enquanto o app carrega. Quando ele fica pronto, o conjunto se
 * recentra, a marca NANQUIM sai de trás da gota, e a abertura some por cima
 * da tela inicial. Curvas e tempos revisados com o review-animations:
 * entrada com ease-out forte, movimento na tela com ease-in-out forte, e a
 * abertura segura os toques até sumir.
 */
public final class NanquimSplash {
    private static final String TAG = "nanquim.Splash";
    private static final int BG = 0xFF24231F;
    private static final int FILL_MS = 400;
    private static final int MOVE_MS = 260;
    private static final int BEAT_MS = 60;
    private static final int OUT_MS = 200;

    private static FrameLayout sRoot;
    private static LinearLayout sMark;
    private static TextView sWord;
    private static TextView sStatus;
    private static long sFillEnd;
    private static boolean sHiding;

    private NanquimSplash() {}

    // cubic-bezier(0.23, 1, 0.32, 1): entra rápido e assenta
    private static PathInterpolator easeOut() {
        return new PathInterpolator(0.23f, 1f, 0.32f, 1f);
    }

    // cubic-bezier(0.77, 0, 0.175, 1): movimento dentro da tela
    private static PathInterpolator easeInOut() {
        return new PathInterpolator(0.77f, 0f, 0.175f, 1f);
    }

    public static void show(Activity a) {
        if (sRoot != null) {
            return;
        }
        NanquimCrashLog.saveLastCrash(a);
        try {
            build(a);
        } catch (Throwable t) {
            Log.e(TAG, "não consegui montar a abertura", t);
            sRoot = null;
        }
    }

    private static void build(Activity a) {
        DisplayMetrics dm = a.getResources().getDisplayMetrics();
        final float d = dm.density;
        final int dropSize = (int) (88 * d);

        final FrameLayout root = new FrameLayout(a);
        root.setBackgroundColor(BG);
        root.setClickable(true);   // não deixa toques passarem enquanto a abertura está na tela
        root.setFocusable(true);

        // --- gota e marca, lado a lado --------------------------------------
        final LinearLayout mark = new LinearLayout(a);
        mark.setOrientation(LinearLayout.HORIZONTAL);
        mark.setGravity(Gravity.CENTER_VERTICAL);

        final ImageView drop = new ImageView(a);
        drop.setImageResource(R.drawable.nanquim_drop);
        mark.addView(drop, new LinearLayout.LayoutParams(dropSize, dropSize));

        // a marca fica escondida atrás da gota e sai dela (máscara do lado da gota)
        final FrameLayout wordMask = new FrameLayout(a);
        wordMask.setClipChildren(true);
        wordMask.setClipToPadding(true);
        final TextView word = new TextView(a);
        word.setText("NANQUIM");
        word.setTextColor(0xFFEEEAE0);
        word.setTextSize(TypedValue.COMPLEX_UNIT_DIP, 60);
        word.setTypeface(font(a, R.font.brand_bold, Typeface.create("sans-serif", Typeface.BOLD)));
        word.setLetterSpacing(-0.015f);
        word.setIncludeFontPadding(false);
        // marca NANQUIM: Chillax Bold engrossada (contorno de 0,045 em), como no app
        word.getPaint().setStyle(android.graphics.Paint.Style.FILL_AND_STROKE);
        word.getPaint().setStrokeJoin(android.graphics.Paint.Join.ROUND);
        word.getPaint().setStrokeWidth(0.045f * 60 * d);
        word.setPadding((int) (6 * d), (int) (8 * d), (int) (6 * d), (int) (8 * d));
        wordMask.addView(word, new FrameLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT,
                ViewGroup.LayoutParams.WRAP_CONTENT, Gravity.CENTER_VERTICAL));
        LinearLayout.LayoutParams wp = new LinearLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT,
                ViewGroup.LayoutParams.WRAP_CONTENT);
        wp.leftMargin = (int) (18 * d);
        mark.addView(wordMask, wp);
        root.addView(mark, new FrameLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT,
                ViewGroup.LayoutParams.WRAP_CONTENT, Gravity.CENTER));

        // --- carregamento: discreto, embaixo, só se demorar ------------------
        final TextView status = new TextView(a);
        status.setText("Carregando…");
        status.setTextColor(0xFFA8A398);
        status.setTextSize(TypedValue.COMPLEX_UNIT_DIP, 14);
        status.setSingleLine(true);
        status.setAlpha(0f);
        FrameLayout.LayoutParams sp = new FrameLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT,
                ViewGroup.LayoutParams.WRAP_CONTENT, Gravity.CENTER_HORIZONTAL | Gravity.BOTTOM);
        sp.bottomMargin = (int) (48 * d);
        root.addView(status, sp);

        // por cima de tudo: a decorView não é trocada quando o Qt chama setContentView
        ViewGroup decor = (ViewGroup) a.getWindow().getDecorView();
        decor.addView(root, new FrameLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT));
        sRoot = root;
        sMark = mark;
        sWord = word;
        sStatus = status;

        // Antes de medir, a gota fica vazia (recorte de altura zero) e a marca
        // fica escondida; os deslocamentos dependem do tamanho de verdade.
        drop.setClipBounds(new Rect(0, dropSize, dropSize, dropSize));
        mark.setAlpha(0f);
        mark.addOnLayoutChangeListener(new View.OnLayoutChangeListener() {
            @Override
            public void onLayoutChange(View v, int l, int t, int r, int b, int ol, int ot, int orr, int ob) {
                if (r - l <= 0 || sHiding) {
                    return;
                }
                // o conjunto começa com a gota sozinha no centro da tela
                final int markWidth = r - l;
                mark.setTranslationX(markWidth / 2f - dropSize / 2f);
                word.setTranslationX(-(word.getWidth() + 2 * d));
                if (mark.getAlpha() == 0f) {
                    mark.setAlpha(1f);
                    fill(drop, dropSize);
                }
            }
        });

        // se o carregamento demorar, aparece o que está acontecendo
        status.animate().alpha(1f).setStartDelay(1500).setDuration(300).setInterpolator(easeOut()).start();

        // nunca deixar a abertura presa: some sozinha depois de 90 s
        new Handler(Looper.getMainLooper()).postDelayed(NanquimSplash::hideNow, 90000);
    }

    // a tinta sobe a partir da ponta da pena; a ponta fica parada enquanto a gota cresce
    private static void fill(final ImageView drop, final int size) {
        drop.setPivotX(size * 0.5f);
        drop.setPivotY(size * 0.94f);
        drop.setScaleX(0.96f);
        drop.setScaleY(0.96f);
        ValueAnimator ink = ValueAnimator.ofFloat(0f, 1f);
        ink.setDuration(FILL_MS);
        ink.setInterpolator(easeOut());
        ink.addUpdateListener(an -> {
            final float p = (float) an.getAnimatedValue();
            drop.setClipBounds(new Rect(0, Math.round(size * (1f - p)), size, size));
            final float s = 0.96f + 0.04f * p;
            drop.setScaleX(s);
            drop.setScaleY(s);
        });
        ink.addListener(new AnimatorListenerAdapter() {
            @Override
            public void onAnimationEnd(android.animation.Animator animation) {
                drop.setClipBounds(null);
            }
        });
        sFillEnd = SystemClock.uptimeMillis() + FILL_MS;
        ink.start();
    }

    // Montserrat (SIL OFL) em res/font; em Android antigo fica a fonte do sistema
    private static Typeface font(Activity a, int id, Typeface fallback) {
        if (Build.VERSION.SDK_INT >= 26) {
            try {
                return a.getResources().getFont(id);
            } catch (Exception e) {
                Log.w(TAG, "fonte indisponível", e);
            }
        }
        return fallback;
    }

    public static void setStatus(final Activity a, final String text) {
        if (a == null || text == null || text.isEmpty()) {
            return;
        }
        a.runOnUiThread(() -> {
            if (sStatus != null) {
                sStatus.setText(text);
            }
        });
    }

    public static void hide(Activity a) {
        if (a != null) {
            a.runOnUiThread(NanquimSplash::hideNow);
        }
    }

    private static void hideNow() {
        final FrameLayout root = sRoot;
        if (root == null || sHiding) {
            return;
        }
        sHiding = true;
        // a gota termina de encher antes de a marca sair
        final long wait = Math.max(0, sFillEnd - SystemClock.uptimeMillis());

        if (sStatus != null) {
            sStatus.animate().cancel();
            sStatus.animate().alpha(0f).setStartDelay(0).setDuration(150).setInterpolator(easeOut()).start();
        }
        // o conjunto se recentra e a marca sai de trás da gota, juntos
        if (sMark != null) {
            sMark.animate().translationX(0f).setStartDelay(wait).setDuration(MOVE_MS)
                    .setInterpolator(easeInOut()).start();
        }
        if (sWord != null) {
            sWord.animate().translationX(0f).setStartDelay(wait).setDuration(MOVE_MS)
                    .setInterpolator(easeInOut()).start();
        }
        // a abertura some enquanto a marca ainda se acomoda (antes: ~730 ms depois de pronto; agora ~330 ms)
        root.animate().alpha(0f).setStartDelay(wait + MOVE_MS / 2).setDuration(OUT_MS)
                .setInterpolator(easeOut()).withEndAction(() -> {
            ViewGroup parent = (ViewGroup) root.getParent();
            if (parent != null) {
                parent.removeView(root);
            }
            sRoot = null;
            sMark = null;
            sWord = null;
            sStatus = null;
            sHiding = false;
        }).start();
    }

}

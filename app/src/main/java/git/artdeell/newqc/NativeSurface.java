package git.artdeell.newqc;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.SurfaceTexture;
import android.util.Log;
import android.view.Surface;
import android.widget.FrameLayout;

public class NativeSurface extends FrameLayout {
    private boolean hasSurface = false;
    private SurfaceTexture surfaceTexture;
    private Surface surface;

    public NativeSurface(Context context) {
        super(context);
    }

    public void setSurfaceTexture(SurfaceTexture surfaceTexture, int w, int h) {
        post(()->setSurfaceTextureInternal(surfaceTexture, w, h));
    }

    private void setSurfaceTextureInternal(SurfaceTexture surfaceTexture, int w, int h) {
        this.surfaceTexture = surfaceTexture;
        surface = new Surface(surfaceTexture);
        int widthMeasureSpec = MeasureSpec.makeMeasureSpec(w, MeasureSpec.EXACTLY);
        int heightMeasureSpec = MeasureSpec.makeMeasureSpec(h, MeasureSpec.EXACTLY);
        hasSurface = true;
        measure(widthMeasureSpec, heightMeasureSpec);
        invalidate();
    }

    private void draw() {
        Canvas trueCanvas = surface.lockHardwareCanvas();
        draw(trueCanvas);
        surface.unlockCanvasAndPost(trueCanvas);
    }

    public void updateSurfaceTexture() {
        if(!hasSurface) return;
        surfaceTexture.updateTexImage();
        post(this::draw);
    }

    public void destroySurfaceTexture() {
        hasSurface = false;
        onDetachedFromWindow();
        if(surfaceTexture != null) surfaceTexture.release();
    }
}

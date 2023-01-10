/*
    Defines the PPEffect for neural rendering.
*/
var Effect = new Type( Std.module('Effects/PPEffect') );


// Define Scripted State
static skip = new MinSG.ScriptedState();
skip.doEnableState @(override) := fn(node, rp) { return MinSG.STATE_SKIP_RENDERING; };

Effect._constructor ::= fn() {
    var flow = RenderFlow.getFlow();

    // TODO createHDRTexture??
    var bitmap = new Util.Bitmap(flow.getDimX(),flow.getDimY(), flow.getFormat() == 'MONO' ? Util.Bitmap.MONO : Util.Bitmap.RGB);
    this.colorTexture := Rendering.createTextureFromBitmap(bitmap);

    if(flow.getPrerender()) {
        this.fbo := new Rendering.FBO;
        renderingContext.pushAndSetFBO(fbo);

        this.preColorTexture := Rendering.createStdTexture(flow.getPrerenderDimX(), flow.getPrerenderDimY(), false);
        this.preDepthTexture := Rendering.createDepthTexture(flow.getPrerenderDimX(), flow.getPrerenderDimY());

        fbo.attachColorTexture(renderingContext, preColorTexture);
        fbo.attachDepthTexture(renderingContext, preDepthTexture);

        out("status: ", this.fbo.getStatusMessage(renderingContext), "\n");
        renderingContext.popFBO();
    }
};

/*! ---|> PPEffect  */
Effect.begin @(override) ::= fn(){
    var flow = RenderFlow.getFlow();

    if(flow.getPrerender()){   
        renderingContext.pushAndSetFBO(fbo);

        renderingContext.pushViewport();
        renderingContext.setViewport(0, 0, flow.getPrerenderDimX(), flow.getPrerenderDimY());

        PADrend.renderScene(PADrend.getRootNode(), void, PADrend.getRenderingFlags(), PADrend.getBGColor(), PADrend.getRenderingLayers());
        renderingContext.finish();

        renderingContext.popViewport();
        renderingContext.popFBO();
    }
    
    if(flow.getPythonPath()) {
        if(flow.getPrerender())
            RenderFlow.pythonPRenderTexture(renderingContext, preColorTexture, colorTexture, flow.getDataFromFloat()==true, flow.getFormat() == 'MONO_COLORMAP');
        else
            RenderFlow.pythonRenderTexture(colorTexture, flow.getDataFromFloat()==true, flow.getFormat() == 'MONO_COLORMAP');
    }
    else if(flow.getPrerender() && flow.getPrerenderDirect()) {
        RenderFlow.directPrerender(renderingContext, preColorTexture, colorTexture, !(flow.getPrerenderDirectCache() === false), flow.getFormat() == 'MONO_COLORMAP');

    } else {
        var data = flow.render(flow.getPrerender() ? preColorTexture.getData(renderingContext) : void);
        RenderFlow.setTextureData(colorTexture, data, flow.getDataFromFloat() == true, flow.getFormat() == 'MONO_COLORMAP');
    }


    PADrend.getRootNode().addState(skip);
};

/*! ---|> PPEffect  */
Effect.end @(override) ::=fn(){
    PADrend.getRootNode().removeState(skip);

    renderingContext.clearScreen(PADrend.getBGColor());

    var flow = RenderFlow.getFlow();
    var s = flow.getPrerender() && flow.getPrerenderSplitscreen();

    var vp = [renderingContext.getViewport().getWidth(), renderingContext.getViewport().getHeight()];
    if(s) vp[0] = vp[0] / 2 - 4;

    var r = [vp[0] / flow.getDimX(), vp[1] / flow.getDimY()].min();
    var screenRect = new Geometry.Rect((vp[0] - flow.getDimX() * r) / 2, (vp[1] - flow.getDimY() * r) / 2, flow.getDimX() * r, flow.getDimY() * r);

    if(s) screenRect.setX(vp[0] + 8);
    Rendering.drawTextureToScreen(renderingContext, screenRect, [this.colorTexture], [new Geometry.Rect(0, 0, 1, 1)]);

    if(s) {
        var r2 = [vp[0] / flow.getPrerenderDimX(), vp[1] / flow.getPrerenderDimY()].min();
        var screenRect2 = new Geometry.Rect((vp[0] - flow.getPrerenderDimX() * r2), (vp[1] - flow.getPrerenderDimY() * r2) / 2, flow.getPrerenderDimX() * r2, flow.getPrerenderDimY() * r2);
        Rendering.drawTextureToScreen(renderingContext, screenRect2, [this.preColorTexture], [new Geometry.Rect(0, 0, 1, 1)]);
    }
};

return new Effect;
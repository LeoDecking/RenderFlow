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
    this.colorTexture := Rendering.createStdTexture(flow.getDimX(), flow.getDimY(), false);

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

    var prerender = void;
    if(flow.getPrerender()){   
        renderingContext.pushAndSetFBO(fbo);

        renderingContext.pushViewport();
        renderingContext.setViewport(0, 0, flow.getPrerenderDimX(), flow.getPrerenderDimY());

        PADrend.renderScene(PADrend.getRootNode(), void, PADrend.getRenderingFlags(), PADrend.getBGColor(), PADrend.getRenderingLayers());
        renderingContext.finish();

        renderingContext.popViewport();
        renderingContext.popFBO();

        // TODO, I have added .getData to E_Texture.cpp
        prerender = preColorTexture.getData(renderingContext);
    }
    
    var data = flow.render(prerender);

    // TODO, I have added .setData to E_Bitmap.cpp
    // TODO _FLOAT?
    var bitmap = new Util.Bitmap(flow.getDimX(), flow.getDimY(), flow.getFormat() == 'MONO' ? Util.Bitmap.MONO : Util.Bitmap.RGB);
    bitmap.setData(flow.getFormat() == 'MONO_COLORMAP' ? RenderFlow.colormap(data) : data);

    colorTexture = Rendering.createTextureFromBitmap(bitmap);
    PADrend.getRootNode().addState(skip);
};

/*! ---|> PPEffect  */
Effect.end @(override) ::=fn(){
    PADrend.getRootNode().removeState(skip);

    var flow = RenderFlow.getFlow();
    var vp = renderingContext.getViewport();

    var r = [vp.getWidth() / flow.getDimX(), vp.getHeight() / flow.getDimY()].min();
    var screenRect = new Geometry.Rect((vp.getWidth() - flow.getDimX() * r) / 2, (vp.getHeight() - flow.getDimY() * r) / 2, flow.getDimX() * r, flow.getDimY() * r);

    Rendering.drawTextureToScreen(renderingContext, screenRect, [this.colorTexture], [new Geometry.Rect(0, 0, 1, 1)]);
};

return new Effect;
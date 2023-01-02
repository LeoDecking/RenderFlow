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
    if(flow.getFormat() == 'MONO_COLORMAP')
        data = RenderFlow.colormap(data);

    // TODO, I have added .setData to E_Bitmap.cpp
    // TODO _FLOAT?
    var dim = flow.getDim();
    if(flow.getPrerender() && flow.getPrerenderSplitscreen())
        [data, dim] = RenderFlow.splitscreen(prerender, flow.getPrerenderDim(), data, flow.getDim());

    var bitmap = new Util.Bitmap(dim[0], dim[1], flow.getFormat() == 'MONO' ? Util.Bitmap.MONO : Util.Bitmap.RGB);
    bitmap.setData(data);

    colorTexture = Rendering.createTextureFromBitmap(bitmap);
    PADrend.getRootNode().addState(skip);
};

/*! ---|> PPEffect  */
Effect.end @(override) ::=fn(){
    PADrend.getRootNode().removeState(skip);

    var flow = RenderFlow.getFlow();
    var vp = renderingContext.getViewport();
    var dim = (flow.getPrerender() && flow.getPrerenderSplitscreen()) ? RenderFlow.splitscreenDim(flow.getPrerenderDim(), flow.getDim()) : flow.getDim();

    var r = [vp.getWidth() / dim[0], vp.getHeight() / dim[1]].min();
    var screenRect = new Geometry.Rect((vp.getWidth() - dim[0] * r) / 2, (vp.getHeight() - dim[1] * r) / 2, dim[0] * r, dim[1] * r);

    Rendering.drawTextureToScreen(renderingContext, screenRect, [this.colorTexture], [new Geometry.Rect(0, 0, 1, 1)]);
};

return new Effect;
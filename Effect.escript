/*
    Defines the PPEffect for neural rendering.
*/
var Effect = new Type( Std.module('Effects/PPEffect') );


// Define Scripted State
static SkipRendering = new Type(MinSG.ScriptedState);
SkipRendering.doEnableState @(override) ::= fn(node, rp) {
    return MinSG.STATE_SKIP_RENDERING;
};

Effect._constructor ::= fn() {
    var flow = RenderFlow.getFlow();
    this.colorTextureOut := Rendering.createHDRTexture(flow.getDimX(), flow.getDimY(), false);

    if(flow.getPrerender()) {
        this.fbo := new Rendering.FBO;
        renderingContext.pushAndSetFBO(fbo);

        this.colorTextureIn := Rendering.createHDRTexture(inDimensions[0], inDimensions[1], false);
        this.depthTexture := Rendering.createDepthTexture(inDimensions[0], inDimensions[1]);

        this.fbo.attachColorTexture(renderingContext, colorTextureIn);
        this.fbo.attachDepthTexture(renderingContext, depthTexture);

        // out("status: ", this.fbo.getStatusMessage(renderingContext), "\n"); // TODO remove?
        renderingContext.popFBO();
    }

    this.skip := new SkipRendering(); // TODO remove?
};

/*! ---|> PPEffect  */
Effect.begin @(override) ::= fn(){
    var flow = RenderFlow.getFlow();

    var prerender = void;
    if(flow.getPrerender()){        
        renderingContext.pushAndSetFBO(fbo);

        renderingContext.pushViewport();
        renderingContext.setViewport(0, 0, flow.getPrerenderDimX(), flow.getPrerenderDimY());

        PADrend.getRootNode().removeState(skip);
        PADrend.renderScene(PADrend.getRootNode(), void, PADrend.getRenderingFlags(), PADrend.getBGColor(), PADrend.getRenderingLayers());
        renderingContext.finish();

        renderingContext.popViewport();
        renderingContext.popFBO();
        // TODO colorTextureIn from Texture to Array
    }

    
    var data = flow.render(prerender);
    
    // TODO, I have added .setData to E_Bitmap.cpp
    // TODO _FLOAT?
    var bitmap = new Util.Bitmap(flow.getDimX(), flow.getDimY(), flow.getFormat() == 'MONO' ? Util.Bitmap.MONO : Util.Bitmap.RGB);
    bitmap.setData(flow.getFormat() == 'MONO_COLORMAP' ? RenderFlow.colormap(output) : output);

    colorTextureOut = Rendering.createTextureFromBitmap(bitmap);

    
    PADrend.getRootNode().addState(skip);
};

/*! ---|> PPEffect  */
Effect.end @(override) ::=fn(){
    PADrend.getRootNode().removeState(skip);

    var vp = renderingContext.getViewport();

    var r = [vp.getWidth() / outDimensions[0], vp.getHeight() / outDimensions[1]].min();

    Rendering.drawTextureToScreen(renderingContext, new Geometry.Rect((vp.getWidth()-outDimensions[0]*r)/2, (vp.getHeight()-outDimensions[1]*r)/2, outDimensions[0]*r, outDimensions[1]*r), [this.colorTextureOut], [new Geometry.Rect(0, 0, 1, 1)]);
};

return new Effect;
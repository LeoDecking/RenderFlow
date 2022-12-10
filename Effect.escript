/*
    Defines the PPEffect for neural rendering.
*/
var Effect = new Type( Std.module('Effects/PPEffect') );

static inDimensions = [324, 324];
static outDimensions = [648, 648];

// Define Scripted State
static SkipRendering = new Type(MinSG.ScriptedState);
SkipRendering.doEnableState @(override) ::= fn(node, rp) {
    return MinSG.STATE_SKIP_RENDERING;
};

Effect._constructor ::= fn() {
    this.fbo := new Rendering.FBO;

    this.colorTextureIn := Rendering.createHDRTexture(inDimensions[0], inDimensions[1], false);
    this.depthTexture := Rendering.createDepthTexture(inDimensions[0], inDimensions[1]);

    this.colorTextureOut := Rendering.createHDRTexture(outDimensions[0], outDimensions[1], false);

    this.fbo.attachColorTexture(renderingContext, colorTextureIn);
    this.fbo.attachDepthTexture(renderingContext, depthTexture);

    out(this.fbo.getStatusMessage(renderingContext), "\n"); // TODO remove?

    this.skip := new SkipRendering();

    renderingContext.popFBO();
};

/*! ---|> PPEffect  */
Effect.begin @(override) ::= fn(){
    renderingContext.pushAndSetFBO(fbo);

    renderingContext.pushViewport();
    renderingContext.setViewport(0, 0, inDimensions[0], inDimensions[1]);

    PADrend.getRootNode().removeState(skip);
    PADrend.renderScene(PADrend.getRootNode(), void, PADrend.getRenderingFlags(), PADrend.getBGColor(), PADrend.getRenderingLayers());
    renderingContext.finish();

    PADrend.getRootNode().addState(skip);

    renderingContext.popViewport();
	renderingContext.popFBO();

    // colorTextureOut = RenderFlow.renderFromTexture(renderingContext, colorTextureIn);
    colorTextureOut = RenderFlow.renderFromCamera(renderingContext, PADrend.getActiveCamera());

};

/*! ---|> PPEffect  */
Effect.end @(override) ::=fn(){
    Rendering.drawTextureToScreen(renderingContext, new Geometry.Rect(0, 0, outDimensions[0], outDimensions[1]), [this.colorTextureOut], [new Geometry.Rect(0, 0, 1, 1)]);
};

return new Effect;
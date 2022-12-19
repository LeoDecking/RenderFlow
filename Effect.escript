/*
    Defines the PPEffect for neural rendering.
*/
var Effect = new Type( Std.module('Effects/PPEffect') );

static inDimensions = [324, 324];
static outDimensions = [64, 64];

// Define Scripted State
static SkipRendering = new Type(MinSG.ScriptedState);
SkipRendering.doEnableState @(override) ::= fn(node, rp) {
    return MinSG.STATE_SKIP_RENDERING;
};

Effect._constructor ::= fn() {
    // this.fbo := new Rendering.FBO;
    // renderingContext.pushAndSetFBO(fbo);

    // this.colorTextureIn := Rendering.createHDRTexture(inDimensions[0], inDimensions[1], false);
    // this.depthTexture := Rendering.createDepthTexture(inDimensions[0], inDimensions[1]);

    this.colorTextureOut := Rendering.createHDRTexture(outDimensions[0], outDimensions[1], false);

    // this.fbo.attachColorTexture(renderingContext, colorTextureIn);
    // this.fbo.attachDepthTexture(renderingContext, depthTexture);

    // out("status: ", this.fbo.getStatusMessage(renderingContext), "\n"); // TODO remove?

    this.skip := new SkipRendering();

    // renderingContext.popFBO();
};

/*! ---|> PPEffect  */
Effect.begin @(override) ::= fn(){
    // renderingContext.pushAndSetFBO(fbo);

    // renderingContext.pushViewport();
    // renderingContext.setViewport(0, 0, inDimensions[0], inDimensions[1]);

    // PADrend.getRootNode().removeState(skip);
    // PADrend.renderScene(PADrend.getRootNode(), void, PADrend.getRenderingFlags(), PADrend.getBGColor(), PADrend.getRenderingLayers());
    // renderingContext.finish();

    // renderingContext.popViewport();
	// renderingContext.popFBO();



    // colorTextureOut = RenderFlow.renderFromTexture(renderingContext, colorTextureIn);
    // colorTextureOut = RenderFlow.renderFromCamera(renderingContext, PADrend.getActiveCamera());

    var cam = PADrend.getActiveCamera();
    // // var angles = RenderFlow.getCameraAngles(cam);

    // // std::cout << x.getX() << " " << x.getY() << " " << x.getZ() << std::endl;
    // // std::cout << y.getX() << " " << y.getY() << " " << y.getZ() << std::endl;
    // // out(cam.getWorldPosition().getX() + ", " + cam.getWorldPosition().getY() + ", " + cam.getWorldPosition().getZ() + "; " + angleH + "°, " + angleV + "°");

    var input = [cam.getWorldPosition().getX() / 10, cam.getWorldPosition().getZ() / 10];
    // out(input[0], ", ", input[1],"\n");
    var output = RenderFlow.predict(input);
    
    // // TODO, I have added .setData to E_Bitmap.cpp
    var bitmap = new Util.Bitmap(outDimensions[0], outDimensions[1], Util.Bitmap.RGB);
    bitmap.setData(RenderFlow.colormap(output));

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
static dims = new Map();

RenderFlow._screenshot := fn(x, y, bgColor=PADrend.getBGColor()) {

    bgColor = new Util.Color4f(0,0,0,1);

    if(dims[x+", "+y]) {
	    renderingContext.pushAndSetFBO(dims[x+", "+y][0]);
    } else {
        var fbo = new Rendering.FBO;
        renderingContext.pushAndSetFBO(fbo);

        var colorTexture = Rendering.createStdTexture(x, y, false);
        var depthTexture = Rendering.createDepthTexture(x, y);

        fbo.attachColorTexture(renderingContext, colorTexture);
        fbo.attachDepthTexture(renderingContext, depthTexture);

        dims[x+", "+y] = [fbo, colorTexture, depthTexture];
    }

	renderingContext.pushViewport();
	renderingContext.setViewport(0, 0, x, y);
	PADrend.renderScene(PADrend.getRootNode(), void, PADrend.getRenderingFlags(), bgColor, PADrend.getRenderingLayers());
	renderingContext.finish();
	renderingContext.popViewport();


    // twice, because the scene isn't updated otherwise
	renderingContext.pushViewport();
	renderingContext.setViewport(0, 0, x, y);
	PADrend.renderScene(PADrend.getRootNode(), void, PADrend.getRenderingFlags(), bgColor, PADrend.getRenderingLayers());
	renderingContext.finish();
	renderingContext.popViewport();

	renderingContext.popFBO();

	return [renderingContext, dims[x+", "+y][1]];
};

RenderFlow.screenshot := fn(x, y, asFloat=false, bgColor=PADrend.getBGColor()) {
    return RenderFlow.getTextureData(renderingContext, _screenshot(x, y, bgColor)[1], asFloat);
};

// _screenshot;
// RenderFlow.screenshot = screenshot;
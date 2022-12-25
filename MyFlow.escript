var myFlow = new RenderFlow.Flow({
    NAME: 'paderbornMap'
    MODEL: '../extPlugins/RenderFlow/model'
    PRERENDER: false
    // PRERENDER_DIM
    DIM: [64, 64]
    FORMAT: 'MONO_COLORMAP'
});

myFlow.render @(override) := fn(prerender) {
    var cam = PADrend.getActiveCamera();
    // // var angles = RenderFlow.getCameraAngles(cam);

    // // std::cout << x.getX() << " " << x.getY() << " " << x.getZ() << std::endl;
    // // std::cout << y.getX() << " " << y.getY() << " " << y.getZ() << std::endl;
    // // out(cam.getWorldPosition().getX() + ", " + cam.getWorldPosition().getY() + ", " + cam.getWorldPosition().getZ() + "; " + angleH + "°, " + angleV + "°");

    var input = [cam.getWorldPosition().getX() / 10, cam.getWorldPosition().getZ() / 10];
    // out(input[0], ", ", input[1],"\n");
    var output = RenderFlow.predict(input);
    
    return output;
};

RenderFlow.myFlow := myFlow;
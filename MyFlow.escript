var myFlow = new RenderFlow.Flow({
    RenderFlow.Flow.NAME: 'paderbornMap',

    RenderFlow.Flow.MODEL: '../extPlugins/RenderFlow/model',
    RenderFlow.Flow.MODEL_FROZEN_GRAPH: false,
    RenderFlow.Flow.MODEL_SHAPE: [2],
    RenderFlow.Flow.MODEL_INPUT: 'serving_default_dense_6_input:0',
    RenderFlow.Flow.MODEL_OUTPUT: 'StatefulPartitionedCall:0',

    RenderFlow.Flow.PRERENDER: false,
    
    RenderFlow.Flow.DATA_FROM_FLOAT: true,
    RenderFlow.Flow.DIM: [64, 64],
    RenderFlow.Flow.FORMAT: 'MONO_COLORMAP'
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
var myFlow = new RenderFlow.Flow({
    RenderFlow.Flow.NAME: 'myFlow',

    RenderFlow.Flow.MODEL: __DIR__ + '/model',
    RenderFlow.Flow.MODEL_SHAPE: [2],

    RenderFlow.Flow.PRERENDER: false,
    
    RenderFlow.Flow.DATA_FROM_FLOAT: true,
    RenderFlow.Flow.DIM: [64, 64],
    RenderFlow.Flow.FORMAT: 'MONO_COLORMAP'
});

static cache = false;

myFlow.onActivate @(override) := fn() {
    outln("myFlow activated");
};
myFlow.onDeactivate @(override) := fn() {
    outln("myFlow deactivated");
};

myFlow.render @(override) := fn() {
    var cam = PADrend.getActiveCamera();

    var input = [cam.getWorldPosition().getX() / 10, cam.getWorldPosition().getZ() / 10];
    var output = RenderFlow.predict(input, cache);
    return output;
};

// RenderFlow.myFlow := myFlow;
RenderFlow.register(myFlow);
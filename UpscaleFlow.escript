var upscaleFlow = new RenderFlow.Flow({
    RenderFlow.Flow.NAME: 'upscale',
    RenderFlow.Flow.MODEL: '../extPlugins/RenderFlow/upscaleModel/model.pb',
    RenderFlow.Flow.MODEL_FROZEN_GRAPH: true,
    RenderFlow.Flow.MODEL_SHAPE: [324, 324, 3],
    RenderFlow.Flow.MODEL_INPUT: 'input:0',
    RenderFlow.Flow.MODEL_OUTPUT: 'Identity:0',
    RenderFlow.Flow.PRERENDER: true,
    RenderFlow.Flow.PRERENDER_DIM: [324, 324],
    RenderFlow.Flow.DIM: [648, 648],
    RenderFlow.Flow.FORMAT: 'RGB'
});

upscaleFlow.render @(override) := fn(prerender) {
    var output = RenderFlow.predict(prerender);
    
    var r = new Array();
    for(var i = 0; i < output.size(); i++) {
        var o = output[i];
        if(o < 0) o = 0;
        else if (o > 1) o = 255;
        else o = (o * 255).floor();
        r.append([o]);
    }

    return r;
};

RenderFlow.upscaleFlow := upscaleFlow;
var upscaleFlow = new RenderFlow.Flow({
    RenderFlow.Flow.NAME: 'upscale',

    RenderFlow.Flow.MODEL: '../extPlugins/RenderFlow/upscaleModel/model.pb',
    RenderFlow.Flow.MODEL_FROZEN_GRAPH: true,
    RenderFlow.Flow.MODEL_SHAPE: [324, 324, 3],
    RenderFlow.Flow.MODEL_INPUT: 'input:0',
    RenderFlow.Flow.MODEL_OUTPUT: 'Identity:0',

    RenderFlow.Flow.PRERENDER: true,
    RenderFlow.Flow.PRERENDER_DIM: [324, 324],
    RenderFlow.Flow.PRERENDER_DIRECT: true,
    RenderFlow.Flow.PRERENDER_SPLITSCREEN: true,
    
    RenderFlow.Flow.DIM: [648, 648],
    RenderFlow.Flow.FORMAT: 'RGB'
});

// static lastPrerender = void;
// static lastRender = void;


// upscaleFlow.render @(override) := fn(prerender) {
//     // if(lastPrerender == prerender) return lastRender;

//     var in = RenderFlow.uint8ToFloat(prerender);
//     var output = RenderFlow.predict(in);
//     // var result = RenderFlow.floatToUint8(output);

//     // result = splitscreen(prerender, result);

//     // lastPrerender = prerender;
//     // lastRender = result;

//     // out(result);

//     return output;
// };



RenderFlow.upscaleFlow := upscaleFlow;
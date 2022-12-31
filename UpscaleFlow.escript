var upscaleFlow = new RenderFlow.Flow({
    RenderFlow.Flow.NAME: 'upscale',
    RenderFlow.Flow.MODEL: '../extPlugins/RenderFlow/upscaleModel/model.pb',
    RenderFlow.Flow.MODEL_FROZEN_GRAPH: true,
    RenderFlow.Flow.MODEL_SHAPE: [324, 324, 3],
    RenderFlow.Flow.MODEL_INPUT: 'input:0',
    RenderFlow.Flow.MODEL_OUTPUT: 'Identity:0',
    RenderFlow.Flow.PRERENDER: true,
    RenderFlow.Flow.PRERENDER_DIM: [324, 324],
    RenderFlow.Flow.DIM: [2 * 648 + 4, 648],
    RenderFlow.Flow.FORMAT: 'RGB'
});

static lastPrerender = void;
static lastRender = void;

static splitscreen = fn(prerender, render) {
    var r = new Array();

    for(var y = 0; y < 324; y++) {
        for(var x = 0; x < 324; x++) {
            r.append([prerender[y * 324 * 3 + (3 * x)], prerender[y * 324 * 3 + (3 * x + 1)], prerender[y * 324 * 3 + (3 * x + 2)]]);
            r.append([prerender[y * 324 * 3 + (3 * x)], prerender[y * 324 * 3 + (3 * x + 1)], prerender[y * 324 * 3 + (3 * x + 2)]]);
        }
        for(var x = 0; x < 3 * 4; x++)
            r.append([0]);
        
        for(var x = 0; x < 648 * 3; x++)
            r.append([render[(2 * y) * 648 * 3 + x]]);


        for(var x = 0; x < 324; x++) {
            r.append([prerender[y * 324 * 3 + (3 * x)], prerender[y * 324 * 3 + (3 * x + 1)], prerender[y * 324 * 3 + (3 * x + 2)]]);
            r.append([prerender[y * 324 * 3 + (3 * x)], prerender[y * 324 * 3 + (3 * x + 1)], prerender[y * 324 * 3 + (3 * x + 2)]]);
        }

        for(var x = 0; x < 3 * 4; x++)
            r.append([0]);

        for(var x = 0; x < 648 * 3; x++)
            r.append([render[(2 * y + 1) * 648 * 3 + x]]);
    }

    return r;
};

static upscale = fn(prerender) {
    var r = new Array();

    for(var y = 0; y < 324; y++) {
        for(var x = 0; x < 324; x++) {
            r.append([prerender[y * 324 * 3 + (3 * x)], prerender[y * 324 * 3 + (3 * x + 1)], prerender[y * 324 * 3 + (3 * x + 2)]]);
            r.append([prerender[y * 324 * 3 + (3 * x)], prerender[y * 324 * 3 + (3 * x + 1)], prerender[y * 324 * 3 + (3 * x + 2)]]);
        }
         for(var x = 0; x < 324; x++) {
            r.append([prerender[y * 324 * 3 + (3 * x)], prerender[y * 324 * 3 + (3 * x + 1)], prerender[y * 324 * 3 + (3 * x + 2)]]);
            r.append([prerender[y * 324 * 3 + (3 * x)], prerender[y * 324 * 3 + (3 * x + 1)], prerender[y * 324 * 3 + (3 * x + 2)]]);
        }
    }

    return r;
};

// TODO slow

upscaleFlow.render @(override) := fn(prerender) {

    if(lastPrerender == prerender) return lastRender;

    var in = RenderFlow.uint8ToFloat(prerender);
    var output = RenderFlow.predict(in);
    var result = RenderFlow.floatToUint8(output);

    result = splitscreen(prerender, result);

    lastPrerender = prerender;
    lastRender = result;

    out(result);

    return result;
};



RenderFlow.upscaleFlow := upscaleFlow;
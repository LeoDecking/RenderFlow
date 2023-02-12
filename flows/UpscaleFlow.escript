var upscaleFlow = new RenderFlow.Flow({
    RenderFlow.Flow.NAME: 'upscale',

    RenderFlow.Flow.MODEL: __DIR__ + '/upscaleModel/saved',
    RenderFlow.Flow.MODEL_SHAPE: [324, 324, 3],
    RenderFlow.Flow.MODEL_INPUT: 'serving_default',
    RenderFlow.Flow.MODEL_OUTPUT: 'out',

    RenderFlow.Flow.PRERENDER: true,
    RenderFlow.Flow.PRERENDER_DIM: [324, 324],
    RenderFlow.Flow.PRERENDER_DIRECT: true,
    RenderFlow.Flow.PRERENDER_DIRECT_CACHE: false,
    RenderFlow.Flow.PRERENDER_SPLITSCREEN: true,
    
    RenderFlow.Flow.DIM: [648, 648],
    RenderFlow.Flow.FORMAT: 'RGB'
});


RenderFlow.upscaleFlow := upscaleFlow;
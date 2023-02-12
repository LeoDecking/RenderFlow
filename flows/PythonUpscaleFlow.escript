var pythonUpscaleFlow = new RenderFlow.Flow({
    RenderFlow.Flow.NAME: 'pythonUpscale',
    RenderFlow.Flow.PYTHON_PATH: __DIR__ + '/Upscaling.py',

    RenderFlow.Flow.PRERENDER: true,
    RenderFlow.Flow.PRERENDER_DIM: [324, 324],
    RenderFlow.Flow.PRERENDER_DIRECT_CACHE: false,
    RenderFlow.Flow.PRERENDER_DIRECT: true,
    RenderFlow.Flow.PRERENDER_SPLITSCREEN: true,
    
    RenderFlow.Flow.DIM: [648, 648],
    RenderFlow.Flow.FORMAT: 'RGB'
});

RenderFlow.pythonUpscaleFlow := pythonUpscaleFlow;
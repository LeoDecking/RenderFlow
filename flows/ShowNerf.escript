var showNerf = new RenderFlow.Flow({
    RenderFlow.Flow.NAME: 'showNerf',
    RenderFlow.Flow.PYTHON_PATH: __DIR__+'/ShowNerf.py',

    RenderFlow.Flow.PRERENDER: true,
    RenderFlow.Flow.PRERENDER_DIM: [100, 100],
    RenderFlow.Flow.PRERENDER_SPLITSCREEN: true,
    RenderFlow.Flow.PRERENDER_DIRECT_CACHE: false,

    
    RenderFlow.Flow.DATA_FROM_FLOAT: false,
    RenderFlow.Flow.DIM: [100, 100]
});
// RenderFlow.register(showNerf);
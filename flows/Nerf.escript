var nerf = new RenderFlow.Flow({
    RenderFlow.Flow.NAME: 'nerf',
    RenderFlow.Flow.PYTHON_PATH: __DIR__+'/Nerf.py',

    RenderFlow.Flow.PRERENDER: true,
    RenderFlow.Flow.PRERENDER_DIM: [128, 128],
    RenderFlow.Flow.PRERENDER_SPLITSCREEN: true,
    RenderFlow.Flow.PRERENDER_DIRECT_CACHE: false,
    
    RenderFlow.Flow.DATA_FROM_FLOAT: false,
    RenderFlow.Flow.DIM: [128, 128]
});
RenderFlow.register(nerf);
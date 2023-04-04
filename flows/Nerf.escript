var nerf = new RenderFlow.Flow({
    RenderFlow.Flow.NAME: 'nerf',
    RenderFlow.Flow.PYTHON_PATH: __DIR__+'/Nerf.py',

    RenderFlow.Flow.PRERENDER: false,
    
    RenderFlow.Flow.DATA_FROM_FLOAT: false,
    RenderFlow.Flow.DIM: [100, 100]
});
RenderFlow.register(nerf);
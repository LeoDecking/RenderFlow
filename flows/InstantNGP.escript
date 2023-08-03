var instantNgp = new RenderFlow.Flow({
    RenderFlow.Flow.NAME: 'instant-ngp',
    RenderFlow.Flow.PYTHON_PATH: __DIR__+'/InstantNGP.py',

    RenderFlow.Flow.PRERENDER: true,
    RenderFlow.Flow.PRERENDER_DIM: [512, 512],
    RenderFlow.Flow.PRERENDER_SPLITSCREEN: true,
    RenderFlow.Flow.PRERENDER_DIRECT_CACHE: false,
    
    RenderFlow.Flow.DATA_FROM_FLOAT: false,
    RenderFlow.Flow.DIM: [512, 512]
});
RenderFlow.register(instantNgp);
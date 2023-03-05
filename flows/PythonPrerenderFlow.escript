var pythonPrerenderFlow = new RenderFlow.Flow({
    RenderFlow.Flow.NAME: 'pythonPrerender',
    RenderFlow.Flow.PYTHON_PATH: __DIR__+'/Prerender.py',

    RenderFlow.Flow.PRERENDER: true,
    RenderFlow.Flow.PRERENDER_DIM: [648, 648],
    RenderFlow.Flow.PRERENDER_SPLITSCREEN: true,
    
    RenderFlow.Flow.DIM: [648, 648],
    RenderFlow.Flow.FORMAT: 'RGB'
});

// RenderFlow.pythonPrerenderFlow := pythonPrerenderFlow;
RenderFlow.register(pythonPrerenderFlow);
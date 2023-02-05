var pythonPrerenderFlow = new RenderFlow.Flow({
    RenderFlow.Flow.NAME: 'python',
    RenderFlow.Flow.PYTHON_PATH: __DIR__+'/PrerenderTest.py',

    RenderFlow.Flow.PRERENDER: true,
    RenderFlow.Flow.PRERENDER_DIM: [648, 648],
    RenderFlow.Flow.PRERENDER_DIRECT_CACHE: false,
    RenderFlow.Flow.PRERENDER_SPLITSCREEN: true,
    
    RenderFlow.Flow.DIM: [648, 648],
    RenderFlow.Flow.FORMAT: 'RGB'
});

RenderFlow.pythonPrerenderFlow := pythonPrerenderFlow;
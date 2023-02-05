var pythonFlow = new RenderFlow.Flow({
    RenderFlow.Flow.NAME: 'python',
    RenderFlow.Flow.PYTHON_PATH: __DIR__+'/Test.py',

    RenderFlow.Flow.PRERENDER: false,
    
    RenderFlow.Flow.DIM: [648, 648],
    RenderFlow.Flow.FORMAT: 'RGB'
});

RenderFlow.pythonFlow := pythonFlow;
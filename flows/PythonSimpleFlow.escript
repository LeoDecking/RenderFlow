var pythonSimpleFlow = new RenderFlow.Flow({
    RenderFlow.Flow.NAME: 'pythonSimpleFlow',
    RenderFlow.Flow.PYTHON_PATH: __DIR__ + '/Simple.py',
    
    RenderFlow.Flow.DIM: [100, 100],
    RenderFlow.Flow.FORMAT: 'RGB'
});

RenderFlow.pythonSimpleFlow := pythonSimpleFlow;
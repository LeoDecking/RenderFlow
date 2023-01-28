var pythonMyFlow = new RenderFlow.Flow({
    RenderFlow.Flow.NAME: 'pythonMyFlow',
    RenderFlow.Flow.PYTHON_PATH: 'MyFlow',

    RenderFlow.Flow.PRERENDER: false,
    
    RenderFlow.Flow.DATA_FROM_FLOAT: true,
    RenderFlow.Flow.DIM: [64, 64],
    RenderFlow.Flow.FORMAT: 'MONO_COLORMAP'
});
RenderFlow.pythonMyFlow := pythonMyFlow;
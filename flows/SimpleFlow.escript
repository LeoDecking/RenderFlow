var simpleFlow = new RenderFlow.Flow({
    RenderFlow.Flow.NAME: 'simpleFlow',
    RenderFlow.Flow.DIM: [100, 100],
    RenderFlow.Flow.FORMAT: 'RGB'
});

simpleFlow.onActivate @(override) := fn() {
    outln("simpleFlow activated");
};
simpleFlow.onDeactivate @(override) := fn() {
    outln("simpleFlow deactivated");
};

simpleFlow.render @(override) := fn() {
    var output = [];

    for(var i = 0; i < 100 * 100 * 3; i++) {
        output[i] = ((i / 3) + (i % 3) * 40) % 256;
    }
    
    return output;
};

RenderFlow.simpleFlow := simpleFlow;
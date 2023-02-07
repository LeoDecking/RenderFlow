static plugin = new Plugin({
		Plugin.NAME : 'RenderFlow', // ideally the same as the subdirectory this file is in
		Plugin.DESCRIPTION : 'Use tensorflow models for rendering',
		Plugin.VERSION : 0.2,
		Plugin.AUTHORS : "Leo Decking",
		Plugin.OWNER : "Leo Decking",
		Plugin.LICENSE : "Some License", // TODO
		Plugin.REQUIRES : [ ], // will be explained later
		Plugin.EXTENSION_POINTS : [ ] // will be explained later
});

static loadedFlow = void;
static loadedModel = void;

// This function will be called only if this plugin is marked as active
plugin.init @(override) := fn() {
	declareNamespace($RenderFlow); // Define the namespace RenderFlow
	RenderFlow.activate := activate;
	RenderFlow.deactivate := deactivate;
	RenderFlow.getCameraAngles := getCameraAngles;
	RenderFlow.getFlow := fn() { return loadedFlow; };

	load(__DIR__ + "/Flow.escript");
	load(__DIR__ + "/flows/MyFlow.escript"); // TODO remove
	load(__DIR__ + "/flows/UpscaleFlow.escript"); // TODO remove
	load(__DIR__ + "/flows/PythonFlow.escript"); // TODO remove
	load(__DIR__ + "/flows/PythonMyFlow.escript"); // TODO remove
	load(__DIR__ + "/flows/PythonPrerenderFlow.escript"); // TODO remove
	load(__DIR__ + "/flows/PythonUpscaleFlow.escript"); // TODO remove

	 // Create an instance of the LibraryLoader.
    var loader = new (Std.module("LibUtilExt/LibraryLoader"));
    
    // We need to add our build path to the LibraryLoader so it can find the library
    loader.addSearchPath(__DIR__ + "/build/");
    
    // Searches for the file 'libMyProject.so' (linux) or 'libMyProject.dll' (windows) 
    // and calls the C entry point function 'loadLibary'.
    if(!loader.loadLibary("RenderFlow")) {
        return false;
    }
    
    

	return true; // true means that we have initialized it without any errors
};

static activate = fn(flow) {
	if(loadedFlow && loadedFlow != flow) {
		deactivate();
	}
	if(!loadedFlow) {
		if(flow.getModel() && loadedModel != flow.getModel() + flow.getModelInput() + flow.getModelOutput()) {
			RenderFlow.loadModel(__DIR__ + "/PythonTensorflow.py", flow.getModel(), flow.getModelShape(), flow.getModelInput(), flow.getModelOutput());
			loadedModel = flow.getModel() + flow.getModelInput() + flow.getModelOutput();
		}

		loadedFlow = flow;

		PADrend.executeCommand(fn(){PPEffectPlugin.loadAndSetEffect("../extPlugins/RenderFlow/Effect.escript");});
	}
	// python is reloaded every time
	if(flow.getPythonPath()) RenderFlow.pythonInit(flow.getPythonPath());
};
static deactivate = fn() {
	if(loadedFlow) {
		PADrend.executeCommand(fn(){PPEffectPlugin.setEffect(false);});
		loadedFlow = void;
	}
};

static getCameraAngles = fn(cam) {
    var x = cam.getWorldTransformationMatrix().transformDirection([1, 0, 0]);
    var angleH = x.getX().acos().radToDeg();
    if (x.getX() == 1)
        angleH = 0;
    if (x.getZ() < 0)
        angleH = 360 - angleH;

    var y = cam.getWorldTransformationMatrix().transformDirection([0, 1, 0]);
    var angleV = y.getY().acos().radToDeg();
    if (y.getY() == 1)
        angleH = 0;
    if (y.getX() < 0)
        angleV = 360 - angleV;

	return [angleH, angleV];
};

return plugin; // Important: You HAVE to return a plugin object
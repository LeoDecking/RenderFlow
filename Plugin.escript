static plugin = new Plugin({
		Plugin.NAME : 'RenderFlow', // ideally the same as the subdirectory this file is in
		Plugin.DESCRIPTION : 'Use tensorflow models or Python for rendering',
		Plugin.VERSION : 0.2,
		Plugin.AUTHORS : "Leo Decking",
		Plugin.OWNER : "Leo Decking",
		Plugin.LICENSE : "",
		Plugin.REQUIRES : [ ], // will be explained later
		Plugin.EXTENSION_POINTS : [ ] // will be explained later
});

static loadedFlow = void;
static loadedModel = void;

static flows = [];

// This function will be called only if this plugin is marked as active
plugin.init @(override) := fn() {
	declareNamespace($RenderFlow); // Define the namespace RenderFlow
	RenderFlow.register := register;
	RenderFlow.activate := activate;
	RenderFlow.deactivate := deactivate;
	RenderFlow.getCameraAngles := getCameraAngles;
	RenderFlow.getFlow := fn() { return loadedFlow; };

	load(__DIR__ + "/Screenshot.escript");

	load(__DIR__ + "/Flow.escript");
	load(__DIR__ + "/flows/SimpleFlow.escript");
	load(__DIR__ + "/flows/MyFlow.escript");
	load(__DIR__ + "/flows/UpscaleFlow.escript");
	load(__DIR__ + "/flows/PythonSimpleFlow.escript");
	load(__DIR__ + "/flows/PythonMyFlow.escript");
	load(__DIR__ + "/flows/PythonPrerenderFlow.escript");
	load(__DIR__ + "/flows/PythonUpscaleFlow.escript");
	load(__DIR__ + "/flows/TinyNerf.escript");
	load(__DIR__ + "/flows/InstantNGP.escript");

	module.on('PADrend/gui', initGUI);

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

static initGUI = fn(gui) {
	outln("RenderFlow: Init GUI...");
		
	// Adds the menu entry 'RenderFlow' to the plugins menu.
	gui.register('PADrend_PluginsMenu.RenderFlow',[
		{
			GUI.TYPE : GUI.TYPE_MENU,
			GUI.LABEL : "RenderFlow",
			GUI.MENU : 'PADrend_RenderFlowMenu',
			GUI.MENU_WIDTH : 150
		}
	]);

	gui.register('PADrend_RenderFlowMenu.main',fn(){
		var entries = [];
		entries += "*Flows*";

		foreach(flows as var flow) {
			entries += {
				GUI.TYPE : GUI.TYPE_BOOL,
				GUI.LABEL : flow.getName(),
				GUI.DATA_VALUE : loadedFlow == flow,
				GUI.ON_DATA_CHANGED : [flow]=>fn(flow,value){
					gui.closeAllMenus();
					if(loadedFlow == flow) deactivate();
					else activate(flow);
				},
				GUI.TOOLTIP : (flow.getPythonPath() ? "Python: " + flow.getPythonPath() : "") + (flow.getPythonPath() && flow.getModel() ? "\n":"") + (flow.getModel() ? "Model: " + flow.getModel() : "")
			};
		}
		
		return entries;
	});
};

static register = fn(flow) {
	flows.append([flow]);
};

static activate = fn(flow) {
	if(loadedFlow)
		deactivate();

	
	if(flow.getModel())
		RenderFlow.loadModel(__DIR__ + "/PythonTensorflow.py", flow.getModel(), flow.getModelShape(), flow.getModelInput(), flow.getModelOutput());
	
	loadedFlow = flow;
	if(flow.getPythonPath()) RenderFlow.pythonInit(flow.getPythonPath());

	PADrend.executeCommand(fn(){PPEffectPlugin.loadAndSetEffect("../extPlugins/RenderFlow/Effect.escript");});

	flow.onActivate();
};
static deactivate = fn() {
	if(loadedFlow) {
		PADrend.executeCommand(fn(){PPEffectPlugin.setEffect(false);});

		if(loadedFlow.getPythonPath()) RenderFlow.finalizeModule();
		if(loadedFlow.getModel()) RenderFlow.unloadModel();

		loadedFlow.onDeactivate();
		loadedFlow = void;
	}
};

static getCameraAngles = fn(cam=PADrend.getActiveCamera()) {
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
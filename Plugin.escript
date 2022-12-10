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

// This function will be called only if this plugin is marked as active
plugin.init @(override) := fn() {
	// declareNamespace($RenderFlow); // Define the namespace RenderFlow
	// RenderFlow.someValue := 42; // now you can access the RenderFlow namespace
	// load(__DIR__ + "/someOtherFile.escript"); // You can also execute other escript files from here

	 // Create an instance of the LibraryLoader.
    var loader = new (Std.module("LibUtilExt/LibraryLoader"));
    
    // We need to add our build path to the LibraryLoader so it can find the library
    loader.addSearchPath(__DIR__ + "/build/");
    
    // Searches for the file 'libMyProject.so' (linux) or 'libMyProject.dll' (windows) 
    // and calls the C entry point function 'loadLibary'.
    if(!loader.loadLibary("RenderFlow")) {
        return false;
    }
    
    // Call the helloWorld method of our library
    RenderFlow.helloWorld();

	return true; // true means that we have initialized it without any errors
};

static srLoaded = false;

plugin.activate := fn() {
	if(!srLoaded) {
		PADrend.executeCommand(fn(){PPEffectPlugin.loadAndSetEffect("../extPlugins/RenderFlow/Effect.escript");});
		srLoaded = true;
	}
};
plugin.deactivate := fn() {
	if(srLoaded) {
		PADrend.executeCommand(fn(){PPEffectPlugin.setEffect(false);});
		srLoaded = false;

		// re-enable rendering of the RootNode
		var states = PADrend.getRootNode().getStates();
		if (states.size() >= 1) {
			PADrend.getRootNode().removeState(states[states.size()-1]); // states[0] == lighting
		}
		else {
			Runtime.warn("Could not reactivate the root node!");
		}
	}
};

return plugin; // Important: You HAVE to return a plugin object
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
	RenderFlow.colormap := colormap;
	RenderFlow.getFlow := fn() { return loadedFlow; };

	load(__DIR__ + "/Flow.escript");
	load(__DIR__ + "/MyFlow.escript"); // TODO remove

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
		if(!loadedModel != flow.getModel()) RenderFlow.loadModel(flow.getModel());
		loadedFlow = flow;
		loadedModel = flow.getModel();
		PADrend.executeCommand(fn(){PPEffectPlugin.loadAndSetEffect("../extPlugins/RenderFlow/Effect.escript");});
	}
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

static cmap = [[68, 1, 84], [68, 2, 86], [69, 4, 87], [69, 5, 89], [70, 7, 90], [70, 8, 92], [70, 10, 93], [70, 11, 94], [71, 13, 96], [71, 14, 97], [71, 16, 99], [71, 17, 100], [71, 19, 101], [72, 20, 103], [72, 22, 104], [72, 23, 105], [72, 24, 106], [72, 26, 108], [72, 27, 109], [72, 28, 110], [72, 29, 111], [72, 31, 112], [72, 32, 113], [72, 33, 115], [72, 35, 116], [72, 36, 117], [72, 37, 118], [72, 38, 119], [72, 40, 120], [72, 41, 121], [71, 42, 122], [71, 44, 122], [71, 45, 123], [71, 46, 124], [71, 47, 125], [70, 48, 126], [70, 50, 126], [70, 51, 127], [70, 52, 128], [69, 53, 129], [69, 55, 129], [69, 56, 130], [68, 57, 131], [68, 58, 131], [68, 59, 132], [67, 61, 132], [67, 62, 133], [66, 63, 133], [66, 64, 134], [66, 65, 134], [65, 66, 135], [65, 68, 135], [64, 69, 136], [64, 70, 136], [63, 71, 136], [63, 72, 137], [62, 73, 137], [62, 74, 137], [62, 76, 138], [61, 77, 138], [61, 78, 138], [60, 79, 138], [60, 80, 139], [59, 81, 139], [59, 82, 139], [58, 83, 139], [58, 84, 140], [57, 85, 140], [57, 86, 140], [56, 88, 140], [56, 89, 140], [55, 90, 140], [55, 91, 141], [54, 92, 141], [54, 93, 141], [53, 94, 141], [53, 95, 141], [52, 96, 141], [52, 97, 141], [51, 98, 141], [51, 99, 141], [50, 100, 142], [50, 101, 142], [49, 102, 142], [49, 103, 142], [49, 104, 142], [48, 105, 142], [48, 106, 142], [47, 107, 142], [47, 108, 142], [46, 109, 142], [46, 110, 142], [46, 111, 142], [45, 112, 142], [45, 113, 142], [44, 113, 142], [44, 114, 142], [44, 115, 142], [43, 116, 142], [43, 117, 142], [42, 118, 142], [42, 119, 142], [42, 120, 142], [41, 121, 142], [41, 122, 142], [41, 123, 142], [40, 124, 142], [40, 125, 142], [39, 126, 142], [39, 127, 142], [39, 128, 142], [38, 129, 142], [38, 130, 142], [38, 130, 142], [37, 131, 142], [37, 132, 142], [37, 133, 142], [36, 134, 142], [36, 135, 142], [35, 136, 142], [35, 137, 142], [35, 138, 141], [34, 139, 141], [34, 140, 141], [34, 141, 141], [33, 142, 141], [33, 143, 141], [33, 144, 141], [33, 145, 140], [32, 146, 140], [32, 146, 140], [32, 147, 140], [31, 148, 140], [31, 149, 139], [31, 150, 139], [31, 151, 139], [31, 152, 139], [31, 153, 138], [31, 154, 138], [30, 155, 138], [30, 156, 137], [30, 157, 137], [31, 158, 137], [31, 159, 136], [31, 160, 136], [31, 161, 136], [31, 161, 135], [31, 162, 135], [32, 163, 134], [32, 164, 134], [33, 165, 133], [33, 166, 133], [34, 167, 133], [34, 168, 132], [35, 169, 131], [36, 170, 131], [37, 171, 130], [37, 172, 130], [38, 173, 129], [39, 173, 129], [40, 174, 128], [41, 175, 127], [42, 176, 127], [44, 177, 126], [45, 178, 125], [46, 179, 124], [47, 180, 124], [49, 181, 123], [50, 182, 122], [52, 182, 121], [53, 183, 121], [55, 184, 120], [56, 185, 119], [58, 186, 118], [59, 187, 117], [61, 188, 116], [63, 188, 115], [64, 189, 114], [66, 190, 113], [68, 191, 112], [70, 192, 111], [72, 193, 110], [74, 193, 109], [76, 194, 108], [78, 195, 107], [80, 196, 106], [82, 197, 105], [84, 197, 104], [86, 198, 103], [88, 199, 101], [90, 200, 100], [92, 200, 99], [94, 201, 98], [96, 202, 96], [99, 203, 95], [101, 203, 94], [103, 204, 92], [105, 205, 91], [108, 205, 90], [110, 206, 88], [112, 207, 87], [115, 208, 86], [117, 208, 84], [119, 209, 83], [122, 209, 81], [124, 210, 80], [127, 211, 78], [129, 211, 77], [132, 212, 75], [134, 213, 73], [137, 213, 72], [139, 214, 70], [142, 214, 69], [144, 215, 67], [147, 215, 65], [149, 216, 64], [152, 216, 62], [155, 217, 60], [157, 217, 59], [160, 218, 57], [162, 218, 55], [165, 219, 54], [168, 219, 52], [170, 220, 50], [173, 220, 48], [176, 221, 47], [178, 221, 45], [181, 222, 43], [184, 222, 41], [186, 222, 40], [189, 223, 38], [192, 223, 37], [194, 223, 35], [197, 224, 33], [200, 224, 32], [202, 225, 31], [205, 225, 29], [208, 225, 28], [210, 226, 27], [213, 226, 26], [216, 226, 25], [218, 227, 25], [221, 227, 24], [223, 227, 24], [226, 228, 24], [229, 228, 25], [231, 228, 25], [234, 229, 26], [236, 229, 27], [239, 229, 28], [241, 229, 29], [244, 230, 30], [246, 230, 32], [248, 230, 33], [251, 231, 35], [253, 231, 37]];
// data: [0,1)^*
static colormap = fn(data) {
	var r = new Array();
    for(var i = 0; i < data.size(); i++) {
        var o = data[i];
        if(o < 0) o = 0;
        else if (o > 1) o = 255;
        else o = (o * 255).floor();
        r.append([cmap[o][0], cmap[o][1], cmap[o][2]]);
    }
	return r;
};

return plugin; // Important: You HAVE to return a plugin object
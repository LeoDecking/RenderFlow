T = new Type;

T._properties @(private) := void;

T.NAME ::= 'name';
T.MODEL ::= 'model';
T.PRERENDER ::= 'prerender';
T.PRERENDER_DIM ::= 'prerenderDim';
T.DIM ::= 'dim';
T.FORMAT ::= 'format';

/*
    NAME:           string
    MODEL:          path
    PRERENDER:      boolean
    PRERENDER_DIM:  [x, y]
    DIM:            [x, y]
    FORMAT:         RGB | MONO | MONO_COLORMAP
*/

T._constructor ::= fn( Map properties){
	_properties = properties.clone();
};

T.getName		::= fn(){	return this._pluginProperties[T.NAME];	};
T.getModel		::= fn(){	return this._pluginProperties[T.MODEL];	};
T.getPrerender		::= fn(){	return this._pluginProperties[T.PRERENDER];	};
T.getPrerenderDim		::= fn(){	return this._pluginProperties[T.PRERENDER_DIM];	};
T.getPrerenderDimX		::= fn(){	return this._pluginProperties[T.PRERENDER_DIM][0];	};
T.getPrerenderDimY		::= fn(){	return this._pluginProperties[T.PRERENDER_DIM][1];	};
T.getDim		::= fn(){	return this._pluginProperties[T.DIM];	};
T.getDimX		::= fn(){	return this._pluginProperties[T.DIM][0];	};
T.getDimY		::= fn(){	return this._pluginProperties[T.DIM][1];	};
T.getFormat		::= fn(){	return this._pluginProperties[T.FORMAT];	};
T.getProperties 	::= fn(){	return this._properties;	};
T.getProperty		::= fn(key){	return this._properties[key];	};

T.render := fn(prerender) {
    var r = new Array();
    
    for(var i = 0; i < getDimX() * getDimY(); i++) {
        if(getFormat() == "RGB")
            r.append([i % 256, i % 256, i % 256]);
        else
            r.append([i % 256]);
    }

	return r;
};


RenderFlow.Flow := T;
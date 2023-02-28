#ifndef E_RENDERFLOW_PYTHONMODULE_H_
#define E_RENDERFLOW_PYTHONMODULE_H_

#include <EScript/EScript.h>
#include <EScript/Objects/ReferenceObject.h>

#include "PythonModule.h"

class E_PythonModule : public EScript::ReferenceObject<PythonModule> {

    ES_PROVIDES_TYPE_NAME(PythonModule)
public:
    static EScript::Type * getTypeObject();
    static void init(EScript::Namespace &lib);

    template<typename...args> explicit E_PythonModule(args&&... params) : ReferenceObject_t(E_PythonModule::getTypeObject(), std::forward<args>(params)...) {}

    virtual ~E_PythonModule() {}

};

ES_CONV_EOBJ_TO_OBJ(E_PythonModule, PythonModule&, **eObj)
ES_CONV_EOBJ_TO_OBJ(E_PythonModule, PythonModule*, &**eObj)

ES_CONV_OBJ_TO_EOBJ(PythonModule&, E_PythonModule, new E_PythonModule(obj))
ES_CONV_OBJ_TO_EOBJ(PythonModule*, E_PythonModule, new E_PythonModule(*obj))


#endif
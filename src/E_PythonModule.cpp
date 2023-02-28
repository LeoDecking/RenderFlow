#include <EScript/StdObjects.h>
#include <string>
#include <iostream>
#include <cstdlib>
#include "PythonRender.h"
#include "E_PythonModule.h"

EScript::Type *E_PythonModule::getTypeObject()
{
    static EScript::ERef<EScript::Type> typeObject = new EScript::Type(EScript::Object::getTypeObject());
    return typeObject.get();
}

void E_PythonModule::init(EScript::Namespace &lib)
{
    EScript::Type *typeObject = getTypeObject();
    declareConstant(&lib, getClassName(), typeObject);

    // path, reload=true
    ES_CONSTRUCTOR(typeObject, 1, 2, {
        PythonRender::init(rt);
        return EScript::create(new PythonModule(parameter[0].toString(), parameter[1].toBool(true)));
    });

    auto a = EScript::Array::create();

    ES_MFUNCTION(typeObject, PythonModule, "execute", 1, -1, {
        EScript::Array *a = EScript::Array::create();
        for (int i = 1; i < parameter.size(); i++)
            a->pushBack(parameter[i].get());

        return thisObj->execute(parameter[0].toString(), a);
    });
}

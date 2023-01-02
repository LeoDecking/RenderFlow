#include "ELibRenderFlow.h"

#include <cppflow/cppflow.h>

#include <E_Util/E_Utils.h>
#include <EScript/Basics.h>
#include <EScript/Objects/Collections/Array.h>
#include <EScript/StdObjects.h>

#include <Geometry/Angle.h>
#include <Geometry/Vec3.h>

#include <E_Rendering/E_RenderingContext.h>
#include <E_Rendering/Texture/E_Texture.h>

#include <E_Geometry/E_Vec3.h>
#include <MinSG/Core/Nodes/CameraNode.h>
#include <E_MinSG/Core/Nodes/E_CameraNode.h>

#include <MinSG/Core/FrameContext.h>
#include <Rendering/RenderingContext/RenderingContext.h>
#include <Rendering/Texture/Texture.h>
#include <Rendering/Texture/TextureUtils.h>
#include <Util/Graphics/Bitmap.h>
#include <Util/Graphics/PixelFormat.h>

#include <iostream> // TODO remove
#include <algorithm>

namespace RenderFlow
{

    cppflow::model *model;
    std::string inputOperation;
    std::string outputOperation;
    std::vector<int64_t> shape;

    EScript::Array *uint8ToFloat(EScript::Array *in) {
        std::vector<float> out;
        for (int i = 0; i < in->size(); i++)
            out.push_back(in->at(i).toFloat() / 255);

        return EScript::Array::create(out);
    }
    EScript::Array *floatToUint8(EScript::Array *in) {
        std::vector<int> out;
        for (int i = 0; i < in->size(); i++)
            out.push_back(std::clamp(in->at(i).toFloat(), 0.0f, 1.0f) * 255);

        return EScript::Array::create(out);
    }

    EScript::Array *predict(EScript::Array *in)
    {
        std::vector<float> inVector;
        for (int i = 0; i < in->size(); i++)
        {
            inVector.push_back(in->at(i).toFloat());
        }

        cppflow::tensor input = cppflow::tensor(inVector, shape);
        input = cppflow::expand_dims(input, 0);

        cppflow::tensor output = (*model)({{inputOperation, input}}, {{outputOperation}})[0];
        output = cppflow::squeeze(output, {0});

        if (output.dtype() == TF_HALF)
            output = cppflow::cast(output, TF_HALF, TF_FLOAT);
        return EScript::Array::create(output.get_data<float>());
    }

    // Output "Hello World!" to the console.
    void helloWorld()
    {
        std::cout << "Hello World!" << std::endl;

        auto a = cppflow::tensor({1.0, 2.0, 3.0});
        // Create a tensor of shape 3 filled with 1.0, b = [1.0, 1.0, 1.0]
        auto b = cppflow::fill({3}, 1.0);

        std::cout << a + b << std::endl;
    }

    // Initializes your EScript bindings
    void init(EScript::Namespace *lib)
    {
        ES_FUNCTION(lib, "helloWorld", 0, 0,
                    {
                        helloWorld();
                        return thisEObj;
                    });

        ES_FUNCTION(lib, "loadModel", 5, 5,
                    {
                        std::cout << "load: '" << parameter[0].toString() << "'" << std::endl;

                        if (parameter[1].toBool())
                            model = new cppflow::model(parameter[0].toString(), cppflow::model::FROZEN_GRAPH);
                        else
                            model = new cppflow::model(parameter[0].toString());

                        EScript::Array *spapeArray = parameter[2].to<EScript::Array *>(rt);
                        shape.clear();
                        for (int i = 0; i < spapeArray->size(); i++)
                        {
                            shape.push_back(spapeArray->at(i).to<int64_t>(rt));
                        }

                        inputOperation = parameter[3].toString();  // TODO check if exists
                        outputOperation = parameter[4].toString(); // TODO check if exists
                                                                   // for (std::string s : model->get_operations())

                        return thisEObj;
                    });

        ES_FUNCTION(lib, "uint8ToFloat", 1, 1,
                    {
                        return uint8ToFloat(parameter[0].to<EScript::Array *>(rt));
                    });

        ES_FUNCTION(lib, "floatToUint8", 1, 1,
                    {
                        return floatToUint8(parameter[0].to<EScript::Array *>(rt));
                    });


        ES_FUNCTION(lib, "predict", 1, 1, {
            return predict(parameter[0].to<EScript::Array *>(rt));
        });
    }

}
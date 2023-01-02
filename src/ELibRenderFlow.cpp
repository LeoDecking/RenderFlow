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
#include <time.h>
#include <algorithm>

namespace RenderFlow
{

    cppflow::model *model;
    std::string inputOperation;
    std::string outputOperation;
    std::vector<int64_t> shape;

    EScript::Array *predict(EScript::Array *in)
    {
        clock_t start, end;
        start = clock();

        std::vector<float> inVector;
        for (int i = 0; i < in->size(); i++)
        {
            inVector.push_back(in->at(i).toFloat());
        }

        clock_t start1, end1;
        start1 = clock();

        cppflow::tensor input = cppflow::tensor(inVector, shape);
        input = cppflow::expand_dims(input, 0);

        cppflow::tensor output = (*model)({{inputOperation, input}}, {{outputOperation}})[0];
        output = cppflow::squeeze(output, {0});

        if (output.dtype() == TF_HALF)
            output = cppflow::cast(output, TF_HALF, TF_FLOAT);

        end1 = clock();
        std::cout << "predict: " << (end1-start1) << std::endl;

        EScript::Array *result = EScript::Array::create(output.get_data<float>());

        end = clock();
        std::cout << "predict': " << (end-start) << std::endl;

        return result;
    }

    EScript::Array *uint8ToFloat(EScript::Array *in)
    {
        clock_t start, end;
        start = clock();

        std::vector<float> out;
        for (int i = 0; i < in->size(); i++)
            out.push_back(in->at(i).toFloat() / 255);

        std::cout << "uint8ToFloat: " << (clock() - start) << std::endl;

        EScript::Array *result = EScript::Array::create(out);

        end = clock();
        std::cout << "uint8ToFloat': " << (end-start) << std::endl;

        return result;
    }

    EScript::Array *floatToUint8(EScript::Array *in)
    {
        clock_t start, end;
        start = clock();

        std::vector<int> out;
        for (int i = 0; i < in->size(); i++)
            out.push_back(std::clamp(in->at(i).toFloat(), 0.0f, 1.0f) * 255);

        std::cout << "floatToUint8: " << (clock()-start) << std::endl;
        EScript::Array *result = EScript::Array::create(out);
        
        end = clock();
        std::cout << "floatToUint8': " << (end-start) << std::endl;

        return result;
    }

    EScript::Array *splitscreenDim(EScript::Array *dim1, EScript::Array *dim2)
    {
        int zoom1 = std::max(1, dim2->at(1).toInt() / dim1->at(1).toInt());
        int zoom2 = std::max(1, dim1->at(1).toInt() / dim2->at(1).toInt());

        int width1 = zoom1 * dim1->at(0).toInt();
        int width2 = zoom2 * dim2->at(0).toInt();
        int height1 = zoom1 * dim1->at(1).toInt();
        int height2 = zoom2 * dim2->at(1).toInt();

        int height = std::max(height1, height2);
        int offset1 = (height - height1) / 2;
        int offset2 = (height - height2) / 2;

        int width = width1 + width2;
        int border = width / 300;
        width += border;

        return EScript::Array::create(std::vector<int>{width, height});
    }

    EScript::Array *splitscreen(EScript::Array *in1, EScript::Array *dim1, EScript::Array *in2, EScript::Array *dim2)
    {
        clock_t start, end;
        start = clock();

        std::vector<uint8_t> inV1;
        for (int i = 0; i < in1->size(); i++)
        {
            inV1.push_back(in1->at(i).toInt());
        }
        std::vector<uint8_t> inV2;
        for (int i = 0; i < in2->size(); i++)
        {
            inV1.push_back(in2->at(i).toInt());
        }

        clock_t start1, end1;
        start1 = clock();

        int zoom1 = std::max(1, dim2->at(1).toInt() / dim1->at(1).toInt());
        int zoom2 = std::max(1, dim1->at(1).toInt() / dim2->at(1).toInt());

        int width1 = zoom1 * dim1->at(0).toInt();
        int width2 = zoom2 * dim2->at(0).toInt();
        int height1 = zoom1 * dim1->at(1).toInt();
        int height2 = zoom2 * dim2->at(1).toInt();

        int height = std::max(height1, height2);
        int offset1 = (height - height1) / 2;
        int offset2 = (height - height2) / 2;

        int width = width1 + width2;
        int border = width / 300;
        width += border;

        std::vector<int> out(3 * width * height, 0);

        for (int y = 0; y < height1; y++)
            for (int x = 0; x < width1; x++)
                for (int i = 0; i < 3; i++)
                    out[3 * (width * (y + offset1) + x) + i] = in1->at(3 * ((width1 / zoom1) * (y / zoom1) + (x / zoom1)) + i).toInt();
        for (int y = 0; y < height2; y++)
            for (int x = 0; x < width2; x++)
                for (int i = 0; i < 3; i++)
                    out[3 * (width * (y + offset2) + width1 + border + x) + i] = in2->at(3 * ((width2 / zoom2) * (y / zoom2) + (x / zoom2)) + i).toInt();

        std::cout << "splitscreen: " << (clock()-start1) << std::endl;

        EScript::Array *array = EScript::Array::create();
        array->pushBack(EScript::Array::create(out));
        array->pushBack(EScript::Array::create(std::vector<int>{width, height}));

        end = clock();
        std::cout << "splitscreen': " << (end-start) << std::endl;

        return array;
    }

    // TODO remove
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

        ES_FUNCTION(lib, "predict", 1, 1, {
            return predict(parameter[0].to<EScript::Array *>(rt));
        });

        ES_FUNCTION(lib, "uint8ToFloat", 1, 1,
                    {
                        return uint8ToFloat(parameter[0].to<EScript::Array *>(rt));
                    });

        ES_FUNCTION(lib, "floatToUint8", 1, 1,
                    {
                        return floatToUint8(parameter[0].to<EScript::Array *>(rt));
                    });

        ES_FUNCTION(lib, "splitscreenDim", 2, 2,
                    {
                        return splitscreenDim(parameter[0].to<EScript::Array *>(rt), parameter[1].to<EScript::Array *>(rt));
                    });

        ES_FUNCTION(lib, "splitscreen", 4, 4,
                    {
                        return splitscreen(parameter[0].to<EScript::Array *>(rt), parameter[1].to<EScript::Array *>(rt), parameter[2].to<EScript::Array *>(rt), parameter[3].to<EScript::Array *>(rt));
                    });
    }

}
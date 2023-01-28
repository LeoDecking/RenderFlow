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
#include <Rendering/Texture/Texture.h>

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

#include "PythonRender.h"

namespace RenderFlow
{

    std::string loadedPython;

    cppflow::model *model;
    std::string inputOperation;
    std::string outputOperation;
    std::vector<int64_t> shape;

    std::vector<float> lastInput;
    std::vector<int> lastInputI;
    std::vector<float> lastOutput;

    std::vector<float> predict(std::vector<float> &in, bool cache)
    {
        if (cache && in.size() == lastInput.size())
        {
            bool same = true;
            for (int i = 0; i < in.size(); i++)
                if (in[i] != lastInput[i])
                {
                    same = false;
                    break;
                }
            if (same)
                return lastOutput;
        }

        cppflow::tensor input = cppflow::tensor(in, shape);
        input = cppflow::expand_dims(input, 0);

        cppflow::tensor output = (*model)({{inputOperation, input}}, {{outputOperation}})[0];
        output = cppflow::squeeze(output, {0});

        if (output.dtype() == TF_HALF)
            output = cppflow::cast(output, TF_HALF, TF_FLOAT);

        std::vector<float> out = output.get_data<float>();

        if (cache)
        {
            lastInput = in;
            lastOutput = out;
        }

        return out;
    }
    std::vector<float> prerenderedPython(std::vector<int> &in, bool cache)
    {
        if (cache && in.size() == lastInputI.size())
        {
            bool same = true;
            for (int i = 0; i < in.size(); i++)
                if (in[i] != lastInputI[i])
                {
                    same = false;
                    break;
                }
            if (same)
                return lastOutput;
        }

        std::vector<float> out = PythonRender::render(in);

        if (cache)
        {
            lastInputI = in;
            lastOutput = out;
        }

        return out;
    }

    EScript::Array *uint8ToFloat(EScript::Array *in)
    {
        // clock_t start, end;
        // start = clock();

        std::vector<float> out;
        for (int i = 0; i < in->size(); i++)
            out.push_back(in->at(i).toFloat() / 255);

        // std::cout << "uint8ToFloat: " << (clock() - start) << std::endl;

        EScript::Array *result = EScript::Array::create(out);

        // end = clock();
        // std::cout << "uint8ToFloat': " << (end - start) << std::endl;

        return result;
    }

    EScript::Array *floatToUint8(EScript::Array *in)
    {
        // clock_t start, end;
        // start = clock();

        std::vector<int> out;
        for (int i = 0; i < in->size(); i++)
            out.push_back(std::clamp(in->at(i).toFloat(), 0.0f, 1.0f) * 255);

        // std::cout << "floatToUint8: " << (clock() - start) << std::endl;
        EScript::Array *result = EScript::Array::create(out);

        // end = clock();
        // std::cout << "floatToUint8': " << (end - start) << std::endl;

        return result;
    }

    std::vector<int> getTextureData(Rendering::RenderingContext &context, Rendering::Texture &texture)
    {
        texture.downloadGLTexture(context);
        Util::Bitmap *bitmap = texture.getLocalBitmap();

        const uint32_t rowSize = bitmap->getWidth() * bitmap->getPixelFormat().getComponentCount();

        std::vector<int> data;
        for (uint32_t y = 0; y < bitmap->getHeight(); y++)
            for (uint32_t x = 0; x < rowSize; x++) // flip rows
                data.push_back(*(bitmap->data() + (bitmap->getHeight() - 1 - y) * rowSize + x));

        return data;
    }

    std::vector<float> getTextureDataAsFloat(Rendering::RenderingContext &context, Rendering::Texture &texture)
    {
        texture.downloadGLTexture(context);
        Util::Bitmap *bitmap = texture.getLocalBitmap();

        const uint32_t rowSize = bitmap->getWidth() * bitmap->getPixelFormat().getComponentCount();

        std::vector<float> data;
        for (uint32_t y = 0; y < bitmap->getHeight(); y++)
            for (uint32_t x = 0; x < rowSize; x++) // flip rows
                data.push_back((*(bitmap->data() + (bitmap->getHeight() - 1 - y) * rowSize + x)) / 255.0f);

        return data;
    }

    std::vector<std::vector<uint8_t>> cmap = {{68, 1, 84}, {68, 2, 86}, {69, 4, 87}, {69, 5, 89}, {70, 7, 90}, {70, 8, 92}, {70, 10, 93}, {70, 11, 94}, {71, 13, 96}, {71, 14, 97}, {71, 16, 99}, {71, 17, 100}, {71, 19, 101}, {72, 20, 103}, {72, 22, 104}, {72, 23, 105}, {72, 24, 106}, {72, 26, 108}, {72, 27, 109}, {72, 28, 110}, {72, 29, 111}, {72, 31, 112}, {72, 32, 113}, {72, 33, 115}, {72, 35, 116}, {72, 36, 117}, {72, 37, 118}, {72, 38, 119}, {72, 40, 120}, {72, 41, 121}, {71, 42, 122}, {71, 44, 122}, {71, 45, 123}, {71, 46, 124}, {71, 47, 125}, {70, 48, 126}, {70, 50, 126}, {70, 51, 127}, {70, 52, 128}, {69, 53, 129}, {69, 55, 129}, {69, 56, 130}, {68, 57, 131}, {68, 58, 131}, {68, 59, 132}, {67, 61, 132}, {67, 62, 133}, {66, 63, 133}, {66, 64, 134}, {66, 65, 134}, {65, 66, 135}, {65, 68, 135}, {64, 69, 136}, {64, 70, 136}, {63, 71, 136}, {63, 72, 137}, {62, 73, 137}, {62, 74, 137}, {62, 76, 138}, {61, 77, 138}, {61, 78, 138}, {60, 79, 138}, {60, 80, 139}, {59, 81, 139}, {59, 82, 139}, {58, 83, 139}, {58, 84, 140}, {57, 85, 140}, {57, 86, 140}, {56, 88, 140}, {56, 89, 140}, {55, 90, 140}, {55, 91, 141}, {54, 92, 141}, {54, 93, 141}, {53, 94, 141}, {53, 95, 141}, {52, 96, 141}, {52, 97, 141}, {51, 98, 141}, {51, 99, 141}, {50, 100, 142}, {50, 101, 142}, {49, 102, 142}, {49, 103, 142}, {49, 104, 142}, {48, 105, 142}, {48, 106, 142}, {47, 107, 142}, {47, 108, 142}, {46, 109, 142}, {46, 110, 142}, {46, 111, 142}, {45, 112, 142}, {45, 113, 142}, {44, 113, 142}, {44, 114, 142}, {44, 115, 142}, {43, 116, 142}, {43, 117, 142}, {42, 118, 142}, {42, 119, 142}, {42, 120, 142}, {41, 121, 142}, {41, 122, 142}, {41, 123, 142}, {40, 124, 142}, {40, 125, 142}, {39, 126, 142}, {39, 127, 142}, {39, 128, 142}, {38, 129, 142}, {38, 130, 142}, {38, 130, 142}, {37, 131, 142}, {37, 132, 142}, {37, 133, 142}, {36, 134, 142}, {36, 135, 142}, {35, 136, 142}, {35, 137, 142}, {35, 138, 141}, {34, 139, 141}, {34, 140, 141}, {34, 141, 141}, {33, 142, 141}, {33, 143, 141}, {33, 144, 141}, {33, 145, 140}, {32, 146, 140}, {32, 146, 140}, {32, 147, 140}, {31, 148, 140}, {31, 149, 139}, {31, 150, 139}, {31, 151, 139}, {31, 152, 139}, {31, 153, 138}, {31, 154, 138}, {30, 155, 138}, {30, 156, 137}, {30, 157, 137}, {31, 158, 137}, {31, 159, 136}, {31, 160, 136}, {31, 161, 136}, {31, 161, 135}, {31, 162, 135}, {32, 163, 134}, {32, 164, 134}, {33, 165, 133}, {33, 166, 133}, {34, 167, 133}, {34, 168, 132}, {35, 169, 131}, {36, 170, 131}, {37, 171, 130}, {37, 172, 130}, {38, 173, 129}, {39, 173, 129}, {40, 174, 128}, {41, 175, 127}, {42, 176, 127}, {44, 177, 126}, {45, 178, 125}, {46, 179, 124}, {47, 180, 124}, {49, 181, 123}, {50, 182, 122}, {52, 182, 121}, {53, 183, 121}, {55, 184, 120}, {56, 185, 119}, {58, 186, 118}, {59, 187, 117}, {61, 188, 116}, {63, 188, 115}, {64, 189, 114}, {66, 190, 113}, {68, 191, 112}, {70, 192, 111}, {72, 193, 110}, {74, 193, 109}, {76, 194, 108}, {78, 195, 107}, {80, 196, 106}, {82, 197, 105}, {84, 197, 104}, {86, 198, 103}, {88, 199, 101}, {90, 200, 100}, {92, 200, 99}, {94, 201, 98}, {96, 202, 96}, {99, 203, 95}, {101, 203, 94}, {103, 204, 92}, {105, 205, 91}, {108, 205, 90}, {110, 206, 88}, {112, 207, 87}, {115, 208, 86}, {117, 208, 84}, {119, 209, 83}, {122, 209, 81}, {124, 210, 80}, {127, 211, 78}, {129, 211, 77}, {132, 212, 75}, {134, 213, 73}, {137, 213, 72}, {139, 214, 70}, {142, 214, 69}, {144, 215, 67}, {147, 215, 65}, {149, 216, 64}, {152, 216, 62}, {155, 217, 60}, {157, 217, 59}, {160, 218, 57}, {162, 218, 55}, {165, 219, 54}, {168, 219, 52}, {170, 220, 50}, {173, 220, 48}, {176, 221, 47}, {178, 221, 45}, {181, 222, 43}, {184, 222, 41}, {186, 222, 40}, {189, 223, 38}, {192, 223, 37}, {194, 223, 35}, {197, 224, 33}, {200, 224, 32}, {202, 225, 31}, {205, 225, 29}, {208, 225, 28}, {210, 226, 27}, {213, 226, 26}, {216, 226, 25}, {218, 227, 25}, {221, 227, 24}, {223, 227, 24}, {226, 228, 24}, {229, 228, 25}, {231, 228, 25}, {234, 229, 26}, {236, 229, 27}, {239, 229, 28}, {241, 229, 29}, {244, 230, 30}, {246, 230, 32}, {248, 230, 33}, {251, 231, 35}, {253, 231, 37}};

    void setTextureData(Rendering::Texture &texture, std::vector<float> &data, bool floatToUint8, bool colormap)
    {
        Util::Bitmap *bitmap = texture.getLocalBitmap();

        std::vector<uint8_t> dataVector;

        const uint32_t rowSize = bitmap->getWidth() * bitmap->getPixelFormat().getComponentCount();
        if (data.size() != rowSize * bitmap->getHeight())
        {
            std::cerr << "data has wrong size: " << data.size() << ", but should be " << bitmap->getWidth() << " * " << bitmap->getHeight() << " * " << bitmap->getPixelFormat().getComponentCount() << " = " << rowSize * bitmap->getHeight() << std::endl;
            return;
        }

        for (uint32_t y = 0; y < bitmap->getHeight(); y++)
            for (uint32_t x = 0; x < rowSize; x++) // flip rows
            {
                uint32_t i = (bitmap->getHeight() - 1 - y) * rowSize + x;

                uint8_t d = colormap       ? cmap[floatToUint8 ? std::clamp(data[i / 3], 0.0f, 1.0f) * 255
                                                               : (uint8_t)data[i / 3]][i % 3]
                            : floatToUint8 ? std::clamp(data[i], 0.0f, 1.0f) * 255
                                           : data[i];

                dataVector.push_back(d);
            }

        bitmap->setData(dataVector);

        texture.dataChanged();
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

        // data, cache=true
        ES_FUNCTION(lib, "predict", 1, 2, {
            EScript::Array *in = parameter[0].to<EScript::Array *>(rt);
            std::vector<float> inVector;
            for (int i = 0; i < in->size(); i++)
                inVector.push_back(in->at(i).toFloat());

            return EScript::Array::create(predict(inVector, parameter[1].toBool(true)));
        });

        ES_FUNCTION(lib, "uint8ToFloat", 1, 1,
                    {
                        return uint8ToFloat(parameter[0].to<EScript::Array *>(rt));
                    });

        ES_FUNCTION(lib, "floatToUint8", 1, 1,
                    {
                        return floatToUint8(parameter[0].to<EScript::Array *>(rt));
                    });

        // renderingContext, texture, asFloat=false
        ES_FUNCTION(lib, "getTextureData", 2, 3, {
            return parameter[2].toBool(false) ? EScript::Array::create(getTextureDataAsFloat(parameter[0].to<Rendering::RenderingContext &>(rt), parameter[1].to<Rendering::Texture &>(rt)))
                                              : EScript::Array::create(getTextureData(parameter[0].to<Rendering::RenderingContext &>(rt), parameter[1].to<Rendering::Texture &>(rt)));
        });

        // texture, data, floatToUint8, colormap
        ES_FUNCTION(lib, "setTextureData", 2, 4, {
            EScript::Array *in = parameter[1].to<EScript::Array *>(rt);
            std::vector<float> inVector;
            for (int i = 0; i < in->size(); i++)
                inVector.push_back(in->at(i).toFloat());

            setTextureData(parameter[0].to<Rendering::Texture &>(rt), inVector, parameter[2].toBool(false), parameter[3].toBool(false));

            return EScript::value(nullptr);
        });

        // renderingContext, in, out, cache, colormap
        ES_FUNCTION(lib, "directPrerender", 3, 5, {
            // time_t start = clock();
            std::vector<float> in = getTextureDataAsFloat(parameter[0].to<Rendering::RenderingContext &>(rt), parameter[1].to<Rendering::Texture &>(rt));
            // std::cout << "time getTextureDataAsFloat: " << clock() - start << "ms\n";

            // start = clock();
            std::vector<float> out = predict(in, parameter[3].toBool(true));
            // std::cout << "time predict: " << clock() - start << "ms\n";

            // start = clock();
            setTextureData(parameter[2].to<Rendering::Texture &>(rt), out, true, parameter[4].toBool(false));
            // std::cout << "time setTextureData: " << clock() - start << "ms\n";

            return EScript::value(nullptr);
        });

        // module
        ES_FUNCTION(lib, "pythonInit", 1, 1,
                    {
                        if (!PythonRender::init(rt, parameter[0].toString()))
                        {
                            return false;
                            loadedPython = "";
                        }
                        loadedPython = parameter[0].toString();

                        return true;
                    });

        // crashes with numpy
        // ES_FUNCTION(lib, "pythonFinalize", 0, 0,
        //             {
        //                 return PythonRender::finalize();
        //             });

        // out, floatToUint8=false, colormap=false
        ES_FUNCTION(lib, "pythonRenderTexture", 1, 3, {
            // time_t start = clock();
            std::vector<int> in = {};
            std::vector<float> out = PythonRender::render(in);
            // std::cout << "time python: " << clock() - start << "ms\n";

            // start = clock();
            setTextureData(parameter[0].to<Rendering::Texture &>(rt), out, parameter[1].toBool(false), parameter[2].toBool(false));
            // std::cout << "time setTextureData: " << clock() - start << "ms\n";

            return EScript::value(nullptr);
        });
        // renderingContext, in, out, cache=true, floatToUint8=false, colormap=false
        ES_FUNCTION(lib, "pythonPRenderTexture", 3, 6, {
            // time_t start = clock();
            std::vector<int> in = getTextureData(parameter[0].to<Rendering::RenderingContext &>(rt), parameter[1].to<Rendering::Texture &>(rt));
            // std::cout << "time getTextureDataAsFloat: " << clock() - start << "ms\n";
            // start = clock();

            std::vector<float> out = prerenderedPython(in, parameter[3].toBool(true));

            // std::cout << "time predict: " << clock() - start << "ms\n";

            // start = clock();
            setTextureData(parameter[2].to<Rendering::Texture &>(rt), out, parameter[4].toBool(false), parameter[5].toBool(false));
            // std::cout << "time setTextureData: " << clock() - start << "ms\n";

            return EScript::value(nullptr);
        });
    }

}
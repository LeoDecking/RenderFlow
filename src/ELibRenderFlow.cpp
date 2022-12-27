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

#include <iostream>

namespace RenderFlow
{


    cppflow::model *model;
    std::string inputOperation;
    std::string outputOperation;
    std::vector<int64_t> shape;

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
        
        if(output.dtype() == TF_HALF)
            output = cppflow::cast(output, TF_HALF, TF_FLOAT);
        return EScript::Array::create(output.get_data<float>());
    }

    // Util::Reference<Rendering::Texture> renderFromTexture(Rendering::RenderingContext &rc,
    //                                                       Rendering::Texture &input_texture)
    // {

    //     // auto input = input_texture.getTexImage(rc, input_width, input_height, 3);

    //     Util::Bitmap *bitmap = new Util::Bitmap(output_width, output_height, Util::PixelFormat::RGB);

    //     std::vector<uint8_t> data;
    //     for (int i = 0; i < output_width * output_height * 3; i++)
    //     {
    //         data.push_back(i % 256);
    //     }

    //     bitmap->setData(data);

    //     return Rendering::TextureUtils::createTextureFromBitmap(*bitmap);
    // }

    // // todo movement radius
    // Util::Reference<Rendering::Texture> renderFromCamera(Rendering::RenderingContext &rc, MinSG::CameraNode &cam)
    // {
    //     Geometry::Vec3 x = cam.getWorldMatrix().transformDirection(Geometry::Vec3(1, 0, 0));
    //     Geometry::Angle angleH = Geometry::Angle::rad((float)acos(x.getX()));
    //     if (x.getX() == 1)
    //         angleH = Geometry::Angle::deg(0);
    //     if (x.getZ() < 0)
    //         angleH = Geometry::Angle::deg(360) - angleH;

    //     Geometry::Vec3 y = cam.getWorldMatrix().transformDirection(Geometry::Vec3(0, 1, 0));
    //     Geometry::Angle angleV = Geometry::Angle::rad((float)acos(y.getY()));
    //     if (y.getY() == 1)
    //         angleV = Geometry::Angle::deg(0);
    //     if (y.getX() < 0)
    //         angleV = Geometry::Angle::deg(360) - angleV;

    //     // std::cout << x.getX() << " " << x.getY() << " " << x.getZ() << std::endl;
    //     // std::cout << y.getX() << " " << y.getY() << " " << y.getZ() << std::endl;
    //     std::cout << cam.getWorldPosition().getX() << ", " << cam.getWorldPosition().getY() << ", " << cam.getWorldPosition().getZ() << "; " << angleH.deg() << "°, " << angleV.deg() << "°" << std::endl;

    //     cppflow::tensor input = {cam.getWorldPosition().getX(), cam.getWorldPosition().getZ()};
    //     input = input / 10.0f;
    //     input = cppflow::expand_dims(input, 0);
    //     // for (std::string s : model->get_operations())
    //     //     std::cout << s << std::endl;
    //     cppflow::tensor output = (*model)({{"serving_default_dense_input:0", input}}, {{"StatefulPartitionedCall:0"}})[0];
    //     output = cppflow::squeeze(output, {0});
    //     output = output * 255.0f;
    //     output = cppflow::clip_by_value(output, 0.0f, 255.0f);
    //     output = cppflow::cast(output, TF_FLOAT, TF_UINT8, true);

    //     // TODO format
    //     std::vector<uint8_t> data;
    //     for (uint8_t c : output.get_data<uint8_t>())
    //     {
    //         data.push_back(colormap[c][0]);
    //         data.push_back(colormap[c][1]);
    //         data.push_back(colormap[c][2]);
    //     }

    //     Util::Bitmap *bitmap = new Util::Bitmap(output_width, output_height, Util::PixelFormat::RGB);

    //     bitmap->setData(data);

    //     return Rendering::TextureUtils::createTextureFromBitmap(*bitmap);
    // }

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
                        std::cout << "load: '"<<parameter[0].toString()<<"'"<<std::endl;
                        
                        if(parameter[1].toBool())
                            model = new cppflow::model(parameter[0].toString(), cppflow::model::FROZEN_GRAPH);
                        else
                            model = new cppflow::model(parameter[0].toString());

                        EScript::Array *spapeArray =  parameter[2].to<EScript::Array *>(rt);
                        shape.clear();
                        for (int i = 0; i < spapeArray->size(); i++)
                        {
                            shape.push_back(spapeArray->at(i).to<int64_t>(rt));
                        }

                        inputOperation =  parameter[3].toString(); // TODO check if exists
                        outputOperation =  parameter[4].toString(); // TODO check if exists
                         // for (std::string s : model->get_operations())
            
                        return thisEObj;
                    });

        // ES_FUNCTION(lib, "renderFromTexture", 2, 2, {
        //     return EScript::create(
        //         renderFromTexture(parameter[0].to<Rendering::RenderingContext &>(rt),
        //                           *parameter[1].to<Rendering::Texture *>(rt))
        //             .get());
        // });

        // ES_FUNCTION(lib, "renderFromCamera", 2, 2, {
        //     return EScript::create(
        //         renderFromCamera(parameter[0].to<Rendering::RenderingContext &>(rt),
        //                          *parameter[1].to<MinSG::CameraNode *>(rt))
        //             .get());
        // });

        ES_FUNCTION(lib, "predict", 1, 1, {
            return predict(parameter[0].to<EScript::Array *>(rt));
        });
    }

}
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
// #include <E_Geometry/E_>
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

    int input_width = 324, input_height = 324;
    // int output_width = 648, output_height = 648;
    int output_width = 64, output_height = 64;

    // https://www.kennethmoreland.com/color-advice/
    // viridis
    // TODO license
    const std::vector<std::vector<uint8_t>> colormap = {{68, 1, 84}, {68, 2, 86}, {69, 4, 87}, {69, 5, 89}, {70, 7, 90}, {70, 8, 92}, {70, 10, 93}, {70, 11, 94}, {71, 13, 96}, {71, 14, 97}, {71, 16, 99}, {71, 17, 100}, {71, 19, 101}, {72, 20, 103}, {72, 22, 104}, {72, 23, 105}, {72, 24, 106}, {72, 26, 108}, {72, 27, 109}, {72, 28, 110}, {72, 29, 111}, {72, 31, 112}, {72, 32, 113}, {72, 33, 115}, {72, 35, 116}, {72, 36, 117}, {72, 37, 118}, {72, 38, 119}, {72, 40, 120}, {72, 41, 121}, {71, 42, 122}, {71, 44, 122}, {71, 45, 123}, {71, 46, 124}, {71, 47, 125}, {70, 48, 126}, {70, 50, 126}, {70, 51, 127}, {70, 52, 128}, {69, 53, 129}, {69, 55, 129}, {69, 56, 130}, {68, 57, 131}, {68, 58, 131}, {68, 59, 132}, {67, 61, 132}, {67, 62, 133}, {66, 63, 133}, {66, 64, 134}, {66, 65, 134}, {65, 66, 135}, {65, 68, 135}, {64, 69, 136}, {64, 70, 136}, {63, 71, 136}, {63, 72, 137}, {62, 73, 137}, {62, 74, 137}, {62, 76, 138}, {61, 77, 138}, {61, 78, 138}, {60, 79, 138}, {60, 80, 139}, {59, 81, 139}, {59, 82, 139}, {58, 83, 139}, {58, 84, 140}, {57, 85, 140}, {57, 86, 140}, {56, 88, 140}, {56, 89, 140}, {55, 90, 140}, {55, 91, 141}, {54, 92, 141}, {54, 93, 141}, {53, 94, 141}, {53, 95, 141}, {52, 96, 141}, {52, 97, 141}, {51, 98, 141}, {51, 99, 141}, {50, 100, 142}, {50, 101, 142}, {49, 102, 142}, {49, 103, 142}, {49, 104, 142}, {48, 105, 142}, {48, 106, 142}, {47, 107, 142}, {47, 108, 142}, {46, 109, 142}, {46, 110, 142}, {46, 111, 142}, {45, 112, 142}, {45, 113, 142}, {44, 113, 142}, {44, 114, 142}, {44, 115, 142}, {43, 116, 142}, {43, 117, 142}, {42, 118, 142}, {42, 119, 142}, {42, 120, 142}, {41, 121, 142}, {41, 122, 142}, {41, 123, 142}, {40, 124, 142}, {40, 125, 142}, {39, 126, 142}, {39, 127, 142}, {39, 128, 142}, {38, 129, 142}, {38, 130, 142}, {38, 130, 142}, {37, 131, 142}, {37, 132, 142}, {37, 133, 142}, {36, 134, 142}, {36, 135, 142}, {35, 136, 142}, {35, 137, 142}, {35, 138, 141}, {34, 139, 141}, {34, 140, 141}, {34, 141, 141}, {33, 142, 141}, {33, 143, 141}, {33, 144, 141}, {33, 145, 140}, {32, 146, 140}, {32, 146, 140}, {32, 147, 140}, {31, 148, 140}, {31, 149, 139}, {31, 150, 139}, {31, 151, 139}, {31, 152, 139}, {31, 153, 138}, {31, 154, 138}, {30, 155, 138}, {30, 156, 137}, {30, 157, 137}, {31, 158, 137}, {31, 159, 136}, {31, 160, 136}, {31, 161, 136}, {31, 161, 135}, {31, 162, 135}, {32, 163, 134}, {32, 164, 134}, {33, 165, 133}, {33, 166, 133}, {34, 167, 133}, {34, 168, 132}, {35, 169, 131}, {36, 170, 131}, {37, 171, 130}, {37, 172, 130}, {38, 173, 129}, {39, 173, 129}, {40, 174, 128}, {41, 175, 127}, {42, 176, 127}, {44, 177, 126}, {45, 178, 125}, {46, 179, 124}, {47, 180, 124}, {49, 181, 123}, {50, 182, 122}, {52, 182, 121}, {53, 183, 121}, {55, 184, 120}, {56, 185, 119}, {58, 186, 118}, {59, 187, 117}, {61, 188, 116}, {63, 188, 115}, {64, 189, 114}, {66, 190, 113}, {68, 191, 112}, {70, 192, 111}, {72, 193, 110}, {74, 193, 109}, {76, 194, 108}, {78, 195, 107}, {80, 196, 106}, {82, 197, 105}, {84, 197, 104}, {86, 198, 103}, {88, 199, 101}, {90, 200, 100}, {92, 200, 99}, {94, 201, 98}, {96, 202, 96}, {99, 203, 95}, {101, 203, 94}, {103, 204, 92}, {105, 205, 91}, {108, 205, 90}, {110, 206, 88}, {112, 207, 87}, {115, 208, 86}, {117, 208, 84}, {119, 209, 83}, {122, 209, 81}, {124, 210, 80}, {127, 211, 78}, {129, 211, 77}, {132, 212, 75}, {134, 213, 73}, {137, 213, 72}, {139, 214, 70}, {142, 214, 69}, {144, 215, 67}, {147, 215, 65}, {149, 216, 64}, {152, 216, 62}, {155, 217, 60}, {157, 217, 59}, {160, 218, 57}, {162, 218, 55}, {165, 219, 54}, {168, 219, 52}, {170, 220, 50}, {173, 220, 48}, {176, 221, 47}, {178, 221, 45}, {181, 222, 43}, {184, 222, 41}, {186, 222, 40}, {189, 223, 38}, {192, 223, 37}, {194, 223, 35}, {197, 224, 33}, {200, 224, 32}, {202, 225, 31}, {205, 225, 29}, {208, 225, 28}, {210, 226, 27}, {213, 226, 26}, {216, 226, 25}, {218, 227, 25}, {221, 227, 24}, {223, 227, 24}, {226, 228, 24}, {229, 228, 25}, {231, 228, 25}, {234, 229, 26}, {236, 229, 27}, {239, 229, 28}, {241, 229, 29}, {244, 230, 30}, {246, 230, 32}, {248, 230, 33}, {251, 231, 35}, {253, 231, 37}};

    cppflow::model *model;

    Util::Reference<Rendering::Texture> renderFromTexture(Rendering::RenderingContext &rc,
                                                          Rendering::Texture &input_texture)
    {

        // auto input = input_texture.getTexImage(rc, input_width, input_height, 3);

        Util::Bitmap *bitmap = new Util::Bitmap(output_width, output_height, Util::PixelFormat::RGB);

        std::vector<uint8_t> data;
        for (int i = 0; i < output_width * output_height * 3; i++)
        {
            data.push_back(i % 256);
        }

        bitmap->setData(data);

        return Rendering::TextureUtils::createTextureFromBitmap(*bitmap);
    }

    // todo movement radius
    Util::Reference<Rendering::Texture> renderFromCamera(Rendering::RenderingContext &rc, MinSG::CameraNode &cam)
    {
        Geometry::Vec3 x = cam.getWorldMatrix().transformDirection(Geometry::Vec3(1, 0, 0));
        Geometry::Angle angleH = Geometry::Angle::rad((float)acos(x.getX()));
        if (x.getX() == 1)
            angleH = Geometry::Angle::deg(0);
        if (x.getZ() < 0)
            angleH = Geometry::Angle::deg(360) - angleH;

        Geometry::Vec3 y = cam.getWorldMatrix().transformDirection(Geometry::Vec3(0, 1, 0));
        Geometry::Angle angleV = Geometry::Angle::rad((float)acos(y.getY()));
        if (y.getY() == 1)
            angleV = Geometry::Angle::deg(0);
        if (y.getX() < 0)
            angleV = Geometry::Angle::deg(360) - angleV;

        // std::cout << x.getX() << " " << x.getY() << " " << x.getZ() << std::endl;
        // std::cout << y.getX() << " " << y.getY() << " " << y.getZ() << std::endl;
        std::cout << cam.getWorldPosition().getX() << ", " << cam.getWorldPosition().getY() << ", " << cam.getWorldPosition().getZ() << "; " << angleH.deg() << "°, " << angleV.deg() << "°" << std::endl;

        cppflow::tensor input = {cam.getWorldPosition().getX(), cam.getWorldPosition().getZ()};
        input = input / 10.0f;
        input = cppflow::expand_dims(input, 0);
        // for (std::string s : model->get_operations())
        //     std::cout << s << std::endl;
        cppflow::tensor output = (*model)({{"serving_default_dense_input:0", input}}, {{"StatefulPartitionedCall:0"}})[0];
        output = cppflow::squeeze(output, {0});
        output = output * 255.0f;
        output = cppflow::cast(output, TF_FLOAT, TF_UINT8);

        // TODO format
        std::vector<uint8_t> data;
        for (uint8_t c : output.get_data<uint8_t>())
        {
            data.push_back(colormap[c][0]);
            data.push_back(colormap[c][1]);
            data.push_back(colormap[c][2]);
        }
        Util::Bitmap *bitmap = new Util::Bitmap(output_width, output_height, Util::PixelFormat::RGB);

        bitmap->setData(data);

        return Rendering::TextureUtils::createTextureFromBitmap(*bitmap);
    }

    EScript::Array *predict(EScript::Array *in)
    {
        std::vector<float> inVector;
        for (int i = 0; i < in->size(); i++)
        {
            inVector.push_back(in->at(i).toFloat());
        }

        cppflow::tensor input = cppflow::tensor(inVector, {(int64_t)inVector.size()});
        input = cppflow::expand_dims(input, 0);
        // for (std::string s : model->get_operations())
        //     std::cout << s << std::endl;
        cppflow::tensor output = (*model)({{"serving_default_dense_input:0", input}}, {{"StatefulPartitionedCall:0"}})[0];
        output = cppflow::squeeze(output, {0});

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

        ES_FUNCTION(lib, "loadModel", 0, 0,
                    {
                        model = new cppflow::model(("../extPlugins/RenderFlow/model"));
                        // model = m;
                        return thisEObj;
                    });

        ES_FUNCTION(lib, "renderFromTexture", 2, 2, {
            return EScript::create(
                renderFromTexture(parameter[0].to<Rendering::RenderingContext &>(rt),
                                  *parameter[1].to<Rendering::Texture *>(rt))
                    .get());
        });

        ES_FUNCTION(lib, "renderFromCamera", 2, 2, {
            return EScript::create(
                renderFromCamera(parameter[0].to<Rendering::RenderingContext &>(rt),
                                 *parameter[1].to<MinSG::CameraNode *>(rt))
                    .get());
        });

        ES_FUNCTION(lib, "predict", 1, 1, {
            std::cout << "0" << std::endl;
            return predict(parameter[0].to<EScript::Array *>(rt));
        });
    }

}
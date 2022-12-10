#include "ELibRenderFlow.h"

#include <cppflow/cppflow.h>

#include <EScript/Basics.h>
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
    int output_width = 648, output_height = 648;

    cppflow::model model("../extPlugins/RenderFlow/model");

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

    Util::Reference<Rendering::Texture> renderFromCamera(Rendering::RenderingContext &rc, MinSG::CameraNode &cam)
    {
        Util::Reference<Rendering::Texture> output;

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

        std::cout << x.getX() << " " << x.getY() << " " << x.getZ() << std::endl;
        std::cout << y.getX() << " " << y.getY() << " " << y.getZ() << std::endl;
        std::cout << cam.getWorldPosition().getX() << ", " << cam.getWorldPosition().getY() << ", " << cam.getWorldPosition().getZ() << "; " << angleH.deg() << "°, " << angleV.deg() << "°" << std::endl;

        Util::Bitmap *bitmap = new Util::Bitmap(output_width, output_height, Util::PixelFormat::RGB);

        std::vector<uint8_t> data;
        for (int i = 0; i < output_width * output_height * 3; i++)
        {
            if (i % 3 == 0)
                data.push_back(((int)(cam.getWorldPosition().getX() * 256 / 10)) % 256);
            if (i % 3 == 1)
                data.push_back(((int)(cam.getWorldPosition().getZ() * 256 / 10)) % 256);
            if (i % 3 == 2)
                data.push_back(((int)(angleH.deg() * 256 / 360)) % 256);
        }

        bitmap->setData(data);

        return Rendering::TextureUtils::createTextureFromBitmap(*bitmap);
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
    }

}
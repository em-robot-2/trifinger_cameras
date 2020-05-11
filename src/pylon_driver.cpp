/**
 * @file
 * @brief Driver to interface with the camera using Pylon.
 * @copyright 2020 Max Planck Gesellschaft. All rights reserved.
 * @license BSD 3-clause
 */
#include <trifinger_cameras/pylon_driver.hpp>

#include <chrono>
#include <iostream>

#include <opencv2/opencv.hpp>


namespace trifinger_cameras
{
PylonDriver::PylonDriver(const std::string& device_user_id_to_open)
{
    Pylon::CTlFactory& tl_factory = Pylon::CTlFactory::GetInstance();
    Pylon::PylonInitialize();
    Pylon::DeviceInfoList_t device_list;

    if (tl_factory.EnumerateDevices(device_list) == 0)
    {
        Pylon::PylonTerminate();
        throw std::runtime_error("No devices present, please connect one.");
    }

    else
    {
        Pylon::DeviceInfoList_t::const_iterator device_iterator;
        if (device_user_id_to_open.empty())
        {
            device_iterator = device_list.begin();
            camera_.Attach(tl_factory.CreateDevice(*device_iterator));
            std::cout << "Desired device not found. Creating a camera object "
                         "with the first device id in the device list."
                      << std::endl;
        }
        else
        {
            bool found_desired_device = false;

            for (device_iterator = device_list.begin();
                 device_iterator != device_list.end();
                 ++device_iterator)
            {
                std::string device_user_id_found(
                    device_iterator->GetUserDefinedName());
                if (device_user_id_to_open == device_user_id_found)
                {
                    found_desired_device = true;
                    break;
                }
            }

            if (found_desired_device)
            {
                camera_.Attach(tl_factory.CreateDevice(*device_iterator));
            }
            else
            {
                Pylon::PylonTerminate();
                throw std::runtime_error(
                    "Device id specified doesn't correspond to any "
                    "connected devices, please retry with a valid id.");
            }

            camera_.Open();
            camera_.MaxNumBuffer = 5;
            format_converter_.OutputPixelFormat = Pylon::PixelType_BGR8packed;

            set_camera_configuration(camera_.GetNodeMap());

            camera_.StartGrabbing(Pylon::GrabStrategy_LatestImageOnly);
        }
    }
}

PylonDriver::~PylonDriver()
{
    camera_.StopGrabbing();
    Pylon::PylonTerminate();
}

CameraObservation PylonDriver::get_observation()
{
    CameraObservation image_frame;
    Pylon::CGrabResultPtr ptr_grab_result;

    // FIXME 5second timeout?
    camera_.RetrieveResult(
        5000, ptr_grab_result, Pylon::TimeoutHandling_ThrowException);
    auto current_time = std::chrono::system_clock::now();
    image_frame.time_stamp =
        std::chrono::duration<double>(current_time.time_since_epoch()).count();

    if (ptr_grab_result->GrabSucceeded())
    {
        format_converter_.Convert(pylon_image_, ptr_grab_result);
        // FIXME: clarify if this creates a copy or references the memory of
        // pylon_image_.  In latter case, we have to be careful!
        image_frame.image = cv::Mat(ptr_grab_result->GetHeight(),
                                    ptr_grab_result->GetWidth(),
                                    CV_8UC3,
                                    (uint8_t*)pylon_image_.GetBuffer());
    }
    else
    {
        throw std::runtime_error("Failed to access images from the camera.");
    }
    return image_frame;
}

void PylonDriver::set_camera_configuration(GenApi::INodeMap& nodemap)
{
    Pylon::CFloatParameter exposure_time(nodemap, "ExposureTime");
    exposure_time.SetValue(1500);

    // Pylon::CBooleanParameter enable_frame_rate(nodemap,
    // "EnableAcquisitionFrameRate");
    // enable_frame_rate.SetValue(true);

    Pylon::CFloatParameter frame_rate(nodemap, "AcquisitionFrameRate");
    frame_rate.SetValue(100);

    Pylon::CEnumParameter(nodemap, "BalanceWhiteAuto").SetValue("Once");

    //Pylon::CFeaturePersistence::Save("/tmp/camera_settings.xml",
    //                                 &camera_.GetNodeMap());
    // Pylon::CFeaturePersistence::Load( Filename, &camera.GetNodeMap(),
    // true );
}
}

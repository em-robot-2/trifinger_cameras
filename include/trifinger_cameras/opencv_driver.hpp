/**
 * @file
 * @brief Driver to interface with the camera using OpenCV.
 * @copyright 2020, New York University, Max Planck Gesellschaft. All rights
 *            reserved.
 * @license BSD 3-clause
 */

#pragma once

#include <chrono>
#include <iostream>

#include <opencv2/opencv.hpp>
#include <robot_interfaces/sensors/sensor_driver.hpp>
#include <trifinger_cameras/camera_observation.hpp>

namespace trifinger_cameras
{
/**
 * @brief Driver for interacting with any camera using OpenCV.
 */
class OpenCVDriver : public robot_interfaces::SensorDriver<CameraObservation>
{
public:
    cv::VideoCapture video_capture_;

    OpenCVDriver(int device_id)
    {
        cv::VideoCapture cap(device_id);
        video_capture_ = cap;
    }

    /**
     * @brief Grab a single frame along with its timestamp.
     *
     * @return Image frame consisting of an image matrix and the time at
     * which it was grabbed.
     */
    CameraObservation get_observation()
    {
        if (!video_capture_.isOpened())
        {
            throw std::runtime_error("Could not access camera stream :(");
        }
        else
        {
#ifdef VERBOSE
            std::cout << "Succeeded in accessing camera stream!" << std::endl;
#endif
            CameraObservation image_frame;
            cv::Mat frame;
            auto current_time = std::chrono::system_clock::now();

            video_capture_ >> frame;
            image_frame.image = frame;
            image_frame.time_stamp =
                std::chrono::duration<double>(current_time.time_since_epoch())
                    .count();
            return image_frame;
        }
    }
};

}  // namespace trifinger_cameras

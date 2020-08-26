/**
 * @file
 * @brief Defines the observation structure to be used by any camera.
 * @copyright 2020, Max Planck Gesellschaft. All rights reserved.
 * @license BSD 3-clause
 */

#pragma once

#include <opencv2/opencv.hpp>
#include <serialization_utils/cereal_cvmat.hpp>

namespace trifinger_cameras
{
/**
 * @brief Observation structure to store cv::Mat images with corresponding
 * timestamps.
 *
 */
//template<size_t _width, size_t _height>
struct CameraObservation
{
    static constexpr size_t width = 270;
    static constexpr size_t height = 270;

    cv::Mat image;
    double timestamp;

    CameraObservation():
        image(height, width, CV_8UC1),
        timestamp(0)
    {}

    template <class Archive>
    void serialize(Archive& archive)
    {
        archive(image, timestamp);
    }
};

}  // namespace trifinger_cameras

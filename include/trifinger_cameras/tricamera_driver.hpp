/**
 * @file
 * @brief Wrapper on the Pylon Driver to synchronise three pylon dependent
 * cameras.
 * @copyright 2020, Max Planck Gesellschaft. All rights reserved.
 * @license BSD 3-clause
 */
#pragma once

#include <chrono>

#include <robot_interfaces/sensors/sensor_driver.hpp>
#include <trifinger_cameras/pylon_driver.hpp>
#include <trifinger_cameras/tricamera_observation.hpp>

namespace trifinger_cameras
{
/**
 * @brief Driver to create three instances of the PylonDriver
 * and get observations from them.
 */
class TriCameraDriver
    : public robot_interfaces::SensorDriver<TriCameraObservation>
{
public:
    //! @brief Rate at which images are acquired.
    static constexpr std::chrono::milliseconds rate =
        std::chrono::milliseconds(100);

    /**
     * @param device_id_1 device user id of first camera
     * @param device_id_2 likewise, the 2nd's
     * @param device_id_3 and the 3rd's
     * @param downsample_images If set to true (default), images are
     *     downsampled to half their original size.
     */
    TriCameraDriver(const std::string& device_id_1,
                    const std::string& device_id_2,
                    const std::string& device_id_3,
                    bool downsample_images = true);

    /**
     * @brief Get the latest observation from the three cameras
     * @return TricameraObservation
     */
    TriCameraObservation get_observation();

private:
    std::chrono::time_point<std::chrono::system_clock> last_update_time_;
    PylonDriver camera1_, camera2_, camera3_;
};

}  // namespace trifinger_cameras

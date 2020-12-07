/**
 * @file
 * @brief Wrapper on the Pylon Driver to synchronise three pylon dependent
 * cameras.
 * @copyright 2020, Max Planck Gesellschaft. All rights reserved.
 */
#include <trifinger_cameras/pybullet_tricamera_driver.hpp>

#include <chrono>
#include <thread>

#include <pybind11_opencv/cvbind.hpp>
#include <serialization_utils/cereal_cvmat.hpp>

namespace py = pybind11;

namespace trifinger_cameras
{
PyBulletTriCameraDriver::PyBulletTriCameraDriver()
{
    // initialize Python interpreter if not already done
    if (!Py_IsInitialized())
    {
        py::initialize_interpreter();
    }

    py::gil_scoped_acquire acquire;

    // some imports that are needed later for converting images (numpy array ->
    // cv::Mat)
    numpy_ = py::module::import("numpy");

    // TriFingerCameras gives access to the cameras in simulation
    py::module mod_camera = py::module::import("trifinger_simulation.camera");
    cameras_ = mod_camera.attr("TriFingerCameras")();
}

trifinger_cameras::TriCameraObservation
PyBulletTriCameraDriver::get_observation()
{
    auto start_time = std::chrono::system_clock::now();

    trifinger_cameras::TriCameraObservation tricam_obs;

    {
        py::gil_scoped_acquire acquire;

        py::list images = cameras_.attr("get_bayer_images")();
        for (int i = 0; i < 3; i++)
        {
            // ensure that the image array is contiguous in memory, otherwise
            // conversion to cv::Mat would fail
            auto image = numpy_.attr("ascontiguousarray")(images[i]);
            // convert to cv::Mat
            tricam_obs.cameras[i].image = image.cast<cv::Mat>();
        }
    }

    // run at around 10 Hz
    using namespace std::chrono_literals;
    std::this_thread::sleep_until(start_time + 100ms);

    return tricam_obs;
}

}  // namespace trifinger_cameras


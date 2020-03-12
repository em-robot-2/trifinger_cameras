#!/usr/bin/env python
"""ROS node that detects a charuco board in images and publishes the pose."""
from __future__ import print_function, division

import json
import os
import pickle
import subprocess

import numpy as np
import cv2

import rospy
from geometry_msgs.msg import PoseStamped
from sensor_msgs.msg import Image
from cv_bridge import CvBridge, CvBridgeError
from tf import transformations


class CharucoBoardPosePublisher:

    def __init__(self):
        self.cv_bridge = CvBridge()
        self.image_sub = rospy.Subscriber("/camera60/image_raw", Image, self.callback)
        self.pose_pub = rospy.Publisher("~pose", PoseStamped, queue_size=100)

    def callback(self, msg):
        cv_image = self.cv_bridge.imgmsg_to_cv2(msg, "bgr8")

        # Due to Python version issues, do not call the board detection
        # directly here but store the image to a file in shared memory and call
        # the charuco_board.py script on it...

        # pickle the image to shared memory (cut away the leading "/" from the
        # node name)
        image_path = os.path.join("/dev/shm", rospy.get_name()[1:] + ".pickle")
        with open(image_path, "wb") as file_handle:
            pickle.dump(cv_image, file_handle, pickle.HIGHEST_PROTOCOL)

        # call the script to detect the board
        pose_json = subprocess.check_output(["./charuco_board.py",
                                             "detect_image",
                                             "--no-gui",
                                             "--filename",
                                             image_path])
        pose_dict = json.loads(pose_json)
        rvec = np.asarray(pose_dict["rvec"])
        tvec = np.asarray(pose_dict["tvec"])

        if rvec is not None:
            # convert the Rodrigues vector to a quaternion
            rotation_matrix = np.array([[0, 0, 0, 0],
                                        [0, 0, 0, 0],
                                        [0, 0, 0, 0],
                                        [0, 0, 0, 1]])
            rotation_matrix[:3, :3], _ = cv2.Rodrigues(rvec)
            quaternion = transformations.quaternion_from_matrix(rotation_matrix)

            pose = PoseStamped()
            pose.header = msg.header
            pose.pose.position.x = tvec[0]
            pose.pose.position.y = tvec[1]
            pose.pose.position.z = tvec[2]
            pose.pose.orientation.x = quaternion[0]
            pose.pose.orientation.y = quaternion[1]
            pose.pose.orientation.z = quaternion[2]
            pose.pose.orientation.w = quaternion[3]

            self.pose_pub.publish(pose)


def main():
    rospy.init_node("charuco_pose_publisher")

    node = CharucoBoardPosePublisher()  # noqa
    print("init done")
    rospy.spin()


if __name__ == "__main__":
    main()

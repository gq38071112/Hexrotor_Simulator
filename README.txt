1.The simulator is developed with C++ in Gazebo under ROS. The used ROS version is the long time supported version ROS Kinetic installed in 16.4 Ubuntu.

2.For installing ROS Kinetic please refer to:http://wiki.ros.org/kinetic/Installation

3.For installing Ubuntu 16.4 please refer to:https://www.ubuntu.com/desktop

4.Gazebo 7 should be automated in ROS Kinetic installed when choosing ros-kinetic-desktop-full.
if not, Gazebo could be installed refering to:http://gazebosim.org/. Don't download the newst version. Many functions in simulator is only valid in Gazebo7.

5.To run the simulator following packages must be first installed:
joy
genmsg
tf
roscpp
rospy
std_msg
sensor_msgs
dynamic_reconfigure
controller_manager
effort_controller
position_controller
controller_interface
hardware_interface
joint_state_controller
robot_state_publisher
Installing these packages in Ubntu 16.4 with:$sudo apt-get install ros-kinetic-<package name>

6.To insert new CAD model in simulator a URDF file of the robot and the meshes in STL format are needed. Put the URDF file in the urdf folder and the STL files in the meshes 
folder in the drone package, add gazebo and plugin tags in urdf file and then open the terminal in ROS workspace and run:
$catkin_make                             (package complied£©
$.~/catkin_ws/devel/setup.bash           (attached to the environment)
$roslaunch drone gazebo.launch         £¨start all the useful nodes at the same time)
Sometimes Gazebo will crash. Just repeat the third command and try again, then it would work.

7.By joystick configuration, first connect a Linux supported joystick to computer,then open the terminal and type in:
$ls /dev/input/
Find your device in the window and type in:
$ sudo jstest /dev/input/jsX   (jsX should be replaced with your own device number)
Let the joystick accessible for ROS joy node by:
$ ls -l /dev/input/jsX
$ sudo chmod a+rw /dev/input/jsX
you can see something simular to:crw-rw-XX- 1 root dialout 188, 0 2009-08-14 12:04 /dev/input/jsX, if XX is rw,then the device has been configured.


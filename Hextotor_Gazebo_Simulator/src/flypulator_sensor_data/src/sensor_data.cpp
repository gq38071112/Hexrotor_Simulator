#include <gazebo/transport/transport.hh>
#include <gazebo/msgs/msgs.hh>
#include <gazebo/gazebo.hh>
#include <gazebo/physics/physics.hh>
#include <time.h>
#include <iostream>
#include <math.h>
#include <thread>
#include "ros/ros.h"
#include "ros/callback_queue.h"
#include "ros/subscribe_options.h"
#include "std_msgs/Float32.h"
#include "gazebo/sensors/Sensor.hh"
#include <gazebo/gazebo_config.h>
#include <gazebo/gazebo_client.hh>
#include "ros/ros.h"
#include "sensor_msgs/Imu.h"
#include "geometry_msgs/Vector3Stamped.h"
#include "sensor_msgs/NavSatFix.h"
#include "sensor_msgs/Image.h"
#include "gazebo_msgs/LinkState.h"
#include "std_msgs/Int32.h"
void ImuCallback (const sensor_msgs::ImuConstPtr& msg)
{
  ROS_INFO("I heard Imu");
}
void MagCallback (const geometry_msgs::Vector3StampedConstPtr& msg)
{
  ROS_INFO("I heard Magnet");
}
void GpsCallback (const sensor_msgs::NavSatFixConstPtr& msg)
{
  ROS_INFO("I heard GPS");
}
void GpsVelCallback (const geometry_msgs::Vector3StampedConstPtr& msg)
{
  ROS_INFO("I heard GPSVel");
}
void CameraCallback (const sensor_msgs::ImageConstPtr& msg)
{
    ROS_INFO("I see Camera");
}


int main(int argc, char **argv)
{

  ros::init(argc, argv, "sensor_data");


  ros::NodeHandle n;


  ros::Subscriber subImu = n.subscribe("/IMU/imu_data", 1000, ImuCallback);
  ros::Subscriber subMag = n.subscribe("/MAGNET/magnetic_data", 1000, MagCallback);
  ros::Subscriber subGps = n.subscribe("/GPS/fix", 1000, GpsCallback);
  ros::Subscriber subGpsVel = n.subscribe("/GPS/fix_velocity", 1000, GpsVelCallback);
  ros::Subscriber subCamera = n.subscribe("/camera1/image_raw", 1000, CameraCallback);
  ros::Rate loop_rate(1000000);
  ros::spin();
 return 0;
}

#include "ros/ros.h"
#include "geometry_msgs/Vector3.h"
#include <dynamic_reconfigure/server.h>
#include <wind/wind_speedConfig.h>
double wind_velx=0 ,wind_vely=0, wind_velz=0;
void cb(drone::wind_speedConfig &config, uint32_t level)
{
	wind_velx=config.velx;
        wind_vely=config.vely;
        wind_velz=config.velz;
}
int main(int argc, char **argv)
{

  ros::init(argc, argv, "wind_node");

  ros::NodeHandle n;

  dynamic_reconfigure::Server<drone::wind_speedConfig> server;
  dynamic_reconfigure::Server<drone::wind_speedConfig>::CallbackType f;

 f = boost::bind(&cb, _1, _2);
 server.setCallback(f);
  ros::Publisher windvel_pub = n.advertise<geometry_msgs::Vector3>("/drone/wind_cmd", 1000);

  ros::Rate loop_rate(10);

  /**
   * A count of how many messages we have sent. This is used to create
   * a unique string for each message.
   */

  while (ros::ok())
  {
    /**
     * This is a message object. You stuff it with data, and then publish it.
     */
    geometry_msgs::Vector3 msg;

    msg.x=wind_velx;
    msg.y=wind_vely;
    msg.z=wind_velz;

   /*ROS_INFO("%lf", msg.x);
   ROS_INFO("%lf", msg.y);
   ROS_INFO("%lf", msg.z);*/

    windvel_pub.publish(msg);

    ros::spinOnce();

    loop_rate.sleep();

  }


  return 0;
}

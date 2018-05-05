#ifndef _AERO_PLUGIN_HH_
#define _AERO_PLUGIN_HH_
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
#include "std_msgs/Int32.h"
#include "gazebo/sensors/Sensor.hh"
#include <gazebo/gazebo_config.h>
#include <gazebo/gazebo_client.hh>
#include "ros/ros.h"
#include "geometry_msgs/Vector3.h"
#include "geometry_msgs/Pose.h"
#include "gazebo_msgs/LinkStates.h"
#include <vector>
#include <tf/transform_datatypes.h>
#include "stdio.h"
#include "stdlib.h"
#include <Eigen/Dense>
#include "geometry_msgs/Wrench.h"
#include <gazebo/common/Events.hh>
#include <gazebo/common/PID.hh>
#include <gazebo/common/Time.hh>
#include <fstream>
#define pi 3.14159
namespace gazebo
{
  /// \brief A plugin to control drone
  class AeroPlugin : public ModelPlugin
  {  int N=6;        //number of energie
    double c=0.016;  //blade chord width
    double R=0.15;  //blade radius
    double a=5.7;    //2D_lift_curve_slope
    double th0=0.7;//Profile inclination angle
    double thtw=0.5;//Inclination change along radius
    double pa=th0-0.75*thtw; //blade pitch angle
    double B=0.98;   //tip loss factor
    double pho=1.2;  //air density
    double A=pi*pow(R,2);//rotor aero
    double ki=1.15;  //factor to calculate torque
    double k=4.65;   //factor to calculate torque
    //parameters to calculate induced velocity Vi
    double k0=1.15;
    double k1=-1.125;
    double k2=-1.372;
    double k3=-1.718;
    double k4=-0.655;
    double CD0=0.04; //Profile_Drag_Coefficient from literatur
    double rv=0.7854; //rotor_axis_vertical_axis_angle cos(rv)=cos(pitch)*cos(yaw)
    double m; //drone_masse
    double g=9.81;   //gravity acceleration constant
    double s;         //rotor solidity
    double Vwind_x;   //wind velocity in global x
    double Vwind_y;   //wind velocity in global y
    double Vwind_z;   //wind velocity in global z
    double Vdrone_x;  //drone velocity in global x
    double Vdrone_y;  //drone velocity in global y
    double Vdrone_z;  //drone velocity in global z
    double Vx;        //air velocity in global x
    double Vy;        //air velocity in global y
    double Vz;        //air velocity in global z
    double Vi_h;     //induced velocity for the hovering case
    double force_1,force_2,force_3,force_4,force_5,force_6;//thrust
    double moment_1,moment_2,moment_3,moment_4,moment_5,moment_6;//torque
    double fh_x1,fh_x2,fh_x3,fh_x4,fh_x5,fh_x6,fh_y1,fh_y2,fh_y3,fh_y4,fh_y5,fh_y6;//H Force
    double moment_R1x,moment_R1y,moment_R2x,moment_R2y,moment_R3x,moment_R3y,moment_R4x,moment_R4y,moment_R5x,moment_R5y,moment_R6x,moment_R6y;//roll moment
    double vel_1=1;
    double vel_2=1;
    double vel_3=1;
    double vel_4=1;
    double vel_5=1;
    double vel_6=1;//calculated blade rotate velocity
    double force_x,force_y,force_z,torque_x,torque_y,torque_z;//input wrench
    //default blade rotate direction 1 positive direction,gegen Uhrzeigersinn -1 negative direction
    int di_1=1;
    int di_2=-1;
    int di_3=1;
    int di_4=-1;
    int di_5=1;
    int di_6=-1;
    //thrust force direction
    int di_force1=1;
    int di_force2=1;
    int di_force3=1;
    int di_force4=1;
    int di_force5=1;
    int di_force6=1;
    //real rotate direction
    int di_vel1,di_vel2,di_vel3,di_vel4,di_vel5,di_vel6;
    int bo=0;//bidirectional optional
    double vel_min=50;
    double vel_max=2500;
//input control signal in velocity DOF 6
   double Vx_input,Vy_input,Vz_input,Wx_input,Wy_input,Wz_input;
    double Kv0=890;//KV Value
    double Um0=10;//nominal no-load voltage
    double Im0=0.5;//nominal no-load current
    double Rm=0.101;//resitance
//transformation matrix from global coordinate to body coordinate
    Eigen::Matrix3d T_trans;
    /// \brief Constructor
    public: AeroPlugin() {}

    /// \brief The load function is called by Gazebo when the plugin is
    /// inserted into simulation
    /// \param[in] _model A pointer to the model that this plugin is
    /// attached to.
    /// \param[in] _sdf A pointer to the plugin's SDF element.
    public: virtual void Load(physics::ModelPtr _model, sdf::ElementPtr _sdf)
    {
      if (_model->GetJointCount() == 0)
  {
    std::cerr << "Invalid joint count, Velodyne plugin not loaded\n";
    return;
  }


  // Store the model pointer for convenience.
  this->model = _model;
  // Get the first joint. We are making an assumption about the model
  // having six joints that is the rotational joint.
  this->joint1 = _model->GetJoint("blade_joint1");
  this->joint2 = _model->GetJoint("blade_joint2");
  this->joint3 = _model->GetJoint("blade_joint3");
  this->joint4 = _model->GetJoint("blade_joint4");
  this->joint5 = _model->GetJoint("blade_joint5");
  this->joint6 = _model->GetJoint("blade_joint6");
  //apply joint motors to set joint velocity
  this->joint1->SetParam("fmax", 0, 1000000.0);
  this->joint2->SetParam("fmax", 0, 1000000.0);
  this->joint3->SetParam("fmax", 0, 1000000.0);
  this->joint4->SetParam("fmax", 0, 1000000.0);
  this->joint5->SetParam("fmax", 0, 1000000.0);
  this->joint6->SetParam("fmax", 0, 1000000.0);
  //get the six blade link
  this->link0 = _model->GetChildLink("base_link");
  this->link1 = _model->GetChildLink("blade_Link1");
  this->link2 = _model->GetChildLink("blade_Link2");
  this->link3 = _model->GetChildLink("blade_Link3");
  this->link4 = _model->GetChildLink("blade_Link4");
  this->link5 = _model->GetChildLink("blade_Link5");
  this->link6 = _model->GetChildLink("blade_Link6");
//calculation of constants
m=this->link0->GetInertial()->GetMass();
s=(N*c)/(pi*R);  //rotor solidity
Vi_h=-sqrt((m*g)/(2*N*pho*A*cos(rv)));//induced airflow velocity by hover case
// Initialize ros, if it has not already bee initialized.
if (!ros::isInitialized())
{
  int argc = 0;
  char **argv = NULL;
  ros::init(argc, argv, "gazebo_client",
      ros::init_options::NoSigintHandler);
}
// Create our ROS node.
this->rosNode.reset(new ros::NodeHandle("gazebo_client"));
this->readParamsFromServer();
//Create a wind velocity topic and subscribe to it
ros::SubscribeOptions s1 =
  ros::SubscribeOptions::create<geometry_msgs::Vector3>(
       "/drone/wind_cmd",
      1,

      boost::bind(&AeroPlugin::OnRosWindMsg, this, _1),
      ros::VoidPtr(), &this->rosQueue);
this->rosSubWind = this->rosNode->subscribe(s1);
//subscribe to model link states to get position and orientation for coordinate transformation
ros::SubscribeOptions s2 =
ros::SubscribeOptions::create<gazebo_msgs::LinkStates>(
  "/gazebo/link_states",
    1,

    boost::bind(&AeroPlugin::OnlinkMsg, this, _1),
    ros::VoidPtr(), &this->rosQueue);
this->rosSubLink = this->rosNode->subscribe(s2);
//subscribe to control signal,six global velocity for drone
ros::SubscribeOptions s3 =
ros::SubscribeOptions::create<geometry_msgs::Wrench>(
  "/drone/rotor_cmd",
    1,

    boost::bind(&AeroPlugin::OnControlMsg, this, _1),
    ros::VoidPtr(), &this->rosQueue);
this->rosSubControl = this->rosNode->subscribe(s3);
this->rosQueueThread =
  std::thread(std::bind(&AeroPlugin::QueueThread, this));
// Spin up the queue helper thread.
//update world to apply constant force
this->updateConnection = event::Events::ConnectWorldUpdateBegin(
          boost::bind(&AeroPlugin::OnUpdate, this, _1));
}
public: void OnUpdate(const common::UpdateInfo & _info)
    {
      this->SetVelocity();
      this->SetForce();
      this->SetTorque();

    }

//calculate aerodynamic
public: void OnlinkMsg(const gazebo_msgs::LinkStatesConstPtr &msg)
{   int i;
  //blade1 aero dynamic,Vxx1,Vyy1...airflow velocity in blade local coordinate
  double q1_1,q1_2,q1_3,q1_4,Vxx1,Vyy1,Vzz1,Vxy1,C1,Vi1;
  double CT1,l1,u1,CQ1;//aerodynamic coefficient
  double fh1,a1;//H-force and its orientation in blade rotate xy plane
  double momentR1;//roll moment
   q1_1=msg->pose[2].orientation.x;
   q1_2=msg->pose[2].orientation.y;
   q1_3=msg->pose[2].orientation.z;
   q1_4=msg->pose[2].orientation.w;
   Eigen::Matrix3d T1;
   Eigen::Matrix3d T1_trans;
   T1(0,0)=pow(q1_1,2)-pow(q1_2,2)-pow(q1_3,2)+pow(q1_4,2);
   T1(0,1)=2*q1_1*q1_2-2*q1_3*q1_4;
   T1(0,2)=2*q1_1*q1_3+2*q1_2*q1_4;
   T1(1,0)=2*q1_1*q1_2+2*q1_3*q1_4;
   T1(1,1)=-pow(q1_1,2)+pow(q1_2,2)-pow(q1_3,2)+pow(q1_4,2);
   T1(1,2)=2*q1_2*q1_3-2*q1_1*q1_4;
   T1(2,0)=2*q1_1*q1_3-2*q1_2*q1_4;
   T1(2,1)=2*q1_2*q1_3+2*q1_1*q1_4;
   T1(2,2)=-pow(q1_1,2)-pow(q1_2,2)+pow(q1_3,2)+pow(q1_4,2);
   T1_trans=T1.transpose();
   Eigen::Vector3d V_airflow;
   Eigen::Vector3d V1_local_airflow;
   V_airflow(0)=Vx;
   V_airflow(1)=Vy;
   V_airflow(2)=Vz;
   V1_local_airflow=T1_trans*V_airflow;
   Vxx1=V1_local_airflow(0);
   Vyy1=V1_local_airflow(1);
   Vzz1=V1_local_airflow(2);
   Vxy1=sqrt(pow(Vxx1,2)+pow(Vyy1,2));

    C1=Vzz1/Vi_h;
    if(C1>=0)
    {
    Vi1=-(sqrt(pow((Vzz1/2),2)+pow(Vi_h,2))+Vzz1/2); //Vi induced airflow velocity
    }
    else if(C1>=-2&&C1<0)
    {
    Vi1=Vi_h*(k0+k1*C1+k2*pow(C1,2)+k3*pow(C1,3)+k4*pow(C1,4));
    }
    else
    {
    Vi1=-Vzz1/2+sqrt(pow((Vzz1/2),2)-pow(Vi_h,2));
  }
if(vel_1<=vel_min)
{
  vel_1=vel_min;
  //std::cout<<"Warning! Velocity of blade1 under limit"<<std::endl;
}
else if (vel_1>=vel_max)
{
  vel_1=vel_max;
  //std::cout<<"Warning! Velocity of blade1 over limit"<<std::endl;
}

    force_1=di_force1*0.5*pho*s*a*A*((pa/3)*pow(B,3)*pow((vel_1*R),2)+(pa/2)*B*pow(Vxy1,2)-(-Vi1-Vzz1)*pow(B,2)*vel_1*R/2);
    CT1=abs(force_1)/(0.5*pho*pow((vel_1*R),2)*A);
    l1=(-Vi1-Vzz1)/(vel_1*R);// inflow rate
    u1=Vxy1/(vel_1*R);
    CQ1=ki*l1*CT1+0.25*s*CD0*(1+k*pow(u1,2));
    moment_1=-0.5*pho*pow((vel_1*R),2)*A*R*CQ1*Sgn(this->link1->GetRelativeAngularVel().z);
    momentR1=0.125*s*a*pho*R*A*Vxy1*(((4/3)*th0-thtw)*vel_1*R+Vi1+Vzz1);
if(Vxx1>=0)
{
a1=atan(Vyy1/Vxx1);//orientation of Vxy in blade coordinate
}
else{
a1=pi+atan(Vyy1/Vxx1);
}
fh1=0.25*s*pho*vel_1*R*A*CD0*Vxy1;//H-force
fh_x1=fh1*cos(a1);//H-force in x direction
fh_y1=fh1*sin(a1);//H-force in y direction
moment_R1x=momentR1*cos(a1);//roll moment in x direction
moment_R1y=momentR1*sin(a1);//roll moment in y direction
//blade2 aerodynamic
double q2_1,q2_2,q2_3,q2_4,Vxx2,Vyy2,Vzz2,Vxy2,C2,Vi2;
double CT2,l2,u2,CQ2;
double fh2,a2;
double momentR2;
 q2_1=msg->pose[3].orientation.x;
 q2_2=msg->pose[3].orientation.y;
 q2_3=msg->pose[3].orientation.z;
 q2_4=msg->pose[3].orientation.w;
 Eigen::Matrix3d T2;
 Eigen::Matrix3d T2_trans;
 T2(0,0)=pow(q2_1,2)-pow(q2_2,2)-pow(q2_3,2)+pow(q2_4,2);
 T2(0,1)=2*q2_1*q2_2-2*q2_3*q2_4;
 T2(0,2)=2*q2_1*q2_3+2*q2_2*q2_4;
 T2(1,0)=2*q2_1*q2_2+2*q2_3*q2_4;
 T2(1,1)=-pow(q2_1,2)+pow(q2_2,2)-pow(q2_3,2)+pow(q2_4,2);
 T2(1,2)=2*q2_2*q2_3-2*q2_1*q2_4;
 T2(2,0)=2*q2_1*q2_3-2*q2_2*q2_4;
 T2(2,1)=2*q2_2*q2_3+2*q2_1*q2_4;
 T2(2,2)=-pow(q2_1,2)-pow(q2_2,2)+pow(q2_3,2)+pow(q2_4,2);
 T2_trans=T2.transpose();
 Eigen::Vector3d V2_local_airflow;
 V2_local_airflow=T2_trans*V_airflow;
 Vxx2=V2_local_airflow(0);
 Vyy2=V2_local_airflow(1);
 Vzz2=V2_local_airflow(2);
 Vxy2=sqrt(pow(Vxx2,2)+pow(Vyy2,2));
  C2=Vzz2/Vi_h;
  if(C2>=0)
  {
  Vi2=-(sqrt(pow((Vzz2/2),2)+pow(Vi_h,2))+Vzz2/2);
  }
  else if(C2>=-2&&C2<0)
  {
  Vi2=-Vi_h*(k0+k1*C2+k2*pow(C2,2)+k3*pow(C2,3)+k4*pow(C2,4));
  }
  else
  {
  Vi2=-Vzz2/2+sqrt(pow((Vzz2/2),2)-pow(Vi_h,2));
}

if(vel_2<=vel_min)
{
  vel_2=vel_min;
  //std::cout<<"Warning! Velocity of blade2 under limit"<<std::endl;
}
else if (vel_2>=vel_max)
{
  vel_2=vel_max;
  //std::cout<<"Warning! Velocity of blade 2 over limit"<<std::endl;
}
if(Vxx2>=0)
{
     a2=atan(Vyy2/Vxx2);
   }
   else{
     a2=pi+atan(Vyy2/Vxx2);
   }
     fh2=0.25*s*pho*vel_2*R*A*CD0*Vxy2;
     fh_x2=fh2*cos(a2);
     fh_y2=fh2*sin(a2);
     force_2=di_force2*0.5*pho*s*a*A*((pa/3)*pow(B,3)*pow((vel_2*R),2)+(pa/2)*B*pow(Vxy2,2)-(-Vi2-Vzz2)*pow(B,2)*vel_2*R/2);
      CT2=abs(force_2)/(0.5*pho*pow((vel_2*R),2)*A);
      l2=(-Vi2-Vzz2)/(vel_2*R);
      u2=Vxy2/(vel_2*R);
      CQ2=ki*l2*CT2+0.25*s*CD0*(1+k*pow(u2,2));
      moment_2=-0.5*pho*pow((vel_2*R),2)*A*R*CQ2*Sgn(this->link2->GetRelativeAngularVel().z);
      momentR2=0.125*s*a*pho*R*A*Vxy2*(((4/3)*th0-thtw)*vel_2*R+Vi2+Vzz2);


    moment_R2x=momentR2*cos(a2);
    moment_R2y=momentR2*sin(a2);

//blade3 aerodynamic
double q3_1,q3_2,q3_3,q3_4,Vxx3,Vyy3,Vzz3,Vxy3,C3,Vi3;
double CT3,l3,u3,CQ3;
double fh3,a3;
double momentR3;
 q3_1=msg->pose[4].orientation.x;
 q3_2=msg->pose[4].orientation.y;
 q3_3=msg->pose[4].orientation.z;
 q3_4=msg->pose[4].orientation.w;
 Eigen::Matrix3d T3;
 Eigen::Matrix3d T3_trans;
 T3(0,0)=pow(q3_1,2)-pow(q3_2,2)-pow(q3_3,2)+pow(q3_4,2);
 T3(0,1)=2*q3_1*q3_2-2*q3_3*q3_4;
 T3(0,2)=2*q3_1*q3_3+2*q3_2*q3_4;
 T3(1,0)=2*q3_1*q3_2+2*q3_3*q3_4;
 T3(1,1)=-pow(q3_1,2)+pow(q3_2,2)-pow(q3_3,2)+pow(q3_4,2);
 T3(1,2)=2*q3_2*q3_3-2*q3_1*q3_4;
 T3(2,0)=2*q3_1*q3_3-2*q3_2*q3_4;
 T3(2,1)=2*q3_2*q3_3+2*q3_1*q3_4;
 T3(2,2)=-pow(q3_1,2)-pow(q3_2,2)+pow(q3_3,2)+pow(q3_4,2);
 T3_trans=T3.transpose();
 Eigen::Vector3d V3_local_airflow;
 V3_local_airflow=T3_trans*V_airflow;
 Vxx3=V3_local_airflow(0);
 Vyy3=V3_local_airflow(1);
 Vzz3=V3_local_airflow(2);
 Vxy3=sqrt(pow(Vxx3,2)+pow(Vyy3,2));
  C3=Vzz3/Vi_h;
  if(C3>=0)
  {
  Vi3=-(sqrt(pow((Vzz3/2),2)+pow(Vi_h,2))+Vzz3/2);
  }
  else if(C3>=-2&&C3<0)
  {
  Vi3=-Vi_h*(k0+k1*C3+k2*pow(C3,2)+k3*pow(C3,3)+k4*pow(C3,4));
  }
  else
  {
  Vi3=-Vzz3/2+sqrt(pow((Vzz3/2),2)-pow(Vi_h,2));
}


if(vel_3<=vel_min)
{
  vel_3=vel_min;
 //std::cout<<"Warning! Velocity of blade3 under limit"<<std::endl;
}
else if (vel_3>=vel_max)
{
  vel_3=vel_max;
  //std::cout<<"Warning! Velocity of blade3 over limit"<<std::endl;
}
if(Vxx3>=0)
{
     a3=atan(Vyy3/Vxx3);
   }
   else{
    a3=pi+atan(Vyy3/Vxx3);
   }
     fh3=0.25*s*pho*vel_3*R*A*CD0*Vxy3;
     fh_x3=fh3*cos(a3);
     fh_y3=fh3*sin(a3);

      force_3=di_force3*0.5*pho*s*a*A*((pa/3)*pow(B,3)*pow((vel_3*R),2)+(pa/2)*B*pow(Vxy3,2)-(-Vi3-Vzz3)*pow(B,2)*vel_3*R/2);
      CT3=abs(force_3)/(0.5*pho*pow((vel_3*R),2)*A);
      l3=(-Vi3-Vzz3)/(vel_3*R);
      u3=Vxy3/(vel_3*R);
      CQ3=ki*l3*CT3+0.25*s*CD0*(1+k*pow(u3,2));
      moment_3=-0.5*pho*pow((vel_3*R),2)*A*R*CQ3*Sgn(this->link3->GetRelativeAngularVel().z);
      momentR3=0.125*s*a*pho*R*A*Vxy3*(((4/3)*th0-thtw)*vel_3*R+Vi3+Vzz3);

    moment_R3x=momentR3*cos(a3);
    moment_R3y=momentR3*sin(a3);

//blade4 aerodynamic
double q4_1,q4_2,q4_3,q4_4,Vxx4,Vyy4,Vzz4,Vxy4,C4,Vi4;
double CT4,l4,u4,CQ4;
double fh4,a4;
double momentR4;
 q4_1=msg->pose[5].orientation.x;
 q4_2=msg->pose[5].orientation.y;
 q4_3=msg->pose[5].orientation.z;
 q4_4=msg->pose[5].orientation.w;
 Eigen::Matrix3d T4;
 Eigen::Matrix3d T4_trans;
 T4(0,0)=pow(q4_1,2)-pow(q4_2,2)-pow(q4_3,2)+pow(q4_4,2);
 T4(0,1)=2*q4_1*q4_2-2*q4_3*q4_4;
 T4(0,2)=2*q4_1*q4_3+2*q4_2*q4_4;
 T4(1,0)=2*q4_1*q4_2+2*q4_3*q4_4;
 T4(1,1)=-pow(q4_1,2)+pow(q4_2,2)-pow(q4_3,2)+pow(q4_4,2);
 T4(1,2)=2*q4_2*q4_3-2*q4_1*q4_4;
 T4(2,0)=2*q4_1*q4_3-2*q4_2*q4_4;
 T4(2,1)=2*q4_2*q4_3+2*q4_1*q4_4;
 T4(2,2)=-pow(q4_1,2)-pow(q4_2,2)+pow(q4_3,2)+pow(q4_4,2);
 T4_trans=T4.transpose();
 Eigen::Vector3d V4_local_airflow;
 V4_local_airflow=T4_trans*V_airflow;
 Vxx4=V4_local_airflow(0);
 Vyy4=V4_local_airflow(1);
 Vzz4=V4_local_airflow(2);
 Vxy4=sqrt(pow(Vxx4,2)+pow(Vyy4,2));
  C4=Vzz4/Vi_h;
  if(C4>=0)
  {
  Vi4=-(sqrt(pow((Vzz4/2),2)+pow(Vi_h,2))+Vzz4/2);
  }
  else if(C4>=-2&&C4<0)
  {
  Vi4=-Vi_h*(k0+k1*C4+k2*pow(C4,2)+k3*pow(C4,3)+k4*pow(C4,4));
  }
  else
  {
  Vi4=-Vzz4/2+sqrt(pow((Vzz4/2),2)-pow(Vi_h,2));
}
if(vel_4<=vel_min)
{
  vel_4=vel_min;
  //std::cout<<"Warning! Velocity of blade4 under limit"<<std::endl;
}
else if (vel_4>=vel_max)
{
  vel_4=vel_max;
  //std::cout<<"Warning! Velocity of blade4 over limit"<<std::endl;
}

    force_4=di_force4*0.5*pho*s*a*A*((pa/3)*pow(B,3)*pow((vel_4*R),2)+(pa/2)*B*pow(Vxy4,2)-(-Vi4-Vzz4)*pow(B,2)*vel_4*R/2);
    CT4=abs(force_4)/(0.5*pho*pow((vel_4*R),2)*A);
    l4=(-Vi4-Vzz4)/(vel_4*R);
    u4=Vxy4/(vel_4*R);
    CQ4=ki*l4*CT4+0.25*s*CD0*(1+k*pow(u4,2));
    moment_4=-0.5*pho*pow((vel_4*R),2)*A*R*CQ4*Sgn(this->link4->GetRelativeAngularVel().z);
    momentR4=0.125*s*a*pho*R*A*Vxy4*(((4/3)*th0-thtw)*vel_4*R+Vi4+Vzz4);
if(Vxx4>=0)
{
  a4=atan(Vyy4/Vxx4);
}
else{
  a4=pi+atan(Vyy4/Vxx4);
}
  fh4=0.25*s*pho*vel_4*R*A*CD0*Vxy4;
  fh_x4=fh4*cos(a4);
  fh_y4=fh4*sin(a4);

  moment_R4x=momentR4*cos(a4);
  moment_R4y=momentR4*sin(a4);

//blade5 aerodynamic
double q5_1,q5_2,q5_3,q5_4,Vxx5,Vyy5,Vzz5,Vxy5,C5,Vi5;
double CT5,l5,u5,CQ5;
double fh5,a5;
double momentR5;
 q5_1=msg->pose[6].orientation.x;
 q5_2=msg->pose[6].orientation.y;
 q5_3=msg->pose[6].orientation.z;
 q5_4=msg->pose[6].orientation.w;
 Eigen::Matrix3d T5;
 Eigen::Matrix3d T5_trans;
 T5(0,0)=pow(q5_1,2)-pow(q5_2,2)-pow(q5_3,2)+pow(q5_4,2);
 T5(0,1)=2*q5_1*q5_2-2*q5_3*q5_4;
 T5(0,2)=2*q5_1*q5_3+2*q5_2*q5_4;
 T5(1,0)=2*q5_1*q5_2+2*q5_3*q5_4;
 T5(1,1)=-pow(q5_1,2)+pow(q5_2,2)-pow(q5_3,2)+pow(q5_4,2);
 T5(1,2)=2*q5_2*q5_3-2*q5_1*q5_4;
 T5(2,0)=2*q5_1*q5_3-2*q5_2*q5_4;
 T5(2,1)=2*q5_2*q5_3+2*q5_1*q5_4;
 T5(2,2)=-pow(q5_1,2)-pow(q5_2,2)+pow(q5_3,2)+pow(q5_4,2);
 T5_trans=T5.transpose();
 Eigen::Vector3d V5_local_airflow;
 V5_local_airflow=T5_trans*V_airflow;
 Vxx5=V5_local_airflow(0);
 Vyy5=V5_local_airflow(1);
 Vzz5=V5_local_airflow(2);
 Vxy5=sqrt(pow(Vxx5,2)+pow(Vyy5,2));
  C5=Vzz5/Vi_h;
  if(C5>=0)
  {
  Vi5=-(sqrt(pow((Vzz5/2),2)+pow(Vi_h,2))+Vzz5/2);
  }
  else if(C5>=-2&&C5<0)
  {
  Vi5=-Vi_h*(k0+k1*C5+k2*pow(C5,2)+k3*pow(C5,3)+k4*pow(C5,4));
  }
  else
  {
  Vi5=-Vzz5/2+sqrt(pow((Vzz5/2),2)-pow(Vi_h,2));
}
if(vel_5<=vel_min)
{
  vel_5=vel_min;
  //std::cout<<"Warning! Velocity of blade5 under limit"<<std::endl;
}
else if (vel_5>=vel_max)
{
  vel_5=vel_max;
  //std::cout<<"Warning! Velocity of blade5 over limit"<<std::endl;
}

    force_5=di_force5*0.5*pho*s*a*A*((pa/3)*pow(B,3)*pow((vel_5*R),2)+(pa/2)*B*pow(Vxy5,2)-(-Vi5-Vzz5)*pow(B,2)*vel_5*R/2);
    CT5=abs(force_5)/(0.5*pho*pow((vel_5*R),2)*A);
    l5=(-Vi5-Vzz5)/(vel_5*R);
    u5=Vxy5/(vel_5*R);
    CQ5=ki*l5*CT5+0.25*s*CD0*(1+k*pow(u5,2));
    moment_5=-0.5*pho*pow((vel_5*R),2)*A*R*CQ5*Sgn(this->link5->GetRelativeAngularVel().z);
    momentR5=0.125*s*a*pho*R*A*Vxy5*(((4/3)*th0-thtw)*vel_5*R+Vi5+Vzz5);
if(Vxx5>=0)
{
  a5=atan(Vyy5/Vxx5);
}
else{
  a5=pi+atan(Vyy5/Vxx5);
}
  fh5=0.25*s*pho*vel_5*R*A*CD0*Vxy5;
  fh_x5=fh5*cos(a5);
  fh_y5=fh5*sin(a5);

  moment_R5x=momentR5*cos(a5);
  moment_R5y=momentR5*sin(a5);

//blade6 aerodynamic
double q6_1,q6_2,q6_3,q6_4,Vxx6,Vyy6,Vzz6,Vxy6,C6,Vi6;
double CT6,l6,u6,CQ6;
double fh6,a6;
double momentR6;
 q6_1=msg->pose[7].orientation.x;
 q6_2=msg->pose[7].orientation.y;
 q6_3=msg->pose[7].orientation.z;
 q6_4=msg->pose[7].orientation.w;
 Eigen::Matrix3d T6;
 Eigen::Matrix3d T6_trans;
 T6(0,0)=pow(q6_1,2)-pow(q6_2,2)-pow(q6_3,2)+pow(q6_4,2);
 T6(0,1)=2*q6_1*q6_2-2*q6_3*q6_4;
 T6(0,2)=2*q6_1*q6_3+2*q6_2*q6_4;
 T6(1,0)=2*q6_1*q6_2+2*q6_3*q6_4;
 T6(1,1)=-pow(q6_1,2)+pow(q6_2,2)-pow(q6_3,2)+pow(q6_4,2);
 T6(1,2)=2*q6_2*q6_3-2*q6_1*q6_4;
 T6(2,0)=2*q6_1*q6_3-2*q6_2*q6_4;
 T6(2,1)=2*q6_2*q6_3+2*q6_1*q6_4;
 T6(2,2)=-pow(q6_1,2)-pow(q6_2,2)+pow(q6_3,2)+pow(q6_4,2);
 T6_trans=T6.transpose();
 Eigen::Vector3d V6_local_airflow;
 V6_local_airflow=T6_trans*V_airflow;
 Vxx6=V6_local_airflow(0);
 Vyy6=V6_local_airflow(1);
 Vzz6=V6_local_airflow(2);
 Vxy6=sqrt(pow(Vxx6,2)+pow(Vyy6,2));
  C6=Vzz6/Vi_h;
  if(C6>=0)
  {
  Vi6=-(sqrt(pow((Vzz6/2),2)+pow(Vi_h,2))+Vzz6/2);
  }
  else if(C6>=-2&&C6<0)
  {
  Vi6=-Vi_h*(k0+k1*C6+k2*pow(C6,2)+k3*pow(C6,3)+k4*pow(C6,4));
  }
  else
  {
  Vi6=-Vzz6/2+sqrt(pow((Vzz6/2),2)-pow(Vi_h,2));
}
if(vel_6<=vel_min)
{
  vel_6=vel_min;
  //std::cout<<"Warning! Velocity of blade6 under limit"<<std::endl;
}
else if (vel_6>=vel_max)
{
  vel_6=vel_max;
  //std::cout<<"Warning! Velocity of blade6 over limit"<<std::endl;
}

    force_6=di_force6*0.5*pho*s*a*A*((pa/3)*pow(B,3)*pow((vel_6*R),2)+(pa/2)*B*pow(Vxy6,2)-(-Vi6-Vzz6)*pow(B,2)*vel_6*R/2);
    CT6=abs(force_6)/(0.5*pho*pow((vel_6*R),2)*A);
    l6=(-Vi6-Vzz6)/(vel_6*R);
    u6=Vxy6/(vel_6*R);
    CQ6=ki*l6*CT6+0.25*s*CD0*(1+k*pow(u6,2));
    moment_6=-0.5*pho*pow((vel_6*R),2)*A*R*CQ6*Sgn(this->link6->GetRelativeAngularVel().z);
    momentR6=0.125*s*a*pho*R*A*Vxy6*(((4/3)*th0-thtw)*vel_6*R+Vi6+Vzz6);
if(Vxx6>=0)
{
  a6=atan(Vyy6/Vxx6);
}
else{
a6=pi+atan(Vyy6/Vxx6);
}
  fh6=0.25*s*pho*vel_6*R*A*CD0*Vxy6;
  fh_x6=fh6*cos(a6);
  fh_y6=fh6*sin(a6);

  moment_R6x=momentR6*cos(a6);
  moment_R6y=momentR6*sin(a6);
double ratio1,ratio2,ratio3,ratio4,ratio5,ratio6;
ratio1=-3*R*CQ1*Sgn(this->link1->GetRelativeAngularVel().z)/(s*a*pa*pow(B,3))*di_force1;
ratio2=-3*R*CQ2*Sgn(this->link2->GetRelativeAngularVel().z)/(s*a*pa*pow(B,3))*di_force2;
ratio3=-3*R*CQ3*Sgn(this->link3->GetRelativeAngularVel().z)/(s*a*pa*pow(B,3))*di_force3;
ratio4=-3*R*CQ4*Sgn(this->link4->GetRelativeAngularVel().z)/(s*a*pa*pow(B,3))*di_force4;
ratio5=-3*R*CQ5*Sgn(this->link5->GetRelativeAngularVel().z)/(s*a*pa*pow(B,3))*di_force5;
ratio6=-3*R*CQ6*Sgn(this->link6->GetRelativeAngularVel().z)/(s*a*pa*pow(B,3))*di_force6;
this->pub_ratio = this->rosNode->advertise<geometry_msgs::Wrench>("/drone/thrust_moment_ratio", 1);
 geometry_msgs::Wrench _msg;
  _msg.force.x = ratio1;
  _msg.force.y = ratio2;
  _msg.force.z = ratio3;
  _msg.torque.x = ratio4;
  _msg.torque.y = ratio5;
  _msg.torque.z = ratio6;
  this->pub_ratio.publish(_msg);
    ros::spinOnce();

}

public: void OnRosWindMsg(const geometry_msgs::Vector3ConstPtr &_wind_msg)
{
 Vwind_x=_wind_msg->x;
 Vwind_y=_wind_msg->y;
 Vwind_z=_wind_msg->z;
 Vdrone_x=this->model->GetRelativeLinearVel().x;
 Vdrone_y=this->model->GetRelativeLinearVel().y;
 Vdrone_z=this->model->GetRelativeLinearVel().z;
 Vx=Vwind_x-Vdrone_x;
 Vy=Vwind_y-Vdrone_y;
 Vz=Vwind_z-Vdrone_z;
}

public: void OnControlMsg(const geometry_msgs::WrenchConstPtr &_msg)
{
  double vel1,vel2,vel3,vel4,vel5,vel6;
  vel1=_msg->force.x;
  vel2=_msg->force.y;
  vel3=_msg->force.z;
  vel4=_msg->torque.x;
  vel5=_msg->torque.y;
  vel6=_msg->torque.z;
  if(bo==0)//bidirectional modul
  {
    if(vel1<0)
    {
      vel_1=abs(vel1);
      di_vel1=-di_1;
      di_force1=-1;
    }
    else
    {
    vel_1=vel1;
    di_vel1=di_1;
    di_force1=1;
    }
    if(vel2<0)
    {
      vel_2=abs(vel2);
      di_vel2=-di_2;
      di_force2=-1;
    }
    else
    {
    vel_2=vel2;
    di_vel2=di_2;
    di_force2=1;
    }
    if(vel3<0)
    {
      vel_3=abs(vel3);
      di_vel3=-di_3;
      di_force3=-1;
    }
    else
    {
    vel_3=vel3;
    di_vel3=di_3;
    di_force3=1;
    }
    if(vel4<0)
    {
      vel_4=abs(vel4);
      di_vel4=-di_4;
      di_force4=-1;
    }
    else
    {
    vel_4=vel4;
    di_vel4=di_4;
    di_force4=1;
    }
    if(vel5<0)
    {
      vel_5=abs(vel5);
      di_vel5=-di_5;
      di_force5=-1;
    }
    else
    {
    vel_5=vel5;
    di_vel5=di_5;
    di_force5=1;
    }
    if(vel6<0)
    {
      vel_6=abs(vel6);
      di_vel6=-di_6;
      di_force6=-1;
    }
    else
    {
    vel_6=vel6;
    di_vel6=di_6;
    di_force6=1;
    }
  }
  else if(bo==1)
  {
    if(vel1<0)
    {
      vel_1=0;
    }
    else
    {
      vel_1=vel1;
    }
    di_vel1=di_1;
    di_force1=1;
    if(vel2<0)
    {
      vel_2=0;
    }
    else
    {
      vel_2=vel2;
    }
    di_vel2=di_2;
    di_force2=1;
    if(vel3<0)
    {
      vel_3=0;
    }
    else
    {
      vel_3=vel3;
    }
    di_vel3=di_3;
    di_force3=1;
    if(vel4<0)
    {
      vel_4=0;
    }
    else
    {
      vel_4=vel4;
    }
    di_vel4=di_4;
    di_force4=1;
    if(vel5<0)
    {
      vel_5=0;
    }
    else
    {
      vel_5=vel5;
    }
    di_vel5=di_5;
    di_force5=1;
    if(vel6<0)
    {
      vel_6=0;
    }
    else
    {
      vel_6=vel6;
    }
    di_vel6=di_6;
    di_force6=1;
  }
}

private: void QueueThread()
{
  static const double timeout = 0.01;
  while (this->rosNode->ok())
  {
    this->rosQueue.callAvailable(ros::WallDuration(timeout));
  }
}
//return sgn information
public: int Sgn(const double &num)
{
  if(num<0)
  {return -1;}
  else if (num>0)
  {return 1;}
  else
  {return 0;}
}
//add force to blade link
public: void SetForce()
{
this->link1->AddRelativeForce(math::Vector3(fh_x1, fh_y1, force_1));
this->link2->AddRelativeForce(math::Vector3(fh_x2, fh_y2, force_2));
this->link3->AddRelativeForce(math::Vector3(fh_x3, fh_y3, force_3));
this->link4->AddRelativeForce(math::Vector3(fh_x4, fh_y4, force_4));
this->link5->AddRelativeForce(math::Vector3(fh_x5, fh_y5, force_5));
this->link6->AddRelativeForce(math::Vector3(fh_x6, fh_y6, force_6));
}
//add torque to blade link
public: void SetTorque()
{
this->link1->AddRelativeTorque(math::Vector3(moment_R1x, moment_R1y, moment_1));
this->link2->AddRelativeTorque(math::Vector3(moment_R2x, moment_R2y, moment_2));
this->link3->AddRelativeTorque(math::Vector3(moment_R3x, moment_R3y, moment_3));
this->link4->AddRelativeTorque(math::Vector3(moment_R4x, moment_R4y, moment_4));
this->link5->AddRelativeTorque(math::Vector3(moment_R5x, moment_R5y, moment_5));
this->link6->AddRelativeTorque(math::Vector3(moment_R6x, moment_R6y, moment_6));
}
//apply velocity to joints with 3 metnods
public: void SetVelocity()
{
  
  this->joint1->SetParam("vel", 0, vel_1*di_vel1);
  
  this->joint2->SetParam("vel", 0, vel_2*di_vel2);
  
  this->joint3->SetParam("vel", 0, vel_3*di_vel3);
  
  this->joint4->SetParam("vel", 0, vel_4*di_vel4);
  
  this->joint5->SetParam("vel", 0, vel_5*di_vel5);
 
  this->joint6->SetParam("vel", 0, vel_6*di_vel6);
}



//load parameter from gazebo server
private: void readParamsFromServer()
{

 this->rosNode->param("rotor_number", N, N);
 this->rosNode->param("blade_chord_width", c, c);
 this->rosNode->param("blade_radius", R, R);
 this->rosNode->param("2D_lift_curve_slope", a, a);
 this->rosNode->param("profile_inclination_angle", th0, th0);
 this->rosNode->param("radial_inclination_change", thtw, thtw);
 this->rosNode->param("TLF", B, B);
 this->rosNode->param("air_density", pho, pho);
 this->rosNode->param("Profile_Drag_Coefficient", CD0, CD0);
 this->rosNode->param("rotor_axis_vertical_axis_angle", rv, rv);
 this->rosNode->param("minimal_rotor_velocity", vel_min, vel_min);
 this->rosNode->param("maximal_rotor_velocity", vel_max, vel_max);
 this->rosNode->param("default_blade1_rotation_direction", di_1, di_1);
 this->rosNode->param("default_blade2_rotation_direction", di_2, di_2);
 this->rosNode->param("default_blade3_rotation_direction", di_3, di_3);
 this->rosNode->param("default_blade4_rotation_direction", di_4, di_4);
 this->rosNode->param("default_blade5_rotation_direction", di_5, di_5);
 this->rosNode->param("default_blade6_rotation_direction", di_6, di_6);
 this->rosNode->param("bidirectional_optional", bo, bo);
 this->rosNode->param("KV_value", Kv0, Kv0);
 this->rosNode->param("nominal_no_load_voltage", Um0, Um0);
 this->rosNode->param("nominal_no_load_current", Im0, Im0);
 this->rosNode->param("motor_resitance", Rm, Rm);
}
/// \brief Pointer to the model.
private: physics::ModelPtr model;
// \brief A node used for transport
private: transport::NodePtr node;

/// \brief A subscriber to a named topic.
private: transport::SubscriberPtr sub;
/// \brief Pointer to the joint.
private: physics::JointWrench wrench;
private: physics::JointPtr joint1 ;
private: physics::JointPtr joint2 ;
private: physics::JointPtr joint3 ;
private: physics::JointPtr joint4 ;
private: physics::JointPtr joint5 ;
private: physics::JointPtr joint6 ;
private: physics::LinkPtr link0;
private: physics::LinkPtr link1;
private: physics::LinkPtr link2;
private: physics::LinkPtr link3;
private: physics::LinkPtr link4;
private: physics::LinkPtr link5;
private: physics::LinkPtr link6;
/// \brief A node use for ROS transport
private: std::unique_ptr<ros::NodeHandle> rosNode;
private: event::ConnectionPtr updateConnection;
/// \brief A ROS subscriber
private: ros::Subscriber rosSub;
private: ros::Subscriber rosSubWind;
private: ros::Subscriber rosSubLink;
private: ros::Subscriber rosSubControl;
//private: ros::Publisher  rosPub;
/// \brief A ROS callbackqueue that helps process messages
private: ros::CallbackQueue rosQueue;

/// \brief A thread the keeps running the rosQueue
private: std::thread rosQueueThread;

private: ros::Publisher pub_ratio;


  };

  // Tell Gazebo about this plugin, so that Gazebo can call Load on this plugin.
  GZ_REGISTER_MODEL_PLUGIN(AeroPlugin)
}
#endif

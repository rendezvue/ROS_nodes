/*
 * multimaster_example.cpp
 *
 *  Created on: December 15, 2015
 *      Author: Denis Tananaev
 */


#include "ros/ros.h"
#include "ros/callback_queue.h"
#include "std_msgs/String.h"
#include <string>
#include <iostream>
 #include <sstream>
#include <fstream>
#include <ros/package.h>
#include <nav_msgs/Odometry.h>
#include "multimaster_example/AddTwoInts.h"

namespace ros {
    namespace master {
        void init(const M_string& remappings);
    }
}


static std::string publish_name;

bool add(multimaster_example::AddTwoInts::Request  &req,
    multimaster_example::AddTwoInts::Response &res);

//class which create  multimaster/chatter on the foreign pc 
class foreignTopic{
public:
      foreignTopic();
     ~foreignTopic();
     void callback(const nav_msgs::Odometry::ConstPtr& msg);    
     ros::Publisher m_pub_odom;
     ros::ServiceServer m_service;

private:
     ros::NodeHandle n;
};

foreignTopic::foreignTopic() {
    m_pub_odom= n.advertise<nav_msgs::Odometry>(publish_name + "/odom", 1000);
    m_service = n.advertiseService(publish_name + "/add_two_ints", add);
}
foreignTopic::~foreignTopic(){
}

void foreignTopic::callback(const nav_msgs::Odometry::ConstPtr& msg){
    m_pub_odom.publish(msg);
}

bool add(multimaster_example::AddTwoInts::Request  &req,
         multimaster_example::AddTwoInts::Response &res)
{
  res.sum = req.a + req.b;
  ROS_INFO("request: x=%ld, y=%ld", (long int)req.a, (long int)req.b);
  ROS_INFO("sending back response: [%ld]", (long int)res.sum);
  return true;
}



//The class with functions which read the parameters from launch file and subscribe to the multimaster/chatter
class multimaster{
public:
    multimaster();
    ~multimaster();
    bool getParam();
    std::string foreign_master_uri();
    void init(ros::M_string remappings);


    std::string foreign_master, host_master;
    std::string foreign_ip;
    int foreign_port, msgsFrequency_Hz;
    ros::NodeHandle nh;
private:

};
multimaster::multimaster(){
   
}
multimaster::~multimaster(){
}
std::string multimaster::foreign_master_uri(){
return foreign_master;
}

bool multimaster::getParam(){

     //Read ip adress and port of foreign master from the launch file
    if(!nh.getParam("foreign_ip",foreign_ip)|| foreign_ip==""){
      ROS_ERROR_STREAM("PLEASE SPECIFY THE IP ADRESS OF FOREIGN MASTER IN THE LAUNCH FILE");
     return false;    
    }else if(! nh.getParam("foreign_port",foreign_port)){
      ROS_ERROR_STREAM("PLEASE SPECIFY THE PORT  OF FOREIGN MASTER IN THE LAUNCH FILE");
      return false;    
    }

    // Get the other parameters from the launch file     
    nh.param<int>("msgsFrequency_Hz", msgsFrequency_Hz, 10);
    nh.param<std::string>("publish_name", publish_name, "rdvremote" );
    //Get the host and foreign master_uri
        std::stringstream ss;
        ss <<"http://"<<foreign_ip<<":"<<foreign_port<<"/";
        foreign_master=ss.str();
        host_master=ros::master::getURI(); 
    return true;
}


void multimaster::init(ros::M_string remappings) {
                  
    ros::Rate loop_rate(msgsFrequency_Hz);     
                 foreignTopic pc; 
    //Create subscribers in the host and connect them to the foreign topics 
    remappings["__master"] = host_master;
    ros::master::init(remappings);
    ros::Subscriber subscriberFeedback = nh.subscribe("/odom", 1, &foreignTopic::callback, &pc);  
    remappings["__master"] =  foreign_master;
    ros::master::init(remappings);
    while(ros::ok() && ros::master::check()==true){
        ros::spinOnce(); 
        loop_rate.sleep();
    }

}




int main(int argc, char **argv){
    ros::M_string     remappings;

    //init ROS    
    ros::init(argc, argv,"multimaster_example");
          

     ros::NodeHandle nh;  
    ros::Rate loop_rate_main(200);//check the connection to the master every 200 mc
     float foreign_master_works= false;//set by default that foreign master is turned off


    multimaster mmaster;
   if(mmaster.getParam()==false){
    return 0;
    }

    //remap to the foreign master 
    remappings["__master"] = mmaster.foreign_master_uri();
    ros::master::init(remappings);
   //ros::NodeHandle nn; 

    //first check
if (ros::master::check()==false){
    ROS_ERROR_STREAM("DISCONNECTED FROM THE ROS_MASTER_URI:= "<< mmaster.foreign_master_uri());
}

    while(ros::ok()){
        //check that master is working
        if(ros::master::check()==true && foreign_master_works==false){
            foreign_master_works=true;   
     ROS_INFO_STREAM("CONNECTED TO THE ROS_MASTER_URI:= "<<mmaster.foreign_master_uri());      
               
            mmaster.init(remappings);                
         } else if(ros::master::check()==false && foreign_master_works==true){
                foreign_master_works=false;
               ROS_ERROR_STREAM("DISCONNECTED FROM THE ROS_MASTER_URI:= "<<mmaster.foreign_master_uri());                    
               
           }

   loop_rate_main.sleep();
    }

    return 0;
}

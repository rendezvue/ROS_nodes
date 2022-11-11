/*
 *  File :multimaster.cpp
 *
 *  Created on: Februar 23.02.2017
 *      Author: Denis Tananaev
 */

#include "multimaster/multimaster.h"


multimaster::multimaster(){

    config_name_ = "config"; 
    config_foreign_name_ = "config_foreign";
    folder_path_ = ros::package::getPath("multimaster"); 
 
}

multimaster::~multimaster(){
}

std::string multimaster::foreign_master_uri(){
    return foreign_master;
}

bool multimaster::getHostTopicsList(){
    
std::ifstream file_stream_topics(folder_path_+"/"+ config_name_ + ".yaml");
    if (file_stream_topics.is_open()) { // check if file exsist
        std::string line;
        std::string parser;
        
        while (std::getline(file_stream_topics, line)) {
            std::istringstream iss(line);
            getline(iss, parser, ':');

           //get the list of topics from local computer
            if(parser=="local_pubs"){

                while (getline(file_stream_topics, line)) {
                    std::string delim =",";
                    auto start=line.find("/");
                    auto end = line.find(delim);
                    while(end != std::string::npos){
                        std::string temp=line.substr(start, end - start);
                        hostTopicsList.push_back(temp);
                        std::cout << line.substr(start, end - start) << std::endl;
                        start = end + delim.length();
                        end = line.find(delim, start);
                        } 
                }}

            //get list of transforms from host
            if(parser=="local_tf"){
                std::string delim =",";
                std::string delim_tf ="->";
                auto start=line.find(":")+3;
                auto split=line.find(delim_tf);
                auto end = line.find(delim);
                while(end != std::string::npos){
                        tfTransform temp;
                        temp.from=line.substr(start, split - start);
                        temp.to=line.substr(split+delim_tf.length(), end-split-delim_tf.length());
                        hostTfList.push_back(temp);
                        start = end + delim.length();
                        split=line.find(delim_tf, start);
                        end = line.find(delim, start);
                    } 
                }
        }
        

        
        file_stream_topics.close();
    }else{

        return false;
    }
    return true;
}

//foreign pubs -> config_foreign.yaml 
bool multimaster::getForeignTopicsList(){
    
std::ifstream file_stream_topics(folder_path_+"/"+ config_foreign_name_ + ".yaml");
    if (file_stream_topics.is_open()) { // check if file exsist
        std::string line;
        std::string parser;
        //parser checking
        std::getline(file_stream_topics, line);
        std::istringstream iss(line);
        getline(iss, parser, ':');

        if(parser=="foreign_pubs"){

            while (getline(file_stream_topics, line)) {
                std::string delim =",";
                auto start=line.find("/");
                auto end = line.find(delim);
                while(end != std::string::npos){
                    std::string temp=line.substr(start, end - start);
                    hostTopicsList.push_back(temp);
                    std::cout << line.substr(start, end - start) << std::endl;
                    start = end + delim.length();
                    end = line.find(delim, start);
                    }  

            }
        }
        
        file_stream_topics.close();
    }else{

        return false;
    }
    return true;

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
     nh.param<std::string>("namespace", namesp, "intercom2");


    
    //Get the host and foreign master_uri
        std::stringstream ss;
        ss <<"http://"<<foreign_ip<<":"<<foreign_port<<"/";
        foreign_master=ss.str();
        host_master=ros::master::getURI();
   

    return true;
}


void multimaster::host2foreign(ros::M_string remappings) {
                  
     ros::Rate loop_rate(msgsFrequency_Hz);     
           
     
    
     tf::TransformBroadcaster broadcaster;
   
     ros::Time foreign_time=ros::Time::now();

     remappings["__master"] = host_master;
     ros::master::init(remappings);

     ros::Time host_time=ros::Time::now();
     ros::Duration difference=host_time-foreign_time; 

    relayTopic pc;
    for(int i=0; i<hostTopicsList.size();i++){         
        //Create subscribers in the host and connect them to the foreign topics 
        pc.subscribe(hostTopicsList[i],namesp, nh); 
        std::cout<<"hostTopicsList"<<"["<<i<<"]= "<<hostTopicsList[i].c_str()<<"\n";
    
    } 
    remappings["__master"] =  foreign_master;
    ros::master::init(remappings);

    
    ros::Duration(0.5).sleep();
    while(ros::ok() && ros::master::check()==true){
        ros::spinOnce();
        loop_rate.sleep();

          for(int i=0; i<hostTfList.size();i++){ 
            broadcaster.sendTransform(
            tf::StampedTransform(pc.listen(ros::Time::now()+difference,hostTfList[i].from, hostTfList[i].to)));
            
            }
    }
 }  


void multimaster::foreign2host(ros::M_string remappings) {
         
  
    relayTopic pc2;
  
     for(int i=0; i<foreignTopicsList.size();i++){         
    //Create subscribers in the host and connect them to the foreign topics 
         pc2.subscribe(foreignTopicsList[i],namesp, nh);   
        std::cout<<"foreignTopicsList"<<"["<<i<<"]= "<<foreignTopicsList[i].c_str()<<"\n";
    } 

    ros::Time foreign_time=ros::Time::now(); 
    remappings["__master"] = host_master;
    ros::master::init(remappings);
    ros::Time host_time=ros::Time::now();
    ros::Duration difference=host_time-foreign_time; 
    
     ros::Rate loop_rate(msgsFrequency_Hz); 
      tf::TransformBroadcaster broadcaster;
    ros::Duration(0.5).sleep();
    while(ros::ok() && ros::master::check()==true){
        ros::spinOnce();
        loop_rate.sleep();

              for(int i=0; i<foreignTfList.size();i++){ 
            broadcaster.sendTransform(
            tf::StampedTransform(pc2.listen(ros::Time::now()+difference,foreignTfList[i].from, foreignTfList[i].to)));
            
            }
    }
}




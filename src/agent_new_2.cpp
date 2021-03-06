#include "ros/ros.h"
#include <iostream>
#include <sstream>
#include <cstdlib>
#include "std_msgs/String.h"
#include "std_msgs/Header.h"
#include "main_loop/agent.h"
#include "sensor_msgs/LaserScan.h"
#include <std_msgs/Int32MultiArray.h>
#include <std_msgs/Int32.h>
#include "lidar_2020/alert_range.h"
#include <main_loop/position.h>

class sub_class{
    public:
        void ST1_sub_callback(const std_msgs::Int32MultiArray::ConstPtr& msg);
        void ST2_sub_callback(const std_msgs::Int32MultiArray::ConstPtr& msg); 
        void lidarmsg_sub_callback(const lidar_2020::alert_range::ConstPtr& msg);
        void camera_sub_callback(const main_loop::position::ConstPtr& msg);
        void status_sub_callback(const std_msgs::Int32::ConstPtr& msg);
        void publish_(float time);
        void status_publish();  
        void change_status(int z); 
        int now_status(); 
        int degree_temp = 0 ;
        
        sub_class(int my_pos_x_ = 700, int my_pos_y_ = 300, int ini_status = 0);
        ~sub_class(){};

    private:
        ros::NodeHandle n;
        ros::Publisher status_pub = n.advertise<std_msgs::Int32>("pub_status",1); 
		ros::Publisher agent_pub = n.advertise<main_loop::agent>("agent_msg", 1);
        ros::Subscriber status_sub = n.subscribe<std_msgs::Int32>("update_status", 1, &sub_class::status_sub_callback,this);
		ros::Subscriber ST1_sub = n.subscribe<std_msgs::Int32MultiArray>("rxST1", 1, &sub_class::ST1_sub_callback,this);
        ros::Subscriber ST2_sub = n.subscribe<std_msgs::Int32MultiArray>("rxST2", 1, &sub_class::ST2_sub_callback,this);
        ros::Subscriber camera_sub= n.subscribe<main_loop::position>("enemy_pose", 1, &sub_class::camera_sub_callback,this);
        ros::Subscriber lidarmsg_sub= n.subscribe<lidar_2020::alert_range>("ranging_alert", 1, &sub_class::lidarmsg_sub_callback,this);
        int status_dominate=0;
        int sub_GUI_status;
        int exact_status = 0 ;
        std_msgs::Int32 status;
        main_loop::agent pub_to_main;
};
void sub_class::status_sub_callback(const std_msgs::Int32::ConstPtr& msg){

	sub_GUI_status = msg->data;
    if(status_dominate==0){
        if(sub_GUI_status<4){
            change_status(sub_GUI_status);
        }else if(sub_GUI_status==4){
            status_dominate=1;
            change_status(sub_GUI_status);
        }
    }
    status.data = now_status();
    pub_to_main.status = now_status() ;
}
void sub_class::change_status(int z){
    exact_status = z ; 
}
int sub_class::now_status(){
    return exact_status; 
}
void sub_class::status_publish(){
    status.data = now_status() ;
	status_pub.publish(status);
}
sub_class::sub_class(int my_pos_x_,int my_pos_y_, int ini_status){
    pub_to_main.my_pos_x = 700 ; 
    pub_to_main.my_pos_y = 300 ; 
    pub_to_main.my_degree = 10090 ; 
    pub_to_main.enemy1_x = 5000 ;
    pub_to_main.enemy1_y = 5000 ;
    pub_to_main.enemy2_x = 5000 ;
    pub_to_main.enemy2_y = 5000 ;
    pub_to_main.ally_x = 5000 ;
    pub_to_main.ally_y = 5000 ; 
    status.data = ini_status;
}
void sub_class::camera_sub_callback(const main_loop::position::ConstPtr& msg){
	pub_to_main.enemy1_x = msg->enemy1_x;
    pub_to_main.enemy1_y = msg->enemy1_y;
	pub_to_main.enemy2_x = msg->enemy2_x;
	pub_to_main.enemy2_y = msg->enemy2_y;
}
void sub_class::ST1_sub_callback(const std_msgs::Int32MultiArray::ConstPtr& msg){
    pub_to_main.my_pos_x = msg->data[0] ;
    pub_to_main.my_pos_y = msg->data[1] ;
    degree_temp = msg->data[2] ;
    pub_to_main.my_degree = degree_temp-10000 ;
}
void sub_class::ST2_sub_callback(const std_msgs::Int32MultiArray::ConstPtr& msg){
    pub_to_main.servo_state = msg->data[0] ;
    pub_to_main.stepper = msg->data[1] ;
    pub_to_main.hand = msg->data[2] ;    
}
void sub_class::lidarmsg_sub_callback(const lidar_2020::alert_range::ConstPtr& msg){
    pub_to_main.emergency={};
    if(msg->header.seq>2){
        for(int j=0 ;j<8;j++){
            pub_to_main.emergency.push_back(msg->alert[j]);
        }  
    }
}
void sub_class::publish_(float time ){
    pub_to_main.time =time ; 
    status.data = now_status();
    pub_to_main.status = now_status() ;
    if(status.data!=5){
        pub_to_main.emergency={};
        for(int j=0 ;j<8;j++){
            pub_to_main.emergency.push_back(false);
        }  
    }
    agent_pub.publish(pub_to_main);
}

int main(int argc, char **argv){
    ros::init(argc,argv, "agent_new_2");
    sub_class temp;
    ros::Time begin_time ;
    ros::Time now_time ;
    int count = 0 ; 
    float last_clustering_time = 0 ;
    float clustering_time = 0 ; 
    float temp_timer ;

    while(ros::ok()){
        //when status = a ,timer reset 
        switch(temp.now_status()){
            case 4:
                if(count==0){
                    ROS_INFO("status=4");
                    temp.change_status(4);
                    count =1;                    
                }else{
                    if( temp.degree_temp < 10000){
                        temp.change_status(5);
                        count=0;
                    }
                }
                temp_timer++;
                break;

            case 5:
                if(count==0){
                    ROS_INFO("status=5");
                    temp.change_status(5);
                    begin_time =ros::Time::now();
                    count =1;                    
                }else{
                    if(last_clustering_time>100000){
                        temp.change_status(6);
                        count=0;
                    }
                }
                now_time = ros::Time::now();
                clustering_time = (now_time - begin_time).toSec();
                break;

            case 6:

                break;

            case 7:

                break;

            default :
            
                break;
       }

        last_clustering_time=clustering_time;
        temp.publish_(clustering_time);
        temp.status_publish();
        ros::spinOnce();
    }
}


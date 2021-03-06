//20200328 apdate main  
#include "ros/ros.h"
#include "std_msgs/String.h"
#include <std_msgs/Int32MultiArray.h>
#include "main_loop/path.h"
#include "main_loop/agent.h"
#include "main_loop/goap_.h"
#include <main_loop/from_goap.h>
#include "main_loop/main_state.h"
#include <main_loop/goap_debug.h>
#include "main_loop/main_debug.h"
#include "main_loop/world_state.h"
#include <main_loop/cup.h>
#include <main_loop/ns.h>

#include <iostream>
#include <queue>
#include <vector>
#include <cmath>
#include <stdlib.h>

using namespace std;

class state{
public:
    state(){};
    state(int my_x, int my_y, int deg,  bool block, int tx0_, int tx1_, int tx2_){
        my_pos_x = my_x;
        my_pos_y = my_y;
        is_blocked = block;
        tx0 = tx0_;
        tx1 = tx1_;
        tx2 = tx2_;
        degree = deg;
        a_done = false;
        kill_mission = false;
        replan_mission = false;
        replan_path = false;
        is_blocked = block;
        action_wait = false;
    }
    void ChangeActionDone(bool tf){
        a_done = tf;
    }
    void ChangeKillMission(bool tf){
        kill_mission = tf;
    }
    void ChangeReplanMission(bool tf){
        replan_mission = tf;
    }
    void ChangeActionWait(bool tf){
        action_wait = tf;
    }
    void ChangeReplanPath(bool tf){
        replan_path = tf;
    }
    int MyActionDone(){
        return a_done;
    }
    int MyPosX(){
        return my_pos_x;
    }
    int MyPosY(){
        return my_pos_y;
    }
    int MyDegree(){
        return degree;
    }
    bool KillMission(){
        return kill_mission;
    }
    bool ReplanMission(){
        return replan_mission;
    }
    bool IsBlocked(){
        return is_blocked;
    }
    bool ActionWait(){
        return action_wait;
    }
    bool ReplanPath(){
        return replan_path;
    }
    int MyTx0(){
        return tx0;
    }
    int MyTx1(){
        return tx1;
    }
    int MyTx2(){
        return tx2;
    }
    void RefreshActionState(){
        a_done = false;
        kill_mission = false;
        replan_mission = false;
        replan_path = false;
        is_blocked = false;
        action_wait = false;
    }

private:
    bool a_done;
    int my_pos_x;
    int my_pos_y;
    bool kill_mission;
    bool replan_mission;
    bool replan_path;
    bool is_blocked;
    bool action_wait;
    long int tx0;
    long int tx1;
    long int tx2;
    int degree;
};

class action{
public:
    action(int x, int y, int movement_num[], int what_angle, int how_fast, bool is_wait, int what_mode){
        goal_pos_x = x;
        goal_pos_y = y;
        for(int i = 0; i < 15; i ++){
            movement[i] = movement_num[i];
        }
        angle = what_angle;
        speed = how_fast;
        wait = is_wait;
        mode = what_mode;
    }
    action(){};
    int PosX(){
        return goal_pos_x;
    }
    int PosY(){
        return goal_pos_y;
    }
    int * Movement(){
        return  movement;
    }
    int Speed(){
        return speed;
    }
    bool Wait(){
        return wait;
    }
    int Angle(){
        return angle;
    }
    int Mode(){
        return mode;
    }
private:
    int goal_pos_x;
    int goal_pos_y;
    int movement[15];
    int speed;
    int angle;
    bool wait;
    int mode;
};

enum class ActionMode {SPEED_MODE, POSITION_MODE};
enum class Status {SET_STRATEGY, RESET, SET_INITIAL_POS, STARTING_SCRIPT, READY, RUN, STOP, IDLE};
enum class RobotState {AT_POS, ON_THE_WAY, BLOCKED};

class sub_state{
	public:
    	sub_state();
		~sub_state(){};
		void callback(const main_loop::agent::ConstPtr& msg);
        bool lidar_be_blocked(float speed_degree,float car_degree);
		bool emergency[8];
        int movement_from_goap[15];
        main_loop::agent from_agent;
		
	private:
		ros::NodeHandle n ;
		ros::Subscriber Agent_sub ;		 
};

sub_state::sub_state(){
    from_agent.servo_state = -2 ;
    from_agent.stepper = -2 ;
    from_agent.hand = -2 ;

	Agent_sub = n.subscribe<main_loop::agent>("agent_msg", 1, &sub_state::callback,this);
	for(int j=0;j<8;j++){
        emergency[j]=false;
    }
}

void sub_state::callback(const main_loop::agent::ConstPtr& msg){
    from_agent.my_pos_x = msg->my_pos_x ;
    from_agent.my_pos_y = msg->my_pos_y ;
    from_agent.my_degree = msg->my_degree ; 
    from_agent.status = msg->status;
    from_agent.servo_state = msg->servo_state;
    from_agent.stepper = msg -> stepper ;
    from_agent.hand = msg->hand ; 
    from_agent.enemy1_x = msg->enemy1_x ;
    from_agent.enemy1_y = msg->enemy1_y ;
    from_agent.enemy2_x = msg->enemy2_x ;
    from_agent.enemy2_y = msg->enemy2_y ;
    from_agent.time = msg->time ; 

	emergency[0]=msg->emergency[0];
    emergency[1]=msg->emergency[1];
    emergency[2]=msg->emergency[2];
    emergency[3]=msg->emergency[3];
    emergency[4]=msg->emergency[4];
    emergency[5]=msg->emergency[5];
    emergency[6]=msg->emergency[6];
    emergency[7]=msg->emergency[7];
  	
}

bool sub_state::lidar_be_blocked(float speed_degree,float car_degree){
/*
    if(car_degree>speed_degree){
        if(car_degree-speed_degree<90){
            if( emergency[1]==true || emergency[2]==true || emergency[3]==true ){
                return true ;
            }else{
                return false ;
            }
        }else{
            if( emergency[4]==true || emergency[5]==true || emergency[6]==true ){
                return true ;
            }else{
                return false ;
            }            
        }
    }
*/
    bool blockk=false;
    for(int i=0;i<8;i++){
        if(emergency[i]==true){
            blockk=true;
        }
    }
    return blockk;
    
}



bool at_pos(int x, int y, int deg, int c_x, int c_y, int c_deg, int m, int angle_m){
    bool at_p = false;
    int exact_angle_difference=0 ;
    int angle_difference_1=abs(c_deg-deg);
    int angle_difference_2=360-abs(c_deg-deg);
    if(angle_difference_1 > angle_difference_2){
        exact_angle_difference = angle_difference_2;
    }else{
        exact_angle_difference = angle_difference_1;
    }    
    if(abs(x - c_x) < m && abs(y - c_y) < m && exact_angle_difference < angle_m){
        at_p = true;
    }
    return at_p;
}

int main(int argc, char **argv)
{

    //ros setting
 	ros::init (argc, argv, "main_v1");
	ros::NodeHandle nh;    
	
    ros::Publisher pub_st1 = nh.advertise<std_msgs::Int32MultiArray>("txST1", 1);
	ros::Publisher pub_st2 = nh.advertise<std_msgs::Int32MultiArray>("txST2", 1);

    ros::Publisher pub_goap_response = nh.advertise<main_loop::goap_debug>("Goap_response", 1);
    ros::Publisher pub_main_state = nh.advertise<main_loop::main_debug>("Main_state", 1);

	ros::ServiceClient client_path = nh.serviceClient<main_loop::path>("path_plan");
    ros::ServiceClient client_goap = nh.serviceClient<main_loop::goap_>("goap_test_v1");
    
    ros::ServiceClient client_cup = nh.serviceClient<main_loop::cup>("cup");
    ros::ServiceClient client_ns = nh.serviceClient<main_loop::ns>("ns");

    sub_state temp;
	main_loop::path path_srv;
    main_loop::goap_ goap_srv;
    main_loop::world_state little_ws; 
    main_loop::cup srv_cup;
    main_loop::ns srv_ns;

    // give default value here
    long int r0=0x6000;
    long int r1=0;
    long int r2=0;
    long int r3=0;
    long int rx0=0;
    long int rx1=0;
    long int rx2=0;
    long int rx3=0;

    int goal_covered_counter = 0;
    int cover_limit = 20;
    int old_grab_status[12] = {0};
    int distance_square = 0;
    int action_done = false;
    int kill_mission = false;
    int replan_mission = false;
    int margin = 50;
    int speed_mode_margin = 100;
    int angle_margin = 10;
    int speed_mode_angle_margin = 360;
    int switch_mode_distance = 250000;//square
    int left_layer = 0;
    int right_layer = 0;
    ActionMode m;
    RobotState robot;


    //ros defult value
    int now_my_pos_x;
    int now_my_pos_y;
    float last_degree = 0 ;
    float now_degree = 0 ;
    path_srv.request.goal_pos_x = 0;
    path_srv.request.goal_pos_y = 0;
	path_srv.request.my_pos_x = 700 ;
    path_srv.request.my_pos_y = 300 ;
    path_srv.request.enemy1_x = 1800 ;
    path_srv.request.enemy1_y = 2400 ;
    path_srv.request.enemy2_x = 1800 ;
    path_srv.request.enemy2_y = 2300 ;
    path_srv.request.ally_x = 1800 ;
    path_srv.request.ally_y = 2200 ; 
    goap_srv.request.replan=false;
    goap_srv.request.action_done=false;
    goap_srv.request.pos.push_back(700);
    goap_srv.request.pos.push_back(300);
    goap_srv.request.my_degree = 90 ; 
    goap_srv.request.mission_name = "setting" ;
    goap_srv.request.time = 0 ;
    goap_srv.request.direction = true ; 
    goap_srv.request.kill_mission = false ; 
    goap_srv.request.cup_color = 55 ; 
    goap_srv.request.mission_child_name = "" ; 
    little_ws.lighthouse_done = false ; 
    little_ws.flag_done = false ; 
    int cup_suck = 0;
    int ns_suck = 0;
    srv_cup.request.OUO = 0;
    srv_ns.request.OAO = 0;






    int count = 0 ;
    


    while(ros::ok()){
    
       
        
        //calculate time
	    double begin_time =ros::Time::now().toSec();
        //status update
        int s = temp.from_agent.status; 
        Status stat  = static_cast<Status>(s);
        //service parameter update
        //path plan
        path_srv.request.my_pos_x = temp.from_agent.my_pos_x;
        path_srv.request.my_pos_y = temp.from_agent.my_pos_y;
        if(temp.from_agent.enemy1_x!=0){
            path_srv.request.enemy1_x = temp.from_agent.enemy1_x ;
        }
        if(temp.from_agent.enemy1_y!=0){
            path_srv.request.enemy1_y = temp.from_agent.enemy1_y ;
        }
        if(temp.from_agent.enemy2_x!=0){
            path_srv.request.enemy2_x = temp.from_agent.enemy2_x ;
        }
        if(temp.from_agent.enemy2_y!=0){
            path_srv.request.enemy2_y = temp.from_agent.enemy2_y ;
        }
        path_srv.request.ally_x = 300 ;
        path_srv.request.ally_y = 2200 ;          
        //goap
        
        //debug
        main_loop::goap_debug debug_1;
        main_loop::main_debug debug_2;
       

        switch(stat){
            case Status::SET_STRATEGY: //0
                r0 = 0x6000;
                r1 = 0;
                r2 = 0;
                r3 = 0;

                debug_1.desire_degree=0;
                debug_1.desire_speed=0;
                debug_1.desire_mode=r0;
                debug_1.desire_pos={};
                debug_1.desire_servo_state=0;
                debug_1.desire_stepper=0;
                debug_1.desire_hand=0;
                debug_1.is_wait=0;
                debug_1.mission_name= "setting";
                
                break;

            case Status::READY:{
                count ++ ;
                if(count==1){
                    srv_cup.request.OUO==1;
                }
                if(srv_cup.request.OUO==1){
                    if(client_cup.call(srv_cup)){
                        for(int i;i<5;i++){
                            if(srv_cup.response.CupResult[i]!=0 && srv_cup.response.CupResult[i]!=1 ){
                                cup_suck = 1 ;     
                            }else{
                                srv_cup.request.OUO = 2 ;//finish
                            } 
                            ROS_INFO("cup[0]: %ld", (long int)srv_cup.response.CupResult[0]);
                            ROS_INFO("cup[1]: %ld", (long int)srv_cup.response.CupResult[1]);
                            ROS_INFO("cup[2]: %ld", (long int)srv_cup.response.CupResult[2]);
                            ROS_INFO("cup[3]: %ld", (long int)srv_cup.response.CupResult[3]);
                            ROS_INFO("cup[4]: %ld", (long int)srv_cup.response.CupResult[4]);  
                        }
                    }   
                }
                if(cup_suck == 1 ){
                    srv_cup.request.OUO = 3;
                }
                break;
            }

            case Status::RUN:{ //5
                count ++;
                state            
                current_state(temp.from_agent.my_pos_x,temp.from_agent.my_pos_y,temp.from_agent.my_degree,temp.lidar_be_blocked(0,temp.from_agent.my_degree),
                temp.from_agent.servo_state,temp.from_agent.stepper,temp.from_agent.hand);//<--------get undergoing, finish, my_x, my_y, block from other nodes ()********
                state action_state(0,0,0,false,0,0,0);
                action_state = current_state;
                if(action_done){
                    action_state.ChangeActionDone(true);
                    action_done = false;
                    ROS_INFO ("rx0:%ld ", rx0);                
                    ROS_INFO ("rx1:%ld ", rx1);
                    ROS_INFO ("rx2:%ld ", rx2);
                    ROS_INFO ("mission: %s ", goap_srv.response.mission_name.c_str());

                    if(goap_srv.request.mission_name == "lighthouse" && goap_srv.request.mission_child_name == "goto" ){
                        little_ws.lighthouse_done = true ; 
                    }
                    if(goap_srv.request.mission_name == "hand_push" && goap_srv.request.mission_child_name == "windsock_hand_down"){
                        little_ws.flag_done = true ; 
                    }
                
                }
                if(kill_mission){
                    action_state.ChangeKillMission(true);
                    kill_mission = false;
                }
                goap_srv.request.replan=false;
                goap_srv.request.action_done=action_state.MyActionDone();
                goap_srv.request.pos.push_back(action_state.MyPosX());
                goap_srv.request.pos.push_back(action_state.MyPosY()); 
            
                //----------goap
                if(client_goap.call(goap_srv)){
                    
                    temp.movement_from_goap[0]=goap_srv.response.ST2[0];
                    temp.movement_from_goap[1]=goap_srv.response.ST2[1];
                    temp.movement_from_goap[2]=goap_srv.response.ST2[2];
                    temp.movement_from_goap[3]=goap_srv.response.ST2[3];
                    temp.movement_from_goap[4]=goap_srv.response.ST2[4];
                    temp.movement_from_goap[5]=goap_srv.response.ST2[5];
                    temp.movement_from_goap[6]=goap_srv.response.ST2[6];
                    temp.movement_from_goap[7]=goap_srv.response.ST2[7];
                    temp.movement_from_goap[8]=goap_srv.response.ST2[8];
                    temp.movement_from_goap[9]=goap_srv.response.ST2[9];
                    temp.movement_from_goap[10]=goap_srv.response.ST2[10];
                    temp.movement_from_goap[11]=goap_srv.response.ST2[11];
                    temp.movement_from_goap[12]=goap_srv.response.ST2[12];
                    temp.movement_from_goap[13]=goap_srv.response.ST2[13];
                    temp.movement_from_goap[14]=goap_srv.response.ST2[14];
                    temp.movement_from_goap[15]=goap_srv.response.ST2[15];
                    //for path plan
                    path_srv.request.goal_pos_x = goap_srv.response.pos[0];
                    path_srv.request.goal_pos_y = goap_srv.response.pos[1];
                }else{
                    ROS_ERROR("Failed to call goap_test");
                }
                //
                goap_srv.request.mission_name = goap_srv.response.mission_name ;
                goap_srv.request.mission_child_name = goap_srv.response.mission_child_name ;
                //-------goap end               
                action act(goap_srv.response.pos[0],goap_srv.response.pos[1],temp.movement_from_goap,goap_srv.response.degree,goap_srv.response.speed,goap_srv.response.is_wait,goap_srv.response.mode);
                int desire_pos_x = act.PosX();
                int desire_pos_y = act.PosY();
                int*desire_movement;
                int desire_speed = act.Speed();
                int desire_mode = act.Mode();
                bool desire_wait = act.Wait();
                int desire_angle = act.Angle();
                desire_movement = act.Movement();
                                    
                if(desire_mode == 2){
                    m = ActionMode::SPEED_MODE;
                }
                else{
                    m = ActionMode::POSITION_MODE;
                }
                debug_2.action_done=action_state.MyActionDone();
                            
                switch(m){
                    case ActionMode::POSITION_MODE:{
                        debug_2.robot_state="ActionMode::POSITION_MODE";

                        if(current_state.IsBlocked()){
                            robot = RobotState::BLOCKED;
                        }
                        else{
                            if(at_pos(current_state.MyPosX(),current_state.MyPosY(),current_state.MyDegree(), desire_pos_x, desire_pos_y, desire_angle, margin, angle_margin)){
                                robot = RobotState::AT_POS;
								r0 = 0x4000;
								r1 = desire_pos_x;
								r2 = desire_pos_y;
								r3 = desire_angle;
                            }
                            else{
                                robot = RobotState::ON_THE_WAY;
                            }
                        }
                        
                        switch(robot){
                            case RobotState::AT_POS:{
                                debug_2.robot_case="AT_POS";
                                //rx0
                                long int out = 0;
                                for(int i = 0; i < 12; i ++){
                                    if(desire_movement[i] != -1){
                                        old_grab_status[i] = desire_movement[i];
                                    }
                                }
                                for(int i = 11; i >= 0; i --){
                                    out = out << 2;
                                    out += old_grab_status[i];
                                }
                                rx0 = out;
                                //rx1
                                if(desire_movement[12]!=-1){
                                    left_layer = desire_movement[12];
                                }
                                if(desire_movement[13]!=-1){
                                    right_layer = desire_movement[13];
                                }
                           
                                rx1 = (left_layer << 2) + (right_layer << 6);
                                //rx2
                                if(desire_movement[14]!=-1){
                                    rx2 = desire_movement[14];
                                }
                                //rx3 
                                if(desire_movement[15]!=-1){
                                    rx3 = desire_movement[15];
                                }
                                break;}

                            case RobotState::BLOCKED:
                                debug_2.robot_case="BLOCKED";
                                //return stop; //<-------------tell STM to stop
                                r0 = 0x5000;
                                r1 = 0;
                                r2 = 0;
                                r3 = 0;
                                action_state.ChangeReplanMission(true);   //---------------> GOAP replan == True
                                break;

                            case RobotState::ON_THE_WAY:
                                debug_2.robot_case="ON_THE_WAY";
                                //---------path plan
								
                                distance_square = (current_state.MyPosX() - desire_pos_x)*(current_state.MyPosX() - desire_pos_x) + (current_state.MyPosY() - desire_pos_y)*(current_state.MyPosY() - desire_pos_y);
                                if(distance_square <= switch_mode_distance){
                                    r0 = 0x4000;
                                    r1 = desire_pos_x;
                                    r2 = desire_pos_y;
                                    r3 = desire_angle;
                                    //return pos_mode; //<----------------
                                }
                                else{
									if(client_path.call(path_srv)){
										double clustering_time = ros::Time::now().toSec () - begin_time; 
										now_degree = path_srv.response.degree ; 
									}else{
										ROS_ERROR("Failed to call service path plan");
										ROS_INFO ("position mode");
										ROS_INFO ("desire_pos_x:%d ", desire_pos_x);                
										ROS_INFO ("desire_pos_y:%d ", desire_pos_y);
										ROS_INFO ("my_pos_x:%d ", temp.from_agent.my_pos_x);
										ROS_INFO ("my_pos_y:%d ", temp.from_agent.my_pos_y);
									}
									if(now_degree<0){
										now_degree = last_degree;
									} 
									//-----path plan end 
                                    r0 = 0x3000;
                                    r1 = desire_speed;
                                    r2 = now_degree;
                                    r3 = 0;
                                    if( path_srv.response.blocked == true){ 
                                        goal_covered_counter ++;
                                    }
                                    if( path_srv.response.blocked == false){ 
                                        goal_covered_counter = 0;
                                    }
                                    if(goal_covered_counter > cover_limit){
                                        action_state.ChangeKillMission(true);
                                        goal_covered_counter = 0;
                                    }
                                    //return speed_mode;//<-----------------
                                }
                                break;
							}
							bool c;
							if(rx1==current_state.MyTx1()){
								c = true;
							}
							else{
								c = false;
							}               
							bool b;
							if(rx0==current_state.MyTx0()){
								b = true;
							}
							else{
								b = false;
							}
							bool d;
							if(rx2==current_state.MyTx2()){
								d = true;
							}
							else{
								d = false;
							}
							if(count>3 && at_pos(current_state.MyPosX(),current_state.MyPosY(),current_state.MyDegree(), desire_pos_x, desire_pos_y, desire_angle, margin, angle_margin) && b && c && d){
								ROS_INFO ("rx0:%ld ", rx0);                
								ROS_INFO ("rx1:%ld ", rx1);
								ROS_INFO ("rx2:%ld ", rx2);
								ROS_INFO("complete:%d",count);
								ROS_INFO ("debug_2.robot_case: %s ", debug_2.robot_case.c_str());
								ROS_INFO ("mission: %s ", goap_srv.response.mission_name.c_str());
								ROS_INFO("action done in action state: %d" , action_state.MyActionDone());
								action_done = true;
								goal_covered_counter = 0;
								count = 0; 
							}
							break;
						}


                    case ActionMode::SPEED_MODE:{
                        debug_2.robot_state="ActionMode::POSITION_MODE";
                        debug_2.robot_case="ON_THE_WAY";
                        

                        //==給任務及位置==
                        //基本上手跟平台都不需要給新指令，可以直接刪rx1,rx2
                        //rx0 <- desire movement
                        long int out = 0;
                        for(int i = 0; i < 12; i ++){
                            if(desire_movement[i] == 2){//自動夾給２
                                old_grab_status[i] = 2;
                            }
                         
                        }
                        for(int i = 11; i >= 0; i --){
                            out = out << 2;
                            out += old_grab_status[i];
                        }
                        rx0 = out;
                  
                        //==任務完成轉換器===
                        //這邊需要稍微調整動作的完成，因為自動夾的指令是２但完成後會變１吧？
                        long int finished_out = 0;
                        int copy_old_status[12] = {0};
                        for(int i = 0; i < 12; i ++){
                            copy_old_status[i] = old_grab_status[i];
                        }
                        for(int i = 0; i < 12; i ++){
                            if(desire_movement[i] == 2){//自動夾給２
                                copy_old_status[i] = 1;//轉換成已完成時會變成的１
                            }
                        }
                        for(int i = 11; i >= 0; i --){
                            finished_out = finished_out << 2;
                            finished_out += copy_old_status[i];
                        }

                        //==任務完成==
                        //若自動夾完成則action_done
                        if(count>3 &&current_state.MyTx0()== finished_out){
                            action_done = true;
                            ROS_INFO("speed mode mission action done");
                            //把old_grab_status中有２的換成１（自動夾完成）
                            for(int i = 0; i < 12; i ++){
                            if(old_grab_status[i] == 2){
                                    old_grab_status[i] = 1;
                                }
                            }
                            count = 0;
                        }

                        //==到點==
                        //若到點則代表沒完成任務（任務已被他人完成或失敗）
                        if(count>3 && at_pos(current_state.MyPosX(),current_state.MyPosY(), current_state.MyDegree(), desire_pos_x, desire_pos_y, desire_angle, speed_mode_margin, speed_mode_angle_margin)){
                            action_state.ChangeKillMission(true);
                            action_done = true;
                            ROS_INFO("speed mode pos action done");
                            //把old_grab_status中有２的換回０（自動夾取消）
                            for(int i = 0; i < 12; i ++){
                                if(desire_movement[i] == 2){
                                    old_grab_status[i] = 0;
                                }
                            }
                            count = 0; 
                        }
                        //==未到點==
                        //不然算路徑給速度角度
                        else{
                            distance_square = (current_state.MyPosX() - desire_pos_x)*(current_state.MyPosX() - desire_pos_x) + (current_state.MyPosY() - desire_pos_y)*(current_state.MyPosY() - desire_pos_y);
                            
                            if(distance_square <= switch_mode_distance){
                                r0 = 0x4000;
                                r1 = desire_pos_x;
                                r2 = desire_pos_y;
                                r3 = desire_angle;
                                //return pos_mode; //<----------------
                            }else{
								if(client_path.call(path_srv)){
									double clustering_time = ros::Time::now().toSec () - begin_time; 
									now_degree = path_srv.response.degree ; 
								}else{
									ROS_ERROR("Failed to call service path plan");
									ROS_INFO ("speed mode");
									ROS_INFO ("desire_pos_x:%d ", desire_pos_x);                
									ROS_INFO ("desire_pos_y:%d ", desire_pos_y);
									ROS_INFO ("my_pos_x:%d ", temp.from_agent.my_pos_x);
									ROS_INFO ("my_pos_y:%d ", temp.from_agent.my_pos_y);
								}
								if(now_degree<=0){
									now_degree = last_degree;
								} 
								//-----path plan end 
								r0 = 0x3000;
								r1 = desire_speed;
								r2 = now_degree;
								r3 = 0;
								if( path_srv.response.blocked == true){ 
									goal_covered_counter ++;
								}
								if( path_srv.response.blocked == false){ 
									goal_covered_counter = 0;
								}
								//return speed_mode;//<-----------------
								if(goal_covered_counter > cover_limit){
									action_state.ChangeKillMission(true);
									goal_covered_counter = 0;
									//把old_grab_status中有２的換回０（自動夾取消）
									for(int i = 0; i < 12; i ++){
										if(desire_movement[i] == 2){
											old_grab_status[i] = 0;
										}
									}
								}
							}
                            
                        }
                        //==被阻擋則停車==
                        if(current_state.IsBlocked()){
                        ROS_INFO("speed mode block");
                            r0 = 0x5000;
                            r1 = 0;
                            r2 = 0;
                            r3 = 0;
                            action_state.ChangeReplanMission(true);
                        }
                        break;
                    }


                }
                debug_1.desire_degree=desire_angle;
                debug_1.desire_speed=desire_speed;
                debug_1.desire_mode=r0;
                debug_1.desire_pos.push_back(desire_pos_x);  
                debug_1.desire_pos.push_back(desire_pos_y); 
                debug_1.desire_servo_state=rx0;
                debug_1.desire_stepper=rx1;
                debug_1.desire_hand=rx2;
                debug_1.is_wait=desire_wait;
                debug_1.mission_name = goap_srv.response.mission_name;
                debug_1.mission_child_name = goap_srv.response.mission_child_name;


                 
                break;
                }
            case Status::STOP: //6
                r0 = 0x5000;
                r1 = 0;
                r2 = 0;
                r3 = 0;
                debug_1.desire_degree=0;
                debug_1.desire_speed=0;
                debug_1.desire_mode=r0;
                debug_1.desire_pos={};
                debug_1.desire_servo_state=0;
                debug_1.desire_stepper=0;
                debug_1.desire_hand=0;
                debug_1.is_wait=0;
                debug_1.mission_name="NO";
                break;
        }
        //give ST
        std_msgs::Int32MultiArray for_st1 ;
        for_st1.data.push_back(r0);
        for_st1.data.push_back(r1);
        for_st1.data.push_back(r2);
        for_st1.data.push_back(r3);
        std_msgs::Int32MultiArray for_st2 ;
        for_st2.data.push_back(rx0);
        for_st2.data.push_back(rx1);
        for_st2.data.push_back(rx2);
        for_st2.data.push_back(rx3);

        pub_st1.publish(for_st1);
        pub_st2.publish(for_st2);

        //debug

        debug_2.status=temp.from_agent.status;
        debug_2.pos.push_back(temp.from_agent.my_pos_x);
        debug_2.pos.push_back(temp.from_agent.my_pos_y);
        debug_2.enemy1_x=path_srv.request.enemy1_x;
        debug_2.enemy1_y=path_srv.request.enemy1_y;
        debug_2.enemy2_x=path_srv.request.enemy2_x;
        debug_2.enemy2_y=path_srv.request.enemy2_y;
        debug_2.is_blocked=temp.lidar_be_blocked(0,temp.from_agent.my_degree);
        debug_2.servo_state=temp.from_agent.servo_state;
        debug_2.stepper_state=temp.from_agent.stepper;
        debug_2.hand_state=temp.from_agent.hand;
        debug_2.kill_mission=kill_mission;
        debug_2.goal_covered_counter=goal_covered_counter;
        debug_2.time =temp.from_agent.time;
        pub_goap_response.publish(debug_1);
        pub_main_state.publish(debug_2);


        last_degree = now_degree ; 
        ros::spinOnce();
    }
    return 0;
    
}


//20200328 apdate main  
#include "ros/ros.h"
#include "std_msgs/String.h"
#include <std_msgs/Int32MultiArray.h>
#include "main_loop/path.h"
#include "main_loop/agent.h"
#include "main_loop/goap_.h"
#include <main_loop/from_goap.h>
#include "main_loop/main_state.h"

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
        is_blocked = false;
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
	Agent_sub = n.subscribe<main_loop::agent>("agent_msg", 1, &sub_state::callback,this);
	for(int j=0;j<8;j++){
        emergency[j]=true;
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
}



bool at_pos(int x, int y, int deg, int c_x, int c_y, int c_deg, int m, int angle_m){
    bool at_p = false;
    if(abs(x - c_x) < m && abs(y - c_y) < m && abs(deg - c_deg) < angle_m){
        at_p = true;
    }
    return at_p;
}

int main(int argc, char **argv)
{

    //ros setting
 	ros::init (argc, argv, "main_demo_v2");
	ros::NodeHandle nh;    
	
    ros::Publisher pub_st1 = nh.advertise<std_msgs::Int32MultiArray>("txST1", 1);
	ros::Publisher pub_st2 = nh.advertise<std_msgs::Int32MultiArray>("txST2", 1);

    ros::Publisher pub_goap_response = nh.advertise<main_loop::from_goap>("Goap_response", 1);
    ros::Publisher pub_main_state = nh.advertise<main_loop::main_state>("Main_state", 1);

	ros::ServiceClient client_path = nh.serviceClient<main_loop::path>("path_plan");
    ros::ServiceClient client_goap = nh.serviceClient<main_loop::goap_>("goap_test_v1");
    sub_state temp;
	main_loop::path path_srv;
    main_loop::goap_ goap_srv;

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
    int margin = 30;
    int angle_margin = 10;
    int switch_mode_distance = 4000000;//square
    int left_layer = 0;
    int right_layer = 0;
    ActionMode m;
    RobotState robot;


    //ros defult value
    int now_my_pos_x;
    int now_my_pos_y;
    float last_degree = 0 ;
    float now_degree = 0 ;
    path_srv.request.goal_pos_x = 1600;
    path_srv.request.goal_pos_y = 1800;
	path_srv.request.my_pos_x = 700 ;
    path_srv.request.my_pos_y = 300 ;
    path_srv.request.enemy1_x = 380 ;
    path_srv.request.enemy1_y = 2400 ;
    path_srv.request.enemy2_x = 380 ;
    path_srv.request.enemy2_y = 2300 ;
    path_srv.request.ally_x = 380 ;
    path_srv.request.ally_y = 2200 ; 
    goap_srv.request.replan=false;
    goap_srv.request.action_done=false;
    goap_srv.request.pos.push_back(300);
    goap_srv.request.pos.push_back(300);
    goap_srv.request.my_degree = 90 ; 
    goap_srv.request.mission_name = "setting" ;
    
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
        path_srv.request.enemy1_x = 380 ;
        path_srv.request.enemy1_y = 2400 ;
        path_srv.request.enemy2_x = 380 ;
        path_srv.request.enemy2_y = 2300 ;
        path_srv.request.ally_x = 380 ;
        path_srv.request.ally_y = 2200 ; 
        //goap
        
        //debug
        main_loop::from_goap debug_1;
        main_loop::main_state debug_2;
       



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
            case Status::RUN:{ //5
                state current_state(temp.from_agent.my_pos_x,temp.from_agent.my_pos_y,temp.from_agent.my_degree,false,temp.from_agent.servo_state,temp.from_agent.stepper,temp.from_agent.hand);//<--------get undergoing, finish, my_x, my_y, block from other nodes ()********
                state action_state(0,0,0,false,0,0,0);
                action_state = current_state;
                if(action_done){
                    action_state.ChangeActionDone(true);
                    action_done = false;
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
                    //for path plan
                    path_srv.request.goal_pos_x = goap_srv.response.pos[0];
                    path_srv.request.goal_pos_y = goap_srv.response.pos[1];
                    /*ROS_INFO("goap_srv.response.ST2[0]: %ld", (long int)goap_srv.response.ST2[0]);
                    ROS_INFO("goap_srv.response.ST2[1]: %ld", (long int)goap_srv.response.ST2[1]);
                    ROS_INFO("goap_srv.response.ST2[2]: %ld", (long int)goap_srv.response.ST2[2]);
                    ROS_INFO("goap_srv.response.ST2[3]: %ld", (long int)goap_srv.response.ST2[3]);
                    ROS_INFO("goap_srv.response.ST2[4]: %ld", (long int)goap_srv.response.ST2[4]);
                    ROS_INFO("goap_srv.response.ST2[5]: %ld", (long int)goap_srv.response.ST2[5]);
                    ROS_INFO("goap_srv.response.ST2[6]: %ld", (long int)goap_srv.response.ST2[6]);
                    ROS_INFO("goap_srv.response.ST2[7]: %ld", (long int)goap_srv.response.ST2[7]);
                    ROS_INFO("goap_srv.response.ST2[8]: %ld", (long int)goap_srv.response.ST2[8]);
                    ROS_INFO("goap_srv.response.ST2[9]: %ld", (long int)goap_srv.response.ST2[9]);
                    ROS_INFO("goap_srv.response.ST2[10]: %ld", (long int)goap_srv.response.ST2[10]);
                    ROS_INFO("goap_srv.response.ST2[11]: %ld", (long int)goap_srv.response.ST2[11]);
                    ROS_INFO("goap_srv.response.ST2[12]: %ld", (long int)goap_srv.response.ST2[12]);
                    ROS_INFO("goap_srv.response.ST2[13]: %ld", (long int)goap_srv.response.ST2[13]);
                    ROS_INFO("goap_srv.response.ST2[14]: %ld", (long int)goap_srv.response.ST2[14]);
                    */
                }else{
                    ROS_ERROR("Failed to call goap_test");
                }
                //
                goap_srv.request.mission_name = goap_srv.response.mission_name ;
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
                

                
                if(at_pos(current_state.MyPosX(),current_state.MyPosY(),current_state.MyDegree(), desire_pos_x, desire_pos_y, desire_angle, margin, angle_margin) &&  ((rx0==current_state.MyTx0()) && (rx1 == current_state.MyTx1()) && (rx2 == current_state.MyTx2()))){
                ROS_INFO ("rx0:%ld ", rx0);                
                ROS_INFO ("rx1:%ld ", rx1);
                ROS_INFO ("rx2:%ld ", rx2);
                bool a = at_pos(current_state.MyPosX(),current_state.MyPosY(),current_state.MyDegree(), desire_pos_x, desire_pos_y, desire_angle, margin, angle_margin);
                ROS_INFO("a = %d",a);
                bool b;
                if(rx0==current_state.MyTx0()){
                    b = true;
                }
                else{
                    b = false;
                }
                ROS_INFO("b = %d",b);
                bool c;
                if(rx1==current_state.MyTx1()){
                    c = true;
                }
                else{
                    c = false;
                }
                ROS_INFO("c = %d",c);
                bool d;
                if(rx2==current_state.MyTx2()){
                    d = true;
                }
                else{
                    d = false;
                }
                ROS_INFO("d = %d",d);
                bool e;
                if(abs(current_state.MyPosX() - desire_pos_x) < margin){
                    e = true;
                }
                else{
                    e = false;
                }
                ROS_INFO("e = %d",e);
                bool f;
                if(abs(current_state.MyPosY() - desire_pos_y) < margin){
                    f = true;
                }
                else{
                    f = false;
                }
                ROS_INFO("f = %d",f);
                bool g;
                ROS_INFO("degree = %d",current_state.MyDegree());
                if(abs(current_state.MyDegree() - desire_angle) < angle_margin){
                    g = true;
                }
                else{
                    g = false;
                }
                ROS_INFO("g = %d",g);
                ROS_INFO ("mission: %s ", goap_srv.response.mission_name.c_str());
                count ++ ;
                ROS_INFO("complete:%d",count);
                    action_done = true;
                    goal_covered_counter = 0;
                }
                if(desire_mode == 1){
                    m = ActionMode::POSITION_MODE;
                }
                else{
                    m = ActionMode::SPEED_MODE;
             
                }
                debug_2.action_done=action_state.MyActionDone();
                
                switch(m){
                    case ActionMode::POSITION_MODE:
                        debug_2.robot_state="ActionMode::POSITION_MODE";
                        if(at_pos(current_state.MyPosX(),current_state.MyPosY(),current_state.MyDegree(), desire_pos_x, desire_pos_y, desire_angle, margin, angle_margin)){
                            robot = RobotState::AT_POS;
                        }
                        else{
                            if(current_state.IsBlocked()){
                                robot = RobotState::BLOCKED;
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
                                if(client_path.call(path_srv)){
                                    double clustering_time = ros::Time::now().toSec () - begin_time; 
                                    now_degree = path_srv.response.degree ; 
                                }else{
                                    ROS_ERROR("Failed to call service path plan");
                                }
                                if(now_degree<0){
                                    now_degree = last_degree;
                                } 
                                //-----path plan end 
                                distance_square = (current_state.MyPosX() - desire_pos_x)*(current_state.MyPosX() - desire_pos_x) + (current_state.MyPosY() - desire_pos_y)*(current_state.MyPosY() - desire_pos_y);
                                if(distance_square < switch_mode_distance){
                                    r0 = 0x4000;
                                    r1 = desire_pos_x;
                                    r2 = desire_pos_y;
                                    r3 = desire_angle;
                                    //return pos_mode; //<----------------
                                }
                                else{
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
                        break;

                    case ActionMode::SPEED_MODE:{
                        debug_2.robot_state="ActionMode::SPEED_MODE";
                        long int out=0;
                        rx0 = desire_movement[12];
                        rx1 = desire_movement[13];
                        for(int i = 0; i < 12; i ++){
                            if(desire_movement[i] == 1){
                                old_grab_status[i] = 2;
                            }
                            else if(desire_movement[i] == 0){
                                old_grab_status[i] = 0;
                            }
                        }
                        for(int i = 0; i < 12; i ++){
                            out += old_grab_status[i];
                            out = out << 2;
                        }
                        out = out << 6;
                        rx2 = out;
                        rx3 = desire_movement[14];
                        if(current_state.MyPosX() != desire_pos_x && current_state.MyPosY() != desire_pos_y){
                           if(current_state.MyActionDone() == 1){ //if not at pos and hasn't got cup, keep going
                                //---------path plan
                                if(client_path.call(path_srv)){
                                    double clustering_time = ros::Time::now().toSec () - begin_time; 
                                    now_degree = path_srv.response.degree ; 
                                }else{
                                    ROS_ERROR("Failed to call service path plan");
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
                                    //action_state.ChangeKillMission(true);
                                    action_state.ChangeActionDone(true); //should kill mission but fuck that.. this should still work
                                    goal_covered_counter = 0;
                                    for(int i = 0; i < 12; i ++){
                                       if(desire_movement[i] == 2){
                                            old_grab_status[i] = 0;
                                        }
                                    }
                            	} //if get cup, action_done is then true(MyActionStatus()==2), GOAP will automatically give next action
                           }
                            else{
                                for(int i = 0; i < 12; i ++){
                                       if(desire_movement[i] == 2){
                                            old_grab_status[i] = 1;
                                        }
                                    }
                            }
                        }
                        else{ //at pos, mission has failed..
                            //action_state.ChangeKillMission(true);
                            action_state.ChangeActionDone(true); //should kill mission but fuck that.. this should still work
                            for(int i = 0; i < 12; i ++){
                               if(desire_movement[i] == 2){
                                    old_grab_status[i] = 0;
                                }
                            }
                        }
                        break;}
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
                debug_1.mission_name = goap_srv.response.mission_name ; 
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
        /*
        debug_2.robot_state=0;
        debug_2.robot_case=0;
        */
        debug_2.pos.push_back(temp.from_agent.my_pos_x);
        debug_2.pos.push_back(temp.from_agent.my_pos_y);
        debug_2.is_blocked=false;
        debug_2.servo_state=temp.from_agent.servo_state;
        debug_2.stepper_state=temp.from_agent.stepper;
        debug_2.hand_state=temp.from_agent.hand;
        debug_2.kill_mission=kill_mission;
        debug_2.goal_covered_counter=goal_covered_counter;
        pub_goap_response.publish(debug_1);
        pub_main_state.publish(debug_2);


        last_degree = now_degree ; 
        ros::spinOnce();
    }
    return 0;
}
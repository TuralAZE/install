#include "final_turtlebot_key/final_key.h"
#include "ros/ros.h"
#include "geometry_msgs/Twist.h"
#include "std_msgs/String.h"



int main(int argc, char** argv)
{
    
  const char* msg = R"(
Reading from the keyboard and Publishing to Twist!
------------------------------------------------
w/x : increase/decrease only linear speed by 10%
a/d : increase/decrease only angular speed by 10%
f to quit
)";
  float speed(0.0); // Linear velocity (m/s)
  float turn(0.0); // Angular velocity (rad/s)
   
  ros::init(argc, argv, "turtlebotf_keyboard");
  ros::NodeHandle nh;float x(1), y(1), z(0), th(1);
  ros::Publisher pub = nh.advertise<geometry_msgs::Twist>("cmd_vel", 1);
  geometry_msgs::Twist twist;
  terminal_init();
  set_terminal_raw_mode();
  
  
  printf("%s", msg);
  bool isContinue = true;
  while(isContinue) {
    

     while(true) {
       if (wait_for_key_pressed(100)) break;
       
     }
     
     char c = cin.get();
     switch(c) {
       case 'w':
       if((speed+0.01) > -0.26 && (speed + 0.01) < 0.26)
        {speed = speed + 0.01;}
        
        break;

      case 'x':
        if((speed - 0.01) > -0.26 && (speed - 0.01) < 0.26)
        {speed = speed - 0.01;}
        break;

      case 'a':
        
        if ((turn + 0.05) > -1.82 && (turn + 0.05) < 1.82){
        turn = turn + 0.05;}
        break;

      case 'd':
        if ((turn - 0.05) > -1.82 && (turn - 0.05) < 1.82){
        turn = turn - 0.05;}
        break;
      case 's':
        turn =0;
        speed =0;
        break;
       case 'f':
        isContinue = false;
        break;
       
     }
    printf("\rLast command: %c   ", c  );

    twist.linear.x = speed;
    cout << "\nLinear speed: " << twist.linear.x << endl;

    twist.linear.y = speed;
    
    twist.linear.z = 0;
    
    twist.angular.x = 0;
    twist.angular.y = 0;
    twist.angular.z = turn;
    cout << "Angular speed: " << twist.angular.z << endl;
    cout << "Click F to quit." << endl;
    pub.publish(twist);
    ros::spinOnce();
   }
   
  set_terminal_original_mode();
}
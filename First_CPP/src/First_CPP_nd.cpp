#include <cstdio>
#include <random>
#include <string.h>
#include <chrono>
#include <functional>
#include <memory>
#include <string>

#include <moveit/move_group_interface/move_group_interface.h>
using moveit::planning_interface::MoveGroupInterface;


#include <geometry_msgs/msg/pose_stamped.hpp>



#include "rclcpp/rclcpp.hpp"
#include "visualization_msgs/msg/marker.hpp"

using namespace std::chrono_literals;


class MarkerPublisher : public rclcpp::Node
{
  public:
    MarkerPublisher()
    : Node("marker_publisher")
    {


      marker_pub_= this->create_publisher<visualization_msgs::msg::Marker>("markerPA", 10);



      RCLCPP_INFO(this->get_logger(), "Node has been initialized");
      timer_ = this->create_wall_timer(10000ms, std::bind(&MarkerPublisher::publish_marker, this));
      
    }

  private:
    void publish_marker()
    {
      //--RNG--
      std::random_device rd1;
      std::default_random_engine generator1(rd1()); // rd() provides a random seed
      std::uniform_real_distribution<double> distribution1(-0.25,0.25);

      double x_random = distribution1(generator1);


      std::random_device rd2;
      std::default_random_engine generator2(rd2()); // rd() provides a random seed
      std::uniform_real_distribution<double> distribution2(-0.25,0.25);

      double y_random = distribution2(generator2);


      std::random_device rd3;
      std::default_random_engine generator3(rd3()); // rd() provides a random seed
      std::uniform_real_distribution<double> distribution3(-0.25,0.25);

      double z_random = distribution3(generator3);


      visualization_msgs::msg::Marker marker;
      //não entendi
      marker.header.frame_id = "world";    
      //define das formas basicas, origem 0
      marker.ns = "basic_shapes";
      marker.id = 0;     
      //define a forma como esfera
      marker.type = visualization_msgs::msg::Marker::SPHERE;     
      //add marker
      marker.action = visualization_msgs::msg::Marker::ADD;     
      //posiciona o marker na origem
      marker.pose.position.x = x_random;
      marker.pose.position.y = y_random;
      marker.pose.position.z = z_random;

      marker.pose.orientation.x = 0.0;
      marker.pose.orientation.y = 0.0;
      marker.pose.orientation.z = 0.0;
      marker.pose.orientation.w = 1.0;     
      //tamanho e cor
      marker.scale.x = 0.15;
      marker.scale.y = 0.15;
      marker.scale.z = 0.15;
        
      marker.color.r = 0.0f;
      marker.color.g = 1.0f;
      marker.color.b = 0.0f;
      marker.color.a = 1.0;    
      //tempo de vida do marker
      marker.lifetime = rclcpp::Duration::from_seconds(20);     
      //pub do marker
      marker_pub_->publish(marker);
      RCLCPP_INFO(this->get_logger(), "Marker published");
      

    }
    rclcpp::Publisher<visualization_msgs::msg::Marker>::SharedPtr marker_pub_;
    rclcpp::TimerBase::SharedPtr timer_;
    };


class ArmPublisher
: public rclcpp::Node
{
  public:
    ArmPublisher()
    : Node("arm_publisher")
    {
      arm_pub_= this->create_publisher<geometry_msgs::msg::Pose>("armPA", 10);
      arm_pub_test_= this->create_publisher<visualization_msgs::msg::Marker>("armPubTest", 10);
      arm_goal_= this->create_subscription<visualization_msgs::msg::Marker>("markerPA", 10, std::bind(&ArmPublisher::MoveArm, this, std::placeholders::_1));
      RCLCPP_INFO(this->get_logger(), "Arm Node initialized");
    }

  private:
    void MoveArm(const visualization_msgs::msg::Marker::SharedPtr goalmsg)
    {

      auto move_group_interface = MoveGroupInterface(shared_from_this(), "manipulator");
      
      visualization_msgs::msg::Marker goal = *goalmsg;
      
      arm_pub_test_->publish(goal);
      
      auto const target_pose= [&goal] {
      geometry_msgs::msg::Pose msg;
        msg.orientation.w = 1.0;
        msg.position.x =  0.5;  //goal.pose.position.x;
        msg.position.y =  0.5;  //goal.pose.position.y;
        msg.position.z =  0.5;  //goal.pose.position.z;
        return msg;
     }();
    move_group_interface.setPoseTarget(target_pose);

    // Create a plan to that target pose
    auto const [success, plan] = [&move_group_interface]{
      moveit::planning_interface::MoveGroupInterface::Plan msg;
      auto const ok = static_cast<bool>(move_group_interface.plan(msg));
      return std::make_pair(ok, msg);
    }();

    // Execute the plan
    if(success) {
      move_group_interface.execute(plan);
    } else {
      RCLCPP_INFO(this->get_logger(), "Planning failed!");
    }



    };
    rclcpp::Publisher<geometry_msgs::msg::Pose>::SharedPtr arm_pub_;
    rclcpp::Publisher<visualization_msgs::msg::Marker>::SharedPtr arm_pub_test_;
    rclcpp::Subscription<visualization_msgs::msg::Marker>::SharedPtr arm_goal_;
  };

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);

  std::shared_ptr<MarkerPublisher> marker_node = std::make_shared<MarkerPublisher>();
  std::shared_ptr<ArmPublisher> arm_node = std::make_shared<ArmPublisher>();



  rclcpp::executors::MultiThreadedExecutor executor;
  executor.add_node(marker_node);
  executor.add_node(arm_node);
  // The spin() call is blocking and will process all callbacks until shutdown is called
  executor.spin();

  // rclcpp::spin(std::make_shared<Maininit>());
  rclcpp::shutdown();
  return 0;

}

#include <cmath>
#include <functional>
#include <memory>
#include <chrono>
#include <algorithm>


#include "geometry_msgs/msg/twist.hpp"
#include "rclcpp/rclcpp.hpp"
#include "turtlesim/msg/pose.hpp"
#include "turtlesim/srv/set_pen.hpp"

using std::placeholders::_1;
using namespace std::chrono_literals;

class AmperaDraw : public rclcpp::Node
{
public:
  AmperaDraw()
  : Node("ampera_draw")
  {
    cmd_pub_ = this->create_publisher<geometry_msgs::msg::Twist>("/turtle1/cmd_vel", 10);
    pose_sub_ = this->create_subscription<turtlesim::msg::Pose>(
      "/turtle1/pose", 10, std::bind(&AmperaDraw::pose_callback, this, _1));
    pen_client_ = this->create_client<turtlesim::srv::SetPen>("/turtle1/set_pen");
    timer_ = this->create_wall_timer(20ms, std::bind(&AmperaDraw::timer_callback, this));
  }

private:
  void pose_callback(const turtlesim::msg::Pose::SharedPtr msg)
  {
    pose_ = *msg;
    ada_pose_ = true;
  }

  void timer_callback()
  {
    if (!ada_pose_ || waiting_pen_) return;
    
    switch (step_) {
        case 0: if (garis(2.40, 9.70, 8.60, 9.70)) step_++; break;
        case 1: if (garis(8.60, 9.70, 8.60, 10.40)) step_++; break;
        case 2: if (garis(8.60, 10.40, 2.40, 10.40)) step_++; break;
        case 3: if (garis(2.40, 10.40, 2.40, 9.70)) step_++; break;

        case 4: if (garis(3.26, 9.70, 2.60, 1.90)) step_++; break;
        case 5: if (garis(4.00, 9.70, 3.40, 1.24)) step_++; break;
        case 6: if (garis(7.00, 9.70, 7.60, 1.24)) step_++; break;
        case 7: if (garis(7.74, 9.70, 8.40, 1.90)) step_++; break;

        case 8: if (garis(3.98, 9.44, 7.02, 9.44)) step_++; break;
        case 9: if (garis(3.85, 7.54, 7.15, 7.54)) step_++; break;
        case 10: if (garis(3.83, 7.24, 7.17, 7.24)) step_++; break;
        case 11: if (garis(3.81, 7.00, 7.19, 7.00)) step_++; break;
        case 12: if (garis(3.79, 6.70, 7.21, 6.70)) step_++; break;
        case 13: if (garis(3.64, 4.68, 7.36, 4.68)) step_++; break;
        case 14: if (garis(3.54, 3.18, 7.46, 3.18)) step_++; break;
        case 15: if (garis(3.48, 2.38, 7.52, 2.38)) step_++; break;

        case 16: if (garis(4.76, 3.68, 6.24, 3.68)) step_++; break;
        case 17: if (garis(6.24, 3.68, 6.24, 3.20)) step_++; break;
        case 18: if (garis(6.24, 3.20, 4.76, 3.20)) step_++; break;
        case 19: if (garis(4.76, 3.20, 4.76, 3.68)) step_++; break;

        default:
            cmd_pub_->publish(geometry_msgs::msg::Twist());  
            RCLCPP_INFO(this->get_logger(), "Done boskyuh");
            timer_->cancel();
    }
  }

  bool garis(double x1, double y1, double x2, double y2)
  {
    switch (fase_) {
        case 0: if (pen(true)) fase_++; break;
        case 1: if (go_to(x1, y1)) fase_++; break;
        case 2: if (pen(false)) fase_++; break;
        case 3: if (go_to(x2, y2)) { fase_ = 0; return true; } break;
    }
    return false;
  }

  bool go_to(double x, double y)
  {
    double dx = x - pose_.x, dy = y - pose_.y;
    double distance = std::hypot(dx, dy);
    if (distance < 0.03) { cmd_pub_->publish(geometry_msgs::msg::Twist()); return true; }

    double err = std::atan2(dy, dx) -pose_.theta;
    err = std::atan2(std::sin(err), std::cos(err));

    geometry_msgs::msg::Twist cmd;
    cmd.angular.z = 6.0 * err;
    if (std:: fabs(err) < 0.15) cmd.linear.x = std::clamp(3.0 * distance, 0.3, 3.0);
    cmd_pub_->publish(cmd);
    return false;
  }

  bool pen(bool off)
  {
    if (!pen_client_->service_is_ready()) return false;
    cmd_pub_->publish(geometry_msgs::msg::Twist());
    auto req = std::make_shared<turtlesim::srv::SetPen::Request>();
    req->r = 255; req->g =0; req->b = 0; req->width = 5;
    req->off = off ? 1 : 0;
    waiting_pen_ = true;
    pen_client_->async_send_request(
        req, [this](rclcpp::Client<turtlesim::srv::SetPen>::SharedFuture) {waiting_pen_ = false;} );
    return true;
  }

  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_pub_;
  rclcpp::Subscription<turtlesim::msg::Pose>::SharedPtr pose_sub_;
  rclcpp::Client<turtlesim::srv::SetPen>::SharedPtr pen_client_;

  turtlesim::msg::Pose pose_;
  bool ada_pose_ = false, waiting_pen_ = false;
  int step_ = 0, fase_ = 0;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<AmperaDraw>());
  rclcpp::shutdown();
  return 0;
}
/**
 * @file ros_posemesh_bridge.cpp
 * @brief ROS2 to Posemesh Bridge Node Implementation
 * @author Auki Labs / BIBTV
 */

#include "ros_posemesh_bridge.hpp"

#include <tf2/LinearMath/Quaternion.h>
#include <tf2/LinearMath/Matrix3x3.h>

namespace auki_ros
{

PosemeshBridge::PosemeshBridge()
    : Node("posemesh_bridge")
    , tf_listener_(std::make_unique<tf2_ros::TransformListener>(tf_buffer_))
    , tf_broadcaster_(std::make_unique<tf2_ros::TransformBroadcaster>(*this))
    , domain_state_(DomainState::DISCONNECTED)
{
  RCLCPP_INFO(this->get_logger(), "Initializing ROS Posemesh Bridge...");

  // Initialize parameters
  initParameters();

  // Initialize ROS interfaces
  initRosInterfaces();

  // Connect to domain
  connectToDomain();

  // Start pose publish timer
  pose_publish_timer_ = this->create_wall_timer(
      std::chrono::duration<double>(1.0 / config_.posePublishHz),
      std::bind(&PosemeshBridge::publishPose, this));

  // Start connection check timer
  connection_check_timer_ = this->create_wall_timer(
      std::chrono::seconds(5),
      std::bind(&PosemeshBridge::checkConnection, this));

  RCLCPP_INFO(this->get_logger(), "ROS Posemesh Bridge initialized");
}

PosemeshBridge::~PosemeshBridge()
{
  disconnectFromDomain();
  RCLCPP_INFO(this->get_logger(), "ROS Posemesh Bridge destroyed");
}

void PosemeshBridge::initParameters()
{
  // Declare parameters with defaults
  this->declare_parameter<std::string>("domain_id", "default_domain");
  this->declare_parameter<std::string>("robot_id", "ros_robot_01");
  this->declare_parameter<std::string>("domain_server_url", "localhost:8080");
  this->declare_parameter<std::string>("wallet_private_key", "");
  this->declare_parameter<std::string>("ros_frame_id", "base_link");
  this->declare_parameter<std::string>("posemesh_frame_id", "pmap");
  this->declare_parameter<double>("pose_publish_rate", 10.0);
  this->declare_parameter<bool>("use_relay_fallback", true);
  this->declare_parameter<std::vector<std::string>>("tracked_devices", {});

  // Get parameters
  this->get_parameter("domain_id", config_.domain_id);
  this->get_parameter("robot_id", config_.robot_id);
  this->get_parameter("domain_server_url", config_.domain_server_url);
  this->get_parameter("wallet_private_key", config_.wallet_private_key);
  this->get_parameter("ros_frame_id", config_.ros_frame_id);
  this->get_parameter("posemesh_frame_id", config_.posemesh_frame_id);
  this->get_parameter("pose_publish_rate", config_.posePublishHz);
  this->get_parameter("use_relay_fallback", config_.useRelayFallback);
  this->get_parameter("tracked_devices", config_.trackedDevices);

  RCLCPP_INFO(this->get_logger(), "Configuration loaded:");
  RCLCPP_INFO(this->get_logger(), "  Domain ID: %s", config_.domain_id.c_str());
  RCLCPP_INFO(this->get_logger(), "  Robot ID: %s", config_.robot_id.c_str());
  RCLCPP_INFO(this->get_logger(), "  Server URL: %s", config_.domain_server_url.c_str());
  RCLCPP_INFO(this->get_logger(), "  Pose Publish Rate: %.1f Hz", config_.posePublishHz);
}

void PosemeshBridge::initRosInterfaces()
{
  // Publishers
  pose_pub_ = this->create_publisher<geometry_msgs::msg::PoseStamped>(
      "/posemesh/robot_pose", rclcpp::QoS(10).best_effort());
  status_pub_ = this->create_publisher<std_msgs::msg::String>(
      "/posemesh/domain_status", rclcpp::QoS(1).reliable());

  // Subscribers
  pose_sub_ = this->create_subscription<geometry_msgs::msg::PoseStamped>(
      "/robot_pose", rclcpp::QoS(10),
      std::bind(&PosemeshBridge::onRosPose, this, std::placeholders::_1));

  odom_sub_ = this->create_subscription<nav_msgs::msg::Odometry>(
      "/odom", rclcpp::QoS(10),
      std::bind(&PosemeshBridge::onRosOdometry, this, std::placeholders::_1));

  RCLCPP_INFO(this->get_logger(), "ROS interfaces initialized");
}

void PosemeshBridge::connectToDomain()
{
  domain_state_ = DomainState::CONNECTING;
  publishStatus("Connecting to domain: " + config_.domain_id);

  RCLCPP_INFO(this->get_logger(), "Connecting to posemesh domain: %s", 
              config_.domain_id.c_str());

  // TODO: Initialize actual Posemesh SDK and connect
  // posemesh_sdk_ = std::make_unique<PosemeshSDK>(config_.domain_id, ...);
  // posemesh_sdk_->connect();

  // Placeholder: simulate connection
  rclcpp::sleep_for(std::chrono::milliseconds(100));

  domain_state_ = DomainState::CONNECTED;
  publishStatus("Connected to domain: " + config_.domain_id);
  RCLCPP_INFO(this->get_logger(), "Connected to posemesh domain");
}

void PosemeshBridge::disconnectFromDomain()
{
  if (domain_state_ == DomainState::DISCONNECTED)
    return;

  RCLCPP_INFO(this->get_logger(), "Disconnecting from posemesh domain...");
  
  // TODO: Disconnect from Posemesh SDK
  // posemesh_sdk_->disconnect();

  domain_state_ = DomainState::DISCONNECTED;
  publishStatus("Disconnected from domain");
}

void PosemeshBridge::publishPose()
{
  if (domain_state_ != DomainState::CONNECTED && 
      domain_state_ != DomainState::CALIBRATED)
  {
    return;
  }

  try
  {
    auto transform = getRobotTransform();
    
    if (transform.header.frame_id.empty())
    {
      RCLCPP_DEBUG_THROTTLE(this->get_logger(), *this->get_clock(), 1000, 
                           "No transform available");
      return;
    }

    // Convert to posemesh frame
    geometry_msgs::msg::Pose pose;
    pose.position.x = transform.transform.translation.x;
    pose.position.y = transform.transform.translation.y;
    pose.position.z = transform.transform.translation.z;
    pose.orientation = transform.transform.rotation;

    geometry_msgs::msg::Pose pm_pose = convertToPosemeshFrame(pose);

    // TODO: Publish to posemesh network
    // posemesh_sdk_->publishPose(config_.robot_id, pm_pose, now());

    RCLCPP_DEBUG(this->get_logger(), "Published pose to posemesh");
  }
  catch (const std::exception &e)
  {
    RCLCPP_ERROR(this->get_logger(), "Failed to publish pose: %s", e.what());
  }
}

void PosemeshBridge::onRosPose(const geometry_msgs::msg::PoseStamped::SharedPtr msg)
{
  // TODO: Publish to posemesh network
  // posemesh_sdk_->publishPose(config_.robot_id, msg->pose, msg->header.stamp);
}

void PosemeshBridge::onRosOdometry(const nav_msgs::msg::Odometry::SharedPtr msg)
{
  // Handle odometry as fallback if /tf is not available
  // TODO: Implement odometry-based pose publishing
}

void PosemeshBridge::onPosemeshPose(const PosemeshPose &pose)
{
  // Convert from posemesh frame to ROS frame
  geometry_msgs::msg::Pose ros_pose = convertToRosFrame(pose.pose);

  // Publish to ROS topic
  geometry_msgs::msg::PoseStamped pose_msg;
  pose_msg.header.stamp = pose.timestamp;
  pose_msg.header.frame_id = config_.ros_frame_id;
  pose_msg.pose = ros_pose;
  
  pose_pub_->publish(pose_msg);

  // Also broadcast as TF
  geometry_msgs::msg::TransformStamped tf_msg;
  tf_msg.header.stamp = pose.timestamp;
  tf_msg.header.frame_id = "world";
  tf_msg.child_frame_id = pose.device_id;
  tf_msg.transform.translation.x = ros_pose.position.x;
  tf_msg.transform.translation.y = ros_pose.position.y;
  tf_msg.transform.translation.z = ros_pose.position.z;
  tf_msg.transform.rotation = ros_pose.orientation;

  tf_broadcaster_->sendTransform(tf_msg);
}

geometry_msgs::msg::TransformStamped PosemeshBridge::getRobotTransform()
{
  geometry_msgs::msg::TransformStamped transform;
  
  try
  {
    transform = tf_buffer_.lookupTransform(
        "world", 
        config_.ros_frame_id,
        tf2::TimePointZero);
  }
  catch (const tf2::TransformException &ex)
  {
    RCLCPP_DEBUG_THROTTLE(this->get_logger(), *this->get_clock(), 1000, 
                         "Transform not available: %s", ex.what());
  }
  
  return transform;
}

geometry_msgs::msg::Pose PosemeshBridge::convertToPosemeshFrame(
    const geometry_msgs::msg::Pose &pose)
{
  // TODO: Implement actual coordinate transformation
  // This depends on how the domain is calibrated
  return pose;
}

geometry_msgs::msg::Pose PosemeshBridge::convertToRosFrame(
    const geometry_msgs::msg::Pose &pose)
{
  // TODO: Implement actual coordinate transformation
  // This depends on how the domain is calibrated
  return pose;
}

void PosemeshBridge::publishStatus(const std::string &status)
{
  std_msgs::msg::String msg;
  msg.data = status;
  status_pub_->publish(msg);
}

void PosemeshBridge::checkConnection()
{
  // TODO: Check actual SDK connection status
  // if (!posemesh_sdk_->isConnected())
  // {
  //   RCLCPP_WARN(this->get_logger(), "Connection lost, attempting reconnect...");
  //   connectToDomain();
  // }
}

std::string PosemeshBridge::domainStateToString(DomainState state)
{
  switch (state)
  {
    case DomainState::DISCONNECTED: return "DISCONNECTED";
    case DomainState::CONNECTING: return "CONNECTING";
    case DomainState::CONNECTED: return "CONNECTED";
    case DomainState::AUTHENTICATED: return "AUTHENTICATED";
    case DomainState::CALIBRATED: return "CALIBRATED";
    case DomainState::ERROR: return "ERROR";
    default: return "UNKNOWN";
  }
}

} // namespace auki_ros

// Main function
int main(int argc, char *argv[])
{
  rclcpp::init(argc, argv);
  
  auto node = std::make_shared<auki_ros::PosemeshBridge>();
  
  RCLCPP_INFO(node->get_logger(), "Starting ROS Posemesh Bridge node...");
  
  rclcpp::spin(node);
  
  rclcpp::shutdown();
  return 0;
}

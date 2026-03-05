/**
 * @file ros_posemesh_bridge.hpp
 * @brief ROS2 to Posemesh Bridge Node
 * @author Auki Labs / BIBTV
 */

#ifndef ROS_POSEMESH_BRIDGE_HPP
#define ROS_POSEMESH_BRIDGE_HPP

#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include <tf2_ros/transform_listener.hpp>
#include <tf2_ros/transform_broadcaster.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <std_msgs/msg/string.hpp>
#include <sensor_msgs/msg/laser_scan.hpp>

#include <memory>
#include <string>
#include <vector>

namespace auki_ros
{

/**
 * @brief Connection state to posemesh domain
 */
enum class DomainState
{
  DISCONNECTED,
  CONNECTING,
  CONNECTED,
  AUTHENTICATED,
  CALIBRATED,
  ERROR
};

/**
 * @brief Pose data from posemesh network
 */
struct PosemeshPose
{
  std::string device_id;
  geometry_msgs::msg::Pose pose;
  rclcpp::Time timestamp;
  float confidence;
};

/**
 * @brief Configuration for the bridge node
 */
struct BridgeConfig
{
  std::string domain_id;
  std::string robot_id;
  std::string domain_server_url;
  std::string wallet_private_key;
  std::string ros_frame_id;
  std::string posemesh_frame_id;
  double posePublishRate;
  double posePublishHz;
  bool useRelayFallback;
  std::vector<std::string> trackedDevices;
};

/**
 * @class PosemeshBridge
 * @brief Main bridge node for connecting ROS robots to posemesh
 */
class PosemeshBridge : public rclcpp::Node
{
public:
  /**
   * @brief Constructor
   */
  PosemeshBridge();

  /**
   * @brief Destructor
   */
  ~PosemeshBridge();

private:
  // ROS Publishers
  rclcpp::Publisher<geometry_msgs::msg::PoseStamped>::SharedPtr pose_pub_;
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr status_pub_;

  // ROS Subscribers
  rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr pose_sub_;
  rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr odom_sub_;

  // TF
  std::unique_ptr<tf2_ros::TransformListener> tf_listener_;
  std::unique_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;

  // Timers
  rclcpp::TimerBase::SharedPtr pose_publish_timer_;
  rclcpp::TimerBase::SharedPtr connection_check_timer_;

  // Configuration
  BridgeConfig config_;

  // State
  DomainState domain_state_;
  std::string connection_error_;

  // Posemesh SDK (placeholder - will be replaced with actual SDK)
  // std::unique_ptr<PosemeshSDK> posemesh_sdk_;

  /**
   * @brief Initialize parameters from config file
   */
  void initParameters();

  /**
   * @brief Initialize ROS publishers and subscribers
   */
  void initRosInterfaces();

  /**
   * @brief Connect to posemesh domain
   */
  void connectToDomain();

  /**
   * @brief Disconnect from posemesh domain
   */
  void disconnectFromDomain();

  /**
   * @brief Publish robot pose to posemesh
   */
  void publishPose();

  /**
   * @brief Handle incoming pose from posemesh network
   * @param pose Pose data from posemesh
   */
  void onPosemeshPose(const PosemeshPose &pose);

  /**
   * @brief Handle pose subscription from ROS
   * @param msg Pose message
   */
  void onRosPose(const geometry_msgs::msg::PoseStamped::SharedPtr msg);

  /**
   * @brief Handle odometry subscription from ROS
   * @param msg Odometry message
   */
  void onRosOdometry(const nav_msgs::msg::Odometry::SharedPtr msg);

  /**
   * @brief Get current robot pose from TF
   * @return TransformStamped or empty if unavailable
   */
  geometry_msgs::msg::TransformStamped getRobotTransform();

  /**
   * @brief Convert pose from ROS frame to posemesh frame
   * @param pose ROS pose
   * @return Converted pose
   */
  geometry_msgs::msg::Pose convertToPosemeshFrame(const geometry_msgs::msg::Pose &pose);

  /**
   * @brief Convert pose from posemesh frame to ROS frame
   * @param pose Posemesh pose
   * @return Converted pose
   */
  geometry_msgs::msg::Pose convertToRosFrame(const geometry_msgs::msg::Pose &pose);

  /**
   * @brief Publish status message
   * @param status Status string
   */
  void publishStatus(const std::string &status);

  /**
   * @brief Check connection and attempt reconnect if needed
   */
  void checkConnection();

  /**
   * @brief Convert DomainState to string
   * @param state Domain state
   * @return String representation
   */
  std::string domainStateToString(DomainState state);
};

} // namespace auki_ros

#endif // ROS_POSEMESH_BRIDGE_HPP

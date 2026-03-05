"""
@file bringup.launch.py
@brief Launch file for ROS Posemesh Bridge
@author Auki Labs / BIBTV
"""

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, LogInfo
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    """Generate launch description for the posemesh bridge."""

    # Launch arguments
    domain_id_arg = DeclareLaunchArgument(
        'domain_id',
        default_value='test_domain',
        description='Posemesh domain ID to connect to'
    )

    robot_id_arg = DeclareLaunchArgument(
        'robot_id',
        default_value='ros_robot_01',
        description='Unique identifier for this robot'
    )

    domain_server_arg = DeclareLaunchArgument(
        'domain_server_url',
        default_value='localhost:8080',
        description='Posemesh domain server URL'
    )

    pose_rate_arg = DeclareLaunchArgument(
        'pose_publish_rate',
        default_value='10.0',
        description='Rate to publish pose to posemesh (Hz)'
    )

    # Posemesh bridge node
    posemesh_bridge_node = Node(
        package='auki_posemesh_bridge',
        executable='posemesh_bridge',
        name='posemesh_bridge',
        parameters=[{
            'domain_id': LaunchConfiguration('domain_id'),
            'robot_id': LaunchConfiguration('robot_id'),
            'domain_server_url': LaunchConfiguration('domain_server_url'),
            'pose_publish_rate': LaunchConfiguration('pose_publish_rate'),
            'ros_frame_id': 'base_link',
            'posemesh_frame_id': 'pmap',
            'use_relay_fallback': True,
        }],
        output='screen',
        emulate_tty=True,
    )

    return LaunchDescription([
        domain_id_arg,
        robot_id_arg,
        domain_server_arg,
        pose_rate_arg,
        LogInfo(msg='Starting ROS Posemesh Bridge...'),
        posemesh_bridge_node,
    ])

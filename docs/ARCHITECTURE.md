# Architecture Documentation

## Overview

The ROS Posemesh Bridge enables ROS2 robots to connect to the Posemesh decentralized spatial computing network. This allows robots to:

- Share their pose with other devices in the network
- Receive pose data from other robots and AR devices
- Participate in collaborative spatial computing sessions

## System Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                        ROS Posemesh Bridge                       │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────────┐  │
│  │   ROS 2      │    │   TF2       │    │   Posemesh SDK   │  │
│  │   Layer      │    │   Layer     │    │   Layer          │  │
│  └──────┬───────┘    └──────┬───────┘    └────────┬─────────┘  │
│         │                   │                     │             │
│  ┌──────▼───────┐    ┌──────▼───────┐    ┌────────▼─────────┐  │
│  │ Subscribers  │    │ Transform    │    │ P2P Networking   │  │
│  │ - /robot_pose│    │ Buffer/List. │    │ - libp2p         │  │
│  │ - /odom      │    │              │    │ - Domain Conn.   │  │
│  └──────┬───────┘    └─────────────┘    └────────┬─────────┘  │
│         │                                        │              │
│  ┌──────▼────────────────────────────────────────▼───────────┐ │
│  │                    Coordinate Transform                      │ │
│  │  ROS Frame (base_link) <-> Posemesh Frame (pmap)          │ │
│  └─────────────────────────────────────────────────────────────┘ │
│                                                                  │
└──────────────────────────────────────────────────────────────────┘
                              │
                              │ P2P / Relay
                              ▼
┌──────────────────────────────────────────────────────────────────┐
│                      Posemesh Network                             │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐             │
│  │   Domain    │  │   Hagall    │  │   Other     │             │
│  │   Server    │  │   Relay     │  │   Devices   │             │
│  └─────────────┘  └─────────────┘  └─────────────┘             │
└──────────────────────────────────────────────────────────────────┘
```

## Components

### 1. ROS 2 Layer

**Publishers:**
- `/posemesh/robot_pose` - Pose of other devices from posemesh
- `/posemesh/domain_status` - Connection status

**Subscribers:**
- `/robot_pose` - External pose (optional)
- `/odom` - Odometry data

### 2. TF2 Layer

Handles coordinate transformations between:
- Robot's local frame (`base_link`)
- World frame (`world`)
- Posemesh domain frame (`pmap`)

### 3. Posemesh SDK Layer

(To be implemented when SDK is available)

- Domain connection management
- Pose publishing/subscribing
- Authentication via wallet
- Relay fallback for NAT traversal

## Data Flow

### Outgoing (Robot -> Posemesh)

```
1. Get transform from TF: world -> base_link
2. Convert to posemesh frame (if calibrated)
3. Publish to posemesh network via SDK
4. Other devices receive pose via P2P
```

### Incoming (Posemesh -> Robot)

```
1. Receive pose from posemesh network
2. Convert from posemesh frame to ROS frame
3. Publish to /posemesh/robot_pose
4. Broadcast TF: world -> device_id
```

## Configuration

See `config/params.yaml` for configuration options.

Key parameters:
- `domain_id` - Posemesh domain to connect to
- `robot_id` - Unique identifier for this robot
- `ros_frame_id` - Robot's base frame in ROS
- `pose_publish_rate` - How often to publish pose

## Building

```bash
# Clone the repository
cd ~/colcon_ws/src
git clone https://github.com/bibtv/auki.git

# Build
cd ~/colcon_ws
colcon build --packages-select auki_posemesh_bridge

# Source and run
source install/setup.bash
ros2 launch auki_posemesh_bridge bringup.launch.py
```

## Topics

| Topic | Type | Direction | Description |
|-------|------|-----------|-------------|
| `/robot_pose` | PoseStamped | In | External pose input |
| `/odom` | Odometry | In | Odometry data |
| `/posemesh/robot_pose` | PoseStamped | Out | Other device poses |
| `/posemesh/domain_status` | String | Out | Connection status |

## Coordinate Frames

- `world` - Fixed world frame
- `base_link` - Robot's base frame (configurable)
- `pmap` - Posemesh domain frame (set during calibration)

## Future Work

- [ ] Implement actual Posemesh SDK integration
- [ ] Add support for domain calibration
- [ ] Implement multi-robot coordination
- [ ] Add SLAM integration
- [ ] Support for sensor data sharing (lidar, camera)

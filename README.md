# ROS to Posemesh Bridge Architecture

## Overview

```
┌─────────────┐     ┌──────────────────┐     ┌─────────────┐
│  ROS Robot  │────▶│  Posemesh Bridge │────▶│ Posemesh   │
│  (Any ROS2  │     │  Node (C++/Py)   │     │ Domain     │
│   robot)    │◀────│                  │◀────│            │
└─────────────┘     └──────────────────┘     └─────────────┘
      │                     │                       │
      │ /tf, /odom,        │ P2P via               │ Calibration,
      │ /scan, /image     │ libp2p                │ spatial data
      │                   │                       │
      ▼                   ▼                       ▼
┌─────────────┐     ┌──────────────────┐     ┌─────────────┐
│ Sensor/Actuator│    │  Other Robots/   │     │  Domain     │
│   Data       │    │  AR Devices       │     │  Owner      │
└─────────────┘     └──────────────────┘     └─────────────┘
```

## Bridge Node Design

### Core Responsibilities

1. **Pose Publishing** - Send robot pose to posemesh domain
2. **Pose Subscription** - Receive poses from other devices
3. **Domain Communication** - Calibrate with domain, exchange spatial data
4. **Coordinate Transform** - Convert ROS ↔ Posemesh coordinate systems

### ROS Topics

| Direction | Topic | Type | Purpose |
|-----------|-------|------|---------|
| In | `/tf` | `tf2_msgs/TFMessage` | Robot pose |
| In | `/odom` | `nav_msgs/Odometry` | Odometry backup |
| In | `/scan` | `sensor_msgs/LaserScan` | For mapping |
| Out | `/posemesh/robot_pose` | `geometry_msgs/PoseStamped` | Other devices' poses |
| Out | `/posemesh/domain_status` | `std_msgs/String` | Domain connection status |

### Architecture Layers

```
┌─────────────────────────────────────────┐
│           ROS 2 Interface               │
│  (Subscribers, Publishers, Actions)    │
├─────────────────────────────────────────┤
│         Coordinate Transform            │
│  (ROS ↔ Posemesh frame conversion)    │
├─────────────────────────────────────────┤
│           Posemesh SDK                 │
│  (libposemesh C++ / Python bindings)   │
├─────────────────────────────────────────┤
│         P2P Networking (libp2p)        │
│  (QUIC, WebRTC, mDNS, relay)           │
└─────────────────────────────────────────┘
```

## Implementation Options

### Option A: C++ Bridge (Recommended)
- Direct SDK bindings (no wrapper overhead)
- Best performance for real-time pose sync
- More complex to develop

```cpp
// Pseudocode
class PosemeshBridge {
  PosemeshSDK sdk;
  rclcpp::Subscription<tf2_msgs::msg::TFMessage>::SharedPtr tf_sub;
  rclcpp::Publisher<geometry_msgs::msg::PoseStamped>::SharedPtr pose_pub;
  
  void on_tf(const tf2_msgs::msg::TFMessage::SharedPtr msg) {
    // Extract robot pose from /tf
    // Publish to posemesh domain
    sdk.publish_pose(robot_pose);
  }
  
  void on_posemesh_pose(const Pose& pose) {
    // Convert to ROS coordinate frame
    // Publish to /posemesh/robot_pose
  }
};
```

### Option B: Python Bridge (Faster Dev)
- Use ctypes or pybind11 bindings
- Easier to prototype
- Slightly higher latency

### Option C: ROS2 Action/Service Hybrid
- Action for domain calibration (long-running)
- Service for pose queries
- Parameter server for config

## Domain Interaction Flow

```
┌────────────┐     ┌────────────┐     ┌────────────┐
│   Bridge   │────▶│   Domain   │────▶│  Calibration│
│   Node     │     │   Server   │     │   Data     │
└────────────┘     └────────────┘     └────────────┘
      │                   │
      │  1. Connect      │  2. Authenticate
      │  3. Request      │  4. Return lighthouses
      │     calibration  │     + spatial data
      │
      │  5. Publish pose │
      │     continuously │
      ▼
┌────────────┐
│ Other       │◀─── P2P pose exchange
│ Devices     │
└────────────┘
```

## Key Challenges

| Challenge | Solution |
|-----------|----------|
| Coordinate frames | Need lighthouse markers or known transform |
| Real-time performance | Use ROS2 QoS best-effort for pose sync |
| Authentication | Wallet-based (posemesh uses token economy) |
| Network reliability | Relay fallback when P2P fails |

## Files to Create

```
ros_posemesh_bridge/
├── CMakeLists.txt
├── package.xml
├── include/
│   └── posemesh_bridge.hpp
├── src/
│   ├── posemesh_bridge_node.cpp
│   └── coordinate_transform.cpp
├── launch/
│   └── bringup.launch.py
├── config/
│   └── params.yaml
└── README.md
```

## Next Steps

1. **Get domain access** - Create/test a posemesh domain
2. **Build SDK** - Compile libposemesh for your platform
3. **Create minimal bridge** - Publish dummy pose → domain
4. **Add real tf** - Hook up actual robot data

---

Want me to create the skeleton code for any of these components?

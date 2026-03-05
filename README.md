# ROS Posemesh Bridge

<p align="center">
  <img src="docs/images/architecture.png" alt="ROS Posemesh Bridge Architecture" width="600">
</p>

Connect ROS2 robots to the [Posemesh](https://posemesh.org/) decentralized spatial computing network.

## Overview

The ROS Posemesh Bridge enables robots to:
- **Share pose data** with other devices on the posemesh network
- **Receive poses** from other robots and AR devices
- **Participate** in collaborative spatial computing

Built on top of the [Auki Labs](https://auki.com) posemesh protocol.

## Features

- 🔌 Connect ROS robots to posemesh domains
- 📍 Real-time pose synchronization
- 🔄 TF2 integration for coordinate transforms
- ⚙️ Configurable publish rates
- 🛡️ Relay fallback for NAT traversal

## Requirements

- ROS2 (Humble or later)
- C++17 compiler
- Posemesh SDK (coming soon)

## Quick Start

### Installation

```bash
# Clone the repository
cd ~/colcon_ws/src
git clone https://github.com/bibtv/auki.git

# Build
cd ~/colcon_ws
colcon build --packages-select auki_posemesh_bridge
```

### Configuration

Edit `config/params.yaml`:

```yaml
domain_id: "your_domain_id"
robot_id: "your_robot_name"
domain_server_url: "localhost:8080"
ros_frame_id: "base_link"
pose_publish_rate: 10.0
```

### Running

```bash
# Source workspace
source install/setup.bash

# Launch with default config
ros2 launch auki_posemesh_bridge bringup.launch.py

# Or with arguments
ros2 launch auki_posemesh_bridge bringup.launch.py \
  domain_id:=test_domain \
  robot_id:=my_robot
```

## Project Structure

```
auki/
├── include/              # Header files
│   └── ros_posemesh_bridge.hpp
├── src/                 # Source files
│   └── ros_posemesh_bridge.cpp
├── launch/              # Launch files
│   └── bringup.launch.py
├── config/              # Configuration
│   └── params.yaml
├── docs/                # Documentation
│   └── ARCHITECTURE.md
├── CMakeLists.txt
├── package.xml
└── README.md
```

## Documentation

- [Architecture](docs/ARCHITECTURE.md) - Detailed system architecture
- [ROS Topics](docs/ARCHITECTURE.md#topics) - Available topics
- [Configuration](config/params.yaml) - Parameter reference

## ROS Topics

| Topic | Type | Description |
|-------|------|-------------|
| `/robot_pose` | PoseStamped | External pose input |
| `/odom` | Odometry | Odometry data |
| `/posemesh/robot_pose` | PoseStamped | Other device poses |
| `/posemesh/domain_status` | String | Connection status |

## Coordinate Frames

- `world` - Fixed world frame
- `base_link` - Robot's base frame
- `pmap` - Posemesh domain frame

## Related Projects

- [posemesh](https://github.com/aukilabs/posemesh) - Core posemesh protocol
- [auki_robotics](https://github.com/aukilabs/auki_robotics) - Auki robotics tools
- [ConjureKit](https://conjurekit.dev) - AR SDK (predecessor to posemesh SDK)

## License

MIT License - see [LICENSE](LICENSE) for details.

## Credits

- [Auki Labs](https://auki.com) - Posemesh protocol
- [Posemesh Foundation](https://posemesh.org) - Decentralized spatial computing

---

Built by [BIBTV](https://github.com/bibtv) 🤖

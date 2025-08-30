# Solidworks-Controlled-Stacking-Robot
This project extracts pick-up and drop-off coordinates from two SolidWorks Assembly files: one showing the robot’s starting environment and one showing the final stacked layout. Macros created using the SolidWorks API extract the coordinates and generate a temporary PowerShell script. The script sends the data to the computer port connected to the robot’s Arduino, which moves the robot accordingly.
## Robot Info
- **Kinematics**: Uses a coordinate system and inverse kinematics to calculate reachable positions
- **Gearboxes**: Three custom 1:20 cycloidal gearboxes provide high torque and compact form factor
- **Payload Capacity**: Supports up to 5 kg of stacked weight within an 80 cm diameter workspace, using a vacuum suction mechanism for reliable pick-and-place operations

CAD Link:
- 📁 [6DOF Arm v1 – GrabCAD](https://grabcad.com/library/6dof-arm-v1-1)

## Demo Video

Watch the robot in action:  
[![Robot Demo](https://img.youtube.com/vi/8PGG2dZVc4I/0.jpg)](https://www.youtube.com/shorts/8PGG2dZVc4I)


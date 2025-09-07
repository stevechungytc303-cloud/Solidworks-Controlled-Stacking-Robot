# Solidworks-Controlled-Stacking-Robot
This project uses macros created with the SolidWorks API to extract pick-up and drop-off coordinates from two SolidWorks assembly files: one showing the robot's starting environment and one showing the final stacked layout. After the extraction, a temporary PowerShell script is generated to send the coordinates to the computer port connected to the robot. With the coordinates, the robot starts picking up and placing objects to recreate the final stacked layout.
(More below)
## Robot Info
![Robotic Arm](SolidworksControlledStackingRobotImage.png)
- **Gearboxes**: Three custom-designed 1:20 cycloidal gearboxes that provide high torque and compact form factor
- **Payload Capacity**: Can lift and move objects up to 3kg within a 40cm radius using a vacuum suction mechanism for reliable pick-and-place operations
- **Kinematics**: Uses coordinate system and inverse kinematics to calculate joint angles for reaching target positions

**CAD Link**: (https://grabcad.com/library/solidworks-controlled-stacking-robot-1)

## Demo Video

Watch the robot in action:  
[![Robot Demo](https://img.youtube.com/vi/8PGG2dZVc4I/0.jpg)](https://www.youtube.com/shorts/8PGG2dZVc4I)


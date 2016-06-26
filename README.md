## Single Axis Stepper Motor Emulator

**_Overview_**          
This is a simple DC brushed motor controller with encoder feedback, step-direction interface, and optical home switch. It was originally built for a modular robotics application using low-power DC motors and very coarse (low resolution) encoders. This circuit may find application in CNC and 3D printing applications, but it is likely that certain modifications will need to be made (in software for example) to make use of higher resolution encoders. 

**_Principle of Operation_**   
The block diagram below illustrates the basic principle of operation. The microcontroller is a Microchip PIC 18F1320 and the motor driver is a LMD18200. Depending on the level of the Direction input, pulses on the Step line will increment or decrement an internal counter in the PIC. A separate counter tracks the actual position of the motor by decoding encoder pulses. An error signal is defined by the difference of the counter representing desired position and the one representing actual position. The LMD18200 receives a motor command that is simply proportional to the error. PID control is possible but not currently used. If the Step input is held high for longer than a certain threshold, the controller switches state to "homing mode" in which it runs the motor at constant speed until the optical home switch is activated. After the home switch is activated the controller returns to normal operation.

![alt text](https://raw2.github.com/mattmoses/SingleAxisEmulator/master/blockDiagram.png)
[alt text](blockDiagram.png)

**_Downloads_**   
[Schematic](https://github.com/mattmoses/SingleAxisEmulator/blob/master/singleAxisMotor3.pdf)   
[Code](https://github.com/mattmoses/SingleAxisEmulator/tree/master/verticalMotor)     
[Gerber Files](https://github.com/mattmoses/SingleAxisEmulator/tree/master/gerbers)   
[Eagle PCB Files](https://github.com/mattmoses/SingleAxisEmulator/tree/master/eagleFiles)     
[Parts List](https://github.com/mattmoses/SingleAxisEmulator/blob/master/singleAxisMotor3_parts.txt)   

**_License_**   
This work is licensed under the [GNU General Public License](http://www.gnu.org/licenses/gpl.html).

**_How to Cite_**   
If you would like to reference this work in a publication, please refer to the [github url](https://github.com/mattmoses/SingleAxisEmulator) and cite this paper:

[An architecture for universal construction via modular robotic components](http://dx.doi.org/10.1016/j.robot.2013.08.005)    
*Robotics and Autonomous Systems*   
Matthew S. Moses, Hans Ma, Kevin C. Wolfe, Gregory S. Chirikjian (2013)

**_Credits_**   
This work was performed in [Gregory Chirikjian's](http://scholar.google.com/citations?user=qoIuyMoAAAAJ) [Robot & Protein Kinematics Lab](https://rpk.lcsr.jhu.edu/Publications) at Johns Hopkins University. The work was partially supported by the National Science Foundation under Grant No. IIS-0915542. Any opinions, findings, and conclusions or recommendations expressed in this material are those of the authors and do not necessarily reflect the views of the National Science Foundation.

Matt Moses created the schematic and ciruit board layout.    
Kevin Wolfe created the custom part library "modular_robot_parts.lbr".   
Kevin Wolfe authored the initial version of the control software.   
Matt Moses modified Kevin's original code. (Good parts belong to Kevin. Bad parts are Matt's fault.)   
John Swensen provided helpful advice.






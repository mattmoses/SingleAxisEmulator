## Single Axis Stepper Motor Emulator

**_Overview_**          
This is a simple DC brushed motor controller with encoder feedback, step-direction interface, and optical home switch. It was originally built for a modular robotics application using low-power DC motors and very coarse (low resolution) encoders. This circuit may find application in CNC and 3D printing applications, but it is likely that certain modifications will need to be made (in software for example) to make use of higher resolution encoders. 

**_Principle of Operation_**   
The block diagram below illustrates the basic principle of operation. The microcontroller is a Microchip PIC 18F1320 and the motor controller is a LMD18200.

![alt text](https://raw2.github.com/mattmoses/SingleAxisEmulator/master/blockDiagram.png)

**_Downloads_**   
[Schematic](https://github.com/mattmoses/SingleAxisEmulator/blob/master/singleAxisMotor3.pdf)   
Code   
[Gerber Files](https://github.com/mattmoses/SingleAxisEmulator/tree/master/gerbers)   
[Eagle PCB Files](https://github.com/mattmoses/SingleAxisEmulator/tree/master/eagleFiles)     
Parts List   

**_License_**   
This work is licensed under the [GNU General Public License](http://www.gnu.org/licenses/gpl.html).

**_How to Cite_**   
If you would like to reference this work in a publication, please refer to the [github url](https://github.com/mattmoses/SingleAxisEmulator) and cite this paper:

[An architecture for universal construction via modular robotic components](http://dx.doi.org/10.1016/j.robot.2013.08.005)    
*Robotics and Autonomous Systems*   
Matthew S. Moses, Hans Ma, Kevin C. Wolfe, Gregory S. Chirikjian (2013)

**_Credits_**   
This work was performed in Prof. Greg Chirikjian's [Robot & Protein Kinematics Lab](https://rpk.lcsr.jhu.edu/Publications) at Johns Hopkins University. The work was partially supported by the National Science Foundation under Grant No. IIS-0915542. Any opinions, findings, and conclusions or recommendations expressed in this material are those of the authors and do not necessarily reflect the views of the National Science Foundation.

The schematic and ciruit board layout was done by Matt Moses   
The initial code was authored by Kevin C. Wolfe. The code available on this page is a result of Matt Moses ~~hacking up~~ modifying Kevin's work.
John Swensen provided helpful advice.






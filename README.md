# quaternion-smooth-rotation-by-opengl
## 项目功能
1.	读取三角形网格数据文件

2.	使用 OpenGL 绘制该模型(能够对模型进行平移、旋转)

3.	交互设定模型的两个位置和姿态，利用四元数对物体进行平滑的移动和旋转，使得物体在平移过程中能够平滑旋转。

由于欧拉角有gimbal lock问题，且绕不同轴的转动次序不同可能导致得到的状态不同，故而不采用欧拉角实现。而四元数很适合做旋转，故而采用四元数实现
## 运行条件
具有opengl的glut库
## 程序清单
readObj.h文件为用于读入三角形网格的类，Opengl.cpp为实现程序功能的主程序。.obj是示例三角形网格文件。
## 操作方式
采用鼠标左键拉动物体平移，鼠标右键拉动物体旋转，键盘的w键放大物体，s键缩小物体。

交互式设定两个姿态时,先按“r”完成复位，先进行平移，平移后再进行旋转。调整好姿态后按下“q”说明记录当前姿态（即平滑旋转转到的姿态）。再次复位，复位后再进行平移旋转（旋转必须在平移之后，旋转之后不再平移，除非再次复位），调整好姿态后按下“e”说明从当前姿态开始平滑旋转。每次放缩之后要求复位，并重新进行初始和终止姿态的设置。
## 实现细节
### 1. 读入obj三角形网格文件
采用blender生成三角形网格，输入的obj文件中的“f”项只指向对应的三个点，而不指向它的法向信息等。简化对obj文件的读入。
读入obj文件时，根据“v”开头的项目，建立节点。根据“f”开头的项目建立三角形的faces数组。设置draw()函数，使用glVertex3f完成三角形网格绘制。
### 2.	对模型进行平移旋转（也可以放缩）
在readObj.h中设置读入obj文件的绘制函数draw()。将display设为窗口绘制的回调函数，在display中调用draw()。根据从外设得到的需要对物体进行的操作，通过glScalef，glTranslatef，glRotatef完成变换。

采用鼠标控制旋转和平移，用键盘控制放缩。用MouseFunc()和MouseMotion()作为鼠标事件的回调函数。通过鼠标左键按下后的移动情况进行平移。计算屏幕中鼠标的x位移与y位移，对应物体的移动距离。设置KeyBoards()作为键盘事件的回调函数，根据”w”与“s”按下的次数对物体进行放缩。

### 3.	交互式平滑旋转（物体的移动路径为直线，旋转路径平滑）
openGL不支持四元数，故而自己实现了旋转矩阵转化为四元数的函数convertToQuaternion, 四元数到旋转矩阵的转化函数QuaternionToMatrix, 四元数插值函数SLERP.

用户按下按键”q”后,通过glGetFloatv得到当前MODELVIEW下的矩阵。设置timesCounter变量表示放缩的累积值, curTransX与curTransY表示从上次置位开始到当前状态的累积位移量。通过这三个参数，变换MODELVIEW下的矩阵，得到当前状态下的旋转矩阵matend。用convertToQuaternion将旋转矩阵转化为终止时候的四元数tos。当用户按下“e”键后，通过上述方式得到起始四元数matbegin。

设置idle函数，该idle函数在每次要平滑旋转时，运行display2。在display2中设置参数t(t在idle过程中每次递增0.01，表示当前四元数插值的位置),调用SLERP,计算当前四元数的值，再转化为旋转矩阵multi。调用glMultMatrixf将该变换应用到模型上。同时根据上述参数timesCounter，curTransX与curTransY对物体进行放缩，旋转。最终绘制出当前时刻的物体姿态

四元数操作的实现参考了Nick Bobic的Rotating Objects using Quaternions中的解法。

## 参考链接
https://www.cnblogs.com/feiquan/p/8207407.html
https://blog.csdn.net/m0_37876745/article/details/78147046
Rotating Objects using Quaternions. Nick Bobic
https://blog.csdn.net/freehawkzk/article/details/81150764
https://blog.csdn.net/qweewqpkn/article/details/46332331
https://blog.csdn.net/shimazhuge/article/details/25135009
https://blog.csdn.net/cjneo/article/details/50455973

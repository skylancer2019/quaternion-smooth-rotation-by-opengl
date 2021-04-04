#include "readObj.h"
#include <Windows.h>
#include <GL/glut.h>  // GLUT, include glu.h and gl.h

readObj re;
GLfloat angle = 0.0f;
float mouseX, mouseY;
bool mouseLeftDown=false;
bool mouseRightDown=false;
bool idleBegin = false;
bool needRecord = false;
bool needReset = false;
float cameraDistanceX;
float cameraDistanceY;
float cameraDistanceZ;
float cameraAngleX;
float cameraAngleY;
float curTransX;
float curTransY;//记录当前总体的位置
float recordX;
float recordY;
float intervalX;
float intervalY;

float times = 1;
GLdouble timesCounter = 1;
GLfloat curwidth;
GLfloat curheight;
GLfloat storeX;
GLfloat storeY;
GLfloat matend[16];//当前变换矩阵
GLfloat matbegin[16];//起始处的变换矩阵
GLfloat curChange[4][4];
GLfloat multi[16];

GLfloat t;
int refreshMills = 60;
//GLfloat iden[16] = { 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 };
float nx; float ny; float nz;//得到转轴的向量
struct Quaternion
{
	GLfloat x;
	GLfloat y;
	GLfloat z;
	GLfloat w;

};
Quaternion froms;
Quaternion tos;

Quaternion convertToQuaternion(GLfloat mat[])
{
	Quaternion temp;
	GLfloat m[4][4];
	for (int j = 0; j < 4; j++)
	{
		for (int i = 0; i < 4; i++)
		{
			m[j][i] = mat[j * 4 + i];
		}
	}
	float tr, s, q[4];
	int i, j, k;
	int nxt[3] = { 1,2,0 };
	tr = m[0][0] + m[1][1] + m[2][2];//对角线元素和
	if (tr > 0.0)
	{
		s = sqrt(tr + 1.0);
		temp.w = s / 2.0;
		s = 0.5 / s;
		temp.x = (m[1][2] - m[2][1]) * s;
		temp.y = (m[2][0] - m[0][2]) * s;
		temp.z = (m[0][1] - m[1][0]) * s;

	}
	else
	{
		i = 0;
		if (m[1][1] > m[0][0])i = 1;
		if (m[2][2] > m[i][i])i = 2;
		j = nxt[i];
		k = nxt[j];
		s = sqrt((m[i][i] - (m[j][j] + m[k][k])) + 1.0);
		q[i] = s * 0.5;
		if (s != 0.0)
		{
			s = 0.5 / s;
		}
		q[3] = (m[j][k] - m[k][j]) * s;
		q[j] = (m[i][j] + m[j][i]) * s;
		q[k] = (m[i][k] + m[k][i]) * s;
		temp.x = q[0];
		temp.y = q[1];
		temp.z = q[2];
		temp.w = q[3];
	}
	return temp;

}
void QuaternionToMatrix(Quaternion q)
{
	float wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;
	x2 = q.x + q.x; y2 = q.y + q.y;
	z2 = q.z + q.z;
	xx = q.x * x2; xy = q.x * y2; xz = q.x * z2;
	yy = q.y * y2; yz = q.y * z2;
	zz = q.z * z2;
	wx = q.w * x2; wy = q.w * y2; wz = q.w * z2;

	curChange[0][0] = 1.0 - (yy + zz); curChange[1][0] = xy - wz;
	curChange[2][0] = xz + wy; curChange[3][0] = 0.0;
	curChange[0][1] = xy + wz; curChange[1][1] = 1.0 - (xx + zz);
	curChange[2][1] = yz - wx; curChange[3][1] = 0.0;

	curChange[0][2] = xz - wy; curChange[1][2] = yz + wx;
	curChange[2][2] = 1.0 - (xx + yy); curChange[3][2] = 0.0;

	curChange[0][3] = 0; curChange[1][3] = 0;
	curChange[2][3] = 0; curChange[3][3] = 1;
	int c = 0;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			multi[c] = curChange[i][j];
			c++;
		}
	}

	
}
Quaternion QuatMul(Quaternion q1, Quaternion q2)
{
	Quaternion res;
	float A, B, C, D, E, F, G, H;
	A = (q1.w + q1.x) * (q2.w + q2.x);
	B = (q1.z - q1.y) * (q2.y - q2.z);
	C = (q1.w - q1.x) * (q2.y + q2.z);
	D = (q1.y = q1.z) * (q2.w - q2.x);
	E = (q1.x + q1.z) * (q2.x + q2.y);
	F = (q1.x - q1.z) * (q2.x - q2.y);
	G = (q1.w + q1.y) * (q2.w - q2.z);
	H = (q1.w - q1.y) * (q2.w + q2.z);
	res.w = B + (-E - F + G + H) / 2;
	res.x = A - (E + F + G + H) / 2;
	res.y = C + (E - F + G - H) / 2;
	res.z = D + (E - F - G + H) / 2;
	return res;
}
Quaternion SLERP(Quaternion from, Quaternion to, float t)
{
	Quaternion res;
	float tol[4];
	double omega, cosom, sinom, scale0, scale1;
	cosom = from.x * to.x + from.y * to.y + from.z * to.z + from.w * to.w;
	//得到cos角度
	if (cosom < 0.0)//为了防止走优弧，需要对角度特判
	{
		cosom = -cosom;
		tol[0] = -to.x;
		tol[1] = -to.y;
		tol[2] = -to.z;
		tol[3] = -to.w;

	}
	else
	{
		tol[0] = to.x;
		tol[1] = to.y;
		tol[2] = to.z;
		tol[3] = to.w;
	}//若为负数，则四元数取反
	float DELTA = 0.0005f;
	if ((1 - cosom) > DELTA)//防止角度太小，而溢出 
	{
		omega = acos(cosom);
		sinom = sin(omega);
		scale0 = sin((1.0 - t) * omega) / sinom;
		scale1 = sin(t * omega) / sinom;
	}
	else
	{
		scale0 = 1.0 - t;
		scale1 = t;
	}
	res.x = scale0 * from.x + scale1 * tol[0];
	res.y = scale0 * from.y + scale1 * tol[1];
	res.z = scale0 * from.z + scale1 * tol[2];
	res.w = scale0 * from.w + scale1 * tol[3];
	return res;

}

void display2()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	
	if (idleBegin)
	{
		
		curTransX += intervalX;
		curTransY += intervalY;

		Quaternion cur = SLERP(froms, tos, t);
		QuaternionToMatrix(cur);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		
		glScalef(timesCounter,timesCounter,timesCounter);//假设并不改变图像
		glTranslatef(curTransX, curTransY, 0);
		glMultMatrixf(multi);
		
		re.draw();

		glutSwapBuffers();
	}
	t += 0.01f;
	if (t > 1)
	{
			idleBegin = false;
			return;
	}

}
void neckAndHead()
{
	glPushMatrix();
	glTranslatef(0, 0.12, 0);
	glPushMatrix();//脖子
	glTranslatef(0, 0.185, 0);
	//进入头部坐标
	glColor3d(1, 1, 0);
	glutWireCube(0.3);
	//	glutSolidCube(0.3);
	glPopMatrix();
	glColor3d(1, 1, 0);
	glutSolidCube(0.12);
	glPopMatrix();//返回肩膀坐标系
}
void test()
{
	glPushMatrix();//Pelvis坐标
	glTranslatef(0, 0.56, 0);
	glPushMatrix();//mainBody坐标
	glTranslatef(0, 0.56, 0);
	glPushMatrix();//肩膀坐标
	neckAndHead();
	//此时栈顶仍为肩膀坐标
	glPopMatrix();//
	glPopMatrix();
	glPopMatrix();//
}
void display()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	if (needReset)
	{
		glLoadIdentity();
		glScalef(timesCounter,timesCounter,timesCounter);
		needReset = false;
	}
	{
		glScalef(times, times, times);
		glRotatef(cameraAngleX, 1, 0, 0);
		glRotatef(cameraAngleY, 0, 1, 0);

		glTranslatef(cameraDistanceX, cameraDistanceY, 0);
	}
	
	re.draw();
	
	glutSwapBuffers();
	//angle += 0.02f;
	
}

void reshape(GLsizei width, GLsizei height)
{
	if (height == 0)height = 1;
	float size = 2;
	GLfloat aspect = (GLfloat)width / (GLfloat)height;
	curwidth = width; curheight = height;
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (width > height)
	{
		glOrtho(-size * aspect,size * aspect, -size,size,-size,size);
	}
	else
	{
		glOrtho(-size, size, -size / aspect, size / aspect,-size,size);
	}

}
void MouseMotion(int x,int y)
{
	cameraAngleX = cameraAngleY = 0;
	cameraDistanceX = cameraDistanceY = 0;
	cameraDistanceZ = 0;
	//mouseLeftDown = false;
	if (mouseRightDown)
	{
		cameraAngleY += (x - mouseX) * 0.1f;
		cameraAngleX += (y - mouseY) * 0.1f;
		mouseX = x;
		mouseY = y;
		glutPostRedisplay();
	}
	if (mouseLeftDown)
	{
		cameraDistanceX += (x - mouseX) * 0.008f;
		cameraDistanceY += -(y - mouseY) * 0.008f;
		curTransX += cameraDistanceX;
		curTransY += cameraDistanceY;
		mouseX = x;
		mouseY = y;
		glutPostRedisplay();
	}
}
void MouseFunc(int button, int state, int x, int y)
{
	mouseX = x;
	mouseY = y;
	times = 1;
	mouseRightDown = false;
	mouseLeftDown = false; cameraDistanceZ=cameraDistanceX = cameraDistanceY = 0;
	if (button == GLUT_LEFT_BUTTON)
	{
		if (state == GLUT_DOWN)
		{
			mouseLeftDown = true;
		}
		else
		{
			if (state == GLUT_UP)
			{
				mouseLeftDown = false;
			}
		}
	}
	else
	{
		if (button == GLUT_RIGHT_BUTTON)
		{
			if (state == GLUT_DOWN)
			{
				mouseRightDown = true;
			}
			else
			{
				if (state == GLUT_UP)
				{
					mouseRightDown = false;
				}
			}
		}

	}

}

//先定义结束节点的位置，然后从某一节点处开始转动
void KeyBoards(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'w':
		times = 0.08f + 1;
		timesCounter *= 1.08f;
		glutPostRedisplay();
		break;
	case 's':
		times = -0.08f + 1;
		timesCounter *= 0.92f;
		glutPostRedisplay();
		break;
	/*case 'z'://no use
		cameraDistanceZ = -0.1f;
		glutPostRedisplay();
		break;
    
	case 'x'://no use
		cameraDistanceZ = 0.1f;
		glutPostRedisplay();
		break;*/
	case 'r'://reset
		needReset = true;
		curTransX = 0;
		curTransY = 0;
		glutPostRedisplay();
		break;
	case 'q'://记录旋转结束节点的位置以及状态
		needRecord = true;
		glGetFloatv(GL_MODELVIEW_MATRIX, matend);//得到当前局部坐标的旋转矩阵
		
		recordX = curTransX;
		recordY = curTransY;
		matend[0]=matend[0] / timesCounter; matend[1]=matend[1] / timesCounter;  matend[2]=matend[2] / timesCounter;
		matend[3]=matend[3] / timesCounter; matend[4]=matend[4] / timesCounter; matend[5]=matend[5] / timesCounter;
		matend[6]=matend[6] / timesCounter; matend[7]=matend[7] / timesCounter; matend[8]=matend[8] / timesCounter; matend[9]=matend[9] / timesCounter; matend[10]=matend[10] / timesCounter;  matend[11]=matend[11] / timesCounter;
	//	matend[12] = 0; matend[13] = 0; matend[14] = 0; 
		tos = convertToQuaternion(matend);
		break;
	case 'e'://开始绘制运动曲线
		idleBegin = true;	
		glGetFloatv(GL_MODELVIEW_MATRIX, matbegin);//得到局部坐标的旋转矩阵
		matbegin[0] /= timesCounter; matbegin[1] /=timesCounter; matbegin[2] /= timesCounter; matbegin[3] /= timesCounter;
		matbegin[4] /= timesCounter; matbegin[5] /= timesCounter; matbegin[6] /= timesCounter; matbegin[7] /= timesCounter;
		matbegin[8] /= timesCounter; matbegin[9] /= timesCounter; matbegin[10] /= timesCounter; matbegin[11] /= timesCounter;
		//matbegin[12] = 0; matbegin[13] = 0; matbegin[14] = 0;
		froms = convertToQuaternion(matbegin);
		intervalX = (recordX - curTransX) / 100;//100控制转动速度
		intervalY = (recordY - curTransY) / 100;
		t = 0;
		//glutPostRedisplay();
	}
}
void idle(int value)
{
	if (idleBegin)              
	{
		//reshape(curwidth, curheight);
		display2();
	}
	glutTimerFunc(refreshMills, idle, 0);
}
int main(int argc,char** argv)
{
	t = 0;
	curTransX = 0;
	curTransY = 0;
	re.parse("test2.obj");
	glutInit(&argc, argv);
	
	glutInitWindowSize(1000, 600);
	glutInitWindowPosition(50, 50);
	glutCreateWindow("OpenGL test1");
//	glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	glutInitDisplayMode(GLUT_DOUBLE);
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);

	glutMouseFunc(MouseFunc);
	glutMotionFunc(MouseMotion);
	glutKeyboardFunc(KeyBoards);
	glutTimerFunc(0, idle, 0);

	GLfloat light_position[] = { 1.0, 1.0, 1.0, 1.0 };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	GLfloat light_ambient[] = { 0, 1, 1, 1 };
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT1);
	glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);//设置光的环境强度
	GLfloat light_diffuse[] = { 0, 0, 0, 1 };
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT1);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);//散色光，GL_LIGHT0默认为（1,1,1,1）GL_LIGHT1-7默认为（0, 0, 0, 1）
	GLfloat light_diffuse1[] = { 1, 1, 1, 0 };
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT1);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light_diffuse1);
	glutMainLoop();
	return 0;

}
#pragma once
#include <iostream>
#include <stdio.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <GL/glut.h>
using namespace std;
class readObj {
public:
	struct vertex {
		double x, y, z;
	};
	struct face
	{
		unsigned int v1, v2, v3;
	};
	readObj();
	vector<vertex> vertexes;
	vector<face> faces;
public:
	void parse(const char* filename);
	void draw();
};

readObj::readObj()
{
	vertexes.clear();
	faces.clear();
}
void readObj::draw()
{
	glBegin(GL_TRIANGLES);
	for (int i = 0; i < faces.size(); i++) {
		//下标减一
		float move_y = 0.5; //控制模型的位置
		vertex v1 = vertexes[faces[i].v1 - 1];
		vertex v2 = vertexes[faces[i].v2 - 1];
		vertex v3 = vertexes[faces[i].v3 - 1];
		glColor3f(0.3, 0.5, 0);
		glVertex3f(v1.x, v1.y - move_y, v1.z);

	//	glColor3f(1, 1, 0);
		glVertex3f(v2.x, v2.y - move_y, v2.z);

		//glColor3f(0.5, 0.5, 0);
		glVertex3f(v3.x, v3.y - move_y, v3.z);
	}
	glEnd();
}
void readObj::parse(const char* filename)
{
	ifstream inFile(filename, ios::in | ios::binary);
	char cur[100];
	while (inFile.getline(cur,100))
	{
		if (cur[0] == 'v'&&cur[1]==' ')
		{
			string str = "";
			for (int i = 2; i < 100; i++)
			{
				str += cur[i];
			}
			istringstream ss(str);
			vertex temp;
			ss >> temp.x; ss >> temp.y; ss >> temp.z;
			vertexes.push_back(temp);
		}
		if (cur[0] == 'f' && cur[1] == ' ')
		{
			string str = "";
			for (int i = 2; i < 100; i++)
			{
				str += cur[i];
			}
			istringstream ss(str);
			face temp;
			ss >> temp.v1; ss >> temp.v2; ss >> temp.v3;
			faces.push_back(temp);
		}

	}
}
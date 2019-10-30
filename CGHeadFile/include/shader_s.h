#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
using namespace std;
using namespace glm;
class Shader
{
public:
	unsigned int ID;//����ID

	/*�����ɫ�������Ƿ�ɹ�*/
	void CheckShaderCompile(unsigned int vertexShader, unsigned int fragmentShader)
	{
		unsigned int shaders[] = { vertexShader ,fragmentShader };

		int success;//�趨�Ƿ�ɹ��ı��
		for (int i = 0; i < 2; i++)
		{
			glGetShaderiv(shaders[i], GL_COMPILE_STATUS, &success);//������״̬
			if (!success)//������벻ͨ��
			{
				char infoLog[512];//������Ϣ
				glGetShaderInfoLog(shaders[i], 512, NULL, infoLog);//��¼������Ϣ
				switch (i)
				{
				case 0:
					cout << "������ɫ�������������" << infoLog << endl;
					break;
				case 1:
					cout << "Ƭ����ɫ�������������" << infoLog << endl;
					break;
				}
			}
		}
	}

	/*�����ɫ�������Ƿ�ɹ�*/
	void CheckShaderLink(unsigned int shaderProgram)
	{
		int success;//�趨�Ƿ�ɹ��ı��
		glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);//�������״̬
		if (!success)
		{
			char infoLog[512];//������Ϣ
			glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);//��¼������Ϣ
			cout << "��ɫ���������Ӵ���" << infoLog << endl;
		}
	}

	Shader(const char* vertexPath, const char* fragmentPath)//��������ȡ��������ɫ��
	{
		/*1.���ļ�·���л�ȡ����/Ƭ����ɫ��*/
		string vertexCode;
		string fragmentCode;
		ifstream vShaderFile;
		ifstream fShaderFile;
		// ��֤ifstream��������׳��쳣��
		vShaderFile.exceptions(ifstream::failbit | ifstream::badbit);
		fShaderFile.exceptions(ifstream::failbit | ifstream::badbit);
		try
		{
			// ���ļ�
			vShaderFile.open(vertexPath);
			fShaderFile.open(fragmentPath);
			stringstream vShaderStream, fShaderStream;
			// ��ȡ�ļ��Ļ������ݵ���������
			vShaderStream << vShaderFile.rdbuf();
			fShaderStream << fShaderFile.rdbuf();
			// �ر��ļ�������
			vShaderFile.close();
			fShaderFile.close();
			// ת����������string
			vertexCode = vShaderStream.str();
			fragmentCode = fShaderStream.str();
		}
		catch (ifstream::failure e)
		{
			cout << "�ļ���ȡʧ�ܣ�" << endl;
		}
		const char* vShaderCode = vertexCode.c_str();
		const char* fShaderCode = fragmentCode.c_str();

		/*2.������ɫ��*/
		unsigned int vertex, fragment;
		int success;
		char infoLog[512];

		/*������ɫ��*/
		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vShaderCode, NULL);
		glCompileShader(vertex);

		/*Ƭ����ɫ��*/
		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fShaderCode, NULL);
		glCompileShader(fragment);

		/*��ӡ�������(�����)*/
		CheckShaderCompile(vertex, fragment);

		/*��ɫ������*/
		ID = glCreateProgram();
		glAttachShader(ID, vertex);
		glAttachShader(ID, fragment);
		glLinkProgram(ID);

		/*��ӡ���Ӵ���(�����)*/
		CheckShaderLink(ID);

		/*ɾ���Ѿ����ӵ�����ģ�����Ҫ����ɫ��*/
		glDeleteShader(vertex);
		glDeleteShader(fragment);
	}

	void use()//������ɫ������
	{
		glUseProgram(ID);
	}

	/*uniform���ߺ���*/
	void setBool(const string& name, bool value) const
	{
		glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
	}
	void setInt(const string& name, int value) const
	{
		glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
	}
	void setFloat(const string& name, float value) const
	{
		glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
	}
	void setVec2(const string& name, const vec2& value) const
	{
		glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}
	void setVec2(const string& name, float x, float y) const
	{
		glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
	}
	void setVec3(const string& name, const vec3& value) const
	{
		glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}
	void setVec3(const string& name, float x, float y, float z) const
	{
		glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
	}
	void setVec4(const string& name, const vec4& value) const
	{
		glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}
	void setVec4(const string& name, float x, float y, float z, float w)
	{
		glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
	}
	void setMat2(const string& name, const mat2& mat) const
	{
		glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}
	void setMat3(const string& name, const mat3& mat) const
	{
		glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}
	void setMat4(const string& name, const mat4& mat) const
	{
		glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}
};

#endif // !SHADER_H
#version 330 core

layout (location = 0) in vec3 aPos;//��������
layout (location = 1) in vec3 aColor;//��ɫ����
layout (location = 2) in vec2 aTexCoord;//��������

out vec3 ourColor;//��ɫ���
out vec2 TexCoord;//�������

uniform mat4 model;//ģ�;���
uniform mat4 view;//��ͼ����
uniform mat4 projection;//ͶӰ����

void main()
{
	//�����һ���Ĳü��ռ�����
    gl_Position = (projection * view * model) * vec4(aPos, 1.0);
    ourColor = aColor;
	TexCoord = aTexCoord;
}
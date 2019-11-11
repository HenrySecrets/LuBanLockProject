#version 330 core

layout (location = 0) in vec3 aPos;//顶点输入
layout (location = 1) in vec3 aColor;//颜色输入
layout (location = 2) in vec2 aTexCoord;//纹理输入

out vec3 ourColor;//颜色输出
out vec2 TexCoord;//纹理输出

void main()
{
    gl_Position = vec4(aPos, 1.0);//定义归一化的裁剪空间坐标//
    ourColor = aColor;
	TexCoord = aTexCoord;
}
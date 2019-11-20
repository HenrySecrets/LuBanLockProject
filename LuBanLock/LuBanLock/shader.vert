#version 330 core

layout (location = 0) in vec3 aPos;//顶点输入
layout (location = 1) in vec3 aColor;//颜色输入
layout (location = 2) in vec2 aTexCoord;//纹理输入

out vec3 ourColor;//颜色输出
out vec2 TexCoord;//纹理输出

uniform mat4 model;//模型矩阵
uniform mat4 view;//视图矩阵
uniform mat4 projection;//投影矩阵

void main()
{
	//定义归一化的裁剪空间坐标
    gl_Position = (projection * view * model) * vec4(aPos, 1.0);
    ourColor = aColor;
	TexCoord = aTexCoord;
}
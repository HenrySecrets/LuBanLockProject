#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <shader_s.h>
#include <camera.h>
#include <iostream>
#include <vector>
using namespace std;
using namespace glm;

/*………………………………全局变量………………………………*/
Camera camera;//相机
float deltaTime;//每帧的时间差

/*………………………………可配置区………………………………*/
/*当窗口大小改变时，将调用该回调函数以调整视口大小*/
void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}
/*光标位置回调函数*/
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
}
/*鼠标滚轮回调函数*/
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
}
/*按键回调函数*/
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	glfwSetWindowShouldClose(window, key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE);//当按下后抬起Esc键则关闭窗口
}
/*窗口结构体*/
struct Window
{
private:
	/*设置回调函数*/
	void SetCallback(GLFWwindow* window)
	{
		glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
		glfwSetCursorPosCallback(window, CursorPosCallback);
		glfwSetScrollCallback(window, ScrollCallback);
		glfwSetKeyCallback(window, KeyCallback);
	}
public:
	GLFWwindow* window = NULL;
	const char* windowTitle = "LuBanLock";//窗口名称
	float windowWidth = 1280.0f;//窗口宽度
	float windowHeight = 720.0f;//窗口高度
	Window()
	{
		if (window = glfwCreateWindow(windowWidth, windowHeight, windowTitle, NULL, NULL))//创建一个窗口对象，参数分别为宽、高、窗口名称，后两个暂时忽略
		{
			glfwMakeContextCurrent(window);//将窗口上下文设置为当前线程的主上下文
			SetCallback(window);
		}
		else
		{
			cout << "窗口创建失败！";
			glfwTerminate();
		}
	}
	/*设置渲染缓冲*/
	void SetRenderBuffer()
	{
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);//设置窗口填充颜色
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//设置颜色与深度缓冲
	}
};
/*模型结构体*/
struct Model
{
private:
	float* vertices_data;//顶点数据
	int* indices_data;//索引数据
	enum class ObjectType { VAO, VBO, EBO };//对象类型
	unsigned int VBO;//顶点缓存对象
	unsigned int VAO;//顶点数组对象
	unsigned int EBO;//索引缓冲对象
	mat4 model, view, projection;//模型、视图、投影矩阵
	/*初始化VBO与VAO*/
	void InitVBOandVAO()
	{
		/*将接收的顶点数据存储下来*/
		vertices_data = new float[verticesData[0].size()];
		for (size_t i = 0; i < verticesData[0].size(); i++)
		{
			vertices_data[i] = verticesData[0][i];
		}

		glGenBuffers(1, &VBO);//生成VBO
		BindObject(ObjectType::VBO);
		glBufferData(GL_ARRAY_BUFFER, verticesData[0].size() * sizeof(float), vertices_data, GL_STATIC_DRAW);//配置顶点数据
		glGenVertexArrays(1, &VAO);//生成VAO
		BindObject(ObjectType::VAO);
	}
	/*初始化EBO*/
	void InitEBO()
	{
		/*将接收的索引数据存储下来*/
		indices_data = new int[dataIndices[0].size()];
		for (size_t i = 0; i < dataIndices[0].size(); i++)
		{
			indices_data[i] = dataIndices[0][i];
		}
		glGenBuffers(1, &EBO);//生成EBO
		BindObject(ObjectType::EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, dataIndices[0].size() * sizeof(int), indices_data, GL_STATIC_DRAW);//配置索引数据
	}
	/*绑定对象*/
	void BindObject(ObjectType objectType)
	{
		switch (objectType)
		{
		case Model::ObjectType::VAO:
			glBindVertexArray(VAO);//绑定VAO
			break;
		case Model::ObjectType::VBO:
			glBindBuffer(GL_ARRAY_BUFFER, VBO);//绑定VBO
			break;
		case Model::ObjectType::EBO:
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);//绑定EBO
			break;
		}
	}
public:
	/*只包含顶点数据*/
	Model(vector<float> vertices_data)
	{
		verticesData.push_back(vertices_data);
		InitVBOandVAO();
	}
	/*包含顶点与索引数据*/
	Model(vector<float> vertices_data, vector<int> indices_data)
	{
		verticesData.push_back(vertices_data);
		dataIndices.push_back(indices_data);
		InitVBOandVAO();
		InitEBO();
	}
	enum class MapType { DiffuseMap, SpecularMap };//贴图类型
	unsigned int diffuseMap;//漫反射贴图
	unsigned int specularMap;//高光贴图
	vector<vector<float>> verticesData;//顶点数据
	vector<vector<int>> dataIndices;//数据索引
	vec3 position;//模型位置
	/*配置属性(顶点位置、纹理坐标等)*/
	void ConfigAttribute(unsigned int location, unsigned int count, GLenum type, bool normalized, unsigned int stride, const void* pointer)
	{
		glVertexAttribPointer(location, count, type, normalized, stride, pointer);
		glEnableVertexAttribArray(location);
	}
	/*解绑对象*/
	void UnBindObject()
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);//解绑VBO、EBO
		glBindVertexArray(0);//解绑VAO
	}
	/*生成纹理*/
	unsigned int GenTexture(char const* path)
	{
		unsigned int textureID;
		glGenTextures(1, &textureID);//生成材质

		int width, height, nrComponents;
		unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);//加载图片
		if (data)//如果加载图片成功
		{
			GLenum format = (nrComponents == 3) ? GL_RGB : ((nrComponents == 4) ? GL_RGBA : GL_RED);//根据图片信息设定色彩模式

			glBindTexture(GL_TEXTURE_2D, textureID);//绑定图片到纹理
			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);//生成纹理
			glGenerateMipmap(GL_TEXTURE_2D);//生成多级渐远纹理

			/*设置环绕与过滤模式*/
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			stbi_image_free(data);//释放图片信息
		}
		else
		{
			cout << "纹理无法从以下路径加载：" << path << endl;
			stbi_image_free(data);
		}

		return textureID;
	}
	/*绑定纹理*/
	void BindTexture(MapType mapType)
	{
		switch (mapType)
		{
		case Model::MapType::DiffuseMap:
			glActiveTexture(GL_TEXTURE0);//激活纹理
			glBindTexture(GL_TEXTURE_2D, diffuseMap);//绑定纹理
			break;
		case Model::MapType::SpecularMap:
			glActiveTexture(GL_TEXTURE1);//激活纹理
			glBindTexture(GL_TEXTURE_2D, specularMap);//绑定纹理
			break;
		}
	}
	/*设置变换矩阵*/
	void SetMatrix(Window window, bool isOrtho, Shader modelShader)
	{
		view = camera.GetViewMatrix();//试图矩阵即为相机视图
		projection = (isOrtho) ? ortho(0.0f, window.windowWidth, 0.0f, window.windowHeight, 0.1f, 100.0f) : perspective(radians(camera.Zoom), window.windowWidth / window.windowHeight, 0.1f, 100.0f);//设定投影视图是正交或透视
		modelShader.setMat4("model", model);//设置模型矩阵
		modelShader.setMat4("view", view);//设置视图矩阵
		modelShader.setMat4("projection", projection);//设置投影矩阵
	}
	/*不使用索引，绘制对象*/
	void DrawObject(GLenum mode, unsigned int first, unsigned int size)
	{
		glBindVertexArray(VAO);
		glDrawArrays(mode, first, size);
	}
	/*使用索引，绘制对象*/
	void DrawObject(GLenum mode, unsigned int count, GLenum type, const void* indices)
	{
		glBindVertexArray(VAO);
		glDrawElements(mode, count, type, indices);
	}
	/*释放空间*/
	void ReleaseSpace()
	{
		glDeleteBuffers(1, &VBO);
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &EBO);
	}
	~Model()
	{
		/*释放空间*/
		delete[] vertices_data;
		delete[] indices_data;
		verticesData[0].clear();
		dataIndices[0].clear();
		verticesData.clear();
		dataIndices.clear();
	}
};
/*设置测试*/
void SetTest()
{
	glEnable(GL_DEPTH_TEST);//开启深度测试
}

/*………………………………非配置区………………………………*/
/*初始化GLFW*/
void InitGLFW()
{
	glfwInit();//初始化GLFW

	/*glfwWindowHint用于配置GLFW，第一个参数是选项名称，第二个是设置选项的一个整形的值*/
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);//设定主版本号为3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);//设定次版本号为3
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);//告诉GLFW使用核心模式
}
/*初始化GLAD以在后面调用OpenGL函数*/
void InitGLAD()
{
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))//glfwGetProcAddress是用于加载系统相关的OpenGL函数指针地址的函数(必须在设定好线程上下文之后调用)
	{
		cout << "初始化GLAD失败" << endl;
		glfwTerminate();
	}
}
/*获取时间插值*/
void GetDeltaTime()
{
	float lastFrame = 0.0f;
	float currentTime = 0.0f;
	currentTime = glfwGetTime();//1.记录当前帧对应时间
	deltaTime = currentTime - lastFrame;//3.从第二帧开始，获得与前帧的时间差
	lastFrame = currentTime;//2.缓存当前帧时间，作为下一帧的前帧时间
}

/*………………………………程序入口………………………………*/
int main()
{
	/*………………………………初始化部分………………………………*/
	InitGLFW();
	Window mainWindow;
	InitGLAD();

	/*………………………………加载着色器………………………………*/
	Shader ModelShader("shader.vert", "shader.frag");

	/*………………………………模型部分………………………………*/
	Model triangle
	(
		{	//位置				 //颜色			     //纹理坐标
			0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,//右上
			0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,//右下
			-0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f,   0.0f, 0.0f,//左下
			-0.5f,  0.5f, 0.0f,  1.0f, 1.0f, 0.0f,   0.0f, 1.0f// 左上
		},
		{	//索引
			0, 1, 3, // 第一个三角形
			1, 2, 3  // 第二个三角形
		}
		);
	triangle.ConfigAttribute(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	triangle.ConfigAttribute(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	triangle.ConfigAttribute(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	triangle.UnBindObject();

	/*………………………………贴图部分………………………………*/
	stbi_set_flip_vertically_on_load(true);//设定加载图片时反转y轴
	triangle.diffuseMap = triangle.GenTexture("Resources/Textures/wall.jpg");
	triangle.specularMap = triangle.GenTexture("Resources/Textures/awesomeface.png");
	ModelShader.use();
	ModelShader.setInt("texture1", 0);
	ModelShader.setInt("texture2", 1);

	/*………………………………变换矩阵设置………………………………*/
	//triangle.SetMatrix(mainWindow, false, ModelShader);

	/*………………………………测试设置………………………………*/
	//SetTest();

	/*………………………………渲染循环………………………………*/
	while (!glfwWindowShouldClose(mainWindow.window))//glfwWindowShouldClose函数将检查GLFW是否被要求退出
	{
		mainWindow.SetRenderBuffer();
		GetDeltaTime();
		triangle.BindTexture(Model::MapType::DiffuseMap);
		triangle.BindTexture(Model::MapType::SpecularMap);
		triangle.DrawObject(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glfwSwapBuffers(mainWindow.window);//交换缓冲
		glfwPollEvents();//检查是否有事件被触发并更新窗口状态以及调用对应的回调函数
	}

	/*………………………………程序结束后处理………………………………*/
	triangle.ReleaseSpace();
	glfwTerminate();
	return 0;
}
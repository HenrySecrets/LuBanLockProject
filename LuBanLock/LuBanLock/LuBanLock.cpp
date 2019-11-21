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
double mouseScrollDelta = 0.0f;
float modelScaleSensitivity = 0.05f;//模型缩放敏感度

bool isMouseOnWindow = false;//鼠标是否在窗口上
bool isLeftMouseButtonRepeat = false;//鼠标左键是否按住
bool isLeftMouseButtonRelease = true;//鼠标左键是否抬起
bool isRightMouseButtonRepeat = false;//鼠标右键是否按住
bool isRightMouseButtonRelease = true;//鼠标右键是否抬起

unsigned int isTranslate = false;//是否平移
unsigned int isRotate = false;//是否旋转

vec2 mousePositionDelta = vec2(0);//鼠标位置增量
float modelTranslateSensitivity = 0.002f;//模型平移敏感度
float modelRotateSensitivity = 0.1f;//模型旋转敏感度

mat4 view, projection;//视图、投影矩阵
Camera camera(vec3(0, 0, 3));//相机
float deltaTime;//每帧的时间差

/*………………………………可配置区………………………………*/
/*当窗口大小改变时，将调用该回调函数以调整视口大小*/
void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

/*光标进入窗口回调函数*/
void CursorEnterCallback(GLFWwindow* window, int isEnter)
{
	isMouseOnWindow = isEnter;
}

vec2 lastmousePosition = vec2(540, 540);//上一帧鼠标位置
/*光标位置回调函数*/
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (isLeftMouseButtonRepeat || isRightMouseButtonRepeat)//如果按住鼠标
	{
		if (!isLeftMouseButtonRelease || !isRightMouseButtonRelease)//如果鼠标没有抬起
		{
			lastmousePosition = vec2(xpos, ypos);//重新记录鼠标位置，以确保鼠标位置增量为0
			isRightMouseButtonRelease = isLeftMouseButtonRelease = true;
		}
		mousePositionDelta = vec2(xpos - lastmousePosition.x, lastmousePosition.y - ypos);
	}
	lastmousePosition = vec2(xpos, ypos);
}
/*鼠标按键回调函数*/
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	switch (action)
	{
	case GLFW_PRESS://按下鼠标的一帧调用
		switch (button)
		{
		case GLFW_MOUSE_BUTTON_LEFT://鼠标左键
			isLeftMouseButtonRelease = !(isTranslate = isLeftMouseButtonRepeat = true);
			break;
		case GLFW_MOUSE_BUTTON_RIGHT://鼠标右键
			isRightMouseButtonRelease = !(isRotate = isRightMouseButtonRepeat = true);
			break;
		}
		break;
	case GLFW_RELEASE://抬起鼠标的一帧调用
		switch (button)
		{
		case GLFW_MOUSE_BUTTON_LEFT://鼠标左键
			isTranslate = isLeftMouseButtonRepeat = !(isLeftMouseButtonRelease = true);
			break;
		case GLFW_MOUSE_BUTTON_RIGHT://鼠标右键
			isRotate = isRightMouseButtonRepeat = !(isRightMouseButtonRelease = true);
			break;
		}
		break;
	}
}

/*鼠标滚轮回调函数*/
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	mouseScrollDelta = yoffset;
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
		glfwSetCursorEnterCallback(window, CursorEnterCallback);
		glfwSetMouseButtonCallback(window, MouseButtonCallback);
		glfwSetCursorPosCallback(window, CursorPosCallback);
		glfwSetScrollCallback(window, ScrollCallback);
		glfwSetKeyCallback(window, KeyCallback);
	}
public:
	GLFWwindow* window = NULL;
	const char* windowTitle = "LuBanLock";//窗口名称
	float windowWidth = 1080.0f;//窗口宽度
	float windowHeight = 1080.0f;//窗口高度
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
		lastmousePosition = vec2(windowWidth / 2, windowHeight / 2);
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

	mat4 model;//模型矩阵

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
		for (size_t i = 0; i < vertices_data.size(); i++)
		{
			vertices_data[i] /= 30;
		}
		verticesData.push_back(vertices_data);
		InitVBOandVAO();
	}
	/*包含顶点与索引数据*/
	Model(vector<float> vertices_data, vector<int> indices_data)
	{
		for (size_t i = 0; i < vertices_data.size(); i++)
		{
			vertices_data[i] /= 30;
		}
		for (size_t i = 0; i < indices_data.size(); i++)
		{
			--indices_data[i];
		}
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
	/*设置变换属性*/
	vec3 translateSum;//与世界坐标系原点的位移差
	void SetTransform(Shader shader, vec3 translateDelta, vec2 RotateAngle, vec3 scaleDelta)
	{
		//缩放
		model = scale(model, scaleDelta);

		//进行世界坐标系下旋转
		if (isTranslate)//如果进行位移
		{
			translateSum += translateDelta;//记录总位移量
		}
		model = translate(mat4(1), -translateSum) * model;
		model = rotate(mat4(1), radians(RotateAngle.x), vec3(0, 1, 0)) * rotate(mat4(1), radians(RotateAngle.y), vec3(-1, 0, 0)) * model;
		model = translate(mat4(1), translateSum) * model;

		//平移
		model = translate(mat4(1), translateDelta) * model;
	}

	/*设置模型、视图、投影矩阵*/
	void SetMVPMatrix(Shader shader)
	{
		shader.setMat4("model", model);
		shader.setMat4("view", view);
		shader.setMat4("projection", projection);
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
/*射线结构体*/
struct Ray
{
public:
	//射线方向
	vec3 rayDirection(vec2 mousePosition, Window window, mat4 viewMatrix, mat4 projectionMatrix)
	{
		vec3 NDCPosition((2 * mousePosition.x) / window.windowWidth - 1, 1 - (2 * mousePosition.y) / window.windowHeight, 1);//标准设备坐标
		vec4 clipCoordinatePosition(NDCPosition, 1);//裁剪坐标
		vec4 eyeCoordinatePosition = inverse(projectionMatrix) * clipCoordinatePosition;//视觉坐标
		vec4 worldCoordinatePosition = inverse(viewMatrix) * eyeCoordinatePosition;//世界坐标
		if (worldCoordinatePosition.w != 0)
		{
			worldCoordinatePosition.x /= worldCoordinatePosition.w;
			worldCoordinatePosition.y /= worldCoordinatePosition.w;
			worldCoordinatePosition.z /= worldCoordinatePosition.w;
		}
		worldCoordinatePosition.z = -worldCoordinatePosition.z;
		return normalize(vec3(worldCoordinatePosition) - camera.Position);
	}
	//投射射线
	bool castRay(vec3 rayDirection, float rayMaxLength)
	{
		vec3 rayEndPosition = vec3(camera.Position.x, camera.Position.y, -camera.Position.z);//射线末端位置
		while (distance(vec3(camera.Position.x, camera.Position.y, -camera.Position.z), rayEndPosition) < rayMaxLength)
		{
			if (rayEndPosition.z >= 6.5)
			{
				return true;
			}
			rayEndPosition += rayDirection;
		}
		return false;
	}
};
/*设置视图*/
mat4 SetView()
{
	return (view = camera.GetViewMatrix());//视图矩阵即为相机视图
}

/*设置投影*/
mat4 SetProjection(Window window, bool isOrtho)
{
	return (projection = (isOrtho) ? ortho(-window.windowWidth / 500, window.windowWidth / 500, -window.windowHeight / 500, window.windowHeight / 500, 0.1f, 100.0f) : perspective(radians(camera.Zoom), window.windowWidth / window.windowHeight, 0.1f, 100.0f));//设定投影视图是正交或透视
}
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
	Window mainWindow;//主窗口
	InitGLAD();
	Ray ray;//射线
	/*………………………………加载着色器………………………………*/
	Shader modelShader("shader.vert", "shader.frag");//模型着色器

	/*………………………………模型部分………………………………*/
	Model triangle1({
			3.7655 ,19.6012 ,-7.8016,    0.0,0.0,0.0,
		   -3.8723, 19.5486, -7.7825,    0.0,0.0,0.0,
			3.8320, 19.6129, -3.9705,     0.0,0.0,0.0,
		   -3.8974 ,-19.6128 ,-3.9031,    0.0,0.0,0.0,
		   -3.8983, -19.5799, -7.7563,     0.0,0.0,0.0,
			3.7869, -19.5799, -7.8020,     0.0,0.0,0.0,
			3.7983 ,7.8741, -7.8352,     0.0,0.0,0.0,
			3.8151,-7.9171, -4.0358,     0.0,0.0,0.0,
			3.7983, -7.8740,-7.8352,     0.0,0.0,0.0,
		   -3.9311 ,7.8741 ,-7.7678,      0.0,0.0,0.0,
		   -3.9311, -7.8740, -7.7678,     0.0,0.0,0.0,
		   -3.8817 ,7.9171, -3.9686,       0.0,0.0,0.0,
		   -3.8569, -7.9182 ,-3.9429,     0.0,0.0,0.0,
			3.7907, 7.9184 ,-4.0099,        0.0,0.0,0.0,
			3.8321 ,-19.6128, -3.9705,    0.0,0.0,0.0,
		   -3.8974, 19.6129, -3.9031,     0.0,0.0,0.0,
			3.8536, -7.9792 ,-0.1388,     0.0,0.0,0.0,
		   -3.8309 ,-7.9579, -0.0718,     0.0,0.0,0.0,
			3.8324 ,-19.6012, -0.1384,     0.0,0.0,0.0,
		   -3.8524, 7.9792, -0.0714,      0.0,0.0,0.0,
			3.8536, 19.5799, -0.1388,      0.0,0.0,0.0,
		   -3.8309, 19.6012, -0.0718,      0.0,0.0,0.0,
		   -3.8524 ,-19.5799, -0.0714,     0.0,0.0,0.0,
			3.8324, 7.9579 ,-0.1384,       0.0,0.0,0.0,
		}, {
			 1, 2,3,
			 4  ,   5  , 6 ,
			 7 ,  8 ,  9 ,
			 10 , 9 ,11  ,
			 12 , 11,13  ,
			 14 ,13, 8 ,
			 6 , 8, 15  ,
			 8 , 6, 9  ,
			 7 , 1, 14 ,
			  11 , 9, 5 ,
			 7 , 10, 1 ,
			 11 , 5 ,4  ,
			 2 , 12, 16 ,
			  12,  2, 10  ,
			 17,  18 ,19,
			 20  ,21 ,22 ,
			 19 , 4, 15 ,
			 23 , 18, 4  ,
			  18 , 8 ,13 ,
			  19 , 15, 8  ,
			 22 , 3, 16  ,
			 21 , 14, 3 ,
			 24  ,20, 14 ,
			 20 , 22 ,12,
			 12 , 10, 11 ,
			 4  ,13, 11  ,
			 18 , 13, 4  ,
			 18 , 17, 8 ,
			  18 , 23, 19  ,
			 20 , 24, 21  ,
			  8, 17 ,19  ,
			  7 , 14, 8 ,
			  14,  12 ,13  ,
			  20 , 12 ,14,
			  19,  23, 4 ,
			  6 , 15, 4  ,
			  10,  2, 1 ,
			  10 ,7 ,9  ,
			  21 , 24, 14  ,
			  1 , 3 ,14  ,
			  9 , 6, 5 ,
			  22 , 16, 12 ,
			  22 , 21, 3 ,
			  2 , 16, 3 ,
		});
	triangle1.ConfigAttribute(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);//解析顶点位置
	//triangle1.ConfigAttribute(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	//triangle1.ConfigAttribute(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	triangle1.UnBindObject();

	Model triangle2({
			-19.544, -4.003, 7.79,     0.0,0.0,0.0,  0.0,1.0,//左上前：
		  19.614, -3.661, 7.79,     0.0,0.0,0.0,  1.0,1.0,//右上前：
		  19.643, -3.664, 0.112,      0.0,0.0,0.0,  1.0,0.0,//右下前：
		  -19.544, -4.024, 0.105,    0.0,0.0,0.0,  0.0,0.0,//左下前：
		  //后面
		  -19.611, 3.682, 7.769,     0.0,0.0,0.0,    0.0,1.0,//左上后
		   19.547, 4.024, 7.769,      0.0,0.0,0.0,     1.0,1.0,//右上后
		   19.576, 3.996, 0.101,      0.0,0.0,0.0,   1.0,0.0,//右下后
		  -19.611, 3.661, 0.084,     0.0,0.0,0.0,    0.0, 0.0,//右下后
		  //左面
		  -19.611, 3.682, 7.769,     0.0,0.0,0.0,    0.0,1.0,
		  -19.544, -4.003, 7.79,     0.0,0.0,0.0, 1.0,1.0,
		  -19.544, -4.024, 0.105,    0.0,0.0,0.0, 1.0,0.0,
		  -19.611, 3.661, 0.084,     0.0,0.0,0.0,    0.0, 0.0,
		  //右面
		   19.547, 4.024, 7.769,      0.0,0.0,0.0,      0.0,1.0,
			  19.614, -3.661, 7.79,     0.0,0.0,0.0,  1.0,1.0,
		   19.643, -3.664, 0.112,      0.0,0.0,0.0,  1.0,0.0,
			19.576, 3.996, 0.101,      0.0,0.0,0.0,   0.0,0.0,
			//上面
			-19.611, 3.682, 7.769,     0.0,0.0,0.0,    0.0,1.0,
			19.547, 4.024, 7.769,      0.0,0.0,0.0,     1.0,1.0,
			19.614, -3.661, 7.79,     0.0,0.0,0.0,  1.0,0.0,
			-19.544, -4.003, 7.79,     0.0,0.0,0.0,  0.0, 0.0,
			//下面
			-19.611, 3.661, 0.084,     0.0,0.0,0.0,    0.0,1.0,
			19.576, 3.996, 0.101,      0.0,0.0,0.0,   0.0,0.0,
			19.643, -3.664, 0.112,      0.0,0.0,0.0,  1.0,0.0,
			-19.544, -4.024, 0.105,    0.0,0.0,0.0,  0.0,0.0,
		}, {
			1,2,3,
			  3,4,1,
			  5,6,7,
			  7,8,5,
			  9,10,11,
			  11,12,9,
			  13,14,15,
			  15,16,13,
			  17,18,19,
			  19,20,17,
			  21,22,23,
			  23,24,21,
		});
	triangle2.ConfigAttribute(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	//triangle1.ConfigAttribute(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	//triangle1.ConfigAttribute(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	triangle2.UnBindObject();
	/*………………………………贴图部分………………………………*/
	//stbi_set_flip_vertically_on_load(true);//设定加载图片时反转y轴
	//triangle1.diffuseMap = triangle1.GenTexture("Resources/Textures/wall.jpg");
	//triangle1.specularMap = triangle1.GenTexture("Resources/Textures/awesomeface.png");
	//modelShader.use();
	//modelShader.setInt("texture1", 0);
	//modelShader.setInt("texture2", 1);

	/*………………………………测试设置………………………………*/
	//SetTest();

	/*………………………………非循环渲染………………………………*/

	/*………………………………渲染循环………………………………*/
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	while (!glfwWindowShouldClose(mainWindow.window))//glfwWindowShouldClose函数将检查GLFW是否被要求退出
	{
		mainWindow.SetRenderBuffer();
		GetDeltaTime();
		//triangle1.BindTexture(Model::MapType::DiffuseMap);
		//triangle1.BindTexture(Model::MapType::SpecularMap);

		modelShader.use();
		SetView();
		SetProjection(mainWindow, false);

		triangle1.SetTransform(modelShader, vec3(mousePositionDelta * (modelTranslateSensitivity * isTranslate), 0), mousePositionDelta * (modelRotateSensitivity * isRotate), vec3(1 + mouseScrollDelta * 0.1f));
		triangle1.SetMVPMatrix(modelShader);
		triangle1.DrawObject(GL_TRIANGLES, 132, GL_UNSIGNED_INT, 0);

		triangle2.SetTransform(modelShader, vec3(mousePositionDelta * (modelTranslateSensitivity * isTranslate), 0), mousePositionDelta * (modelRotateSensitivity * isRotate), vec3(1 + mouseScrollDelta * 0.1f));
		triangle2.SetMVPMatrix(modelShader);
		triangle2.DrawObject(GL_TRIANGLES, 132, GL_UNSIGNED_INT, 0);

		mouseScrollDelta = 0.0f;
		mousePositionDelta = vec2(0);

		if (isMouseOnWindow)
		{
			ray.castRay(ray.rayDirection(lastmousePosition, mainWindow, view, projection), 10);
		}

		glfwSwapBuffers(mainWindow.window);//交换缓冲
		glfwPollEvents();//检查是否有事件被触发并更新窗口状态以及调用对应的回调函数
	}

	/*………………………………程序结束后处理………………………………*/
	triangle1.ReleaseSpace();
	glfwTerminate();
	return 0;
}
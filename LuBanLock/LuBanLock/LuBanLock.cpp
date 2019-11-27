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

float modelReductionMultiple = 30;//模型缩小倍数

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

	vec3 position = vec3(0);//模型位置
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
	enum class MapType { DiffuseMap, SpecularMap };//贴图类型
	unsigned int diffuseMap;//漫反射贴图
	unsigned int specularMap;//高光贴图
	vector<vector<float>> verticesData;//顶点数据
	vector<vector<int>> dataIndices;//数据索引
	vector<vec3> animationPoint;//动画结点

	/*只包含顶点数据*/
	Model(vector<float> vertices_data)
	{
		for (size_t i = 0; i < vertices_data.size(); i++)
		{
			vertices_data[i] /= modelReductionMultiple;
		}
		verticesData.push_back(vertices_data);
		InitVBOandVAO();
	}
	/*包含顶点与索引数据*/
	Model(vector<float> vertices_data, vector<int> indices_data, vector<vec3>animationPoint)
	{
		for (size_t i = 0; i < vertices_data.size(); i++)
		{
			vertices_data[i] /= modelReductionMultiple;
		}
		for (size_t i = 0; i < indices_data.size(); i++)
		{
			--indices_data[i];
		}
		verticesData.push_back(vertices_data);
		dataIndices.push_back(indices_data);
		InitVBOandVAO();
		InitEBO();
		this->animationPoint.push_back(vec3(0));
		this->animationPoint.insert(this->animationPoint.end(), animationPoint.begin(), animationPoint.end());
	}

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
	void SetTransform(vec3 translateDelta, vec2 RotateAngle, vec3 scaleDelta)
	{
		//缩放
		model = scale(mat4(1), scaleDelta) * model;

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

	//设置动画
	void SetAnimation(float animationTime)
	{
		int animationIndex = 0;
		//确定动画索引
		while (animationTime >= ((animationIndex) * (1.0f / (animationPoint.size() - 1))))//如果动画时间大于某个动画结点对应的时间
		{
			++animationIndex;//动画索引自增
		}
		vec3 targetPosition =
			(
			(
				(animationTime - ((animationIndex - 1) * (1.0f / (animationPoint.size() - 1)))) / (1.0f / (animationPoint.size() - 1))
				)
				* (animationPoint[animationIndex] - animationPoint[animationIndex - 1])
				)
			+ animationPoint[animationIndex - 1];//获取移动插值
		model = translate(model, targetPosition - position);//获取物体目标位置
		position = targetPosition;//记录目标位置
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
	//~Model()
	//{
	//	/*释放空间*/
	//	delete[] vertices_data;
	//	delete[] indices_data;
	//	verticesData[0].clear();
	//	dataIndices[0].clear();
	//	verticesData.clear();
	//	dataIndices.clear();
	//}
};
/*设置视图*/
void SetView()
{
	view = translate(view, vec3(0, 0, -3));
}

/*设置投影*/
void SetProjection(Window window, bool isOrtho)
{
	projection = (isOrtho) ? ortho(-window.windowWidth / 1000, window.windowWidth / 1000, -window.windowHeight / 1000, window.windowHeight / 1000, 0.1f, 100.0f) : perspective(radians(45.0f), window.windowWidth / window.windowHeight, 0.1f, 100.0f);//设定投影视图是正交或透视
}

vector<Model*> models;//模型数组
/*初始化动画顺序*/
void InitAnimationSequence(vector<Model*> _models, vector<int> animation_sequence)
{
	for (size_t i = 0; i < _models.size(); i++)//记录所有模型
	{
		models.push_back(_models[animation_sequence[i]]);
	}
}

/*播放动画*/
void PlayAnimation(float animationTime)
{
	int animationSequenceIndex = 0;//动画顺序索引
	for (size_t i = 0; i < models.size(); i++)
	{
		//获取动画顺序索引
		if (animationTime >= (animationSequenceIndex + 1) * (1.0f / models.size()))
		{
			++animationSequenceIndex;
		}
	}
	animationTime -= (animationSequenceIndex) * (1.0f / models.size());
	models[animationSequenceIndex]->SetAnimation(animationTime / (1.0f / models.size()));//播放对应物体的动画
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
	/*………………………………加载着色器………………………………*/
	Shader modelShader("shader.vert", "shader.frag");//模型着色器

	/*………………………………模型部分………………………………*/
	Model model1({
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
		}, {
			vec3(1,0,0),
			vec3(1, -3, 0)
		});
	model1.ConfigAttribute(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);//解析顶点位置
	//model1.ConfigAttribute(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	//model1.ConfigAttribute(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	model1.UnBindObject();
	//
	Model model2({
		//-19.544, -4.003, 7.79,     0.0,0.0,0.0,  0.0,30.0,//左上前：
		//  19.614, -3.661, 7.79,     0.0,0.0,0.0,  30.0,30.0,//右上前：
		//  19.643, -3.664, 0.112,      0.0,0.0,0.0,  30.0,0.0,//右下前：
		//  -19.544, -4.024, 0.105,    0.0,0.0,0.0,  0.0,0.0,//左下前：
		//  //后面
		//  -19.611, 3.682, 7.769,     0.0,0.0,0.0,    0.0,30.0,//左上后
		//   19.547, 4.024, 7.769,      0.0,0.0,0.0,   30.0,30.0,//右上后
		//   19.576, 3.996, 0.101,      0.0,0.0,0.0,   30.0,0.0,//右下后
		//  -19.611, 3.661, 0.084,     0.0,0.0,0.0,    0.0, 0.0,//右下后
		//  //左面
		//  -19.611, 3.682, 7.769,     0.0,0.0,0.0,    0.0,30.0,
		//  -19.544, -4.003, 7.79,     0.0,0.0,0.0,  30.0,30.0,
		//  -19.544, -4.024, 0.105,    0.0,0.0,0.0,  30.0,0.0,
		//  -19.611, 3.661, 0.084,     0.0,0.0,0.0,    0.0,0.0,
		//  //右面
		//   19.547, 4.024, 7.769,      0.0,0.0,0.0,    0.0,30.0,
		//   19.614, -3.661, 7.79,     0.0,0.0,0.0,  30.0,30.0,
		//   19.643, -3.664, 0.112,      0.0,0.0,0.0,  30.0,0.0,
		//	19.576, 3.996, 0.101,      0.0,0.0,0.0,   0.0,0.0,
		//	//上面
		//	-19.611, 3.682, 7.769,     0.0,0.0,0.0,    0.0,30.0,
		//	19.547, 4.024, 7.769,      0.0,0.0,0.0,    30.0,30.0,
		//	19.614, -3.661, 7.79,     0.0,0.0,0.0,  30.0,0.0,
		//	-19.544, -4.003, 7.79,     0.0,0.0,0.0,  0.0,0.0,
		//	//下面
		//	-19.611, 3.661, 0.084,     0.0,0.0,0.0,    0.0,30.0,
		//	19.576, 3.996, 0.101,      0.0,0.0,0.0,    0.0,0.0,
		//	19.643, -3.664, 0.112,      0.0,0.0,0.0,  30.0,0.0,
		//	-19.544, -4.024, 0.105,    0.0,0.0,0.0,  0.0,0.0,

		 19.5767 ,7.7818 ,3.8303,
 -19.5858, 0.0946 ,-3.8536,
 -19.5692, 0.0947, 3.8313,
 -19.5692 ,7.7818, 3.8313,
  19.5605, 7.7774 ,-3.8550,
  19.5530, 0.0948 ,-3.8550,
  19.5692 ,0.0945 ,3.8304,
 -19.5859 ,7.7774 ,-3.8536,
		}, {
7, 6 ,5,
 3, 8 ,2,
 2, 8 ,6,
 3, 2 ,7,
 4, 7 ,1,
 8 ,4, 5,
 4 ,3, 7,
 4, 1 ,5,
 5 ,1 ,7,
 3 ,4 ,8,
 8 ,5, 6,
 2, 6, 7,
		}, {
			vec3(1,0,0),
			vec3(1,1,0)
		});
	model2.ConfigAttribute(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	//model2.ConfigAttribute(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	//model2.ConfigAttribute(2, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(6 * sizeof(float)));
	model2.UnBindObject();
	//
	Model model3({
		-0.0189, 19.6172, 7.4922,
		   3.8369 ,19.6026 ,7.4503,
		  -0.0302 ,19.6352, 3.6277,
		   3.8363 ,-19.6010, 7.4258,
		  -0.0302 ,-19.6194, 3.6274,
		   3.8324, -19.6067, 3.5938,
		   3.8285, 0.0000, 3.6493,
		   3.8351, 7.9566, 3.6509,
		   3.8384, 7.8765, 7.3445,
		  -3.8765, -0.0000, 7.5260,
		  -0.0188, 7.8740, 7.5645,
		  -3.8765, 7.8741, 7.5260,
		  -3.8904, 7.9007, 3.6969,
		  -3.8904, 0.0206 ,3.7064,
		  -0.0233, 0.0206 ,3.6274,
		  -0.0302, 7.8740 ,3.6276,
		   3.8314, -7.9173 ,3.6691,
		   3.8384, -7.8741 ,7.4586,
		  -3.8763, -19.6047 ,7.4929,
		  -0.0188, -7.8741 ,7.5645,
		  -3.8765, -7.8741, 7.5260,
		  -3.8829, -19.6068, 3.6613,
		  -3.8940, -7.8739 ,3.6619,
		  -3.8829, 19.6172 ,3.6614,
		  -3.8765, 19.6091, 7.4933,
		   3.8206, -19.6047 ,-0.2374,
		  -0.0613, -7.9570,-0.2604,
		   3.8180, -7.9321 ,-0.1948,
		  -0.0416, 19.6173 ,-0.2368,
		  -0.0415, 7.9462, -0.2371,
		  -3.8831, 19.6144 ,-0.1707,
		  -0.0415, -19.6067, -0.2371,
		   3.8219, 19.6319, 3.5946,
		   3.8239, 19.6235 ,-0.2074,
		   3.8206, 7.9579 ,-0.2374,
		  -3.8864, 7.9792 ,-0.1705,
		   3.8384, 0.0000 ,7.4586,
		  -0.0188, -0.0001, 7.5645,
		  -0.0302, -7.8741 ,3.6274,
		  -0.0189, -19.6068, 7.4921,
		  -3.8807, -7.8741 ,-0.2534,
		  -3.8810, -19.5923 ,-0.2286,
		  -3.8822, -0.0841 ,-0.1674,
		  -0.0695, -0.1052 ,-0.2032
		}, {
1, 2, 3 ,
 4, 5, 6,
 7, 8, 9,
 10, 11, 12,
 13, 14, 12,
 14, 10, 12,
 7, 15, 16,
 8, 7, 16,
 6, 17, 4,
 18, 4, 17,
 2, 9, 8,
 19, 20, 21,
 2, 11, 9,
 22, 19, 23,
 24, 13, 25,
 12, 25, 13,
 26, 27, 28,
 29, 30, 31,
 5, 32, 26,
 17, 26, 28,
 3, 31, 24,
 33, 8, 34,
 16, 35, 8,
 13, 31, 36,
 18, 7, 37,
 20, 38, 10,
 21, 10, 23,
 39, 15, 17,
 7, 17, 15,
 25, 1, 3,
 22, 5, 19,
 9, 11, 37,
 15, 14, 16,
 14, 13, 16,
 4, 20, 40,
 25, 12, 11,
 41, 27, 42,
 32, 42, 27,
 35, 30, 34,
 42, 32, 5,
 17, 27, 39,
 34, 29, 3,
 36,30, 16,
 37, 38, 20,
 41, 43, 44,
 41, 44, 27,
 15, 44, 43,
 15, 39, 27,
 15, 27, 44,

 23, 14, 43,
 41, 42, 23,
 43, 41, 23,

 10, 14, 23,
 26, 32, 27,
 17, 6,26,
 17, 28, 27,

 42, 22, 23,

 19, 21, 23,
 13, 24, 31,
 30, 36, 31,
 30, 29, 34,
 8, 35, 34,
 8, 33, 2,
 9, 37, 7,
 18, 17, 7,
 4, 18, 20,
 19, 40, 20,
 10, 21, 20,
 20, 18, 37,
 10, 38, 11,
 11, 38, 37,
 11, 1, 25,
 2, 1, 11,
 2, 33, 3,
 3, 24, 25,
 3, 33, 34,
 3, 29, 31,
 16, 30, 35,
 16, 13, 36,
 43, 14, 15,
 26, 6, 5,
 5, 40, 19,
 4, 40, 5,
 5, 22, 42
		}, {
			vec3(2,0,0),
			vec3(2, 2, 0)
		});
	model3.ConfigAttribute(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	//model1.ConfigAttribute(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	//model1.ConfigAttribute(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	model3.UnBindObject();

	Model model4({
	  0.0402, -3.8536 ,4.0228,
  0.0402, -0.0409, 0.1232,
  0.0400 ,-3.8527, 0.1224,
  3.9385 ,-3.8547, -7.8488,
  3.9172 ,-0.0408, -3.9513,
  3.9169 ,-0.0387, -7.8475,
  3.9174 ,-3.8541, -3.9512,
  3.9182, 3.8483, 4.0243,
  3.9388 ,-0.0398, 4.0232,
  3.9177, -0.0396, 7.9985,
  0.0397 ,-0.0387, -19.5381,
  0.0400 ,3.8478 ,-7.8486,
  0.0399, 3.8484 ,-19.5371,
  0.0403, -0.0396 ,19.6821,
  0.0402 ,-0.0271, 7.9983,
  0.0402 ,-3.8527, 7.9985,
  3.9389 ,-3.8532, 0.1216,
  3.9177 ,-0.0408, 0.1215,
  0.0401, -3.8536, -7.8494,
  0.0610 ,-3.8527, -19.5377,
  0.0402, 3.8483, 7.9985,
  0.0403 ,3.8482 ,19.6821,
  3.9172 ,3.8490, -7.8486,
  3.9391, -3.8547, 4.0231,
  3.9393 ,-3.8546, 7.9983,
  3.9184 , -0.0396, 19.6824,
  3.9184 ,-3.8541, 19.6814,
  7.7620 ,-3.8527, 19.6810,
  7.7610 ,-3.8541, 7.9988,
  7.7607, -3.8541, 4.0232,
  7.7603 ,-3.8541, 0.0906,
  7.7599 ,-3.8541, -3.9513,
  7.7583, -3.8527, -19.5398,
  7.7598 ,-3.8541 ,-7.8484,
  3.9161 ,-0.0387, -19.5410,
  7.7586 ,-0.0387, -19.5399,
  7.7375 ,3.8478, -19.5392,
  7.7386, 3.8487, -7.8484,
  3.9161 ,3.8487, -19.5388,
  7.7385 ,3.8478 ,-3.9517,
  3.9177, 3.8478, -3.9515,
  7.7603, -0.0394, -3.9517,
  7.7604 ,-0.0408, 0.1195,
  7.7610, -0.0398, 4.0240,
  7.7605 ,3.8483 ,4.0242,
  3.9182, 3.8485 ,7.9988,
  7.7399, 3.8484 ,7.9988,
  3.9184 ,3.8484, 19.6818,
  7.7622 ,-0.0396, 19.6815,
  7.7405 ,3.8482, 19.6814,
  3.9167 ,-3.8520, 0.1210,
  0.0399 ,-0.0387, -7.8483,
  3.9161, -3.8541 ,-19.5388,
  0.0616 ,-3.8536, 19.6819,
  3.9171 ,-3.8522, 0.1222,
  3.9171 ,3.8475 ,-7.8492,
  3.9179 ,-3.8522, 7.9988,
  0.0400 ,-0.0398, 4.0227,
		}, {
	  1, 2 ,3 ,
	 4, 5, 6,
	 5, 4, 7,
	 8, 9, 10,
	 11, 12, 13,
	 14, 15, 16,
	 17, 18, 5,
	 19, 11, 20,
	 21, 14, 22,
	 6, 5, 23,
	 10, 9, 24,
	 10, 24, 25,
	 26, 27, 28,
	 28, 25, 29,
	 27, 25, 28,
	 24, 30, 29,
	 30, 17, 31,
	 30, 24, 17,
	 17, 32, 31,
	 7, 4, 32,
	 4, 33, 34,
	 35, 36, 33,
	 35, 37, 36,
	 37, 23, 38,
	 39, 23, 37,
	 23, 40, 38,
	 41, 5, 42,
	 5, 43, 42,
	 43, 9, 44,
	 43, 18, 9,
	 9, 45, 44,
	 8, 46, 47,
	 46, 48, 47,
	 26, 49, 50,
	 47, 50, 49,
	 44, 45, 47,
	 36, 38, 42,
	 36, 37, 38,
	 1, 24, 9,
	 3, 17, 24,
	 //51, 3,3 18,17 17,16    //这里估计有问题,改用下面的2个

	51,3,18,
	3,18,17,

	 2, 9, 18,
	 52, 4, 6,
	 19, 20, 4,
	 20, 35, 53,
	 13, 39, 35,
	 13, 23, 39,
	 12, 52, 23,
	 15, 46, 10,
	 21, 22, 48,
	 22, 26, 48,
	 54, 27, 26,
	 54, 25, 27,
	 16, 10, 25,
	 55, 17, 3,
	 17, 55, 3,
	 56, 23, 12,
	 23, 56, 12,
	 57, 25, 16,
	 25, 57, 16,
	 10, 46, 8,
	 2, 58, 9,
	 3, 2, 18,
	 1, 58, 2,
	 9, 58, 1,
	 24, 1, 3,
	 5, 7, 17,
	 17, 3, 51,
	 23, 41, 40,
	 5, 41, 23,
	 52, 6, 23,
	 52, 19, 4,
	 19, 52, 11,
	 11, 52, 12,
	 35, 11, 13,
	 20, 11, 35,
	 35, 39, 37,
	 33, 53, 35,
	 13, 12, 23,
	 15, 21, 46,
	 16, 15, 10,
	 16, 54, 14,
	 21, 15, 14,
	 47, 45, 8,
	 48, 50, 47,
	 48, 46, 21,
	 5, 18, 43,
	 22, 14, 26,
	 50, 48, 26,
	 26, 14, 54,
	 28, 49, 26,
	 49, 44, 47,
	 42, 40, 41,
	 38, 40, 42,

	 //,s o
	  32, 34, 42,
	  49, 28, 29,
	  44, 30, 31,
	  42, 43, 31,
	  29, 30, 44,
	  29, 44, 49,
	  31, 43, 44,
	  31, 32, 42,
	  34, 36, 42,
	  34, 33, 36,

	  //,s 1
	   29, 25, 24,
	   17, 7, 32,
	   4, 34, 32,
	   54, 16, 25,
	   4, 53, 33,
	   20, 53, 4,
	   9, 8, 45,
		}, {
			vec3(-1,0,0),
			vec3(-1, 0.5f, 0)
		});
	model4.ConfigAttribute(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	//model1.ConfigAttribute(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	//model1.ConfigAttribute(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	model4.UnBindObject();

	Model model5({
		/*3.9288, -7.7851, 3.8331,
		  3.9289 ,-3.9975, -0.0923,
		  3.9288 ,-7.7852 ,-0.0924,
		 -0.0703 ,-3.9975 ,-0.0924,
		 -0.0698 ,-7.7863 ,-0.0924,
		 -19.5767 ,-7.7851, 3.8351,
		 -7.9351 ,-3.9975, 3.8347,
		 -19.5803 ,-0.1051, 3.8352,
		 -0.0804 ,-7.7844, 3.8336,
		 -3.8414 ,-3.9974 ,-0.0924,
		 -7.9336 ,-3.9975, -3.8517,
		 -0.0708 ,-3.9976 ,3.8336,
		 -7.9377 ,-0.1051, -3.8516,
		 -19.5880 ,-0.1051, -3.8511,
		  3.9285 ,-0.1051 ,-3.8521,
		 -3.8452 ,-0.1051, -0.0927,
		  3.9286 ,-0.1051, -0.0925,
		  7.9814 ,-0.1051 ,-3.8527,
		  19.5995 ,-0.1051, 3.8323,
		  19.5971 ,-0.1051, -3.8530,
		  3.9288 ,-3.9976 ,3.8335,
		  7.9863 ,-3.9976, 3.8334,
		  7.9824 ,-3.9976, -3.8529,
		  3.9288 ,-3.9976, -3.8525,
		 -19.5878, -3.9976, -3.8511,
		 -19.5831, -7.7851, -0.0936,
		 -7.9361 ,-0.1051, 3.8345,
		 -3.8461 ,-0.1051, -3.8518,
		 -3.8447 ,-3.9976 ,-3.8522,
		  19.5979 ,-3.9975, -3.8533,
		  19.6011 ,-3.9975 ,3.8318,
		  19.5974 ,-7.7844 ,-3.8531,
		  19.5995 ,-7.7844, 3.8323,
		  7.9839, -0.1051 ,3.8329,
		  19.5983 ,-7.7844 - 0.0938,
		 -19.5878 ,-7.7851 - 3.8511,
		 -0.0796 ,-7.7847 - 3.8518,
		  3.9290 ,-7.7847 - 3.8521,*/
			   -19.5803, -0.1051, 3.8352,
		 19.5995 ,-7.7844 ,3.8323,
		 19.5974 ,-7.7844 ,-3.8531,
		-7.9336, -3.9975 ,-3.8517,
		-7.9351 ,-3.9975 ,3.8347,
		-19.5880, -0.1051, -3.8511,
		-7.9361 ,-0.1051 ,3.8345,
		-3.8461, -0.1051 ,-3.8518,
		 3.9285, -0.1051 ,-3.8521,
		 19.5979, -3.9975 ,-3.8533,
		 3.9288 ,-3.9976, -3.8525,
		 3.9286 ,-0.1051, -0.0925,
		-3.8414 ,-3.9974, -0.0924,
		-3.8452, -0.1051 ,-0.0927,
		 19.5995, -0.1051 ,3.8323,
		 3.9288, -7.7851 ,3.8331,
		 7.9863 ,-3.9976 ,3.8334,
		 7.9839 ,-0.1051,3.8329,
		 7.9814 ,-0.1051 ,-3.8527,
		-0.0708, -3.9976 ,3.8336,
		-19.5767, -7.7851, 3.8351,
		-19.5878, -7.7851 ,-3.8511,
		 7.9824 ,-3.9976 ,-3.8529,
		 3.9288 ,-3.9976 ,3.8335,
		 3.9289 ,-3.9975 ,-0.0923,
		-0.0703, -3.9975 ,-0.0924,
		-0.0698 ,-7.7863 ,-0.0924,
		-0.0804, -7.7844, 3.8336,
		-7.9377, -0.1051, -3.8516,
		 19.5971, -0.1051, -3.8530,
		 19.6011 ,-3.9975 ,3.8318,
		-3.8447 ,-3.9976 ,-3.8522,
		-19.5831, -7.7851, -0.0936,
		 19.5983, -7.7844, -0.0938,
		-0.0796, -7.7847,-3.8518,
		 3.9290, -7.7847 ,-3.8521,
		 3.9288 ,-7.7852 ,-0.0924,
		-19.5878, -3.9976, -3.8511
		}, {
			/*1 , 2,  3,
			2 , 4 , 5,
			2, 5, 3,
			6, 7, 8,
			6, 9, 7,
			7, 10, 11,
			12, 10, 7,
			13, 14, 8,
			15, 16, 17,
			18, 19, 20,
			2, 21, 22,
			23, 2, 22,
			2, 23, 24,
			4, 12, 9,
			14, 25, 26,
			27, 7, 13,
			28, 29, 10,
			30, 23, 20,
			17, 2, 24,
			4, 16, 10,
			17, 16, 4,
			17, 4, 2,
			30, 31, 32,
			19, 31, 20,
			33, 22, 1,
			34, 22, 31,
			19, 34, 31,
			34, 23, 22,

			28, 15, 29,
			35, 33, 1,

			7, 27, 8,
			9, 12, 7,
			9, 5, 4,
			7, 11, 13,
			12, 4, 10,
			10, 16, 28,
			6, 8, 14,
			1, 21, 2,
			22, 21, 1,
			22, 33, 31,
			31, 30, 20,
			31, 33, 35,
			18, 34, 19,
			10, 29, 11,
			8, 27, 13,
			24, 15, 17,
			15, 28, 16,
			23, 18, 20,

			14, 13, 11,
			15, 24, 29,

			34, 18, 23,

			5, 6, 26,
			3, 32, 35,
			26, 36, 37,
			30, 32, 38,
			24, 23, 38,
			23, 30, 38,
			38, 37, 24,
			37, 11, 29,
			37, 29, 24,
			1, 3, 35,
			3, 5, 38,
			3, 38, 32,
			5, 37, 38,
			5, 9, 6,
			37, 5, 26,
			11, 36, 25,
			11, 37, 36,
			11, 25, 14,

			25, 36, 26,
			26, 6, 14,
			35, 32, 31,*/

			16, 25,37,
25,26,27,
25,27,37,
21,5,1,
21,28,5,
5,13,4,
20,13,5,
29,6,1,
9,14,12,
19,15,30,
25,24,17,
23,25,17,
25,23,11,
26,20,28,
6,38,33,
7,5,29,
8,32,13,
10,23,30,
12,25,11,
26,14,13,
12,14,26,
12,26,25,
10,31,3,
15,31,30,
2,17,16,
18,17,31,
15,18,31,
18,23,17,
//s,o
8,9,32,
34,2,16,
//s,1
5,7,1,
28,20,5,
28,27,26,
5,4,29,
20,26,13,
13,14,8,
21,1,6,
16,24,25,
17,24,16,
17,2,31,
31,10,30,
31,2,34,
19,18,15,
13,32,4,
1,7,29,
11,9,12,
9,8,14,
23,19,30,
//s,o
6,29,4,
9,11,32,
//s,1
18,19,23,
//s,o
27,21,33,
37,3,34,
33,22,35,
10,3,36,
11,23,36,
23,10,36,
36,35,11,
35,4,32,
35,32,11,
16,37,34,
37,27,36,
37,36,3,
27,35,36,
27,28,21,
35,27,33,
4,22,38,
4,35,22,
4,38,6,
//s,1
38,22,33,
33,21,6,
34,3,31,
		}, {
			vec3(0,1,0),
			vec3(0, 1, 3)
		});
	model5.ConfigAttribute(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	//model1.ConfigAttribute(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	//model1.ConfigAttribute(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	model5.UnBindObject();

	Model model6({
	 -3.9710, -3.8579, 0.0487,
	 -3.9709, -0.0396 ,0.0487,
	 -3.9708, -3.8579, 4.0624,
	 -3.9710 ,-0.0387, -7.8927,
	 -3.9713, -0.0391, -3.9598,
	 -3.9717, -3.8591, -7.8937,
	 -3.9714, -3.8579 ,-3.9567,
	 -3.9698, -0.0387, 7.9373,
	 -3.9706, -0.0392, 4.0638,
	 -3.9711, 3.8502, 4.0643,
	 -0.0954, 3.8496, -7.8947,
	 -0.0943, -0.0387, -7.8944,
	 -0.0977, -0.0387 ,-19.5741,
	 -0.0890, -3.8553, 7.9385,
	 -0.0889, -0.0387 ,7.9380,
	 -0.0884, -0.0387, 19.6425,
	 -0.0955, -3.8569, -7.8951,
	 -0.0886, 3.8496 ,19.6422,
	 -0.0891, 3.8502, 7.9388,
	 -3.9716, 3.8496, -3.9596,
	 -3.9715, 3.8510, -7.8940,
	 -3.9704, -3.8591, 7.9390,
	 -7.8013, -3.8579, 4.0629,
	 -7.8016, -3.8579, 0.0552,
	 -7.8016 ,-3.8579, -3.9554,
	 -7.8018, -3.8579, -7.8917,
	 -7.8021, -3.8553, -19.5725,
	 -7.8021, -0.0387, -19.5729,
	 -3.9720, -0.0387, -19.5753,
	 -3.9719, -3.8579, -19.5739,
	 -3.9628, 3.8505 ,-19.5736,
	 -7.8018, 3.8505, -7.8917,
	 -7.8021, 3.8496,-19.5724,
	 -7.8015, 3.8496, -3.9584,
	 -7.8018 ,-0.0401, -3.9615,
	 -7.8016, -0.0393, 0.0552,
	 -7.8015, -0.0402, 4.0658,
	 -7.8012, 3.8496, 4.0649,
	 -7.8013, 3.8505, 7.9380,
	 -3.9705, 3.8510, 7.9384,
	 -3.9691, 3.8505, 19.6423,
	 -7.8008, 3.8496, 19.6426,
	 -7.8010 ,-0.0387 ,19.6423,
	 -7.8008, -3.8553, 19.6425,
	 -3.9691, -3.8579 ,19.6423,
	 -7.8012, -3.8547, 7.9302,
	 -0.0988 ,-3.8553, -19.5740,
	 -0.0988, 3.8502, -19.5736,
	 -0.0886, -3.8570 ,19.6422,
	 -3.9690 ,-0.0387, 19.6425,
		}, {
1, 2, 3,
 4, 5, 6,
 7, 6, 5,
 8, 9, 10,
 11, 12, 13,
 14, 15, 16,
 5, 2, 1,
 13, 12, 17,
 18, 16, 19,
 20, 5, 21,
 3, 9, 8,
 22, 3, 8,
 23, 1, 3,
 24, 25, 1,
 25, 6, 7,
 26, 27, 6,
 28, 29, 30,
 28, 31, 29,
 32, 21, 33,
 33, 21, 31,
 34, 20, 21,
 34, 35, 20,
 35, 36, 2,
 36, 37, 2,
 38, 10, 9,
 38, 39, 40,
 39, 41, 40,
 42, 43, 41,
 43, 44, 45,
 46, 22, 44,
 44, 22, 45,
 23, 3, 22,
 43, 42, 39,
 39, 38, 37,
 35, 34, 32,
 32, 33, 28,
 4, 6, 12,
 6, 47, 17,
 30, 13, 47,
 31, 48, 13,
 11, 48, 31,
 21, 4, 12,
 40, 19, 8,
 40, 18, 19,
 41, 16, 18,
 45, 49, 16,
 45, 22, 49,
 22, 8, 14,
 2, 9, 3,
 1, 7, 5,
 16, 15, 19,
 16, 49, 14,
 40, 10, 38,
 19, 15, 8,
 10, 40, 8,
 9, 37, 38,
 37, 9, 2,
 2, 5, 35,
 5, 4, 21,
 8, 15, 14,
 13, 48, 11,
 17, 47, 13,
 13, 29, 31,
 28, 33, 31,
 30, 29, 13,
 30, 27, 28,
 31, 21, 11,
 21, 32, 34,
 39, 42, 41,
 40, 41, 18,
 43, 50, 41,
 41, 50, 16,
 16, 50, 45,
 45, 50, 43,
 22, 46, 23,
 23, 24, 1,
 25, 7, 1,
 22, 14, 49,
 25, 26, 6,
 6, 30, 47,
 27, 30, 6,
 6, 17, 12,
 12, 11, 21,
 32, 28, 35,
 39, 37, 43,
 //s o
  26, 28, 27,
  23, 46, 37,
  37, 36, 23,
  46, 43, 37,
  46, 44, 43,
  36, 24, 23,
  26, 35, 28,
  26, 25, 35,
  25, 36, 35,
  25, 24, 36,
  //s 1
   35, 5, 20,
		}, {
			vec3(1,1,1),
			vec3(1, -2, 2)
		});
	model6.ConfigAttribute(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	//model1.ConfigAttribute(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	//model1.ConfigAttribute(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	model6.UnBindObject();
	/*………………………………贴图部分………………………………*/
	stbi_set_flip_vertically_on_load(true);//设定加载图片时反转y轴
	modelShader.use();
	model1.diffuseMap = model1.GenTexture("Resources/Textures/wall.jpg");
	model1.specularMap = model1.GenTexture("Resources/Textures/awesomeface.png");

	modelShader.setInt("texture1", 0);
	//modelShader.setInt("texture2", 1);

	/*………………………………测试设置………………………………*/
	SetTest();

	float time = 0;

	/*………………………………初始化部分………………………………*/
	InitAnimationSequence({ &model1,&model2, &model3, &model4, &model5, &model6 }, { 1,0,2,3,4,5 });

	modelShader.use();
	SetView();
	SetProjection(mainWindow, false);

	/*………………………………渲染循环………………………………*/
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	while (!glfwWindowShouldClose(mainWindow.window))//glfwWindowShouldClose函数将检查GLFW是否被要求退出
	{
		mainWindow.SetRenderBuffer();
		GetDeltaTime();

		if (time < 1)
		{
			PlayAnimation(time);
			time += 0.001f;
		}

		model1.BindTexture(Model::MapType::DiffuseMap);
		model1.BindTexture(Model::MapType::SpecularMap);

		model1.SetTransform(vec3(mousePositionDelta * (modelTranslateSensitivity * isTranslate), 0), mousePositionDelta * (modelRotateSensitivity * isRotate), vec3(1 + mouseScrollDelta * 0.1f));
		model1.SetMVPMatrix(modelShader);
		model1.DrawObject(GL_TRIANGLES, 132, GL_UNSIGNED_INT, 0);

		model2.BindTexture(Model::MapType::DiffuseMap);
		model2.BindTexture(Model::MapType::SpecularMap);

		model2.SetTransform(vec3(mousePositionDelta * (modelTranslateSensitivity * isTranslate), 0), mousePositionDelta * (modelRotateSensitivity * isRotate), vec3(1 + mouseScrollDelta * 0.1f));
		model2.SetMVPMatrix(modelShader);
		model2.DrawObject(GL_TRIANGLES, 12 * 3, GL_UNSIGNED_INT, 0);

		model3.SetTransform(vec3(mousePositionDelta * (modelTranslateSensitivity * isTranslate), 0), mousePositionDelta * (modelRotateSensitivity * isRotate), vec3(1 + mouseScrollDelta * 0.1f));
		model3.SetMVPMatrix(modelShader);
		model3.DrawObject(GL_TRIANGLES, 84 * 3, GL_UNSIGNED_INT, 0);

		model4.SetTransform(vec3(mousePositionDelta * (modelTranslateSensitivity * isTranslate), 0), mousePositionDelta * (modelRotateSensitivity * isRotate), vec3(1 + mouseScrollDelta * 0.1f));
		model4.SetMVPMatrix(modelShader);
		model4.DrawObject(GL_TRIANGLES, 112 * 3, GL_UNSIGNED_INT, 0);

		model5.SetTransform(vec3(mousePositionDelta * (modelTranslateSensitivity * isTranslate), 0), mousePositionDelta * (modelRotateSensitivity * isRotate), vec3(1 + mouseScrollDelta * 0.1f));
		model5.SetMVPMatrix(modelShader);
		model5.DrawObject(GL_TRIANGLES, 72 * 3, GL_UNSIGNED_INT, 0);

		model6.SetTransform(vec3(mousePositionDelta * (modelTranslateSensitivity * isTranslate), 0), mousePositionDelta * (modelRotateSensitivity * isRotate), vec3(1 + mouseScrollDelta * 0.1f));
		model6.SetMVPMatrix(modelShader);
		model6.DrawObject(GL_TRIANGLES, 96 * 3, GL_UNSIGNED_INT, 0);

		mouseScrollDelta = 0.0f;
		mousePositionDelta = vec2(0);

		glfwSwapBuffers(mainWindow.window);//交换缓冲
		glfwPollEvents();//检查是否有事件被触发并更新窗口状态以及调用对应的回调函数
	}

	/*………………………………程序结束后处理………………………………*/
	//model1.ReleaseSpace();
	//model2.ReleaseSpace();
	//model3.ReleaseSpace();
	//model4.ReleaseSpace();

	glfwTerminate();
	return 0;
}
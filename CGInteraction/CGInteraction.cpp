#include<GL/glew.h>
#include <GLFW/glfw3.h>
#include <math.h>
#include<iostream>
#include <iostream>
//#include <freeglut.h>
//#include <GL/glut.h>
//#include <GL/glut.h>
//#include <GLv36/glut.h>
#include<glut.h>
#include <vector>
#include "math3d.h"
#include "Selection.h"
using namespace std;

static int CompilerShader(unsigned int type, const string& source) {
	unsigned int id = glCreateShader(type);
	const char* src = source.c_str();  // "source.c_str()" is just a pointer to the begining of the data
	// "source.c_str()" equals to "&source[0]"
	// Make sure the "source" is stille alive in case of its out-of-scope range
	// The difference between "source.c_str()" and "GetStirng().c_str()" is that the former is represents
	//explict memory while the latter is dynamic and may be sometimes rubbished
	glShaderSource(id, 1, &src, nullptr);
	glCompileShader(id);

	//Error handling
	int result;
	glGetShaderiv(id, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE) // Press F12 to see its value is actually 0
	{
		int length;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
		char* message = (char*)alloca(length *sizeof(char));
		// char message[length] is illegal because the length is variable whilt C++ needs a const here
		glGetShaderInfoLog(id, length, &length, message);
		cout << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex " : "fragment ") << "shader" <<   endl;
		cout << message << endl;
		glDeleteShader(id);
		return 0;
	}
	return id;
}

static unsigned int CreateShader(const string& vertexShader, const string& fragmentShader) { 
	unsigned int programe = glCreateProgram();  // "unsigned int" equals to "GLuint"
	unsigned int vShder = CompilerShader(GL_VERTEX_SHADER, vertexShader);
	unsigned int fShder = CompilerShader(GL_FRAGMENT_SHADER, fragmentShader);

	glAttachShader(programe, vShder);
	glAttachShader(programe, fShder);
	glLinkProgram(programe);
	glValidateProgram(programe);

	glDeleteShader(vShder); // Also you can use glDetachShader()
	glDeleteShader(fShder);

	return programe;
}


///////////////////////////////////
Selection selection;

M3DVector2f left_bottom, right_top;
bool bool_select_area = false;



int viewport[4];

static M3DVector3f corners[] = {
	-25, 25, 25, 
	25, 25, 25, 
	25, -25, 25, 
	-25, -25, 25,
	-25, 25, -25, 
	25, 25, -25, 
	25, -25, -25,
	-25, -25, -25
};


void init(void) 
{
  glClearColor (0.0, 0.0, 0.0, 0.0);
  glShadeModel (GL_FLAT);
}

void display(void)
{
   M3DMatrix44f mat_proj, mat_modelview;

   int width = glutGet( GLUT_WINDOW_WIDTH ), height = glutGet( GLUT_WINDOW_HEIGHT );
   

   glViewport (0, 0, (GLsizei) width, (GLsizei) height); 

   glGetIntegerv(GL_VIEWPORT, viewport);

   glClear (GL_COLOR_BUFFER_BIT);

   glPushAttrib(GL_POLYGON_BIT);
   glPolygonMode( GL_FRONT_AND_BACK, GL_LINE);

   glMatrixMode (GL_PROJECTION);
   glLoadIdentity ();
   gluPerspective(65.0, (GLfloat) width/(GLfloat) height, 1.0, 300);

   // »ñÈ¡Í¶Ó°¾ØÕó
   glGetFloatv(GL_PROJECTION_MATRIX, mat_proj);

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   gluLookAt(0, 0, 70, 0, 0, 0, 0, 1, 0);
   glColor4f(0.1, 0.4, 0.6, 0.7);
   glPushMatrix();

		// »ñÈ¡Ä£ÐÍÊÓÍ¼¾ØÕó
		glGetFloatv(GL_MODELVIEW_MATRIX, mat_modelview);

		glBegin( GL_QUADS );
		glVertex3fv(corners[0]);
		glVertex3fv(corners[1]);
		glVertex3fv(corners[5]);
		glVertex3fv(corners[4]);

		glVertex3fv(corners[4]);
		glVertex3fv(corners[7]);
		glVertex3fv(corners[3]);
		glVertex3fv(corners[0]);

		glVertex3fv(corners[3]);
		glVertex3fv(corners[2]);
		glVertex3fv(corners[6]);
		glVertex3fv(corners[7]);

		glVertex3fv(corners[7]);
		glVertex3fv(corners[6]);
		glVertex3fv(corners[5]);
		glVertex3fv(corners[4]);

		glVertex3fv(corners[5]);
		glVertex3fv(corners[1]);
		glVertex3fv(corners[2]);
		glVertex3fv(corners[6]);

		glVertex3fv(corners[1]);
		glVertex3fv(corners[2]);
		glVertex3fv(corners[3]);
		glVertex3fv(corners[0]);
		glEnd();

   glPopMatrix();
   glPopAttrib();
   
   // ÅäÖÃ

   selection.set_config( corners, 8, left_bottom, right_top, mat_modelview, mat_proj, viewport);

   /************************************************************************/
   /* ¹¹ÔìÒ»¸öÐÂµÄ»·¾³                                                                     */
   /************************************************************************/
   
   if( bool_select_area ){
	   
	   selection.draw_area();
	   selection.highlight_selected_pts();

   }
   glutSwapBuffers();
}

void reshape (int w, int h)
{
   glViewport (0, 0, (GLsizei) w, (GLsizei) h); 
}

void mouse(int button, int state, int x, int y)
{
	int width = glutGet( GLUT_WINDOW_WIDTH ), height = glutGet( GLUT_WINDOW_HEIGHT );

	if( button == GLUT_LEFT_BUTTON ){

		if(state == GLUT_DOWN){
			bool_select_area = true;
		}else if ( state == GLUT_UP)
		{
			bool_select_area = false;
		}

		m3dLoadVector2(left_bottom, x, height - y);
		m3dLoadVector2(right_top, x, height - y);
	}

	glutPostRedisplay();
}

void motion(int x, int y)
{
	int width = glutGet( GLUT_WINDOW_WIDTH ), height = glutGet( GLUT_WINDOW_HEIGHT );

	if( bool_select_area ){
		m3dLoadVector2(right_top, x, height - y);
	}

	glutPostRedisplay();

}

/* ARGSUSED1 */
void keyboard (unsigned char key, int x, int y)
{

}

int main(int argc, char** argv)
{
   glutInit(&argc, argv);
   glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB);
   glutInitWindowSize (500, 500); 
   glutInitWindowPosition (100, 100);
   glutCreateWindow (argv[0]);
   init ();
   glutDisplayFunc(display); 
   glutReshapeFunc(reshape);
   glutKeyboardFunc(keyboard);
   glutMouseFunc(mouse);
   glutMotionFunc(motion);

   glutMainLoop();
   return 0;
}











//
//int main(void)
//{
//	GLFWwindow* window;
//
//	/* Initialize the library */
//	if (!glfwInit())
//		return -1;
//
//	//Initialize with glew.h is vain here bacause it should be innitialized only after the valid window context exists!
//	//glewInit();
//
//	if (glewInit() != GLEW_OK)
//	{
//		std::cout << "Error(Don't worry for this is just a test!)" << std::endl;  // Got an Error printing 
//		// Reason: Because the glewInit() should be after creating a valid rendering context
//		//As documentation said: creat a valid OpenGL rendering context and use glewInit()
//		// So put it below the valid rendering context to solve this problem
//
//		/*
//		1. Shader is just a programme that runs on your GPU.
//2. Vertex shader will be called three times in drawing a tirangle while the fragment shader will be called thousands of times which depends on the size of your drawing object.
//3. The fragment shader is the programme that determines which color this pixcel should be.
//4. Shder also runs on a state machine.
//5.  In C++, the string does not need to  connect with "+", directly using "a" "a" " " is OK to make them in one string
//		*/
//	}
//
//	/* Create a windowed mode window and its OpenGL context */
//	window = glfwCreateWindow(640, 480, "First Learning OpenGL", NULL, NULL);
//	if (!window)
//	{
//		glfwTerminate();
//		return -1;
//	}
//
//	/* Make the window's context current */
//	glfwMakeContextCurrent(window);
//
//	if (glewInit() != GLEW_OK)
//	{
//		std::cout << "Error Again!!!" << std::endl;  // Will not an Error printing 
//	}
//	std::cout << "Your current GL version is : " << glGetString(GL_VERSION) << std::endl;
//
//	float position[6] = {
//		-0.5f, 0.5f,
//		0.5f, 0.5f,
//		0, -0.5f
//	};
//	unsigned int buffer;
//	glGenBuffers(1, &buffer);
//	glBindBuffer(GL_ARRAY_BUFFER, buffer);
//	glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), position, GL_STATIC_DRAW);
//	glEnableVertexAttribArray(0);
//	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8, 0);
//
//	string vertexShader = // These are the shaders!!!
//		"#version 330 core \n" // this make sure that we are using the simple but latest version of glsl 
//		"\n"
//		"layout(location = 0) in vec4 position;\n" // The location is the first index in glVertexAttribPointer
//		// Use vec4 though we set vec2 in the function of glVertexAttribPointer() because the position we get is gonna be the typee of vec4
//		"\n"
//		"void main()\n"
//		"\n"
//		"{\n"
//		"gl_Position = position;\n"
//		"}\n";
//
//	
//	string fragmentShader = // These are the shaders!!!
//		"#version 330 core \n" // this make sure that we are using the simple but latest version of glsl
//		"\n"
//		"layout(location = 0) out vec4 color;\n" // The location can be unwritten as its default is 0
//		"\n"
//		"void main()\n"
//		"{\n"
//		" color = vec4(0.5, 1.0, 0.0, 1.0);\n" // vec4(redValue_>0-1, greenValue. blueValue, Alpha)
//		"}\n";
//
//	unsigned int shader = CreateShader(vertexShader,fragmentShader);
//	glUseProgram(shader);
//
//	glBindBuffer(GL_ARRAY_BUFFER, 0);
//
//
//	/* Loop until the user closes the window */
//	while (!glfwWindowShouldClose(window))
//	{
//		/* Render here */
//		glClear(GL_COLOR_BUFFER_BIT);
//
//		//Lesson1
//		// Draw a triangle using legacy OpenGL
//		/*
//				glBegin(GL_TRIANGLES);
//				glVertex2d(-0.5f, 0.5f);
//				glVertex2d(0.5f,   0.5f);
//				glVertex2d(0,     -0.5f);
//				glEnd();
//
//		*/
//		/*
//		// Draw a circle; Important to know its algorthm.
//		glBegin(GL_LINE_LOOP);
//		for (int i = 0; i < n; ++i){
//			// Circle with beautiful lines using legacy OpenGL
//			//glVertex2f(R * cos(2 * Pi / i * n), R * sin(2 * Pi / i * n));
//			// Circle with single lines using legacy OpenGL
//			glVertex2f(R * sin(2 * Pi / n * i), R * cos(2 * Pi / n * i));
//		}
//		glEnd();
//		*/
//
//		// Lesson2
//		glDrawArrays(GL_TRIANGLES, 0, 3);
//
//		/* Swap front and back buffers */
//		glfwSwapBuffers(window);
//
//		/* Poll for and process events */
//		glfwPollEvents();
//	}
//	glDeleteProgram(shader);
//	glfwTerminate();
//	return 0;
//}
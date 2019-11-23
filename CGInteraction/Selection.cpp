#include "Selection.h"
#include <gl/glut.h>
//#include<glut.h>

//定义 
Selection::Selection(void)
{
	this->vec_selected_pts_index.clear();
	this->pts = NULL;
}

//析构 
Selection::~Selection(void)
{

}

//专门用来划区域的函数 
void Selection::draw_area()
{
	//获取窗口的宽度和高度，这里使用glfw的获取窗口大小的函数也可以，或者直接自己手动设置也可以 
	int width = glutGet(GLUT_WINDOW_WIDTH), height = glutGet(GLUT_WINDOW_HEIGHT);
	//选择矩阵的模式为投影模式
	glMatrixMode(GL_PROJECTION);
	//将矩阵压入堆栈中
	glPushMatrix();
	//加载几何体的ID信息
	glLoadIdentity();
	//使用正交视图函数确定窗口的长度和宽度
	gluOrtho2D(0, width, 0, height);

	//选择矩阵的模式为建模模式
	glMatrixMode(GL_MODELVIEW);
	//将矩阵压入堆栈中
	glPushMatrix();
	//加载几何体的ID信息
	glLoadIdentity();


	/*** 画矩形 ***/
	glColor4f(1.0, 1.0, 0, 0.2);
	glEnable(GL_BLEND);//启用色彩混合，这样就能够开启选中区域高亮显示的功能

	//表示把渲染的图像融合到目标区域。也就是说源的每一个像素的alpha都等于自己的alpha，目标的每一个像素的alpha等于1减去该位置源像素的alpha。 因此不论叠加多少次，亮度是不变的。
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);//第一个参数指出源图元的alpha的取值计算方法，第二个参数指出目标处的alpha的计算方法。
	//比如GL_ZERO表示alpha取值为0，GL_ONE表示1，GL_DST_COLOR表示取目标区域的颜色值，GL_SRC_ALPHA表示取源区域的alpha值等等


	glRectf(m3dGetVectorX(left_bottom), m3dGetVectorY(left_bottom), m3dGetVectorX(right_top), m3dGetVectorY(right_top));
	/*这里的glRectf相当于：
		glBegin(GL_POLYGON);
		glVertex2(x1, y1);
		glVertex2(x2, y1);
		glVertex2(x2, y2);
		glVertex2(x1, y2);
		glEnd();

		所以如果使用快速绘制的方式，就不要使用glRecft，否则重复使用会报错。

		该函数里面的那几个数值表示获取X轴和Y轴的值，使用math3d库就能够通过数组的方式获取上三维物体在二维平面上的上下左右坐标点的信息
	*/

/*** 画矩形虚线 ***/
	glEnable(GL_LINE_STIPPLE);
	glColor4f(1, 0, 0, 0.5);
	glLineStipple(3, 0xAAAA);//第一个次数是重复次数，第一个参数是第二个参数是由1或0组成的16位序列，它们根据需要进行重复，对一条特定的直线进行点画处理
	//这里其实就是虚线虚实结合的效果

	glBegin(GL_LINE_LOOP);//绘制矩形区域
	glVertex2f(m3dGetVectorX(left_bottom), m3dGetVectorY(left_bottom));
	glVertex2f(m3dGetVectorX(right_top), m3dGetVectorY(left_bottom));
	glVertex2f(m3dGetVectorX(right_top), m3dGetVectorY(right_top));
	glVertex2f(m3dGetVectorX(left_bottom), m3dGetVectorY(right_top));
	glEnd();

	//弹出矩阵堆栈
	glPopMatrix();
	glPopMatrix();
	glDisable(GL_LINE_STIPPLE);//关闭虚线绘制功能
	glDisable(GL_BLEND);//关闭渲染
}

//将选中区域高光显示
void Selection::highlight_selected_pts()
{
	int i = 0;

	//保存上一个opengl状态
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixf(proj);//加载几何体的信息到矩阵队列中

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadMatrixf(model_view);

	glPushAttrib(GL_POINT_BIT);
	glPointSize(3);
	glColor4f(0, 1, 0, 1);//设置颜色——这里就是高光显示的部分

	glBegin(GL_POINTS);

	for (i = 0; i != vec_selected_pts_index.size(); i++) {
		glVertex3fv(pts[vec_selected_pts_index[i]]);
	}

	glEnd();
	glPopAttrib();//弹出
	glPopMatrix();
	glPopMatrix();
}

//选中的点的信息
void Selection::get_selected_pts_index(vector<int>& v)
{
	// assign 赋值最快，所以这里用了assign
	v.assign(vec_selected_pts_index.begin(), vec_selected_pts_index.end());
}

void Selection::set_config(M3DVector3f* _pts, int _nr, M3DVector2f _left_bottom, M3DVector2f _right_top, M3DMatrix44f _model_view, M3DMatrix44f _proj, int _viewport[])
{
	pts = _pts;
	nr = _nr;
	
	//复制坐标信息
	m3dCopyVector2(left_bottom, _left_bottom);
	m3dCopyVector2(right_top, _right_top);

	m3dCopyMatrix44(model_view, _model_view);
	m3dCopyMatrix44(proj, _proj);
	memcpy(viewport, _viewport, sizeof(int) * 4);

	/*** 计算被选中的index ***/
	cal_selected_index();
}

//计算被选中的索引，看选中的索引中包不包含几何体位置坐标的信息，如果包含，则将该位置点的做白哦压入栈中
void Selection::cal_selected_index()
{
	vec_selected_pts_index.clear();//清空，相当于初始化

	int i = 0;
	for (i = 0; i != nr; i++) {

		if (drop_in_area(pts[i])) {//查看画的区域是否落在几何体坐标点上
			vec_selected_pts_index.push_back(i);
		}

	}
}

//检测画的区域是否落在坐标点上，二者是否重合
bool Selection::drop_in_area(M3DVector3f x)
{
	//将三维坐标信息转化为投影下得到二维坐标信息
	M3DVector2f win_coord;

	m3dProjectXY(win_coord, model_view, proj, viewport, x);

	if ((win_coord[0] < left_bottom[0] && win_coord[0] < right_top[0]) || (win_coord[0] > left_bottom[0] && win_coord[0] > right_top[0]))
		return false;

	if ((win_coord[1] < left_bottom[1] && win_coord[1] < right_top[1]) || (win_coord[1] > left_bottom[1] && win_coord[1] > right_top[1]))
		return false;

	return true;
}

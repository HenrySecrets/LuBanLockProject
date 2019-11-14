#include "Selection.h"
#include <gl/glut.h>
//#include<glut.h>

//���� 
Selection::Selection(void)
{
	this->vec_selected_pts_index.clear();
	this->pts = NULL;
}

//���� 
Selection::~Selection(void)
{

}

//ר������������ĺ��� 
void Selection::draw_area()
{
	//��ȡ���ڵĿ�Ⱥ͸߶ȣ�����ʹ��glfw�Ļ�ȡ���ڴ�С�ĺ���Ҳ���ԣ�����ֱ���Լ��ֶ�����Ҳ���� 
	int width = glutGet(GLUT_WINDOW_WIDTH), height = glutGet(GLUT_WINDOW_HEIGHT);
	//ѡ������ģʽΪͶӰģʽ
	glMatrixMode(GL_PROJECTION);
	//������ѹ���ջ��
	glPushMatrix();
	//���ؼ������ID��Ϣ
	glLoadIdentity();
	//ʹ��������ͼ����ȷ�����ڵĳ��ȺͿ��
	gluOrtho2D(0, width, 0, height);

	//ѡ������ģʽΪ��ģģʽ
	glMatrixMode(GL_MODELVIEW);
	//������ѹ���ջ��
	glPushMatrix();
	//���ؼ������ID��Ϣ
	glLoadIdentity();


	/*** ������ ***/
	glColor4f(1.0, 1.0, 0, 0.2);
	glEnable(GL_BLEND);//����ɫ�ʻ�ϣ��������ܹ�����ѡ�����������ʾ�Ĺ���

	//��ʾ����Ⱦ��ͼ���ںϵ�Ŀ������Ҳ����˵Դ��ÿһ�����ص�alpha�������Լ���alpha��Ŀ���ÿһ�����ص�alpha����1��ȥ��λ��Դ���ص�alpha�� ��˲��۵��Ӷ��ٴΣ������ǲ���ġ�
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);//��һ������ָ��ԴͼԪ��alpha��ȡֵ���㷽�����ڶ�������ָ��Ŀ�괦��alpha�ļ��㷽����
	//����GL_ZERO��ʾalphaȡֵΪ0��GL_ONE��ʾ1��GL_DST_COLOR��ʾȡĿ���������ɫֵ��GL_SRC_ALPHA��ʾȡԴ�����alphaֵ�ȵ�


	glRectf(m3dGetVectorX(left_bottom), m3dGetVectorY(left_bottom), m3dGetVectorX(right_top), m3dGetVectorY(right_top));
	/*�����glRectf�൱�ڣ�
		glBegin(GL_POLYGON);
		glVertex2(x1, y1);
		glVertex2(x2, y1);
		glVertex2(x2, y2);
		glVertex2(x1, y2);
		glEnd();

		�������ʹ�ÿ��ٻ��Ƶķ�ʽ���Ͳ�Ҫʹ��glRecft�������ظ�ʹ�ûᱨ��

		�ú���������Ǽ�����ֵ��ʾ��ȡX���Y���ֵ��ʹ��math3d����ܹ�ͨ������ķ�ʽ��ȡ����ά�����ڶ�άƽ���ϵ�����������������Ϣ
	*/

/*** ���������� ***/
	glEnable(GL_LINE_STIPPLE);
	glColor4f(1, 0, 0, 0.5);
	glLineStipple(3, 0xAAAA);//��һ���������ظ���������һ�������ǵڶ�����������1��0��ɵ�16λ���У����Ǹ�����Ҫ�����ظ�����һ���ض���ֱ�߽��е㻭����
	//������ʵ����������ʵ��ϵ�Ч��

	glBegin(GL_LINE_LOOP);//���ƾ�������
	glVertex2f(m3dGetVectorX(left_bottom), m3dGetVectorY(left_bottom));
	glVertex2f(m3dGetVectorX(right_top), m3dGetVectorY(left_bottom));
	glVertex2f(m3dGetVectorX(right_top), m3dGetVectorY(right_top));
	glVertex2f(m3dGetVectorX(left_bottom), m3dGetVectorY(right_top));
	glEnd();

	//���������ջ
	glPopMatrix();
	glPopMatrix();
	glDisable(GL_LINE_STIPPLE);//�ر����߻��ƹ���
	glDisable(GL_BLEND);//�ر���Ⱦ
}

//��ѡ������߹���ʾ
void Selection::highlight_selected_pts()
{
	int i = 0;

	//������һ��opengl״̬
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixf(proj);//���ؼ��������Ϣ�����������

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadMatrixf(model_view);

	glPushAttrib(GL_POINT_BIT);
	glPointSize(3);
	glColor4f(0, 1, 0, 1);//������ɫ����������Ǹ߹���ʾ�Ĳ���

	glBegin(GL_POINTS);

	for (i = 0; i != vec_selected_pts_index.size(); i++) {
		glVertex3fv(pts[vec_selected_pts_index[i]]);
	}

	glEnd();
	glPopAttrib();//����
	glPopMatrix();
	glPopMatrix();
}

//ѡ�еĵ����Ϣ
void Selection::get_selected_pts_index(vector<int>& v)
{
	// assign ��ֵ��죬������������assign
	v.assign(vec_selected_pts_index.begin(), vec_selected_pts_index.end());
}

void Selection::set_config(M3DVector3f* _pts, int _nr, M3DVector2f _left_bottom, M3DVector2f _right_top, M3DMatrix44f _model_view, M3DMatrix44f _proj, int _viewport[])
{
	pts = _pts;
	nr = _nr;
	
	//����������Ϣ
	m3dCopyVector2(left_bottom, _left_bottom);
	m3dCopyVector2(right_top, _right_top);

	m3dCopyMatrix44(model_view, _model_view);
	m3dCopyMatrix44(proj, _proj);
	memcpy(viewport, _viewport, sizeof(int) * 4);

	/*** ���㱻ѡ�е�index ***/
	cal_selected_index();
}

//���㱻ѡ�е���������ѡ�е������а�������������λ���������Ϣ������������򽫸�λ�õ������Ŷѹ��ջ��
void Selection::cal_selected_index()
{
	vec_selected_pts_index.clear();//��գ��൱�ڳ�ʼ��

	int i = 0;
	for (i = 0; i != nr; i++) {

		if (drop_in_area(pts[i])) {//�鿴���������Ƿ����ڼ������������
			vec_selected_pts_index.push_back(i);
		}

	}
}

//��⻭�������Ƿ�����������ϣ������Ƿ��غ�
bool Selection::drop_in_area(M3DVector3f x)
{
	//����ά������Ϣת��ΪͶӰ�µõ���ά������Ϣ
	M3DVector2f win_coord;

	m3dProjectXY(win_coord, model_view, proj, viewport, x);

	if ((win_coord[0] < left_bottom[0] && win_coord[0] < right_top[0]) || (win_coord[0] > left_bottom[0] && win_coord[0] > right_top[0]))
		return false;

	if ((win_coord[1] < left_bottom[1] && win_coord[1] < right_top[1]) || (win_coord[1] > left_bottom[1] && win_coord[1] > right_top[1]))
		return false;

	return true;
}

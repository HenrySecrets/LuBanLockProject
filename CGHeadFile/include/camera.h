#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
using namespace glm;

/*����ƶ���ö��*/
enum Camera_Movement
{
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

/*�������Ĭ��ֵ*/
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

class Camera
{
public:
	/*�������*/
	vec3 Position;
	vec3 Front;
	vec3 Up;
	vec3 Right;
	vec3 WorldUp;
	/*ŷ����*/
	float Yaw;
	float Pitch;
	/*����ѡ��*/
	float MovementSpeed;
	float MouseSensitivity;
	float Zoom;

	/*
	����λ�á���������ƫ���ǡ������ǵĹ��캯��
	ͬʱ��ʼ��ǰ����������ƶ��ٶȡ��ƶ����жȺ�����
	*/
	Camera(vec3 position = vec3(0.0f, 0.0f, 0.0f), vec3 up = vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
	{
		Position = position;
		WorldUp = up;
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVectors();
	}
	/*
	����ʽ���캯��
	*/
	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
	{
		Position = vec3(posX, posY, posZ);
		WorldUp = vec3(upX, upY, upZ);
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVectors();
	}

	/*��ȡ��ͼ����*/
	mat4 GetViewMatrix()
	{
		return lookAt(Position, Position + Front, Up);
	}

	/*���ݼ�������ı����λ��*/
	void ProcessKeyboard(Camera_Movement direction, float deltaTime)
	{
		float velocity = MovementSpeed * deltaTime;//����ƶ��ٶ�ͨ��deltaTimeʵ�ָ�˳�����ƶ�Ч��
		switch (direction)
		{
		case FORWARD:
			Position += Front * velocity;
			break;
		case BACKWARD:
			Position -= Front * velocity;
			break;
		case LEFT:
			Position -= Right * velocity;
			break;
		case RIGHT:
			Position += Right * velocity;
			break;
		default:
			break;
		}
	}

	/*ͨ������ƶ������������ת*/
	void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)//constrainPitchΪ�Ƿ����Ƹ�������+-90��֮��
	{
		/*ͨ�����жȵ�����ת�ٶ�*/
		xoffset *= MouseSensitivity;
		yoffset *= MouseSensitivity;

		/*������ƶ��������ת��Ӧ����*/
		Yaw += xoffset;
		Pitch += yoffset;

		/*���Ƹ����Ƿ�Χ�Է�ֹ��ͼ��ת*/
		if (constrainPitch)
		{
			if (Pitch > 89.0f)
			{
				Pitch = 89.0f;
			}
			else if (Pitch < -89.0f)
			{
				Pitch = -89.0f;
			}
		}
		updateCameraVectors();
	}

	/*ͨ�������ֿ�������ӽ�����*/
	void ProcessMouseScroll(float yoffset)
	{
		/*�����ӽ���0��ֵ��45��֮��*/
		if (Zoom >= 1.0f && Zoom <= 45.0f)
		{
			Zoom -= yoffset;
		}
		if (Zoom <= 1.0f)
		{
			Zoom = 1.0f;
		}
		if (Zoom >= 45.0f)
		{
			Zoom = 45.0f;
		}
	}

private:
	/*�������������ķ�������*/
	void updateCameraVectors()
	{
		/*�������ǰ��������*/
		vec3 front;
		front.x = cos(radians(Yaw)) * cos(radians(Pitch));//x������������xzƽ���ͶӰ��x�᷽���ϵ�ͶӰ
		front.y = sin(radians(Pitch));
		front.z = sin(radians(Yaw)) * cos(radians(Pitch));
		Front = normalize(front);//������������׼���Ժ��Դ�С

		Right = normalize(cross(Front, WorldUp));//���ò�˻��ͬʱ��ֱǰ���������������������������������
		Up = normalize(cross(Right, Front));//���ò�˻�ȡͬʱ��ֱ�����������ǰ������������
	}
};
#endif // !CAMERA_H
#include "pch.h"
#include "Wall.h"
#include <iostream>

Wall::Wall(string _fileName, ID3D11Device* _pd3dDevice, IEffectFactory* _EF, Vector3 position, Vector3 scale, float yaw) :CMOGO(_fileName, _pd3dDevice, _EF)
{
	SetPos(position);
	SetScale(scale);
	SetYaw(yaw);

	GameObject::Tick(nullptr); //update my world_transform
}

Wall::~Wall()
{

}

Vector3 Wall::InitialPosition(int arrayNo)
{
	switch (arrayNo)
	{
	case 0:
		return Vector3(10.0f, -65.0f, 50.0f);
		break;
	case 1:
		return Vector3(-173.0f, -65.0f, 200.0f);
		break;
	case 2:
		return Vector3(0.0f, -65.0f, 420.0f);
		break;
	}
}

void Wall::NewPosition(Vector3 newPos)
{
	m_pos = newPos;
}

string Wall::GetModel(int arrayNo)
{
	switch (arrayNo)
	{
	case 0:
		return "block";
		break;
	case 1:
		return "blockRed";
		break;
	case 2:
		return "blockYellow";
		break;
	}
}

Vector3 Wall::InitialScale(int arrayNo)
{
	switch (arrayNo)
	{
	case 0:
		return Vector3(210.0f, 150.0f, 20.0f);
		break;
	case 1:
		return Vector3(70.0f, 80.0f, 10.0f);
		break;
	case 2:
		return Vector3(210.0f, 120.0f, 20.0f);
		break;
	}
}

float Wall::InitialRotation(int arrayNo)
{
	switch (arrayNo)
	{
	case 0:
		return 0.0f;
		break;
	case 1:
		return 0.0f;
		break;
	case 2:
		return 1.57f;
		break;
	}
}

void Wall::Move(float speed, float distance)
{
	if (distanceMoved < distance)
	{
		m_pos.y -= speed;
		distanceMoved += speed;
		GameObject::Tick(nullptr);
	}
}

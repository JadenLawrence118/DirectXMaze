#include "pch.h"
#include "DeathPlane.h"
#include <iostream>

DeathPlane::DeathPlane(string _fileName, ID3D11Device* _pd3dDevice, IEffectFactory* _EF, Vector3 position, Vector3 scale, float yaw) : CMOGO(_fileName, _pd3dDevice, _EF)
{
	SetPos(position);
	SetScale(scale);
	SetYaw(yaw);
}

DeathPlane::~DeathPlane()
{
}

Vector3 DeathPlane::InitialPosition(int arrayNo)
{
	switch (arrayNo)
	{
	case 0:
		return Vector3(-145.0f, -70.0f, 287.0f);
		break;
	case 1:
		return Vector3(129.5f, -70.0f, 80.0f);
		break;
	case 2:
		return Vector3(160.0f, -70.0f, 166.0f);
		break;
	case 3:
		return Vector3(-62.0f, -70.0f, 375.0f);
		break;
	}
}

float DeathPlane::InitialRotation(int arrayNo)
{
	switch (arrayNo)
	{
	case 0:
		return 0.0f;
		break;
	case 1:
		return 1.57f;
		break;
	case 2:
		return 0.0f;
		break;
	case 3:
		return 0.0f;
		break;
	}
}

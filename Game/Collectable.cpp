#include "pch.h"
#include "Collectable.h"
#include <iostream>

Collectable::Collectable(string _fileName, ID3D11Device* _pd3dDevice, IEffectFactory* _EF, Vector3 position, Vector3 scale) : CMOGO(_fileName, _pd3dDevice, _EF)
{
	SetPos(position);
	SetScale(scale);
}

Collectable::~Collectable()
{
}

Vector3 Collectable::InitialPosition(int arrayNo)
{
	switch (arrayNo)
	{
	case 0:
		return Vector3(5.0f, -65.0f, 25.0f);
		break;
	case 1:
		return Vector3(128.0f, -65.0f, 26.0f);
		break;
	case 2:
		return Vector3(300.0f, -65.0f, 160.0f);
		break;
	}
}

string Collectable::GetModel(int arrayNo)
{
	switch (arrayNo)
	{
	case 0:
		return "sphere";
		break;
	case 1:
		return "sphereRed";
		break;
	case 2:
		return "sphereYellow";
		break;
	}
}

Vector3 Collectable::InitialScale(int arrayNo)
{
	switch (arrayNo)
	{
	case 0:
		return Vector3(25.0f);
		break;
	case 1:
		return Vector3(12.5f);
		break;
	case 2:
		return Vector3(25.0f);
		break;
	}
}

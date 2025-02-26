#pragma once
#include "CMOGO.h"


class DeathPlane : public CMOGO
{
public:
	DeathPlane(string _fileName, ID3D11Device* _pd3dDevice, IEffectFactory* _EF, Vector3 position, Vector3 scale, float yaw);
	~DeathPlane();
	Vector3 InitialPosition(int arrayNo);
	float InitialRotation(int arrayNo);
};


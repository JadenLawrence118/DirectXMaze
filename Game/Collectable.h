#pragma once
#include "CMOGO.h"


class Collectable : public CMOGO
{
public:
	Collectable(string _fileName, ID3D11Device* _pd3dDevice, IEffectFactory* _EF, Vector3 position, Vector3 scale);
	~Collectable();
	Vector3 InitialPosition(int arrayNo);
	string GetModel(int arrayNo);
	Vector3 InitialScale(int arrayNo);
};


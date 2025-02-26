#pragma once
#include "CMOGO.h"


class EndPortal : public CMOGO
{
public:
	EndPortal(string _fileName, ID3D11Device* _pd3dDevice, IEffectFactory* _EF, Vector3 position, Vector3 scale);
	~EndPortal();
};


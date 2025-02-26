#include "pch.h"
#include "EndPortal.h"

EndPortal::EndPortal(string _fileName, ID3D11Device* _pd3dDevice, IEffectFactory* _EF, Vector3 position, Vector3 scale) : CMOGO(_fileName, _pd3dDevice, _EF)
{
	SetPitch(1.57);
	SetPos(position);
	SetScale(scale);
}

EndPortal::~EndPortal()
{
}
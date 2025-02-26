#pragma once
#include "CMOGO.h"

struct GameData;

class Wall : public CMOGO
{
public:
	Wall(string _fileName, ID3D11Device* _pd3dDevice, IEffectFactory* _EF, Vector3 position, Vector3 scale, float yaw);
	~Wall();

	Vector3 InitialPosition(int arrayNo);
	void NewPosition(Vector3 newPos);
	string GetModel(int arrayNo);
	Vector3 InitialScale(int arrayNo);
	float InitialRotation(int arrayNo);

	void Tick(GameData* _GD) override { _GD; };
	void Move(float speed, float distance);

	bool moving = false;

	float distanceMoved = 0;

protected:
	
};

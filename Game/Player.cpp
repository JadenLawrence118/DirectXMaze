#include "pch.h"
#include "Player.h"
#include <dinput.h>
#include "GameData.h"
#include <iostream>

Player::Player(string _fileName, ID3D11Device* _pd3dDevice, IEffectFactory* _EF) : CMOGO(_fileName, _pd3dDevice, _EF)
{
	//any special set up for Player goes here
	m_fudge = Matrix::CreateRotationY(XM_PI);

	m_yaw = 3.14;
	m_pos.y = -60.0f;

	SetDrag(5);
	SetPhysicsOn(true);
}

Player::~Player()
{
	//tidy up anything I've created
}


void Player::Tick(GameData* _GD)
{
	
	//TURN AND FORWARD CONTROL HERE
	Vector3 forwardMove = playerSpeed * Vector3::Forward;
	Matrix rotMove = Matrix::CreateRotationY(m_yaw);
	forwardMove = Vector3::Transform(forwardMove, rotMove);
	if (_GD->m_KBS.W)
	{
		m_acc += forwardMove;
	}
	if (_GD->m_KBS.S)
	{
		m_acc -= forwardMove;
	}

	//change orinetation of player
	float rotSpeed = 2.0f * _GD->m_dt;

	// left and right camera movement with mouse
	m_yaw -= _GD->m_MS.x * rotSpeed;

	// up and down camera movement with mouse
	m_pitch -= _GD->m_MS.y * rotSpeed;

	if (m_pitch > XMConvertToRadians(60))
	{
		m_pitch = XMConvertToRadians(60);
	}

	if (m_pitch < XMConvertToRadians(-60))
	{
		m_pitch = XMConvertToRadians(-60);
	}


	if (_GD->m_KBS.A)
	{
		Vector3 leftMove = Vector3::Transform(Vector3::Forward, Matrix::CreateRotationY(m_yaw + XMConvertToRadians(90)));
		m_acc += playerSpeed * leftMove;
	}
	if (_GD->m_KBS.D)
	{
		Vector3 rightMove = Vector3::Transform(Vector3::Forward, Matrix::CreateRotationY(m_yaw + XMConvertToRadians(-90)));
		m_acc += playerSpeed * rightMove;
	}

	if (_GD->m_KBS.LeftShift)
	{
		playerSpeed = playerBaseSpeed * 2;
	}
	else
	{
		playerSpeed = playerBaseSpeed;
	}

	//limit motion of the player
	float length = m_pos.Length();
	float maxLength = 500.0f;
	if (length > maxLength)
	{
		m_pos.Normalize();
		m_pos *= maxLength;
		m_vel *= -0.9; //VERY simple bounce back
	}

	//apply my base behaviour
	CMOGO::Tick(_GD);

	// std::cout << std::to_string(m_pos.x) << ", " << std::to_string(m_pos.y) << ", " << std::to_string(m_pos.z) << std::endl;
}
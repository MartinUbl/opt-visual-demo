/**
 * OptVisualDemo
 *
 * Copyright (c) 2025-present, Martin Ubl
 * Distributed under the MIT license
 */

#pragma once

#include "box2d/box2d.h"
#include "raylib.h"

#include <vector>
#include <mutex>

struct PhysicsConfig {
	b2Vec2 gravity = { 0, 1.0f };
};

class PhysicsWorld {
	private:
		b2WorldId mWorldId;

		b2BodyId mDynamicBallId = { .index1 = -1, .world0 = 0, .generation = 0 };

		std::vector<Vector2> mBallPositions;

		std::mutex mMutex;

	public:
		PhysicsWorld(const PhysicsConfig& cfg);
		virtual ~PhysicsWorld();

		// x and y are screen coordinates
		void Add_Static_Rect_Body(float x, float y, float width, float height);
		// x and y are screen coordinates
		void Add_Dynamic_Ball_Body(float x, float y, float radius, float initialDirection = 0, float initialVelocity = 0, float density = 1.0f);

		void Step(float timeStep);

		const std::vector<Vector2>& Get_Ball_Positions() const { return mBallPositions; }

		void Reset();
};

#include "PhysicsWrapper.h"

#include "raylib.h"

constexpr float Screen_To_Physics_Scale = 0.01f;
constexpr float Physics_To_Screen_Scale = 1.0f / Screen_To_Physics_Scale;

PhysicsWorld::PhysicsWorld(const PhysicsConfig& cfg) {
	b2WorldDef worldDef = b2DefaultWorldDef();

	worldDef.gravity = cfg.gravity;

	mWorldId = b2CreateWorld(&worldDef);

	b2BodyDef groundBodyDef = b2DefaultBodyDef();
	groundBodyDef.position = { 0.0f, Screen_To_Physics_Scale*(- GetScreenHeight() / 2.0f)};
	b2BodyId groundId = b2CreateBody(mWorldId, &groundBodyDef);

	b2Polygon groundBox = b2MakeBox(Screen_To_Physics_Scale*GetScreenWidth(), 1.0f);
	b2ShapeDef groundShapeDef = b2DefaultShapeDef();
	b2CreatePolygonShape(groundId, &groundShapeDef, &groundBox);

	// add sides to collide with the ball
	b2Polygon leftWallBox = b2MakeBox(Screen_To_Physics_Scale * 1.0f, Screen_To_Physics_Scale * GetScreenHeight());
	b2BodyDef leftWallBodyDef = b2DefaultBodyDef();
	leftWallBodyDef.position = { 0, 0.0f };
	b2BodyId leftWallId = b2CreateBody(mWorldId, &leftWallBodyDef);
	b2CreatePolygonShape(leftWallId, &groundShapeDef, &leftWallBox);
	b2Polygon rightWallBox = b2MakeBox(Screen_To_Physics_Scale * 1.0f, Screen_To_Physics_Scale * GetScreenHeight());
	b2BodyDef rightWallBodyDef = b2DefaultBodyDef();
	rightWallBodyDef.position = { Screen_To_Physics_Scale*GetScreenWidth(), 0.0f};
	b2BodyId rightWallId = b2CreateBody(mWorldId, &rightWallBodyDef);
	b2CreatePolygonShape(rightWallId, &groundShapeDef, &rightWallBox);
}

PhysicsWorld::~PhysicsWorld() {
	b2DestroyWorld(mWorldId);
}

void PhysicsWorld::Add_Static_Rect_Body(float x, float y, float width, float height) {
	std::unique_lock<std::mutex> lock(mMutex);
	b2BodyDef bodyDef = b2DefaultBodyDef();
	bodyDef.type = b2_staticBody;
	bodyDef.position = { Screen_To_Physics_Scale*x, Screen_To_Physics_Scale*y };
	b2BodyId bodyId = b2CreateBody(mWorldId, &bodyDef);
	b2Polygon box = b2MakeBox(Screen_To_Physics_Scale*width/2.0f, Screen_To_Physics_Scale*height/2.0f);
	b2ShapeDef shapeDef = b2DefaultShapeDef();
	b2CreatePolygonShape(bodyId, &shapeDef, &box);
}

void PhysicsWorld::Add_Dynamic_Ball_Body(float x, float y, float radius, float initialDirection, float initialVelocity, float density) {
	std::unique_lock<std::mutex> lock(mMutex);
	b2BodyDef bodyDef = b2DefaultBodyDef();
	bodyDef.type = b2_dynamicBody;
	bodyDef.position = { Screen_To_Physics_Scale*x, Screen_To_Physics_Scale*y };
	bodyDef.linearVelocity = { initialVelocity * cosf(initialDirection), initialVelocity * sinf(initialDirection) };
	b2BodyId bodyId = b2CreateBody(mWorldId, &bodyDef);

	b2Circle circle;
	circle.center = { 0.0f, 0.0f };
	circle.radius = Screen_To_Physics_Scale*radius;
	b2ShapeDef shapeDef = b2DefaultShapeDef();
	shapeDef.density = density * 0.1f;
	shapeDef.material.friction = 0.2f;
	shapeDef.material.restitution = 0.6f;
	b2CreateCircleShape(bodyId, &shapeDef, &circle);

	mDynamicBallId = bodyId;
}

void PhysicsWorld::Step(float timeStep) {

	std::unique_lock<std::mutex> lock(mMutex);

	b2World_Step(mWorldId, timeStep, 4);

	if (mDynamicBallId.index1 != -1) {
		b2Vec2 position = b2Body_GetPosition(mDynamicBallId);
		if (mBallPositions.size() >= 1000) {
			mBallPositions.erase(mBallPositions.begin());
		}
		mBallPositions.push_back({ Physics_To_Screen_Scale*position.x, Physics_To_Screen_Scale * position.y });
	}
}

void PhysicsWorld::Reset() {
	std::unique_lock<std::mutex> lock(mMutex);
	mBallPositions.clear();
}

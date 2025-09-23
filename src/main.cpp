/**
 * OptVisualDemo
 * 
 * Copyright (c) 2025-present, Martin Ubl
 * Distributed under the MIT license
 */

#include "raylib.h"

#include "Core/Application.h"
#include "Core/Stage.h"

int main(int argc, char** argv) {

	if (!Application::Instance().Init(argc, argv)) {
		return -1;
	}

	Application::Instance().Request_Stage_Change(NStage::Menu);

	return Application::Instance().Run();
}

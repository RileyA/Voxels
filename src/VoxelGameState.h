//---------------------------------------------------------------------------
//(C) Copyright Riley Adams 2011

//This file is part of Oryx Engine.

// Oryx Engine is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the license, or
// (at your option) any later version.

// Oryx Engine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTEE; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.

// You should have recieved a copy of the GNU General Public License
// along with Oryx Engine. If not, see <http://www.gnu.org/licenses/>
//---------------------------------------------------------------------------

#ifndef VERSUS_GAMESTATE
#define VERSUS_GAMESTATE

#include "Oryx.h"
#include "OryxEngine.h"
#include "OryxGameState.h"
#include "FPSCamera.h"
#include "ExplosionManager.h"
#include "Chunk.h"
#include "ChunkManager.h"
#include "VoxelPeer.h"
#include "OryxMessageAny.h"
#include "OryxEventHandler.h"

#include "ENetSubsystem/ENetSubsystem.h"
#include "ENetSubsystem/Server.h"
#include "ENetSubsystem/Client.h"
#include "BulletSubsystem/BulletSubsystem.h"

// uuuuugly playpen code

namespace Oryx
{
	class VoxelGameState : public GameState
	{
	public:

		VoxelGameState();
		
		void init();
		void update(Real delta);
		void deinit();

		void mouse(const Message& msg);
		StaticText* txt; 
		FPSCamera* mCam;
		Real mTimer;
		Chunk* c;
		bool toggle;
		bool toggle2;
		ExplosionManager* mgr;
		ChunkManager* cmgr;
		int currentMat;
		
		QuantaController* ch;
	};
}

#endif

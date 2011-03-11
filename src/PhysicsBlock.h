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

#ifndef PHYSICS_CHUNK_H
#define PHYSICS_CHUNK_H

#include "Oryx.h"
#include "OryxEngine.h"
#include "BulletSubsystem/BulletSubsystem.h"
#include "ChunkCoords.h"

namespace Oryx
{
	class Chunk;

	/** A block of physics boxes whithin a Chunk */
	class PhysicsBlock
	{
	public:

		CollisionObject* block;
		Chunk* parent;

		int x;
		int y;
		int z;
		Vector3 pos;

		PhysicsBlock(int i, int j, int k,Vector3 p,CollisionObject* bl,Chunk* par)
		{
			pos = p;
			block = bl;
			x = i*CHUNK_STEP;
			y = j*CHUNK_STEP;
			z = k*CHUNK_STEP;
			parent = par;
			if(block)
			{
				block->setUserData(parent);
				//block->setCollisionGroup(COLLISION_GROUP_1);
				//block->setCollisionMask(COLLISION_GROUP_15|COLLISION_GROUP_1|COLLISION_GROUP_3);
			}
		}

		~PhysicsBlock()
		{
			if(block)
				block->_kill();
		}

		void rebuild(MeshData& d);
	};
}

#endif

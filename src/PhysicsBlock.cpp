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

#include "Chunk.h"

namespace Oryx
{
	void PhysicsBlock::rebuild(MeshData& d)
	{
		BulletSubsystem* b = Engine::getPtr()->getSubsystem("BulletSubsystem")->
		castType<BulletSubsystem>();

		if(block)
			block->_kill();

		block = static_cast<CollisionObject*>(b->createStaticTrimesh(d,pos));

		block->setUserData(parent);
		block->setCollisionGroup(COLLISION_GROUP_2);
		block->setCollisionMask(COLLISION_GROUP_15|COLLISION_GROUP_1|COLLISION_GROUP_3);
		/*static_cast<PhysicsCompoundShape*>(block->getShape())->removeAllShapes();
		for(int i=0;i<4;++i)for(int j=0;j<4;++j)for(int k=0;k<4;++k)
		{
			if(parent->blocked[i+x][j+y][k+z])
			{
				// Hard-coding this is slightly more efficient than the cleaner version, so whatever...
				if(!(x+i==CHUNK_SIZE_X-1||!parent->blocked[x+i+1][y+j][z+k])&&
					!(x+i==0||!parent->blocked[x+i-1][y+j][z+k])&&
					!(y+j==CHUNK_SIZE_Y-1||!parent->blocked[x+i][y+j+1][z+k])&&
					!(y+j==0||!parent->blocked[x+i][y+j-1][z+k])&&
					!(z+k==CHUNK_SIZE_Z-1||!parent->blocked[x+i][y+j][z+k+1])&&
					!(z+k==0||!parent->blocked[x+i][y+j][z+k-1]))
					continue;

				static_cast<PhysicsCompoundShape*>(block->getShape())->addShape(b->createBoxShape(Vector3(0.5,0.5,0.5)),
					Vector3(i,j,k));
			}
		}*/
	}
}

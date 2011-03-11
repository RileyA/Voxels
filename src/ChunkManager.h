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

#ifndef VERSUS_CHUNKMGR
#define VERSUS_CHUNKMGR

#include "Oryx.h"
#include "OryxEngine.h"
#include "OryxObject.h"
#include "Chunk.h"
#include "OgreSubsystem/OgreSubsystem.h"
#include "OISSubsystem/OISSubsystem.h"
#include "BulletSubsystem/BulletSubsystem.h"

namespace noise
{
	namespace module
	{
		class Perlin;
	}
}

namespace Oryx
{
	class ChunkManager : public Object
	{
	public:

		ChunkManager(Vector3 position);
		virtual ~ChunkManager();

		void update(Real delta);
		
		void generate(Vector3 pos);
		void killBlocks(Vector3 p,float radius);

		Chunk* getChunk(ChunkCoords c);
		Chunk* createChunk(ChunkCoords c);

		void setMaterial(String material,size_t atlasDimensions);
	
	private:

		std::map<ChunkCoords,Chunk*> mChunks;
		int radius;
		ChunkCoords mLast;
		noise::module::Perlin* mPerlin;

	};
}

#endif	

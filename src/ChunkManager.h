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
	struct ChunkCompare
	{
	public:

		ChunkCompare(ChunkCoords cc, Vector3 playerPos, Vector3 playerView, Chunk* c)
		{
			Vector3 chunkPos;
			pos = cc;
			chunkPos = Vector3(cc.x*16,0,cc.z*16);
			Vector3 ptoC = chunkPos - playerPos;
			Real distance = ptoC.length();
			Real angle = ptoC.angleBetween(playerView);
			priority = static_cast<int>(
				((angle < 70.f) ? (70.f - angle)/7.f : 0) +
				((distance < (16 * 10)) ? (10-distance/16) : 0));
			chunk = c;
		}

		bool operator < (const ChunkCompare& other) const
		{
			return priority < other.priority;
		}

		int priority;
		Chunk* chunk;
		ChunkCoords pos;
	};

	class ChunkManager : public Object
	{
	public:

		ChunkManager(Vector3 position);
		virtual ~ChunkManager();

		void update(Real delta);
		
		void generate(Vector3 pos,Vector3 direction = Vector3::UNIT_Z,bool first = false);
		void killBlocks(Vector3 p,float radius);

		Chunk* getChunk(ChunkCoords c);
		Chunk* createChunk(ChunkCoords c);

		void setMaterial(String material,size_t atlasDimensions);
	
	private:

		std::map<ChunkCoords,Chunk*> mChunks;
		int radius;
		ChunkCoords mLast;
		noise::module::Perlin* mPerlin;

		Real mUpdateTimer;
		const static Real UPDATE_INTERVAL;

		class PerlinVolume
		{
		public:

			PerlinVolume(noise::module::Perlin* p,int x, int y, int z, Real scale=1.f);
			double sample(int x, int y, int z);
			
		private:

			noise::module::Perlin* mNoise;
			double mData[5][1][5];

		};
	};
}

#endif	

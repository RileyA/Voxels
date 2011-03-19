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

#include "Oryx.h"
#include "OryxStringUtils.h"
#include "ChunkManager.h"
#include <stack>
#include <noise/noise.h>

namespace Oryx
{
	const Real ChunkManager::UPDATE_INTERVAL = 0.1f;

	ChunkManager::ChunkManager(Vector3 position)
		:radius(8),mLast(10,10,10)
	{
		mPerlin = new noise::module::Perlin();
		generate(position, Vector3::UNIT_Z, true);
		radius = 2;
		mUpdateTimer = 0.f;
	}
	//-----------------------------------------------------------------------

	ChunkManager::~ChunkManager()
	{
		std::map<ChunkCoords,Chunk*>::iterator it = mChunks.begin();
		while(it!=mChunks.end())
		{
			delete it->second;
			++it;
		}
	}
	//-----------------------------------------------------------------------
	
	void ChunkManager::update(Real delta)
	{
		mUpdateTimer+=delta;
		

		std::stack<Chunk*> updated;
		Real start = TimeManager::getPtr()->getTimeDecimal();

		int chunksRebuilt = 0;

		std::map<ChunkCoords,Chunk*>::iterator it = mChunks.begin();
		while(it!=mChunks.end())
		{
			it->second->update(delta);

			if(it->second->needsUpdate())
			{
				updated.push(it->second);
				it->second->localLighting();

				for(int i=0;i<6;++i)
				{
					// TODO update diagonals as well. optimize?
					if(it->second->neighbors[i])
					{
						updated.push(it->second->neighbors[i]);
						it->second->neighbors[i]->localLighting();
						it->second->neighbors[i]->secondaryLighting();
					}
				}
				it->second->secondaryLighting();
			}
			++it;
		}

		chunksRebuilt = updated.size();

		if(chunksRebuilt>0)
		{
			Logger::getPtr()->logMessage("Lighting Took: "+StringUtils::toString(
				TimeManager::getPtr()->getTimeDecimal()-start));
		}
		
		Real start2 = TimeManager::getPtr()->getTimeDecimal();

		while(!updated.empty())
		{
			updated.top()->build();
			updated.pop();
		}

		if(chunksRebuilt>0)
		{
			Logger::getPtr()->logMessage("Builds Took: "+StringUtils::toString(
				TimeManager::getPtr()->getTimeDecimal()-start2)+" "
				+StringUtils::toString(chunksRebuilt)+" Chunk Updates");
		}
	}
	//-----------------------------------------------------------------------
	
	void ChunkManager::generate(Vector3 position,Vector3 direction, bool first)
	{
		if(first)
		{
			int i = (position.x+CHUNK_SIZE_X/2)/CHUNK_SIZE_X;
			int k = (position.z+CHUNK_SIZE_Z/2)/CHUNK_SIZE_Z;

			ChunkCoords c = (i-radius,0,k-radius);

			for(c.x = i-radius; c.x<=i+radius; ++c.x)
				for(c.z = k-radius; c.z<=k+radius; ++c.z)
					createChunk(c);

			int radius2 = radius==15 ? 5 : 2;
			c = (i-radius2,0,k-radius2);

			for(c.x = i-radius2; c.x<=i+radius2; ++c.x)
				for(c.z = k-radius2; c.z<=k+radius2; ++c.z)
					if(!mChunks[c]->isActive())
						mChunks[c]->setActive(1);
		}
		else if(mUpdateTimer > UPDATE_INTERVAL)
		{
			mUpdateTimer = 0.f;
			std::priority_queue<ChunkCompare> pq;
			int i = (position.x+CHUNK_SIZE_X/2)/CHUNK_SIZE_X;
			int k = (position.z+CHUNK_SIZE_Z/2)/CHUNK_SIZE_Z;
			position.y = 0;

			int rad = 5;

			ChunkCoords c = (i-rad,0,k-rad);
			for(c.x = i-rad; c.x<=i+rad; ++c.x)
				for(c.z = k-rad; c.z<=k+rad; ++c.z)
			{
				Chunk* ch = getChunk(c);
				if(!ch || !ch->isActive())
					pq.push(ChunkCompare(c, position, direction, ch));
			}

			for(int i=0;!pq.empty()&&i<2;++i)
			{
				if(!pq.top().chunk)
					createChunk(pq.top().pos);
				else
					pq.top().chunk->setActive(true);
				pq.pop();
			}
		}
	}
	//-----------------------------------------------------------------------
	
	void ChunkManager::killBlocks(Vector3 p,float radius)
	{

	}
	//-----------------------------------------------------------------------
	
	Chunk* ChunkManager::getChunk(ChunkCoords c)
	{
		if(mChunks.find(c)!=mChunks.end())
			return mChunks[c];
		return 0;
	}
	//-----------------------------------------------------------------------

	// hacky little tree addition thingy
	void addTree(byte data[CHUNK_SIZE_X][CHUNK_SIZE_Y][CHUNK_SIZE_Z], int i, int h, int k)
	{
		if(i>2&&i<CHUNK_SIZE_X-3&&
			k>2&&k<CHUNK_SIZE_Z-3&&rand()%200==0)
		{
			data[i][h+1][k] = 5;
			data[i][h+2][k] = 5;
			data[i][h+3][k] = 5;
			data[i][h+4][k] = 5;
			data[i+1][h+4][k] = 1;
			data[i][h+4][k+1] = 1;
			data[i][h+4][k-1] = 1;
			data[i-1][h+4][k] = 1;
			data[i+1][h+4][k+1] = 1;
			data[i-1][h+4][k-1] = 1;
			data[i+1][h+4][k-1] = 1;
			data[i-1][h+4][k+1] = 1;
			data[i+1][h+4][k+2] = 1;
			data[i][h+4][k+2] = 1;
			data[i-1][h+4][k+2] = 1;
			data[i+1][h+4][k-2] = 1;
			data[i][h+4][k-2] = 1;
			data[i-1][h+4][k-2] = 1;
			data[i+2][h+4][k+1] = 1;
			data[i+2][h+4][k] = 1;
			data[i+2][h+4][k-1] = 1;
			data[i-2][h+4][k-1] = 1;
			data[i-2][h+4][k] = 1;
			data[i-2][h+4][k+1] = 1;
			data[i][h+5][k] = 5;
			data[i+1][h+5][k] = 1;
			data[i][h+5][k+1] = 1;
			data[i][h+5][k-1] = 1;
			data[i-1][h+5][k] = 1;
			data[i+1][h+5][k+1] = 1;
			data[i-1][h+5][k-1] = 1;
			data[i+1][h+5][k-1] = 1;
			data[i-1][h+5][k+1] = 1;
			data[i+1][h+5][k+2] = 1;
			data[i][h+5][k+2] = 1;
			data[i-1][h+5][k+2] = 1;
			data[i+1][h+5][k-2] = 1;
			data[i][h+5][k-2] = 1;
			data[i-1][h+5][k-2] = 1;
			data[i+2][h+5][k+1] = 1;
			data[i+2][h+5][k] = 1;
			data[i+2][h+5][k-1] = 1;
			data[i-2][h+5][k-1] = 1;
			data[i-2][h+5][k] = 1;
			data[i-2][h+5][k+1] = 1;
			data[i][h+6][k] = 5;
			data[i+1][h+6][k] = 1;
			data[i][h+6][k+1] = 1;
			data[i][h+6][k-1] = 1;
			data[i-1][h+6][k] = 1;
			data[i+1][h+6][k+1] = 1;
			data[i-1][h+6][k-1] = 1;
			data[i+1][h+6][k-1] = 1;
			data[i-1][h+6][k+1] = 1;
			data[i+1][h+7][k] = 1;
			data[i][h+7][k+1] = 1;
			data[i][h+7][k-1] = 1;
			data[i-1][h+7][k] = 1;
			data[i][h+7][k] = 1;
		}
	}
	//-----------------------------------------------------------------------

	Chunk* ChunkManager::createChunk(ChunkCoords c)
	{
		if(getChunk(c))
			return 0;

		Chunk* ch = new Chunk(Vector3(c.x*CHUNK_SIZE_X-CHUNK_SIZE_X/2,c.y*CHUNK_SIZE_Y-CHUNK_SIZE_Y/2,
			c.z*CHUNK_SIZE_Z-CHUNK_SIZE_Z/2),this,0);

		PerlinVolume v = PerlinVolume(mPerlin,c.x,c.y,c.z,0.35);// elevantion
		PerlinVolume v2 = PerlinVolume(mPerlin,c.x,c.y,c.z,0.5);// roughness
		PerlinVolume v3 = PerlinVolume(mPerlin,c.x,c.y,c.z,1.5f);// detail
		
		for(int i=0;i<CHUNK_SIZE_X;++i)
			for(int k=0;k<CHUNK_SIZE_Z;++k)
		{
			int height = 20 + v.sample(i,0,k)*15.0 + v2.sample(i,0,k)*v3.sample(i,0,k)*11.0;
			for(int j=0;j<height;++j)
				ch->blocked[i][j][k] = (j==height-1) ? 4 : 3;
			addTree(ch->blocked,i,height-1,k);
		}

		for(int i=0;i<6;++i)
			ch->neighbors[i] = getChunk(c<<i);

		ch->markDirty();
		ch->notifyNeighbors();
		mChunks[c] = ch;
		return ch;
	}
	//-----------------------------------------------------------------------
	
	void ChunkManager::setMaterial(String material,size_t atlasDimensions)
	{
		std::map<ChunkCoords,Chunk*>::iterator it = mChunks.begin();

		while(it!=mChunks.end())
		{
			it->second->setMaterial(material,atlasDimensions);
			++it;
		}
	}
	//-----------------------------------------------------------------------
	

	ChunkManager::PerlinVolume::PerlinVolume(noise::module::Perlin* p,int x, int y, int z, float scale)
	{
		mNoise = p;
		for(int i=0;i<5;++i)for(int j=0;j<1;++j)for(int k=0;k<5;++k)
		{
			double v = mNoise->GetValue(
				(x*16+i*(4))/60.0 * scale,
				(y*16+j*(4))/60.0 * scale,
				(z*16+k*(4))/60.0 * scale);
			mData[i][j][k] = v;
		}
	}
	//-----------------------------------------------------------------------

	double ChunkManager::PerlinVolume::sample(int x, int y, int z)
	{
		/* Trilinear interpolation
		double xd = static_cast<float>(x%4)/4;
		double yd = static_cast<float>(y%8)/8;
		double zd = static_cast<float>(z%4)/4;
		x/=4;
		y/=8;
		z/=4;

		
		// push back along z axis
		double i1 = mData[x][y+1][z]*(1-zd) + mData[x][y+1][z+1]*zd;
		double i2 = mData[x][y][z]*(1-zd) + mData[x][y][z+1]*zd;

		double j1 = mData[x+1][y+1][z]*(1-zd) + mData[x+1][y+1][z+1]*zd;
		double j2 = mData[x+1][y][z]*(1-zd) + mData[x+1][y][z+1]*zd;

		// push down the y axis
		double w1 = i1 * (1 - yd) + i2 * yd;
		double w2 = j1 * (1 - yd) + j2 * yd;

		// and finally interpolate accross the x
		double out = w1 * (1 - xd) + w2 * xd;

		return out;*/

		// bilinear interpolation
		// TODO: make 2d/3d an option, or separate class?
		double xd = static_cast<float>(x%4)/4;
		double zd = static_cast<float>(z%4)/4;
		x/=4;
		z/=4;
		
		// push along x axis
		double r1 = mData[x][0][z] * (1-xd) + mData[x+1][0][z] * xd;
		double r2 = mData[x][0][z+1] * (1-xd) + mData[x+1][0][z+1] * xd;

		double out = r1 * (1 - zd) + r2 * zd;
		return out;
	}
}

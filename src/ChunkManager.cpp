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
		:radius(12),mLast(10,10,10)
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
;
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

			if(mLast.x!=i||mLast.z!=k)//||mLast.y!=j)
			{			
				ChunkCoords c = (i-radius,0,k-radius);

				for(c.x = i-radius; c.x<=i+radius; ++c.x)
					for(c.z = k-radius; c.z<=k+radius; ++c.z)
						createChunk(c);

				int radius2 = radius==12 ? 4 : 2;
				c = (i-radius2,0,k-radius2);
				for(c.x = i-radius2; c.x<=i+radius2; ++c.x)
					for(c.z = k-radius2; c.z<=k+radius2; ++c.z)
							if(!mChunks[c]->isActive())
								mChunks[c]->setActive(1);
			}
			mLast = ChunkCoords(i,0,k);
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

			for(int i=0;!pq.size()<=0&&i<2;++i)
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
	
	Chunk* ChunkManager::createChunk(ChunkCoords c)
	{
		if(getChunk(c))
			return 0;

		byte data[CHUNK_SIZE_X][CHUNK_SIZE_Y][CHUNK_SIZE_Z];

		PerlinVolume v = PerlinVolume(mPerlin,c.x,c.y,c.z);

		memset(data,(byte)0,CHUNK_VOLUME);

		// TODO Use 3d perlin, but use less samples and lerp

		// 3d perlin
		/*for(int i=0;i<CHUNK_SIZE_X;++i)
			for(int j=0;j<CHUNK_SIZE_Y;++j)
				for(int k=0;k<CHUNK_SIZE_Z;++k)
		{
			if(j==0)
				data[i][j][k] = 3;
			else if(j==15)
				data[i][j][k] = 0;
			else
			{
				double height = v.sample(i,j,k);//mPerlin->GetValue(static_cast<double>(i+c.x*16)/30.0,static_cast<double>(j+c.y*16)/30.0,static_cast<double>(k+c.z*16)/30.0);
				if(height<0)
					data[i][j][k] = 0;
				else 
					data[i][j][k] = 3;
			}
		}*/


		for(int i=0;i<CHUNK_SIZE_X;++i)
			//for(int j=0;j<CHUNK_SIZE_Y;++j)
				for(int k=0;k<CHUNK_SIZE_Z;++k)
		{
			//if(v.sample(i,j,k) > 0)
			//	data[i][j][k] = 3;
			//else
			//	data[i][j][k] = 0;
			double height = v.sample(i,0,k);//mPerlin->GetValue(static_cast<double>(i+c.x*16)/60.0,0.0,static_cast<double>(k+c.z*16)/60.0);
			//height+=0.2;
			//if(height>1.0)
			//	height = 1.0;
			height = (height+1.0)/2.0;
			int h = height*20+3;
			h+=20;
			for(int j=0;j<=h;++j)
			{
				int val = 2;
				if(h == j)
					val = 4;
				else if(h-j<4)
					val = 3;
				else if(rand()%100==0)
					val = 1;
				data[i][j][k] = val;
			}
		}

		Chunk* ch = new Chunk(Vector3(c.x*CHUNK_SIZE_X-CHUNK_SIZE_X/2,c.y*CHUNK_SIZE_Y-CHUNK_SIZE_Y/2,
			c.z*CHUNK_SIZE_Z-CHUNK_SIZE_Z/2),this,&data[0][0][0]);

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
	

	ChunkManager::PerlinVolume::PerlinVolume(noise::module::Perlin* p,int x, int y, int z)
	{
		mNoise = p;
		for(int i=0;i<SAMPLE_X;++i)for(int j=0;j<SAMPLE_Y;++j)for(int k=0;k<SAMPLE_Z;++k)
		{
			double v = mNoise->GetValue(
				(x*16+i*(4))/60.0,
				(y*8+j*(2))/60.0,
				(z*16+k*(4))/60.0);

			mData[i][j][k] = v;
		}
	}
	//-----------------------------------------------------------------------

	double ChunkManager::PerlinVolume::sample(int x, int y, int z)
	{
		// good 'ol trilinear interpolation
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

		return out;
	}
}

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
	ChunkManager::ChunkManager(Vector3 position)
		:radius(10),mLast(10,10,10)
	{
		mPerlin = new noise::module::Perlin();
		generate(position);
		radius = 5;
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
	
	void ChunkManager::generate(Vector3 position)
	{
		int i = (position.x+CHUNK_SIZE_X/2)/CHUNK_SIZE_X;
		int k = (position.z+CHUNK_SIZE_Z/2)/CHUNK_SIZE_Z;

		if(mLast.x!=i||mLast.z!=k)//||mLast.y!=j)
		{			
			ChunkCoords c = (i-radius,0,k-radius);

			for(c.x = i-radius; c.x<=i+radius; ++c.x)
				for(c.z = k-radius; c.z<=k+radius; ++c.z)
					createChunk(c);

			int radius2 = radius==10 ? 3 : 2;
			c = (i-radius2,0,k-radius2);
			for(c.x = i-radius2; c.x<=i+radius2; ++c.x)
				for(c.z = k-radius2; c.z<=k+radius2; ++c.z)
						if(!mChunks[c]->isActive())
							mChunks[c]->setActive(1);
		}
		mLast = ChunkCoords(i,0,k);
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

		memset(data,(byte)0,CHUNK_VOLUME);

		for(int i=0;i<CHUNK_SIZE_X;++i)
			for(int j=0;j<CHUNK_SIZE_Y;++j)
				for(int k=0;k<CHUNK_SIZE_Z;++k)
		{
			if(j==0)
				data[i][j][k] = 3;
			else if(j==15)
				data[i][j][k] = 0;
			else
			{
				double height = mPerlin->GetValue(static_cast<double>(i+c.x*16)/30.0,static_cast<double>(j+c.y*16)/30.0,static_cast<double>(k+c.z*16)/30.0);
				if(height<0)
					data[i][j][k] = 0;
				else 
					data[i][j][k] = 3;
			}
		}


		/*for(int i=0;i<CHUNK_SIZE_X;++i)
			//for(int j=0;j<CHUNK_SIZE_Y;++j)
				for(int k=0;k<CHUNK_SIZE_Z;++k)
		{
			double height = mPerlin->GetValue(static_cast<double>(i+c.x*16)/60.0,0.0,static_cast<double>(k+c.z*16)/60.0);
			//height+=0.2;
			//if(height>1.0)
			//	height = 1.0;
			height = (height+1.0)/2.0;
			int h = height*10+3;
			//h+=20;
			//if(h > 12)
			//	h = 12;
			//if(h<2)
			//	h = 2;
			for(int j=0;j<=h;++j)
			{
				int val = 2;
				if(h == j)
					val = 4;
				else if(h-j<4)
					val = 3;
				else if(rand()%100==0)
					val = 1;
				
				//if(h-j<10&&mPerlin->GetValue(static_cast<double>(i+c.x*16)/60.0,static_cast<double>(j)/3.0,static_cast<double>(k+c.z*16)/60.0)<0.0)
				//	val = 0;

				data[i][j][k] = val;
			}
				//data[i][j][k] = 0;
				//if(j<12)
				//{
				//	if(rand()%100==0)
				//		data[i][j][k] = 1;
				//	else
				//		data[i][j][k] = 2;
				//}
				//else if(j<14)
				//	data[i][j][k] = 3;
				//else if(j<15)
				//	data[i][j][k] = 4;
		}*/

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
}

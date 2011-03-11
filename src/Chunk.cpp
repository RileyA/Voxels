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
#include "Chunk.h"
#include "ChunkUtils.h"
#include "ChunkManager.h"
#include "OryxMessageAny.h"
#include "OryxEventHandler.h"
#include "OryxTimeManager.h"
#include "OgreSubsystem/OgreSubsystem.h"
#include "OgreSubsystem/Mesh.h"

namespace Oryx
{
	void quad(Vector3 pos,int normal,MeshData& d,short type,Colour diffuse,size_t atlas);

	//-----------------------------------------------------------------------

	Chunk::Chunk(Vector3 position,ChunkManager* parent,byte* data)
		:mPosition(position),mParent(parent)
	{	
		mAtlasDimensions = 16;
		mMaterial = "MeinKraft";
		mChunk = 0;
		mPosition = position;
		mOgre = Engine::getPtr()->getSubsystem("OgreSubsystem")->castType<OgreSubsystem>();

		memset(light,0,CHUNK_VOLUME);
		memcpy(blocked,data,CHUNK_VOLUME);
	
		BulletSubsystem* b = Engine::getPtr()->getSubsystem("BulletSubsystem")->
			castType<BulletSubsystem>();

		mBlock = new PhysicsBlock(0,0,0,position,0,this);

		mActive = false;
		mDirty = false;

		for(int p=0;p<6;++p)
			neighbors[p] = 0;
	}
	//-----------------------------------------------------------------------
	
	Chunk::~Chunk()
	{
		delete mBlock;
	}
	//-----------------------------------------------------------------------

	void Chunk::update(Real delta){}
	//-----------------------------------------------------------------------
	
	void Chunk::build(bool physics)
	{
		if(!mActive)
		{
			mDirty = false;
			return;
		}
		MeshData d;
		d.addTexcoordSet();
		d.addTexcoordSet();
		physics = mDirty||physics;

		for(int i=0;i<CHUNK_SIZE_X;++i)for(int j=0;j<CHUNK_SIZE_Y;++j)for(int k=0;k<CHUNK_SIZE_Z;++k)
		{
			byte type = blocked[i][j][k];

			if(!type)
			{
				// very ugly, but hard coding this all seems a good deal more efficient 
				// than my attempt at a cleaner version...
				bool adjacents[6] = {
					(i>0&&!blockSolid(blocked[i-1][j][k])) || (i==0&&!neighbors[0]) ||(i==0&&neighbors[0]
						&&!blockSolid(neighbors[0]->blocked[CHUNK_SIZE_X-1][j][k])),
					(i<CHUNK_SIZE_X-1&&!blockSolid(blocked[i+1][j][k])) || (i==CHUNK_SIZE_X-1 &&!neighbors[1])||
						(i==CHUNK_SIZE_X-1&&neighbors[1] && !blockSolid(neighbors[1]->blocked[0][j][k])),
					(j>0&&!blockSolid(blocked[i][j-1][k])) || (j==0&&!neighbors[2]) ||
						(j==0&&neighbors[2]&&!blockSolid(neighbors[2]->blocked[i][CHUNK_SIZE_Y-1][k])),
					(j<CHUNK_SIZE_Y-1&&!blockSolid(blocked[i][j+1][k])) || (j==CHUNK_SIZE_Y-1&&!neighbors[3])||
						(j==CHUNK_SIZE_Y-1&&neighbors[3]&&!blockSolid(neighbors[3]->blocked[i][0][k])),
					(k>0&&!blockSolid(blocked[i][j][k-1])) || (k==0&&!neighbors[4]) ||
						(k==0&&neighbors[4]&&!blockSolid(neighbors[4]->blocked[i][j][CHUNK_SIZE_Z-1])),
					(k<CHUNK_SIZE_Z-1&&!blockSolid(blocked[i][j][k+1])) || (k==CHUNK_SIZE_Z-1&&!neighbors[5])||
						(k==CHUNK_SIZE_Z-1&&neighbors[5]&&!blockSolid(neighbors[5]->blocked[i][j][0]))};

				if(adjacents[0]&&adjacents[1]&&adjacents[2]&&adjacents[3]&&adjacents[4]&&adjacents[5])
					continue;

				ChunkCoords bc(i,j,k);

				byte lit = getLightState(bc);

				byte lt[6] = {0,0,0,0,0,0};
				for(int p=0;p<6;++p)
					if(adjacents[p])
						lt[p] = ChunkUtils::getLight(this,bc<<p);
				
				for(int p=0;p<6;++p)
					if(!adjacents[p])
						makeQuad(bc,Vector3(i,j,k),p,d,MAPPINGS[ChunkUtils::getBlock(this,bc<<p)][5-p],LIGHTVALUES[lit],adjacents,lt);
			}
		}

		if(mChunk)
			mOgre->destroySceneNode(mChunk);

		mChunk = mOgre->createMesh(d);
		mChunk->setMaterialName(mMaterial);
		mOgre->getRootSceneNode()->addChild(mChunk);
		mChunk->setPosition(mPosition);

		//if(physics)
			rebuildPhysics(d);

		mDirty = false;
	}
	//-----------------------------------------------------------------------
	
	void Chunk::killBlock(Vector3 p,Vector3 n)
	{
		if(!mActive)
			return;
		p += OFFSET;
		p -= mPosition;

		Vector3 pn = p-n*0.5f;

		int i = floor(pn.x+0.5);
		int j = floor(pn.y+0.5);
		int k = floor(pn.z+0.5);

		ChunkUtils::setBlock(this,ChunkCoords(i,j,k),0);
	}
	//-----------------------------------------------------------------------
	
	void Chunk::killBlocks(Vector3 p,float radius)
	{
		if(!mActive)
			return;
		p += OFFSET;
		p -= mPosition;

		int i = floor(p.x+0.5);
		int j = floor(p.y+0.5);
		int k = floor(p.z+0.5);

		for(int x=i-radius-2;x<radius+i+2;++x)
			for(int y=j-radius-2;y<radius+j+2;++y)
				for(int z=k-radius-2;z<radius+k+2;++z)
		{
			if(Vector3(x,y,z).distance(p)<radius)
				ChunkUtils::setBlock(this,ChunkCoords(x,y,z),0);
		}
	}
	//-----------------------------------------------------------------------
	
	void Chunk::addBlock(Vector3 p,Vector3 n,byte type)
	{
		if(!mActive)
			return;
		p += OFFSET;
		p-=mPosition;
		float x = (n.x<0) ? -n.x : n.x;
		float y = (n.y<0) ? -n.y : n.y;
		float z = (n.z<0) ? -n.z : n.z;

		Vector3 pn = p+n*-0.4f;

		if(x>y&&x>z)
			n = Vector3(n.x,0,0);
		else if(y>x&&y>z)
			n = Vector3(0,n.y,0);
		else if(z>x&&z>y)
			n = Vector3(0,0,n.z);
		else
			return;

		pn += n*0.6f;

		int i = floor(pn.x+0.5);
		int j = floor(pn.y+0.5);
		int k = floor(pn.z+0.5);

		ChunkUtils::setBlock(this,ChunkCoords(i,j,k),type);
	}
	//-----------------------------------------------------------------------

	void Chunk::rebuildPhysics(MeshData& d)
	{
		if(!mActive)
			return;
		mBlock->rebuild(d);
	}
	//-----------------------------------------------------------------------

	void Chunk::notifyNeighbors()
	{
		for(int i=0;i<6;++i)
		{
			if(neighbors[i])
			{
				neighbors[i]->neighbors[i-AXIS_OFFSET[i]] = this;
				neighbors[i]->markDirty();
			}
		}
	}
	//-----------------------------------------------------------------------

	void Chunk::clearLighting()
	{	
		memset(light,(byte)0,CHUNK_VOLUME);
	}
	//-----------------------------------------------------------------------

	void Chunk::secondaryLighting()
	{
		for(int p=0;p<6;++p)
		{
			if(neighbors[p])
			{
				byte normal = AXIS[p];
				byte aX = p > 1 ? 0 : 1;
				byte aY = p < 4 ? 2 : 1;

				byte d1 = (p&1)==0 ? CHUNKSIZE[normal]-1 : 0;
				byte d2 = (p&1)==0 ? 0 : CHUNKSIZE[normal]-1;
				
				ChunkCoords coords = ChunkCoords(0,0,0);
				coords[normal] = d1;
				
				for(coords[aX]=0;coords[aX]<CHUNKSIZE[aX];++coords[aX])
					for(coords[aY]=0;coords[aY]<CHUNKSIZE[aY];++coords[aY])
				{
					byte value = neighbors[p]->getLightState(coords);
					if(value>1)
					{
						coords[normal] = d2;
						getLighting(coords,value-1);
						coords[normal] = d1;
					}
				}
			}
		}
	}
	//-----------------------------------------------------------------------

	void Chunk::localLighting()
	{
		clearLighting();

		for(int i=0;i<CHUNK_SIZE_X;++i)for(int k=0;k<CHUNK_SIZE_Z;++k)
		{
			int highest = 0;

			int lit = MAX_LIGHT;

			for(int y=CHUNK_SIZE_Y-1;y>=0;--y)
			{
				byte val = blockTransparent(blocked[i][y][k]);
				if(val==0||lit==0)
					break;
				else
					lit-=val-1;
				light[i][y][k] = lit;
			}
		}

		ChunkCoords c = ChunkCoords(0,0,0);

		for(c.x=0;c.x<CHUNK_SIZE_X;++c.x)
			for(c.y=0;c.y<CHUNK_SIZE_Y;++c.y)
				for(c.z=0;c.z<CHUNK_SIZE_Z;++c.z)
		{
			byte ls =getLightState(c); 
			if(ls>1)
				getLighting(c,ls,true);
		}
	}
	//-----------------------------------------------------------------------
	
	void Chunk::setMaterial(String material,size_t atlas)
	{
		if(mChunk&&mActive)
		{
			mMaterial = material;
			mAtlasDimensions = atlas;
			mChunk->setMaterialName(material);
		}
	}
	//-----------------------------------------------------------------------

	void Chunk::getLighting(ChunkCoords& coords, int lightVal, bool emitter)
	{
		int8 trans;
		if(lightVal > 0 && coords.inBounds() && (trans = getTransparency(coords)) &&
			(emitter || setLight(coords,lightVal) ))
		{
			for(int i = BD_LEFT; i <= BD_BACK; ++i)
			{
				coords[AXIS[i]] += AXIS_OFFSET[i];
				getLighting(coords, lightVal - trans);
				coords[AXIS[i]] -= AXIS_OFFSET[i];
			}
		}
	}
	//-----------------------------------------------------------------------


	void Chunk::makeQuad(ChunkCoords& cpos,Vector3 pos,int normal,MeshData& d,short type,float diffuse,bool* adj,byte* lights)
	{
		Vector2 offset;
		int dims = mAtlasDimensions;
		int tp = type-1;
		float gridSize = 1.f/dims;
		offset = Vector2(tp%dims*gridSize,tp/dims*gridSize);
		pos -= OFFSET;
		for(int i=0;i<6;++i)
			d.vertex(pos+BLOCK_VERTICES[normal][5-i],offset+BLOCK_TEXCOORDS[normal][5-i]*gridSize);
		diffuse *= LIGHT_STEPS[5-normal];

		for(int i=0;i<6;++i)
		{
			#ifdef CHUNK_NORMALS
			d.normals.push_back(BLOCK_NORMALS[normal].x);
			d.normals.push_back(BLOCK_NORMALS[normal].y);
			d.normals.push_back(BLOCK_NORMALS[normal].z);
			#endif

			float lighting = diffuse;
			
			#ifdef SMOOTH_LIGHTING

			float ltDiagonal = 0.f;

			if(adj[LIGHTING_COORDS[normal][FILTERVERTEX[i]][0]] ||
				adj[LIGHTING_COORDS[normal][FILTERVERTEX[i]][1]])
			{
				ChunkCoords cc;
				cc = cpos<<LIGHTING_COORDS[normal][FILTERVERTEX[i]][0];
				cc = cc<<LIGHTING_COORDS[normal][FILTERVERTEX[i]][1];
				ltDiagonal = LIGHTVALUES[ChunkUtils::getLight(this,cc)];	
			}

			lighting += LIGHTVALUES[lights[LIGHTING_COORDS[normal][FILTERVERTEX[i]][0]]]
				+ LIGHTVALUES[lights[LIGHTING_COORDS[normal][FILTERVERTEX[i]][1]]] + ltDiagonal;

			lighting/=4.f;

			#endif

			d.diffuse.push_back(lighting);
			d.diffuse.push_back(lighting);
			d.diffuse.push_back(lighting);
			d.diffuse.push_back(1.f);
		}
	}
	//-----------------------------------------------------------------------
}


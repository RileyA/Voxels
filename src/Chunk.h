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

#ifndef VERSUS_CHUNK_H
#define VERSUS_CHUNK_H

#include "Oryx.h"
#include "OryxEngine.h"
#include "OryxObject.h"
#include "OgreSubsystem/OgreSubsystem.h"
#include "OISSubsystem/OISSubsystem.h"
#include "BulletSubsystem/BulletSubsystem.h"

#include "ChunkCoords.h"
#include "PhysicsBlock.h"

namespace Oryx
{
	class ChunkManager;

	/** A "Chunk" of voxels, represented as cubes (a.k.a. Minecraft-style rendering) */
	class Chunk
	{
	public:

		/** Constructor
		 *		@remarks There should be no need to manually construct Chunks if
		 *			you're using a ChunkManager.
		 *		@param position The position of this chunk
		 *		@param parent The ChunkManager this Chunk belongs to */
		Chunk(Vector3 position, ChunkManager* parent,byte* data);

		/** Destructor, cleans up physics and mesh data */
		virtual ~Chunk();

		/** Updates this Chunk
		 *		@param delta The time passed since the last frame (in seconds) */
		void update(Real delta);

		/** Rebuilds the graphical mesh of this object
		 *		@param physics Whether or not to update the physics 
		 *			representation as well*/
		void build(bool physics = false);

		/** Destroys all blocks in a given radius (useful for explosions, etc)
		 *		@param p The origin position 
		 *		@param radius The radius in which to destroy blocks */
		void killBlocks(Vector3 p,float radius);

		/** Destroys a block based on raycast data
		 *		@remarks Since you'll often wind up with a pos/normal pair from
		 *			a raycast, but no idea which block it is, this simplifies things
		 *		@param p The position of the raycast hit 
		 *		@param n The surface normal at the impact point */
		void killBlock(Vector3 p,Vector3 n);

		/** Adds a block based on raycast data 
		 *		@remarks Since you'll often wind up with a pos/normal pair from
		 *			a raycast, but no idea which block it is, this simplifies things
		 *		@param p The position of the raycast hit 
		 *		@param n The surface normal at the impact point */
		void addBlock(Vector3 p,Vector3 n,byte tp=1);

		/** Does a full physics rebuild 
		 *		@remarks This should get called appropriately by build(), but you
		 *			can force a manual rebuild if you so please. */
		void rebuildPhysics(MeshData& d);

		/** Gives this Chunk's neighbors a pointer to this Chunk
		 *		@remarks This should only need to be called by the ChunkManager
		 *			on creation of chunks, but there may be some need to do so
		 *			manually (alternate Chunkmanager implementation, etc...) */
		void notifyNeighbors();

		/** Calculates lighting local to this chunk */
		void localLighting();

		/** Calculates light that "bleeds over" from neighboring chunks */
		void secondaryLighting();

		/** memsets all the lighting values to zero (used when recalculating lighting) */
		void clearLighting();
		
		/** Sets the light at a given coordinate
		 *		@param coords The coordinates (MUST be in chunk bounds, 
		 *			this will segfault otherwise) 
		 *		@return Whether or not the lighting value was set (this function 
		 *			only brightens) */
		bool setLight(const ChunkCoords& coords,byte lit)
		{
			if(light[coords.x][coords.y][coords.z]<lit)
			{
				light[coords.x][coords.y][coords.z] = lit;
				return true;
			}
			return false;
		}

		/** Returns the state of a block using a ChunkCoords object
		 *		@param c The coords (must be in bounds) */
		byte getBlockState(const ChunkCoords& c)
		{
			return blocked[c.x][c.y][c.z];
		}

		/** Returns the lighting state of a block using a ChunkCoords object
		 *		@param c The coords (must be in bounds) */
		byte getLightState(const ChunkCoords& c) const
		{
			return light[c.x][c.y][c.z];
		}

		/** Gets whether or not the block at this point is solid */
		bool isSolid(const ChunkCoords& c)
		{
			return blockSolid(getBlockState(c));
		}
		
		/** Gets whether the block at the coords is transparent (allows light through) */
		byte getTransparency(const ChunkCoords& c)
		{
			return blockTransparent(getBlockState(c));
		}

		/** Gets whether or not this Chunk needs to be updated
		 *		@remarks This should only be true if a block was directly
		 *			added or removed in this chunk or on its border.
		 *		@return Whether or not this chunk needs to be updates*/
		bool needsUpdate()
		{
			return mDirty;
		}

		/** Marks this block as needing an update */
		void markDirty()
		{
			mDirty = true;
		}

		void setMaterial(String mat,size_t atlas);

		bool isActive(){return mActive;}
		void setActive(bool act){mActive = act;markDirty();}
	
		// The states of the voxels
		byte blocked[CHUNK_SIZE_X][CHUNK_SIZE_Y][CHUNK_SIZE_Z];
		byte light[CHUNK_SIZE_X][CHUNK_SIZE_Y][CHUNK_SIZE_Z];
		
		// Neightboring blocks
		Chunk* neighbors[6];

	private:

		/** Recursive lighting function, traverses the chunk from light sources
		 *		stepping the light down one notch each move 
		 *		@param c The coordinates to light from 
		 *		@param light The light value at this point 
		 *		@param If this is an emitter */
		void getLighting(ChunkCoords& c,int lightVal,bool emitter=false);
		
		// TODO: Get rid of one of these, the original need for separate notions
		// of 'solid' and 'transparent' is gone...

		/** Helper that gets whether a type of block is solid */
		bool blockSolid(byte chunkValue)
		{
			#ifdef ALLOW_BLOCK_TRANSPARENCY
				return !TRANSPARENT[chunkValue];
			#else
				return chunkValue;
			#endif
		}

		/** Helper that gets whether a type of block is transparent*/
		byte blockTransparent(byte chunkValue)
		{
			#ifdef ALLOW_BLOCK_TRANSPARENCY
				return TRANSPARENT[chunkValue];
			#else
				return !chunkValue;
			#endif
		}

		/** Adds a 1 meter quad (actually 2 tris) to a given MeshData object */
		void makeQuad(ChunkCoords& cpos,Vector3 pos,int normal,MeshData& d,short type,Real diffuse,bool* adjs,byte* lights);

		// The graphics object
		Mesh* mChunk;

		// The parent ChunkManager, used to get info about block types etc
		ChunkManager* mParent;

		// does this Chunk need an update?
		bool mDirty;

		// Pointer to the graphics Subsystem
		OgreSubsystem* mOgre;

		// This Chunk's poition in world-space
		Vector3 mPosition;

		// The physics objects (4x4x4 compounds of up to 4x4x4 Box primitives each)
		PhysicsBlock* mBlock;//s[CHUNK_STEPS_X][CHUNK_STEPS_Y][CHUNK_STEPS_Z];

		// The current material
		String mMaterial;

		// dimensions of the texture atlas
		size_t mAtlasDimensions;

		int mNumBlocks;

		bool mActive;
	};
}

#endif	

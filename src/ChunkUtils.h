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

#ifndef CHUNKUTILS_H
#define CHUNKUTILS_H

#include "Oryx.h"
#include "Chunk.h"

namespace Oryx
{
	/** A collection of little helper functions */
	class ChunkUtils
	{
	public:
		/** Takes a pointer to a pointer to a Chunk and a set of coords
		 *		and corrects the pointer to point at the appropriate chunk
		 *		based on those coords (and the coords to point at the right
		 *		spot in the Chunk.
		 *	@param c A pointer to a pointer to a chunk 
		 *	@param coords Reference to the coordinates*/
		static Chunk* correctChunkCoords(Chunk* c, ChunkCoords& coords)
		{
			if(!c)
				return c;
			for(int i = 0; i < 3; ++i)
			{
				if(coords[i]<0)
				{
					coords[i]+=CHUNKSIZE[i];
					return correctChunkCoords(c->neighbors[i*2],coords);	
				}
				else if(coords[i]>CHUNKSIZE[i]-1)
				{
					coords[i]-=CHUNKSIZE[i];
					return correctChunkCoords(c->neighbors[i*2+1],coords);
				}
			}
			return c;
		}
		
		/** Sets a given block to a given state
		 *		@param c The Chunk to start in
		 *		@param coords The coordinates to use 
		 *		@param type The type to make this block (0 is nothing) */
		static void setBlock(Chunk* c, ChunkCoords coords, byte type)
		{
			c = correctChunkCoords(c,coords);
			if(c && c->blocked[coords.x][coords.y][coords.z]!=type)
			{
				c->blocked[coords.x][coords.y][coords.z] = type;
				c->markDirty();
			}
		}
		
		/** Gets the lighting value of a set block
		 *		@param Chunk c The Chunk of origin
		 *		@param coords The coordinates to look at */
		static byte getLight(Chunk* c, ChunkCoords coords)
		{
			c = correctChunkCoords(c,coords);
			return c ? c->light[coords.x][coords.y][coords.z] : MAX_LIGHT;
		}

		/** Gets the type value of a set block
		 *		@param Chunk c The Chunk of origin
		 *		@param coords The coordinates to look at */
		static byte getBlock(Chunk* c, ChunkCoords coords)
		{
			c = correctChunkCoords(c,coords);
			return c ? c->blocked[coords.x][coords.y][coords.z] : 0;
		}	
	};
}

#endif

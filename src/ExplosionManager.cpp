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

#include "ExplosionManager.h"

namespace Oryx
{
	ExplosionManager::ExplosionManager()
	{
		
	}

	ExplosionManager::~ExplosionManager()
	{
		for(int i=0;i<mActive.size();++i)
			delete mActive[i];
		for(int i=0;i<mReserve.size();++i)
			delete mReserve[i];
	}

	void ExplosionManager::update(Real delta)
	{	
		for(int i=0;i<mActive.size();++i)
		{
			if(mActive[i]->update())
			{
				mReserve.push_back(mActive[i]);
				mActive.erase(mActive.begin()+i);
				--i;
			}
		}
	}

	void ExplosionManager::createExplosion(Vector3 position)
	{
		//ALSubsystem* al = Engine::getPtr()->getSubsystem("ALSubsystem")->castType<ALSubsystem>();
		//al->play2D("../media/audio/boom.wav");
		if(mReserve.empty())
		{
			mActive.push_back(new Explosion());
			mActive.back()->boom();
			mActive.back()->node->setPosition(position);
		}
		else
		{
			mActive.push_back(mReserve[0]);
			mReserve[0]->boom();
			mReserve[0]->node->setPosition(position);
			mReserve.erase(mReserve.begin());
		}
	}
}

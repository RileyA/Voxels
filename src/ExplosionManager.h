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

#ifndef EXPLOSIONMAN
#define EXPLOSIONMAN

#include "Oryx.h"
#include "OryxObject.h"
#include "OgreSubsystem/OgreSubsystem.h"
#include "ALSubsystem/ALSubsystem.h"
#include "OryxEngine.h"
#include "OryxTimeManager.h"

namespace Oryx
{
	struct Explosion
	{
		SceneNode* node;
		ParticleSystem* flame;
		ParticleSystem* smoke;
		ParticleSystem* sparks;
		ParticleSystem* smallSparks;
		ParticleSystem* trails;
		ParticleSystem* flash;
		float lscale;
		float delay;

		Explosion()
		{
			node = 0;
			flame = 0;
			flash = 0;
			smallSparks = 0;
			trails = 0;
			sparks = 0;
			smoke = 0;
		}

		void scale(Real s)
		{
			lscale = s;
			flame->scale(1/lscale*s);
			smoke->scale(1/lscale*s);
			sparks->scale(1/lscale*s);
			smallSparks->scale(1/lscale*s);
			trails->scale(1/lscale*s);
			flash->scale(1/lscale*s);
			node->setScale(Vector3(s,s,s));
		}

		void boom()
		{
			delay = 0.2f;

			if(!node)
			{
				OgreSubsystem* ogre = Engine::getPtr()->getSubsystem("OgreSubsystem")->castType
					<OgreSubsystem>();
				node = 	ogre->createSceneNode();
				ogre->getRootSceneNode()->addChild(node);
				smoke = ogre->createParticleSystem("explosion/smoke");
				sparks = ogre->createParticleSystem("explosion/longSparks");
				flash = ogre->createParticleSystem("explosion/flash");
				smallSparks = ogre->createParticleSystem("explosion/sparks");
				flame = ogre->createParticleSystem("explosion/flame");
				trails = ogre->createParticleSystem("explosion/smokeTrails");
				node->addChild(flame);
				node->addChild(smoke);
				node->addChild(trails);
				node->addChild(sparks);
				node->addChild(smallSparks);
				node->addChild(flash);
			}
			else
			{
				node->setVisible(true);
				flame->restart();
				smoke->restart();
				trails->restart();
				smallSparks->restart();
				flash->restart();
				sparks->restart();
				//scale(0.4f);
			}

			scale(2.f);
		}

		bool update()
		{
			if(flame->isDone()&&
				sparks->isDone()&&
				smallSparks->isDone()&&
				trails->isDone()&&
				flash->isDone()&&
				smoke->isDone()&&delay<0.f)
			{
				node->setPosition(Vector3(0,0,0));
				node->setVisible(false);
				return true;
			}
			delay-=TimeManager::getPtr()->getDeltaTime();
			return false;
		}
		
	};

	class ExplosionManager : public Object
	{
	public:

		ExplosionManager();
		virtual ~ExplosionManager();
		void createExplosion(Vector3 position);
		void update(Real delta);

	private:

		std::vector<Explosion*> mActive;
		std::vector<Explosion*> mReserve;

	};
}

#endif

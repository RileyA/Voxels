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

// uuuuugly playpen code

#include "Oryx.h"
#include "VoxelGameState.h"
#include "Chunk.h"
#include "OryxStringUtils.h"
#include "OryxLogger.h"
#include "OgreSubsystem/GUIButton.h"
#include "ChaiscriptSubsystem/ChaiscriptSubsystem.h"

namespace Oryx
{
	VoxelGameState::VoxelGameState()
		:GameState()
	{
		toggle = false;
		toggle2 = false;
	}
	
	void VoxelGameState::init()
	{
		mTimer = 0.f;
		currentMat = 0;
		OgreSubsystem* ogre = mEngine->getSubsystem("OgreSubsystem")->castType<OgreSubsystem>();
		BulletSubsystem* bts = mEngine->getSubsystem("BulletSubsystem")->castType<BulletSubsystem>();
		OISSubsystem* ois = mEngine->getSubsystem("OISSubsystem")->castType<OISSubsystem>();
		ois->initInput(ogre->getWindowHandle(),false );
		ogre->setBackgroundColor(Colour(0.6f,0.6f,0.6f));
		ogre->setLinearFog(125.f,175.f,Colour(0.6f,0.6f,0.6f));
		mgr = new ExplosionManager();

		GUIScreen* scrn = ogre->getGUI()->createScreen(ogre->getMainViewport(),"TechDemo","Test");

		txt = new StaticText(scrn->getRootElement(0),
			"Text01",0,20,Vector2(0.01f,0.01f),Vector2(0.5,1.f/12.f),"Test!");
		txt->setColour(Colour(0.9,0.8,0.9));

		// create camera setup
		mCam = new FPSCamera();

		// start up the physics sim
		bts->startSimulation();

		// create voxel chunk generator
		cmgr = new ChunkManager(Vector3(0,-10,0));	

		// set gravity
		bts->setGravity(Vector3(0,-5,0));

		// TODO ch = bts->createQuantaCCT(Vector3(0,50,0));
		//
		ChaiscriptSubsystem* chai = mEngine->getSubsystem("ChaiscriptSubsystem")->castType<ChaiscriptSubsystem>();
		//chai->runString("getAL().play2D(\"../media/audio/boom.wav\");");
	}

	void VoxelGameState::update(Real delta)
	{
		txt->setCaption("FPS: "+StringUtils::toString(1.f/delta));
		OISSubsystem* ois = mEngine->getSubsystem("OISSubsystem")->castType<OISSubsystem>();
		OgreSubsystem* ogre = mEngine->getSubsystem("OgreSubsystem")->castType<OgreSubsystem>();
		BulletSubsystem* bts = mEngine->getSubsystem("BulletSubsystem")->castType<BulletSubsystem>();
	
		Plane pl = Plane(Vector3(0,1,0),0);
		Vector3 move = 
			mCam->getDirection()*((ois->isKeyDown("KC_S")*-1+ois->isKeyDown("KC_W"))*1) +
			mCam->mCamera->getAbsoluteRight()*(ois->isKeyDown("KC_D")-ois->isKeyDown("KC_A"));

		move = pl.projectVector(move);
		if(move.squaredLength()>0)
			move.normalize();

		//TODO ch->setMoveVector(move);

		if(ois->isKeyDown("KC_ESCAPE"))
			sendMessage(MessageAny<String>("kill"),"Engine");

		if(ois->isButtonDown("MB_Right")&&!toggle)
		{
			toggle = true;

			if(ois->isKeyDown("KC_V"))
			{
				++currentMat;
				if(currentMat>2)
					currentMat = 0;
				if(currentMat==0)
					cmgr->setMaterial("Debug1",2);
				else if(currentMat==1)
					cmgr->setMaterial("Debug2",2);
				else if(currentMat==2)
					cmgr->setMaterial("MeinKraft",16);
			}
			else
			{
				RaycastReport r = bts->raycast(mCam->getPosition(),mCam->getDirection(),10000.f,COLLISION_GROUP_3,COLLISION_GROUP_3);

				if(r.hit&&r.userData)
				{
					Chunk* xc = (Chunk*)r.userData;
					if(ois->isKeyDown("KC_G"))
					{
						xc->killBlocks(r.position,4);
						mgr->createExplosion(r.position);
					}
					else if(ois->isKeyDown("KC_T"))
					{
						Mesh* me = ogre->createMesh("Pig.mesh");
						me->setPosition(r.position);
						ogre->getRootSceneNode()->addChild(me);
						me->yaw(rand()%360);
					}
					else
						xc->killBlock(r.position,r.normal);
				}
			}
		}
		else if(!ois->isButtonDown("MB_Right")&&toggle)
		{
			toggle = false;
		}

		if(ois->isButtonDown("MB_Left")&&!toggle2)
		{				
		
			RaycastReport r = bts->raycast(mCam->getPosition(),mCam->getDirection(),10000.f,COLLISION_GROUP_3,COLLISION_GROUP_3);
			toggle2 = true;

			if(r.hit&&r.userData)
			{
				float t1 = TimeManager::getPtr()->getTimeDecimal();
				Chunk* xc = (Chunk*)r.userData;
				xc->addBlock(r.position,r.normal,rand()%4+1);
			}
		}
		else if(!ois->isButtonDown("MB_Left")&&toggle2)
		{
			toggle2 = false;
		}


		mTimer+=delta*(float)rand()/(float)RAND_MAX;
		cmgr->generate(mCam->getPosition());

		// TODO mCam->mPosNode->setPosition(ch->getPosition()+Vector3(0,1.2f,0));
	}

	void VoxelGameState::deinit()
	{
	}
}


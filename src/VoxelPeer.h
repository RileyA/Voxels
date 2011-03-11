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
#include "ENetSubsystem/Client.h"
#include "OgreSubsystem/OgreSubsystem.h"


namespace Oryx
{
	class TestPeer
	{
	public:

		TestPeer(byte id,String name)
		{
			username = name;
			mID = id;
			m = Engine::getPtr()->getSubsystem("OgreSubsystem")->castType<OgreSubsystem>()
				->createMesh("Arrow.mesh");
			Engine::getPtr()->getSubsystem("OgreSubsystem")->castType<OgreSubsystem>()
				->getRootSceneNode()->addChild(m);

			OgreSubsystem* ogre = Engine::getPtr()->getSubsystem("OgreSubsystem")
				->castType<OgreSubsystem>();

		
			txt = new StaticText(ogre->getGUI()->getScreen("Test")->getRootElement(0),
				"Text000",0,20,Vector2(100,100),Vector2(0.5,1.f/12.f),username);
			txt->setAlign(GA_CENTER);
		}
		
		~TestPeer()
		{
			Engine::getPtr()->getSubsystem("OgreSubsystem")->castType<OgreSubsystem>()->destroySceneNode(m);
		}

		StaticText* txt;
		Mesh* m;
		byte mID;
		String username;

	};
}

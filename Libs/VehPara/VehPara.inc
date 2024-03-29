/*
 VehPara.inc
 Copyright � 2022 Abyss Morgan. All rights reserved.

 Website: https://adm.ct8.pl

 Plugins: Streamer, ColAndreas
 Modules: 3DTryg

 File Version: 1.2.0
 Streamer Version: 2.9.5
 ColAndreas Version: 1.5.0
 3DTryg Version: Gen. 2 v1.0.0

 Functions:
 bool:IsPlayerUsingVehPara(playerid);
 bool:IsToggleVehicleParachute(vehicleid);
 ToggleVehicleParachute(vehicleid,bool:toggle);
 SetVehicleParachuteKey(key);

 Callbacks:
 OnVehicleParachuteThrown(playerid,vehicleid);
 OnVehicleParachuteOpened(playerid,vehicleid);
 OnVehicleParachuteOpenFail(playerid,vehicleid);
*/

#if defined _samp_included
	#if (!defined GetPlayerPoolSize || !defined GetSVarInt)
		#error [ADM] This include requires SA:MP version 0.3.7
	#endif
#else
	#error [ADM] This include requires SA:MP Include
#endif

#if ((!defined Streamer_GetItemPos) || (!defined INVALID_STREAMER_ID) || (!defined STREAMER_TYPE_ACTOR) || (!defined RemoveDynamicObjectMaterialText))
	#error [ADM] This include required Streamer v2.9.5 (github.com/samp-incognito/samp-streamer-plugin/releases)
#endif

#if !defined COLANDREAS_VERSION
	#error [ADM] This include required ColAndreas v1.5.0 (github.com/Pottus/ColAndreas/releases)
#elseif (COLANDREAS_VERSION < 10500)
	#error [ADM] Update your ColAndreas v1.5.0 (github.com/Pottus/ColAndreas/releases)
#endif

//Check Version 3DTryg.inc
#if !defined TRYG3D_REMASTERED_INCLUDE
	#error [ADM] This include required 3DTryg.inc Gen. 2 v1.0.0
#elseif !defined TRYG3D_VERSION
	#error [ADM] Update your 3DTryg.inc to Gen. 2 v1.0.0
#elseif (TRYG3D_VERSION < 10000)
	#error [ADM] Update your 3DTryg.inc to Gen. 2 v1.0.0
#endif

#if !defined TRYG3D_MOD_VEHICLE
	#error [ADM] Module not initialized: TRYG3D_MOD_VEHICLE (add #define TRYG3D_ENABLE_VEHICLE before include 3DTryg.inc)
#endif

#if !defined TRYG3D_MOD_UNIVERSAL
	#error [ADM] Module not initialized: TRYG3D_MOD_UNIVERSAL (add #define TRYG3D_ENABLE_UNIVERSAL before include 3DTryg.inc)
#endif

#if !defined TRYG3D_MOD_COLANDREAS
	#error [ADM] Module not initialized: TRYG3D_MOD_COLANDREAS (add #define TRYG3D_ENABLE_COLANDREAS before include 3DTryg.inc)
#endif

#if defined _vehicle_parachute
	#endinput
#endif
#define _vehicle_parachute
#define DYNAMIC_FILTERSCRIPT						"ALS:HOOK:METHOD"

forward OnVehicleParachuteThrown(playerid,vehicleid);
forward OnVehicleParachuteOpened(playerid,vehicleid);
forward OnVehicleParachuteOpenFail(playerid,vehicleid);

new VehicleConfigParachute[63],
	STREAMER_TAG_OBJECT:VehicleParachuteObject[MAX_VEHICLES][3],
	PlayerUsingVehPara[32],
	VEHPARA_KEY = KEY_CROUCH;

//Get Compress Adress
#define GetVehicleParaConfAddress(%0)				(floatround((%0)/32))
#define GetVehicleParaConfBit(%0)					((%0) % 32)

//BitFunctions.inc
#define VP_GetValueBit(%0,%1)						((%0) >>> (%1) & 0x01)
#define VP_SetValueBit(%0,%1,%2)					((%0) = (((%0) & ~(0x01 << (%1))) | ((0x01 << (%1))*(%2))))

#define IsPlayerUsingVehPara(%0)					VP_GetValueBit(PlayerUsingVehPara[GetVehicleParaConfAddress(%0)],GetVehicleParaConfBit(%0))
#define TogglePlayerUsingVehPara(%0,%1)				VP_SetValueBit(PlayerUsingVehPara[GetVehicleParaConfAddress(%0)],GetVehicleParaConfBit(%0),(%1))

#define IsToggleVehicleParachute(%0)				VP_GetValueBit(VehicleConfigParachute[GetVehicleParaConfAddress(%0)],GetVehicleParaConfBit(%0))
#define SetVehicleParachuteKey(%0)					VEHPARA_KEY = (%0)

stock ToggleVehicleParachute(vehicleid,bool:toggle){
	for(new i = 0; i < 3; i++){
		if(IsValidDynamicObject(VehicleParachuteObject[vehicleid][i])) DestroyDynamicObject(VehicleParachuteObject[vehicleid][i]);
		VehicleParachuteObject[vehicleid][i] = STREAMER_TAG_OBJECT:INVALID_STREAMER_ID;
	}
	if(toggle){
		if(!IsToggleVehicleParachute(vehicleid)){
			VehicleParachuteObject[vehicleid][1] = CreateDynamicObject(1310,0.0,0.0,-6000.0,0.0,0.0,0.0,-1,-1);
			VehicleParachuteObject[vehicleid][2] = CreateDynamicObject(1310,0.0,0.0,-6000.0,0.0,0.0,0.0,-1,-1);
			
			new Float:T3D:x,Float:T3D:y,Float:T3D:z;
			GetVehicleModelInfo(GetVehicleModel(vehicleid),VEHICLE_MODEL_INFO_SIZE,T3D:x,T3D:y,T3D:z);
			T3D:z /= 2.0;
		
			AttachDynamicObjectToVehicle(VehicleParachuteObject[vehicleid][1],vehicleid,0.5,0.0,T3D:z+0.1,270.0,0.0,35.0);
			AttachDynamicObjectToVehicle(VehicleParachuteObject[vehicleid][2],vehicleid,-0.5,0.0,T3D:z+0.1,270.0,0.0,325.0);
		}
	}
	VP_SetValueBit(VehicleConfigParachute[GetVehicleParaConfAddress(vehicleid)],GetVehicleParaConfBit(vehicleid),(_:toggle));
}

stock StartVehicleParachuteAction(playerid){
	TogglePlayerUsingVehPara(playerid,1);
	
	new vid = GetPlayerVehicleID(playerid);
	for(new i = 0; i < 3; i++){
		if(IsValidDynamicObject(VehicleParachuteObject[vid][i])) DestroyDynamicObject(VehicleParachuteObject[vid][i]);
		VehicleParachuteObject[vid][i] =  STREAMER_TAG_OBJECT:INVALID_STREAMER_ID;
	}
	VehicleParachuteObject[vid][0] = CreateDynamicObject(18849,0.0,0.0,-6000.0,0.0,0.0,0.0,GetPlayerVirtualWorld(playerid),GetPlayerInterior(playerid));
	
	switch(random(5)){
		case 0: {
			SetDynamicObjectMaterial(VehicleParachuteObject[vid][0],0,18841,"MickyTextures","Smileyface2",0x00000000);
			SetDynamicObjectMaterial(VehicleParachuteObject[vid][0],2,10412,"hotel1","carpet_red_256",0x00FFFFFF);
		}
		case 1: {
			SetDynamicObjectMaterial(VehicleParachuteObject[vid][0],0,18841,"MickyTextures","red032",0x00000000);
			SetDynamicObjectMaterial(VehicleParachuteObject[vid][0],2,10412,"hotel1","carpet_red_256",0x00FFFFFF);
		}
		case 2: {
			SetDynamicObjectMaterial(VehicleParachuteObject[vid][0],0,18841,"MickyTextures","ws_gayflag1",0x00000000);
			SetDynamicObjectMaterial(VehicleParachuteObject[vid][0],2,10412,"hotel1","carpet_red_256",0x00FFFFFF);
		}
		case 3: {
			SetDynamicObjectMaterial(VehicleParachuteObject[vid][0],0,18841,"MickyTextures","waterclear256",0x00000000);
			SetDynamicObjectMaterial(VehicleParachuteObject[vid][0],2,10412,"hotel1","carpet_red_256",0x00FFFFFF);
		}
		case 4: {
			SetDynamicObjectMaterial(VehicleParachuteObject[vid][0],2,10412,"hotel1","carpet_red_256",0x00FFFFFF);
		}
	}
	new Float:T3D:x,Float:T3D:y,Float:T3D:z;
	GetVehicleModelInfo(GetVehicleModel(vid),VEHICLE_MODEL_INFO_SIZE,T3D:x,T3D:y,T3D:z);
	T3D:z /= 2.0;
	AttachDynamicObjectToVehicle(VehicleParachuteObject[vid][0],vid,0.0,0.0,T3D:z+6.0,0.0,0.0,90.0);
}

stock StopVehicleParachuteAction(playerid,vehicleid=INVALID_VEHICLE_ID){
	TogglePlayerUsingVehPara(playerid,0);
	if(vehicleid == INVALID_VEHICLE_ID) vehicleid = GetPlayerVehicleID(playerid);
	ToggleVehicleParachute(vehicleid,false);
}

//Hook: OnPlayerUpdate
public OnPlayerUpdate(playerid){
	if(IsPlayerUsingVehPara(playerid)){
		if(Tryg3D::IsCollisionFlag(Tryg3D::ItemGetCollisionFlags(GetPlayerVehicleID(playerid),item_vehicle),POSITION_FLAG_GROUND)){
			StopVehicleParachuteAction(playerid);
			CallLocalFunction("OnVehicleParachuteThrown","dd",playerid,GetPlayerVehicleID(playerid));
		} else {
			static T3D:vid, Float:T3D:rx, Float:T3D:ry, Float:T3D:rz, Float:T3D:tx, Float:T3D:ty, Float:T3D:tz;
			T3D:vid = GetPlayerVehicleID(playerid);
			T3D:tx = T3D:ty = 0.0;
			Tryg3D::GetVehicleRotation(T3D:vid,T3D:rx,T3D:ry,T3D:rz);
			Tryg3D::ShiftVectorRotation((Tryg3D::DeCompressRotation(T3D:ry)/180.0),(Tryg3D::DeCompressRotation(-T3D:rx)/180.0),0.0,0.0,0.0,T3D:rz,T3D:tx,T3D:ty,T3D:tz);
			SetVehicleVelocity(T3D:vid,(T3D:tx/10.0),(T3D:ty/10.0),-0.2);
		}
	}
	#if defined VP_OnPlayerUpdate
		VP_OnPlayerUpdate(playerid);
	#endif
	return 1;
}

#if defined _ALS_OnPlayerUpdate
	#undef OnPlayerUpdate
#else
	#define _ALS_OnPlayerUpdate
#endif
#define OnPlayerUpdate VP_OnPlayerUpdate
#if defined VP_OnPlayerUpdate
	forward VP_OnPlayerUpdate(playerid);
#endif

//Hook: OnVehicleSpawn
public OnVehicleSpawn(vehicleid){
	ToggleVehicleParachute(vehicleid,false);
	#if defined VP_OnVehicleSpawn
		VP_OnVehicleSpawn(vehicleid);
	#endif
	return 1;
}

#if defined _ALS_OnVehicleSpawn
	#undef OnVehicleSpawn
#else
	#define _ALS_OnVehicleSpawn
#endif
#define OnVehicleSpawn VP_OnVehicleSpawn
#if defined VP_OnVehicleSpawn
	forward VP_OnVehicleSpawn(vehicleid);
#endif

//Hook: OnPlayerEnterVehicle
public OnPlayerEnterVehicle(playerid,vehicleid,ispassenger){
	if(IsValidDynamicObject(VehicleParachuteObject[vehicleid][0])){
		ToggleVehicleParachute(vehicleid,false);
		CallLocalFunction("OnVehicleParachuteThrown","dd",playerid,vehicleid);
	}
	#if defined VP_OnPlayerEnterVehicle
		VP_OnPlayerEnterVehicle(playerid,vehicleid,ispassenger);
	#endif
	return 1;
}

#if defined _ALS_OnPlayerEnterVehicle
	#undef OnPlayerEnterVehicle
#else
	#define _ALS_OnPlayerEnterVehicle
#endif
#define OnPlayerEnterVehicle VP_OnPlayerEnterVehicle
#if defined VP_OnPlayerEnterVehicle
	forward VP_OnPlayerEnterVehicle(playerid,vehicleid,ispassenger);
#endif

//Hook: OnPlayerExitVehicle
public OnPlayerExitVehicle(playerid,vehicleid){
	if(IsPlayerUsingVehPara(playerid)){
		StopVehicleParachuteAction(playerid);
		CallLocalFunction("OnVehicleParachuteThrown","dd",playerid,vehicleid);
	}
	#if defined VP_OnPlayerExitVehicle
		VP_OnPlayerExitVehicle(playerid,vehicleid);
	#endif
	return 1;
}

#if defined _ALS_OnPlayerExitVehicle
	#undef OnPlayerExitVehicle
#else
	#define _ALS_OnPlayerExitVehicle
#endif
#define OnPlayerExitVehicle VP_OnPlayerExitVehicle
#if defined VP_OnPlayerExitVehicle
	forward VP_OnPlayerExitVehicle(playerid,vehicleid);
#endif

//Hook: OnPlayerStateChange
public OnPlayerStateChange(playerid,newstate,oldstate){
	if(oldstate == PLAYER_STATE_DRIVER && IsPlayerUsingVehPara(playerid)){
		StopVehicleParachuteAction(playerid,GetPlayerVehicleID(playerid));
	}
	#if defined VP_OnPlayerStateChange
		VP_OnPlayerStateChange(playerid,newstate,oldstate);
	#endif
	return 1;
}

#if defined _ALS_OnPlayerStateChange
	#undef OnPlayerStateChange
#else
	#define _ALS_OnPlayerStateChange
#endif
#define OnPlayerStateChange VP_OnPlayerStateChange
#if defined VP_OnPlayerStateChange
	forward VP_OnPlayerStateChange(playerid,newstate,oldstate);
#endif

//Hook: OnPlayerKeyStateChange
public OnPlayerKeyStateChange(playerid,newkeys,oldkeys){
	if(IsPlayerInAnyVehicle(playerid) && GetPlayerVehicleSeat(playerid) == 0){
		new vid = GetPlayerVehicleID(playerid);
		if(Tryg3D::KeyPressed(VEHPARA_KEY)){
			if(IsToggleVehicleParachute(vid)){
				if(IsPlayerUsingVehPara(playerid)){
					StopVehicleParachuteAction(playerid);
					CallLocalFunction("OnVehicleParachuteThrown","dd",playerid,vid);
				} else {
					if(Tryg3D::IsCollisionFlag(Tryg3D::ItemGetCollisionFlags(vid,item_vehicle),POSITION_FLAG_AIR) && Tryg3D::GetVehicleSpeed(vid) > 0.0){
						StartVehicleParachuteAction(playerid);
						CallLocalFunction("OnVehicleParachuteOpened","dd",playerid,vid);
					} else {
						CallLocalFunction("OnVehicleParachuteOpenFail","dd",playerid,vid);
					}
				}
			}
		}
	}
	#if defined VP_OnPlayerKeyStateChange
		VP_OnPlayerKeyStateChange(playerid,newkeys,oldkeys);
	#endif
	return 1;
}

#if defined _ALS_OnPlayerKeyStateChange
	#undef OnPlayerKeyStateChange
#else
	#define _ALS_OnPlayerKeyStateChange
#endif
#define OnPlayerKeyStateChange VP_OnPlayerKeyStateChange
#if defined VP_OnPlayerKeyStateChange
	forward VP_OnPlayerKeyStateChange(playerid,newkeys,oldkeys);
#endif

//Hook: PutPlayerInVehicle
stock VP_PutPlayerInVehicle(playerid,vehicleid,seatid){
	if(IsValidDynamicObject(VehicleParachuteObject[vehicleid][0])){
		ToggleVehicleParachute(vehicleid,false);
	}
	return PutPlayerInVehicle(playerid,vehicleid,seatid);
}

#if defined _ALS_PutPlayerInVehicle
    #undef PutPlayerInVehicle
#else
    #define _ALS_PutPlayerInVehicle
#endif
#define PutPlayerInVehicle VP_PutPlayerInVehicle

//Hook: CreateVehicle
stock VP_CreateVehicle(modelid,Float:x,Float:y,Float:z,Float:angle,color1,color2,respawn_delay,addsiren=0){
	new vehicleid = CreateVehicle(modelid,x,y,z,angle,color1,color2,respawn_delay,addsiren);
	ToggleVehicleParachute(vehicleid,false);
	return vehicleid;
}
 
#if defined _ALS_CreateVehicle
	#undef CreateVehicle
#else
	#define _ALS_CreateVehicle
#endif
#define CreateVehicle VP_CreateVehicle
 
//Hook: AddStaticVehicle
stock VP_AddStaticVehicle(modelid,Float:spawn_x,Float:spawn_y,Float:spawn_z,Float:angle,color1,color2){
	new vehicleid = AddStaticVehicle(modelid,spawn_x,spawn_y,spawn_z,angle,color1,color2);
	ToggleVehicleParachute(vehicleid,false);
	return vehicleid;
}
 
#if defined _ALS_AddStaticVehicle
	#undef AddStaticVehicle
#else
	#define _ALS_AddStaticVehicle
#endif
#define AddStaticVehicle VP_AddStaticVehicle
 
//Hook: AddStaticVehicleEx
stock VP_AddStaticVehicleEx(modelid,Float:spawn_x,Float:spawn_y,Float:spawn_z,Float:angle,color1,color2,respawn_delay,addsiren=0){
	new vehicleid = AddStaticVehicleEx(modelid,spawn_x,spawn_y,spawn_z,angle,color1,color2,respawn_delay,addsiren);
	ToggleVehicleParachute(vehicleid,false);
	return vehicleid;
}
 
#if defined _ALS_AddStaticVehicleEx
	#undef AddStaticVehicleEx
#else
	#define _ALS_AddStaticVehicleEx
#endif
#define AddStaticVehicleEx VP_AddStaticVehicleEx
 
//Hook: DestroyVehicle
stock VP_DestroyVehicle(vehicleid){
	ToggleVehicleParachute(vehicleid,false);
	return DestroyVehicle(vehicleid);
}
 
#if defined _ALS_DestroyVehicle
	#undef DestroyVehicle
#else
	#define _ALS_DestroyVehicle
#endif
#define DestroyVehicle VP_DestroyVehicle

//Hook: SetPlayerPos
stock VP_SetPlayerPos(playerid,Float:x,Float:y,Float:z){
	if(IsPlayerUsingVehPara(playerid)){
		StopVehicleParachuteAction(playerid,GetPlayerVehicleID(playerid));
	}
	return SetPlayerPos(playerid,x,y,z);
}
 
#if defined _ALS_SetPlayerPos
	#undef SetPlayerPos
#else
	#define _ALS_SetPlayerPos
#endif
#define SetPlayerPos VP_SetPlayerPos

//Hook: OnPlayerConnect
public OnPlayerConnect(playerid){
	TogglePlayerUsingVehPara(playerid,0);
	#if defined VP_OnPlayerConnect
		VP_OnPlayerConnect(playerid);
	#endif
	return 1;
}

#if defined _ALS_OnPlayerConnect
	#undef OnPlayerConnect
#else
	#define _ALS_OnPlayerConnect
#endif
#define OnPlayerConnect VP_OnPlayerConnect
#if defined VP_OnPlayerConnect
	forward VP_OnPlayerConnect(playerid);
#endif

//EOF
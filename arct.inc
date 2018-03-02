/*
Anti car troll include by RogueDrifter Jan 15th 2018
-=-=-=-=-=-
Last Updated : 2018 Mar 2nd
-=-=-=-=-=-=-=-=-
Callback:
-=-=-=-=-
OnPlayerCarTroll(playerid, vehicleid, trolledid, trolltype);
-=-=-=-=-=-=-=-=--=-=-=-=
Type 1: Teleporting between vehicles (can give cheaters chance to control other's players) (also lag cheat detected here).
Type 2: Control other player's cars or spin/rotate (Done by checking the player's actual vehicle).
Type 3: Fast kick/rotate or teleporting inside drivers.
Type 4: Teleporting inside a vehicle illegally (without PutPlayerInVehicle or OnPlayerEnterVehicle).
Type 5: Remote jacking a vehicle (also tping vehicles detected here).
Type 6: Remote kick a player by tping a vehicle inside of him. (Too fast the server reads it as an unoccupied vehicle).
*/
 
#if defined _AntiCarTroll_included
    #endinput
#endif
 
#define _AntiCarTroll_included
 
#include <a_samp>
 
#if !defined FILTERSCRIPT
 
enum ActPData
{
    bool:ActAntiSpam,
    bool:AntiJackBug,
    bool:IsDPaused,
    bool:ActProtectFalse,
    bool:ActProtectPos,
    bool:ActFixRJackBug,

    Float:ActPX,
    Float:ActPY,
    Float:ActPZ,

    ActRemoteJack,
    pVehicleID,
    pPauseTick,
    ActOVeh,

    ActTCheck,
    ActTimer,
    ResetTAct

};
 
enum ActVData
{
    VehicleTempOwner,
    ActVehEnterer,

    bool:ActETPBugFix,    
    bool:VehicleTaken
};
 
new
    ActPlayer[MAX_PLAYERS][ActPData],
    ActVehicle[MAX_VEHICLES][ActVData],
    SafeTimer;
 
#if !defined IsValidVehicle
    native IsValidVehicle(vehicleid); // Unlock the hidden native
#endif

forward ActPutPlayerInVehicle(playerid, vehicleid, seatid);
forward ActSetPlayerPos(playerid, Float:x, Float:y, Float:z);
forward ActXClearAnimations(playerid, forcesync);

forward ActDelay(playerid);
forward ActResetHandler(handleid, handletype);
forward ActFixPPIV(playerid, vehicleid, seatid);
forward SafeReset();
forward AntiCarTroll(playerid);

 
#if defined OnPlayerEnterVehicleACT
forward OnPlayerEnterVehicleACT(playerid, vehicleid, ispassenger);
#endif
 
#if defined OnPlayerUpdateACT
    forward OnPlayerUpdateACT(playerid);
#endif


#if defined OnPlayerConnectACT
forward OnPlayerConnectACT(playerid);
#endif
 
#if defined OnPlayerDisconnectACT
forward OnPlayerDisconnectACT(playerid, reason);
#endif
 
#if defined OnPlayerExitVehicleACT
forward OnPlayerExitVehicleACT(playerid, vehicleid);
#endif

#if defined OnPlayerDeathACT
forward OnPlayerDeathACT(playerid, killerid, reason);
#endif
 
#if defined OnPlayerStateChangeACT
forward OnPlayerStateChangeACT(playerid,newstate,oldstate);
#endif
 
#if defined OnGameModeInitACT
forward OnGameModeInitACT();
#endif
 
#if defined OnGameModeExitACT
forward OnGameModeExitACT();
#endif

#if defined OnPlayerSpawnAct
    forward OnPlayerSpawnAct(playerid);
#endif

#if defined OnUnoccupiedVehicleUpdateAct
    forward OnUnoccupiedVehicleUpdateAct(vehicleid, playerid, passenger_seat, Float:new_x, Float:new_y, Float:new_z, Float:vel_x, Float:vel_y, Float:vel_z);
#endif

#if defined ManualUse
stock rAct_Detected(playerid, vehicleid, trolledid, trolltype)
{
    #if defined OnPlayerCarTroll
    OnPlayerCarTroll(playerid,vehicleid, trolledid, trolltype);
    #endif

    return 1;
}

#else

stock rAct_Detected(playerid)
{
    new ACT_pName[MAX_PLAYER_NAME],actmsg[80];
    GetPlayerName(playerid,ACT_pName,sizeof(ACT_pName));

    #if defined Extreme_Safe
    SetPlayerHealth(playerid,0);
    format(actmsg,sizeof(actmsg),".: Server has scanned %s for Car Troll Cheats :.",ACT_pName);
    SendClientMessageToAll(0xAA3333AA,actmsg);
    #else

    #if defined BanVersion
    format(actmsg,sizeof(actmsg),".: Server has banned %s, reason: Car Troll Cheats :.",ACT_pName);
    #else
    format(actmsg,sizeof(actmsg),".: Server has kicked %s, reason: Car Troll Cheats :.",ACT_pName);
    #endif

    SendClientMessageToAll(0xFF0000A,actmsg);
    SetTimerEx("ActDelay",250,false,"d",playerid);
    #endif

    return 1;
}
#endif

stock ActClearAnimations(playerid, forcesync = 0)
{
    ActPlayer[playerid][ActFixRJackBug] = true;
    if(!ActPlayer[playerid][ActFixRJackBug]) SetTimerEx("ActResetHandler", 2500, false, "i", playerid, 1);
    ClearAnimations(playerid, forcesync);
    return 1;
}

stock IsPEnExCar(playerid)
{
    switch(GetPlayerAnimationIndex(playerid))
    {
        case 1010..1012: return 1;
        case 1024..1025: return 1;
        case 1043..1045: return 1;
        case 1009, 1041: return 1;
    }
    return 0;
}

stock ActPlayerDown(playerid)
{
    switch(GetPlayerAnimationIndex(playerid))
    {
        case 1208..1209: return 1;
        case 1155..1156: return 1;
    }
    return 0;
}

public SafeReset()
{
    new AllVehicles[MAX_VEHICLES];
    new gpVehicleID;

    for(new i, j = GetPlayerPoolSize(); i <= j; i++)
    {
        if(!IsPlayerConnected(i)) continue;
        
        gpVehicleID = GetPlayerVehicleID(i);
        if(GetPlayerState(i)== PLAYER_STATE_DRIVER && AllVehicles[gpVehicleID] != i )
        {
            AllVehicles[gpVehicleID]= i;
            ActVehicle[gpVehicleID][VehicleTaken]= true;
        }
    }

    for(new x; x<MAX_VEHICLES; x++)
    {
        if(!IsValidVehicle(x)) continue;
        if(AllVehicles[x] != INVALID_PLAYER_ID)
        {
            if(GetPlayerState(AllVehicles[x]) != PLAYER_STATE_DRIVER) AllVehicles[x] = INVALID_PLAYER_ID;
        }
        if(AllVehicles[x] == INVALID_PLAYER_ID && ActVehicle[x][VehicleTaken]) ActVehicle[x][VehicleTaken]= false;
    }
    return 1;
}
 
public AntiCarTroll(playerid)
{
    if(GetTickCount() > (ActPlayer[playerid][pPauseTick]+4000) && !ActPlayer[playerid][IsDPaused]) ActPlayer[playerid][IsDPaused] = true;
    else if(ActPlayer[playerid][IsDPaused]) ActPlayer[playerid][IsDPaused] = false;

    for(new i; i < MAX_VEHICLES; i++)
    {
        if(!IsValidVehicle(i) || !ActVehicle[i][VehicleTaken]) continue;
        if(GetPlayerVehicleID(playerid) == i && playerid != ActVehicle[i][VehicleTempOwner] 
        && !ActPlayer[playerid][AntiJackBug] && !ActPlayer[ActVehicle[i][VehicleTempOwner]][AntiJackBug] 
        && GetPlayerState(playerid) == PLAYER_STATE_DRIVER  && !IsPEnExCar(playerid) 
        && !ActPlayer[ActVehicle[i][VehicleTempOwner]][IsDPaused] && !ActPlayer[playerid][ActAntiSpam])
        {
            ActPlayer[playerid][ActAntiSpam]=true; 
            SetTimerEx("ActResetHandler",2500,false,"d",playerid, 2);
            #if defined ManualUse
            rAct_Detected(playerid, i, ActVehicle[i][VehicleTempOwner], 2);
            #else
            rAct_Detected(playerid);
            #endif
        }
    }
    return 1;
}
 
#if defined ManualUse
    #if defined OnPlayerCarTroll
 
    forward OnPlayerCarTroll(playerid,vehicleid, trolledid, trolltype);// Only use if you defined ManualUse

    #endif

#else
 
public ActDelay(playerid)
{
    #if defined BanVersion
    Ban(playerid);
    #else
    Kick(playerid);
    #endif
    return 1;
}
 
#endif

public OnPlayerUpdate(playerid)
{
    if(GetPlayerState(playerid) == 2 || GetPlayerState(playerid) == 3)
    {
        new ActVeh = GetPlayerVehicleID(playerid);
        if(ActVeh != ActPlayer[playerid][ActOVeh] && !ActPlayer[playerid][ActProtectFalse] && !ActPlayer[playerid][ActAntiSpam])
        {            
            ActPlayer[playerid][ActAntiSpam]=true;
            SetTimerEx("ActResetHandler",2500,false,"d",playerid, 2);
            #if defined ManualUse
            rAct_Detected(playerid, ActVeh, INVALID_PLAYER_ID, 1);
            #else

            rAct_Detected(playerid);
            #endif
            SetVehicleToRespawn(ActVeh);
            ActPlayer[playerid][ActOVeh] = ActVeh;
        }
    }

    if(ActPlayer[playerid][ActRemoteJack] == 1)
    {
        if(GetPlayerSpecialAction(playerid) != SPECIAL_ACTION_ENTER_VEHICLE && !ActPlayer[playerid][ActFixRJackBug])
        {
            ActPlayer[playerid][ActRemoteJack] = 2;
            #if defined ManualUse
            rAct_Detected(playerid,  ActPlayer[playerid][ActOVeh], INVALID_PLAYER_ID, 5);
            #else
            rAct_Detected(playerid);
            #endif
        }
        else ActPlayer[playerid][ActRemoteJack] = 2;
    }

    if(!IsPlayerInRangeOfPoint(playerid, 10.0, ActPlayer[playerid][ActPX], ActPlayer[playerid][ActPY], ActPlayer[playerid][ActPZ]) && ActPlayer[playerid][ActPX] != 0 && ActPlayer[playerid][ActPY] != 0 && ActPlayer[playerid][ActPZ] != 0)
    {
        for(new i, j = GetPlayerPoolSize(); i <= j; i++)
        {
            if(!IsPlayerConnected(i)) continue;
            new Float:ActTempX, Float:ActTempY, Float:ActTempZ;
            GetPlayerPos(i, ActTempX, ActTempY, ActTempZ);
            if(IsPlayerInRangeOfPoint(playerid, 3.0, ActTempX, ActTempY, ActTempZ) && !ActPlayer[playerid][ActProtectPos] && !ActPlayer[i][ActAntiSpam] && GetPlayerState(i) == PLAYER_STATE_DRIVER && i != playerid && GetPlayerState(playerid) == 1 && GetPlayerSurfingVehicleID(playerid) == INVALID_VEHICLE_ID && !ActPlayerDown(playerid))
            {
                ActPlayer[i][ActAntiSpam]=true; 
                SetVehicleToRespawn(GetPlayerVehicleID(i));
                TogglePlayerControllable(i, 0);
                SetPlayerPos(i, ActTempX, ActTempY, ActTempZ);
                SetPlayerHealth(playerid, 0.0);

                SetTimerEx("ActResetHandler", 1500, false, "d", i, 7);
                SetTimerEx("ActResetHandler",1500,false,"d",i, 2);
                #if defined ManualUse
                rAct_Detected(playerid,  GetPlayerVehicleID(i), i, 3);
                #else
                rAct_Detected(playerid);
                #endif
            }
        }
    }
    ActPlayer[playerid][pPauseTick] = GetTickCount();

        #if defined OnPlayerUpdateACT
        return OnPlayerUpdateACT(playerid);
        #else
        return 1;
            #endif
}

public OnGameModeInit()
{
        print("     [[--|| rACT V9.0 ||--]]     ");
        print("     [[--|| Anti car troll by RogueDrifter V 9.0 ||--]]      ");
        print("     [[--|| Successfully Loaded! ||--]]      ");
        SafeTimer = SetTimer("SafeReset",2500,true);

        #if defined OnGameModeInitACT
        return OnGameModeInitACT();
        #else
        return 1;
            #endif
}
 
public OnGameModeExit()
{
    KillTimer(SafeTimer);

    #if defined OnGameModeExitACT
        return OnGameModeExitACT();
        #else
        return 1;
            #endif
}

public OnPlayerEnterVehicle(playerid, vehicleid, ispassenger)
{
    ActPlayer[playerid][ActOVeh] = vehicleid;
    if(!ispassenger) ActPlayer[playerid][ActRemoteJack] = 1;

    if(!ActPlayer[playerid][ActProtectFalse]) ActPlayer[playerid][ActProtectFalse] = true;

    if(!ispassenger && !ActPlayer[playerid][AntiJackBug] && ActVehicle[vehicleid][VehicleTaken])
    {
        ActPlayer[playerid][AntiJackBug]=true;
        SetTimerEx("ActResetHandler",6000,false,"d",playerid, 6);

        ActPlayer[ActVehicle[vehicleid][VehicleTempOwner]][AntiJackBug]=true;
        SetTimerEx("ActResetHandler",6000,false,"d",ActVehicle[vehicleid][VehicleTempOwner], 6);
    }

    else if(!ActVehicle[vehicleid][ActETPBugFix] && !ispassenger && !ActVehicle[vehicleid][VehicleTaken])
    {
        ActVehicle[vehicleid][ActETPBugFix] = true;
        ActVehicle[vehicleid][ActVehEnterer] = playerid;

        SetTimerEx("ActResetHandler",3000,false,"d",vehicleid, 4);
    }

        #if defined OnPlayerEnterVehicleACT
    return OnPlayerEnterVehicleACT(playerid, vehicleid, ispassenger);
        #else
    return 1;
        #endif
}
 
public OnPlayerConnect(playerid)
{
    ActPlayer[playerid][ActAntiSpam]=false;
    ActPlayer[playerid][AntiJackBug]=false;

    ActPlayer[playerid][ActTimer] = SetTimerEx("ActResetHandler", 1000, true, "d", playerid, 0);
    ActPlayer[playerid][ActTCheck] = SetTimerEx("AntiCarTroll", 250, true, "d", playerid);

    ActPlayer[playerid][ActPX] = 0;
    ActPlayer[playerid][ActPY] = 0;
    ActPlayer[playerid][ActPZ] = 0;

        #if defined OnPlayerConnectACT
        return OnPlayerConnectACT(playerid);
        #else
        return 1;
            #endif
}

public OnPlayerSpawn(playerid)
{
    GetPlayerPos(playerid, ActPlayer[playerid][ActPX], ActPlayer[playerid][ActPY], ActPlayer[playerid][ActPZ]);

        #if defined OnPlayerSpawnAct
    return OnPlayerSpawnAct(playerid);
        #else
    return 1;
        #endif
}

public OnUnoccupiedVehicleUpdate(vehicleid, playerid, passenger_seat, Float:new_x, Float:new_y, Float:new_z, Float:vel_x, Float:vel_y, Float:vel_z)
{
    if(!ActVehicle[vehicleid][VehicleTaken])
    {
        for(new i, j = GetPlayerPoolSize(); i <= j; i++)
        {
            if(!IsPlayerConnected(i)) continue;
            if(IsPlayerInRangeOfPoint(i, 4.0, new_x, new_y, new_z) && vel_z == 400.000000)
            {
                if(i == playerid) 
                {
                    SetVehicleToRespawn(vehicleid);
                }
                else if (i != playerid)
                {
                    SetPlayerHealth(playerid, 0.0);
                    SetVehicleToRespawn(vehicleid);
                    #if defined ManualUse
                    rAct_Detected(playerid, vehicleid, i, 6);
                    #else
                    rAct_Detected(playerid);
                    #endif
                }
            }
        }
    }
        #if defined OnUnoccupiedVehicleUpdateAct
    return OnUnoccupiedVehicleUpdateAct(vehicleid, playerid, passenger_seat, Float:new_x, Float:new_y, Float:new_z, Float:vel_x, Float:vel_y, Float:vel_z);
        #else
    return 1;
        #endif
}

public OnPlayerStateChange(playerid,newstate,oldstate)
{
    switch(newstate)
    {
        case 1:
        {
            if(oldstate == 2) ActPlayer[playerid][ResetTAct] = SetTimerEx("ActResetHandler",500,false,"d",playerid, 5), ActPlayer[playerid][ActRemoteJack] = 0;
            if(oldstate ==2 || oldstate == 3) ActPlayer[playerid][ActProtectFalse] = false;
        }
 
        case 2:
        {
            ActPlayer[playerid][pVehicleID] = GetPlayerVehicleID(playerid);
            if(ActVehicle[ActPlayer[playerid][pVehicleID]][VehicleTaken]  && ActPlayer[ActVehicle[ActPlayer[playerid][pVehicleID]][VehicleTempOwner]][pPauseTick])
            {
                new Float:ActPosX, Float:ActPosY, Float:ActPosZ;
                GetPlayerPos(ActVehicle[ActPlayer[playerid][pVehicleID]][VehicleTempOwner], ActPosX, ActPosY, ActPosZ);
                SetPlayerPos(ActVehicle[ActPlayer[playerid][pVehicleID]][VehicleTempOwner], ActPosX, ActPosY, ActPosZ+2);
            }
 
            if(!ActPlayer[playerid][ActProtectFalse] && GetPlayerState(playerid) != 9 && !ActPlayer[playerid][ActAntiSpam])
            {                
                ActPlayer[playerid][ActAntiSpam]=true;
                SetTimerEx("ActResetHandler",2500,false,"d",playerid, 2);
                #if defined ManualUse
                rAct_Detected(playerid, GetPlayerVehicleID(playerid), INVALID_PLAYER_ID, 4);
                #else
                rAct_Detected(playerid);
                #endif
            }

            else if(ActPlayer[playerid][ActProtectFalse]) ActPlayer[playerid][ActProtectFalse] = false;

            if(!ActVehicle[ActPlayer[playerid][pVehicleID]][VehicleTaken])
            {
                KillTimer(ActPlayer[playerid][ResetTAct]);
                ActVehicle[ActPlayer[playerid][pVehicleID]][VehicleTaken] = true;
                ActVehicle[ActPlayer[playerid][pVehicleID]][VehicleTempOwner] = playerid;
            }
        }

        case 3:
        {
            if(!ActPlayer[playerid][ActProtectFalse] && GetPlayerState(playerid) != 9 && !ActPlayer[playerid][ActAntiSpam])
            {            
                ActPlayer[playerid][ActAntiSpam]=true;
                SetTimerEx("ActResetHandler",2500,false,"d",playerid, 2);
                #if defined ManualUse
                rAct_Detected(playerid, GetPlayerVehicleID(playerid), INVALID_PLAYER_ID, 4);
                #else
                rAct_Detected(playerid);
                #endif
            }
            else if(ActPlayer[playerid][ActProtectFalse]) ActPlayer[playerid][ActProtectFalse] = false;
        }
    }

        #if defined OnPlayerStateChangeACT
    return OnPlayerStateChangeACT(playerid,newstate,oldstate);
        #else
    return 1;
        #endif
}

public OnPlayerDeath(playerid, killerid, reason)
{
    ActPlayer[playerid][ActAntiSpam]=false;

    if(GetPlayerState(playerid) == PLAYER_STATE_DRIVER) ActPlayer[playerid][ResetTAct] = SetTimerEx("ActResetHandler",500,false,"d",playerid, 5);

        #if defined OnPlayerDeathACT
        return OnPlayerDeathACT(playerid, killerid, reason);
        #else
        return 1;
            #endif
}
 
public OnPlayerExitVehicle(playerid,vehicleid)
{
    ActPlayer[playerid][ResetTAct] = SetTimerEx("ActResetHandler",500,false,"d",playerid, 5);

    #if defined OnPlayerExitVehicleACT
        return OnPlayerExitVehicleACT(playerid, vehicleid);
        #else
        return 1;
            #endif
}
 
public OnPlayerDisconnect(playerid, reason)
{
    KillTimer(ActPlayer[playerid][ActTimer]);
    KillTimer(ActPlayer[playerid][ActTCheck]);

    new gpVehicleID;
    gpVehicleID = GetPlayerVehicleID(playerid);
 
    ActPlayer[playerid][ActFixRJackBug] = false;
    ActPlayer[playerid][ActProtectPos] = false;
    ActPlayer[playerid][IsDPaused] = false;
    ActPlayer[playerid][ActProtectFalse] = false;
 
    if(GetPlayerState(playerid) == PLAYER_STATE_DRIVER)
    {
        ActVehicle[gpVehicleID][VehicleTaken] =false;
        ActVehicle[gpVehicleID][VehicleTempOwner] = INVALID_PLAYER_ID;
        ActPlayer[playerid][ActRemoteJack] = 0;
    }

        #if defined OnPlayerDisconnectACT
        return OnPlayerDisconnectACT(playerid, reason);
        #else
        return 1;
            #endif
}

public ActResetHandler(handleid, handletype)
{
    switch(handletype)
    {
        case 0:
        {
            GetPlayerPos(handleid, ActPlayer[handleid][ActPX], ActPlayer[handleid][ActPY], ActPlayer[handleid][ActPZ]);
        }
        case 1:
        {
            ActPlayer[handleid][ActFixRJackBug]   = false;
        }
        case 2:
        {
            ActPlayer[handleid][ActAntiSpam]      = false;
        }
        case 3:
        {
            ActPlayer[handleid][ActProtectPos]    = false;
        }
        case 4:
        {
            ActVehicle[handleid][ActETPBugFix]   = false, 
            ActVehicle[handleid][ActVehEnterer] = INVALID_PLAYER_ID;
        }
        case 5:
        {
            ActVehicle[ActPlayer[handleid][pVehicleID]][VehicleTaken] = false,      
            ActVehicle[ActPlayer[handleid][pVehicleID]][VehicleTempOwner] = INVALID_PLAYER_ID;
        }
        case 6:
        {
            ActPlayer[handleid][AntiJackBug]      = false;
        }
        case 7:
        {
            TogglePlayerControllable(handleid, 1);
        }
    }
    return 1;
}

public ActFixPPIV(playerid, vehicleid, seatid) return ActPlayer[playerid][ActProtectFalse]  = true, PutPlayerInVehicle(playerid, vehicleid, seatid);
 
public ActXClearAnimations(playerid, forcesync)
{
    ActPlayer[playerid][ActFixRJackBug] = true;
    if(!ActPlayer[playerid][ActFixRJackBug]) SetTimerEx("ActResetHandler", 2500, false, "i", playerid, 1);
    ClearAnimations(playerid, forcesync);
    return 1;
}

public ActPutPlayerInVehicle(playerid, vehicleid, seatid)
{
    if(ActVehicle[vehicleid][ActETPBugFix]) ActClearAnimations(ActVehicle[vehicleid][ActVehEnterer], 1);
    if(!ActPlayer[playerid][ActProtectFalse]) ActPlayer[playerid][ActProtectFalse] = true;
    ActPlayer[playerid][ActOVeh] = vehicleid;

    if(GetPlayerState(playerid) == 2 || GetPlayerState(playerid) == 3)
    {
        new Float:PX,Float:PY,Float:PZ;
        GetPlayerPos(playerid, PX,PY,PZ);
        SetPlayerPos(playerid, PX,PY,PZ+2);
        SetTimerEx("ActFixPPIV",500,false,"iii",playerid, vehicleid, seatid);
    }
    else PutPlayerInVehicle(playerid, vehicleid, seatid);
    return 1;
}

public ActSetPlayerPos(playerid, Float:x, Float:y, Float:z)
{
    if(!ActPlayer[playerid][ActProtectPos]) SetTimerEx("ActResetHandler",2100,false,"i",playerid, 3);
    ActPlayer[playerid][ActProtectPos] = true;
    SetPlayerPos(playerid, x, y, z);
    return 1;
}

#if defined _ALS_OnUnoccupiedVehicleUpdate
  #undef OnUnoccupiedVehicleUpdate
#else
    #define _ALS_OnUnoccupiedVehicleUpdate
#endif

#if defined _ALS_OnPlayerStateChange
  #undef OnPlayerStateChange
#else
    #define _ALS_OnPlayerStateChange
#endif
 
#if defined _ALS_OnPlayerConnect
  #undef OnPlayerConnect
#else
    #define _ALS_OnPlayerConnect
#endif

#if defined _ALS_OnPlayerSpawn
  #undef OnPlayerSpawn
#else
    #define _ALS_OnPlayerSpawn
#endif

#if defined _ALS_OnPlayerDisconnect
  #undef OnPlayerDisconnect
#else
    #define _ALS_OnPlayerDisconnect
#endif
 
#if defined _ALS_OnPlayerExitVehicle
  #undef OnPlayerExitVehicle
#else
    #define _ALS_OnPlayerExitVehicle
#endif
 
#if defined _ALS_OnPlayerDeath
  #undef OnPlayerDeath
#else
    #define _ALS_OnPlayerDeath
#endif
 
#if defined _ALS_OnGameModeInit
  #undef OnGameModeInit
#else
    #define _ALS_OnGameModeInit
#endif
 
#if defined _ALS_OnGameModeExit
  #undef OnGameModeExit
#else
    #define _ALS_OnGameModeExit
#endif
 
#if defined _ALS_OnPlayerEnterVehicle
  #undef OnPlayerEnterVehicle
#else
    #define _ALS_OnPlayerEnterVehicle
#endif
 
#if defined _ALS_OnPlayerUpdate
  #undef OnPlayerUpdate
#else
    #define _ALS_OnPlayerUpdate
#endif
 
#if defined _ALS_PutPlayerInVehicle
  #undef PutPlayerInVehicle
#else
    #define _ALS_PutPlayerInVehicle
#endif
 
#if defined _ALS_SetPlayerPos
  #undef SetPlayerPos
#else
    #define _ALS_SetPlayerPos
#endif

#if defined _ALS_ClearAnimations
  #undef ClearAnimations
#else
    #define _ALS_ClearAnimations
#endif

#define OnUnoccupiedVehicleUpdate OnUnoccupiedVehicleUpdateAct
#define OnPlayerExitVehicle OnPlayerExitVehicleACT
#define OnGameModeInit OnGameModeInitACT
#define OnPlayerDeath OnPlayerDeathACT
#define OnPlayerDisconnect OnPlayerDisconnectACT
#define OnPlayerSpawn OnPlayerSpawnAct
#define OnPlayerConnect OnPlayerConnectACT
#define OnPlayerStateChange OnPlayerStateChangeACT
#define OnGameModeExit OnGameModeExitACT
#define OnPlayerEnterVehicle OnPlayerEnterVehicleACT
#define OnPlayerUpdate OnPlayerUpdateACT

#define PutPlayerInVehicle ActPutPlayerInVehicle
#define SetPlayerPos ActSetPlayerPos
#define ClearAnimations ActClearAnimations

#else //If it's a filterscript

#if defined _ALS_ClearAnimations
  #undef ClearAnimations
#else
    #define _ALS_ClearAnimations
#endif

#if defined _ALS_PutPlayerInVehicle
  #undef PutPlayerInVehicle
#else
    #define _ALS_PutPlayerInVehicle
#endif

#if defined _ALS_SetPlayerPos
  #undef SetPlayerPos
#else
    #define _ALS_SetPlayerPos
#endif

#define ClearAnimations ActFClearAnimations
#define PutPlayerInVehicle ActFPutPlayerInVehicle
#define SetPlayerPos ActFSetPlayerPos

stock ActFClearAnimations(playerid, forcesync = 0)
    return CallRemoteFunction("ActXClearAnimations","ii", playerid, forcesync);

stock ActFPutPlayerInVehicle(playerid, vehicleid, seatid)
    return CallRemoteFunction("ActPutPlayerInVehicle","iii",playerid, vehicleid, seatid);

stock ActFSetPlayerPos(playerid, Float:x, Float:y, Float:z)
    return CallRemoteFunction("ActSetPlayerPos","ifff",playerid, x, y, z);

#endif

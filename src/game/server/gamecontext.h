/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you miss that file, contact Pikotee, because he changed some stuff here ...			 */
/*	... and would like to be mentioned in credits in case of using his code					 */

#ifndef GAME_SERVER_GAMECONTEXT_H
#define GAME_SERVER_GAMECONTEXT_H

#include <engine/server.h>
#include <engine/console.h>
#include <engine/shared/memheap.h>

#include <game/layers.h>
#include <game/voting.h>

#include "eventhandler.h"
#include "gamecontroller.h"
#include "gameworld.h"
#include "player.h"

#include <minecity/components/localization.h>
//#include <infclassr/geolocation.h>

/*
	Tick
		Game Context (CGameContext::tick)
			Game World (GAMEWORLD::tick)
				Reset world if requested (GAMEWORLD::reset)
				All entities in the world (ENTITY::tick)
				All entities in the world (ENTITY::tick_defered)
				Remove entities marked for deletion (GAMEWORLD::remove_entities)
			Game Controller (GAMECONTROLLER::tick)
			All players (CPlayer::tick)


	Snap
		Game Context (CGameContext::snap)
			Game World (GAMEWORLD::snap)
				All entities in the world (ENTITY::snap)
			Game Controller (GAMECONTROLLER::snap)
			Events handler (EVENT_HANDLER::snap)
			All players (CPlayer::snap)

*/

enum
{
	BROADCAST_PRIORITY_LOWEST=0,
	BROADCAST_PRIORITY_WEAPONSTATE,
	BROADCAST_PRIORITY_EFFECTSTATE,
	BROADCAST_PRIORITY_GAMEANNOUNCE,
	BROADCAST_PRIORITY_SERVERANNOUNCE,
	BROADCAST_PRIORITY_INTERFACE,
};

class CGameContext : public IGameServer
{
	IServer *m_pServer;
	class IConsole *m_pConsole;
	CLayers m_Layers;
	CCollision m_Collision;
	CNetObjHandler m_NetObjHandler;
	CTuningParams m_Tuning;

	static void ConsoleOutputCallback_Chat(const char *pStr, void *pUser);
	
	static void ConTuneParam(IConsole::IResult *pResult, void *pUserData);
	static void ConTuneReset(IConsole::IResult *pResult, void *pUserData);
	static void ConTuneDump(IConsole::IResult *pResult, void *pUserData);
	static void ConChangeMap(IConsole::IResult *pResult, void *pUserData);
	static void ConRestart(IConsole::IResult *pResult, void *pUserData);
	static void ConBroadcast(IConsole::IResult *pResult, void *pUserData);
	static void ConSay(IConsole::IResult *pResult, void *pUserData);
	static void ConSetTeam(IConsole::IResult *pResult, void *pUserData);
	static void ConSetTeamAll(IConsole::IResult *pResult, void *pUserData);
	static void ConAddVote(IConsole::IResult *pResult, void *pUserData);
	static void ConRemoveVote(IConsole::IResult *pResult, void *pUserData);
	static void ConForceVote(IConsole::IResult *pResult, void *pUserData);
	static void ConClearVotes(IConsole::IResult *pResult, void *pUserData);
	static void ConVote(IConsole::IResult *pResult, void *pUserData);
	static bool ConLanguage(IConsole::IResult *pResult, void *pUserData);
	static void ConchainSpecialMotdupdate(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData);

	CGameContext(int Resetting);
	void Construct(int Resetting);

	bool m_Resetting;

	int m_ConsoleOutputHandle_ChatPrint;
	int m_ConsoleOutput_Target;

public:
	IServer *Server() const { return m_pServer; }
	//IStorage *Storage() const { return Storage; }
	class IConsole *Console() { return m_pConsole; }
	CCollision *Collision() { return &m_Collision; }
	CTuningParams *Tuning() { return &m_Tuning; }

	CGameContext();
	~CGameContext();

	void Clear();

	CEventHandler m_Events;
	CPlayer *m_apPlayers[MAX_CLIENTS];

	IGameController *m_pController;
	CGameWorld m_World;

	// helper functions
	class CCharacter *GetPlayerChar(int ClientID);

	// voting
	void StartVote(const char *pDesc, const char *pCommand, const char *pReason);
	void EndVote();
	void SendVoteSet(int ClientID);
	void SendVoteStatus(int ClientID, int Total, int Yes, int No);
	void AbortVoteKickOnDisconnect(int ClientID);
	
	int m_VoteCreator;
	int64 m_VoteCloseTime;
	bool m_VoteUpdate;
	int m_VotePos;
	char m_aVoteDescription[VOTE_DESC_LENGTH];
	char m_aVoteCommand[VOTE_CMD_LENGTH];
	char m_aVoteReason[VOTE_REASON_LENGTH];
	int m_NumVoteOptions;
	int m_VoteEnforce;
	enum
	{
		VOTE_ENFORCE_UNKNOWN=0,
		VOTE_ENFORCE_NO,
		VOTE_ENFORCE_YES,
	};
	CHeap *m_pVoteOptionHeap;
	CVoteOptionServer *m_pVoteOptionFirst;
	CVoteOptionServer *m_pVoteOptionLast;

	// helper functions
	void CreateDamageInd(vec2 Pos, float AngleMod, int Amount);
	void CreateExplosion(vec2 Pos, int Owner, int Weapon, bool NoDamage);
	void CreateHammerHit(vec2 Pos);
	void CreatePlayerSpawn(vec2 Pos);
	void CreateDeath(vec2 Pos, int Who);
	void CreateSound(vec2 Pos, int Sound, int Mask=-1);
	void CreateSoundGlobal(int Sound, int Target=-1);


	enum
	{
		CHAT_ALL=-2,
		CHAT_SPEC=-1,
		CHAT_RED=0,
		CHAT_BLUE=1
	};

	// network
	void SendChatTarget(int To, const char *pText);
	void SendChat(int ClientID, int Team, const char *pText);
	void SendEmoticon(int ClientID, int Emoticon);
	void SendWeaponPickup(int ClientID, int Weapon);
	void SendBroadcast(const char *pText, int ClientID);

	// Dummy
	void NewDummy(int DummyID, bool CustomColor = false, int ColorBody = 12895054, int ColorFeet = 12895054, const char *pSkin = "coala", const char *pName = "Dummy", const char *pClan = "[16x16]", int Country = -1);

	//
	void CheckPureTuning();
	void SendTuningParams(int ClientID);

	// engine events
	virtual void OnInit();
	virtual void OnConsoleInit();
	virtual void OnShutdown();

	virtual void OnTick();
	virtual void OnPreSnap();
	virtual void OnSnap(int ClientID);
	virtual void OnPostSnap();

	virtual void OnMessage(int MsgID, CUnpacker *pUnpacker, int ClientID);

	virtual void OnClientConnected(int ClientID);
	virtual void OnClientEnter(int ClientID);
	virtual void OnClientDrop(int ClientID, const char *pReason);
	virtual void OnClientDirectInput(int ClientID, void *pInput);
	virtual void OnClientPredictedInput(int ClientID, void *pInput);

	virtual bool IsClientReady(int ClientID);
	virtual bool IsClientPlayer(int ClientID);

	virtual const char *GameType();
	virtual const char *Version();
	virtual const char *NetVersion();

	private: //KlickFoots Rconcmdsachen^^

		static void ConTeleport(IConsole::IResult *pResult, void *pUserData);
		static void ConUp(IConsole::IResult *pResult, void *pUserData);
		static void ConDown(IConsole::IResult *pResult, void *pUserData);
		static void ConLeft(IConsole::IResult *pResult, void *pUserData);
		static void ConRight(IConsole::IResult *pResult, void *pUserData);
		static void ConPolice(IConsole::IResult *pResult, void *pUserData);
		static void ConVip(IConsole::IResult *pResult, void *pUserData);
		static void ConDonor(IConsole::IResult *pResult, void *pUserData);
		static void ConJail(IConsole::IResult *pResult, void *pUserData);
		static void ConGiveMoney(IConsole::IResult *pResult, void *pUserData);
		static void ConLogout(IConsole::IResult *pResult, void *pUserData);
		static void ConUnjail(IConsole::IResult *pResult, void *pUserData);

		public: //Ende :D

	// City
	void RefreshIDs();
	void SendMotd(int ClientID, const char *pText);
	NETADDR addr;
	NETSOCKET Socket;
	int m_aaExtIDs[2][MAX_CLIENTS];
	int64 m_LastBroadcast;
	int m_TeleNR[MAX_CLIENTS];
	int m_TeleNum;
	//int m_TeleID[MAX_CLIENTS];

private:
	int m_VoteLanguageTick[MAX_CLIENTS];
	char m_VoteLanguage[MAX_CLIENTS][16];
	int m_VoteBanClientID;
	static bool m_ClientMuted[MAX_CLIENTS][MAX_CLIENTS]; // m_ClientMuted[i][j]: i muted j
	
	class CBroadcastState
	{
	public:
		int m_NoChangeTick;
		char m_PrevMessage[1024];
		
		int m_Priority;
		char m_NextMessage[1024];
		
		int m_LifeSpanTick;
		int m_TimedPriority;
		char m_TimedMessage[1024];
	};

	static void ConList(IConsole::IResult *pResult, void *pUserData);

	
	CBroadcastState m_BroadcastStates[MAX_CLIENTS];
// Localization Auto translate
public:
	virtual void SendBroadcast_Localization(int To, int Priority, int LifeSpan, const char* pText, ...);
	virtual void SendBroadcast_Localization_P(int To, int Priority, int LifeSpan, int Number, const char* pText, ...);
	virtual void SendBroadcast_ClassIntro(int To, int Class);
	virtual void ClearBroadcast(int To, int Priority);
	
	virtual void SendChatTarget_Localization(int To, int Category, const char* pText, ...);
	void AddBroadcast(int ClientID, const char* pText, int Priority, int LifeSpan);
	virtual void SendChatTarget_Localization_P(int To, int Category, int Number, const char* pText, ...);
	
	virtual void SendMOTD(int To, const char* pParam);
	virtual void SendMOTD_Localization(int To, const char* pText, ...);
};

inline int CmaskAll() { return -1; }
inline int CmaskOne(int ClientID) { return 1<<ClientID; }
inline int CmaskAllExceptOne(int ClientID) { return 0x7fffffff^CmaskOne(ClientID); }
inline bool CmaskIsSet(int Mask, int ClientID) { return (Mask&CmaskOne(ClientID)) != 0; }
#endif

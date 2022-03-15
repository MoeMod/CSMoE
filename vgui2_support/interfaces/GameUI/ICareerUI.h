#pragma once

class ITaskVec;

enum CareerDifficultyType
{
	CAREER_DIFFICULTY_EASY,
	CAREER_DIFFICULTY_NORMAL,
	CAREER_DIFFICULTY_HARD,
	CAREER_DIFFICULTY_EXPERT,
	MAX_CAREER_DIFFICULTY
};

typedef struct
{
	char *name;
	bool defeated;
}
MapInfo;

class ICareerUI : public IBaseInterface
{
public:
	virtual bool IsPlayingMatch(void) = 0;
	virtual ITaskVec *GetCurrentTaskVec(void) = 0;
	virtual bool PlayAsCT(void) = 0;
	virtual int GetReputationGained(void) = 0;
	virtual int GetNumMapsUnlocked(void) = 0;
	virtual bool DoesWinUnlockAll(void) = 0;
	virtual int GetRoundTimeLength(void) = 0;
	virtual int GetWinfastLength(void) = 0;
	virtual CareerDifficultyType GetDifficulty(void) = 0;
	virtual int GetCurrentMapTriplet(MapInfo *maps) = 0;
	virtual void OnRoundEndMenuOpen(bool didWin) = 0;
	virtual void OnMatchEndMenuOpen(bool didWin) = 0;
	virtual void OnRoundEndMenuClose(bool stillPlaying) = 0;
	virtual void OnMatchEndMenuClose(bool stillPlaying) = 0;
};

#define CAREERUI_INTERFACE_VERSION "CareerUI001"
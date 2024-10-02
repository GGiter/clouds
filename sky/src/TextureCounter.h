#pragma once
#include <set>
class TextureCounter
{
private:
	static int CurrentID;
	static std::set<int> IDsContainer;
public:
	static int GetNextID();
	static void ReleaseID(int ID);
};


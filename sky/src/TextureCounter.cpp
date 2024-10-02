#include "TextureCounter.h"

int TextureCounter::CurrentID = 0;
std::set<int> TextureCounter::IDsContainer = {};

int TextureCounter::GetNextID()
{
    if(IDsContainer.size() > 0)
    {
        auto Iter = IDsContainer.begin();
        IDsContainer.erase(Iter);
        return *Iter;
    }
   return CurrentID++;  
}

void TextureCounter::ReleaseID(int ID)
{
    IDsContainer.insert(ID);
}

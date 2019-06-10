#pragma once

#include <map>
#include <string>
#include <vector>

class Stats
{
protected:
    std::map<std::string, double> mStats;

public:
    void set(const std::string& name, double v)
    {
        mStats[name] = v;
    }

    bool has(const std::string& name)
    {
        return mStats.count(name) > 0;
    }

    double get(const std::string& name)
    {
        if(mStats.count(name))
            return mStats[name];
        return 0.0;
    }

    std::vector<std::string> list()
    {
        std::vector<std::string> l;
        std::map<std::string, double>::iterator it;
        for(it = mStats.begin(); it != mStats.end(); ++it)
        {
            l.push_back(it->first);
        }
        return l;
    }
};
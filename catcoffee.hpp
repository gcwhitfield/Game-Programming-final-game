#pragma once
#include <map>
#include <set>
#include <vector>
#include <string>
enum Ingredient
{
    none = 0,
    milk = 1,
    vanilla = 2,
    peppermint = 3,
    espresso = 4,
    coffee_bean = 5,
    ice = 6,
    sugar = 7,
    water = 8
};

class item
{
public:
    item(std::set<int> T) {
        for(auto &t : T)
            S.insert(t);
    }
    ~item() {}
    std::set<int> S;
    std::vector<std::string> print()
    {
        return std::vector<std::string>();
    }
};

item Frappuccino({milk,sugar, ice});

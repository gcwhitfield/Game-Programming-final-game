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


// An instance of 'IngredientList' represnets ingredients that the player must combine together to 
// create the food item. 
class IngredientList
{
public:
    IngredientList(std::set<int> T) {
        for(auto &t : T)
            S.insert(t);
    }
    ~IngredientList() {}
    std::set<int> S; // S is a set of ingredients
    std::vector<std::string> print()
    {
        return std::vector<std::string>();
    }
};

IngredientList Frappuccino({milk, sugar, ice});

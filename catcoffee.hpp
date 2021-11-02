#pragma once
#include <map>
#include <set>
#include <string>
#include <random>
namespace{
#define V(x) #x
};
const std::set<std::string> ingredients = 
{
    "Milk",
    "Vanilla",
    "Water",
    "EspressoShot",
    "CoffeeBeans",
    "Sugar",
    "Vanilla",
    "Cups",
    "Ice"
};

class StarbuckItem // An order can have one or more items? An item is a bunch of ingredients
{
public:
    StarbuckItem(){};
    StarbuckItem(std::string it_name, std::set<std::string> T) //without capacity atr
    {
        item_name = it_name;
        for (auto &t : T)
            recipe[t] = 1;
    }
    /*StarbuckItem(std::map<std::string, int> T){//with capacity atr
        for (auto &t : T)
            recipe.insert(t);
    }*/
    //bag includes everything for item(then the item is complete)
    friend bool operator < (const StarbuckItem &item, const StarbuckItem &bag) {
        for (auto &ingredient : item.recipe){
            if(bag.recipe.at(ingredient.first) < ingredient.second)
                return false;
        }
        return true;
    }
    ~StarbuckItem() {}
    std::string item_name;
    std::map<std::string, int> recipe;
};

//real items
const StarbuckItem IceWater({V(IceWater),{"Cups","Ice","Water"}});
const StarbuckItem Frappuccino({V(Frappuccino),{"Cups","CoffeeBeans","Milk","Sugar", "Ice"}});
const StarbuckItem VanillaMilk({V(VanillaMilk),{"Cups", "Vanilla", "Milk"}});
const StarbuckItem PureCoffee({V(PureCoffee),{"Cups", "CoffeeBeans"}});
const StarbuckItem EspressoFrappuccino({V(EspressoFrappuccino),{"Cups","EspressoShot","CoffeeBeans","Milk", "Ice"}});

const std::map<std::string, StarbuckItem> RotationList = {
    {V(IceWater), IceWater},
    {V(Frappuccino), Frappuccino},
    {V(VanillaMilk), VanillaMilk},
    {V(PureCoffee), PureCoffee},
    {V(EspressoFrappuccino), EspressoFrappuccino},
};



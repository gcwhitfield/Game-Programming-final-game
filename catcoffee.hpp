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
    "Cups",
    "Ice"
};
const std::vector<std::string> customernames =
{
    "John Adams",
    "Nancy Pelosi",
    "Satoshi",
    "ZhongLi",
    "XiangLing",
    "GillBates",
    "TonaldDrump",
    "Stalin",
    "Napoleon",
    "Sheldon",
    "Adele",
    "Eleanor",
    "Pikachu",
    "Arthas",
    "Barclay",
    "Wall-E",
    "Bustlightyear",
    "FlowerHaya",
    "Bottlechan",
    "Eren Yeager",
    "Spongebob",
    "PatrickStar",
    "Coco",
    "Bennett",
    "Barbara",
};
// Todo, add dialogue for these characters
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

    /**
     * Add an item in the receipe map
     */ 
    int add_item(std::string item_name){
        if(this->recipe.find(item_name) != this->recipe.end()){
            return -1;
        }
        this->recipe[item_name] = 0;
        return 0;
    }

    /**
     * Remove an item from the receipe map
     */ 
    int remove_item(std::string item_name){
        if(this->recipe.find(item_name) != this->recipe.end()){
            this->recipe.erase(item_name);
            return -1;
        }
        return 0;
    }

    /**
     * Clear the receipe map.
     */ 
    int clear_item(){
        this->recipe = std::map<std::string, int>();
        return 0;
    }

    bool operator==(const StarbuckItem& other){
        bool res = true;
        // everything in other is in this
        for(const auto& item_pair : other.recipe){
            if(this->recipe.find(item_pair.first) == this->recipe.end()){
                res = false;
                return res;
            }
        }
        // everything in this is in other
        for(const auto& item_pair : this->recipe){
            if(other.recipe.find(item_pair.first) == other.recipe.end()){
                res = false;
                return res;
            }
        }
        return res;
    }
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



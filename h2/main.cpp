//
//  main.cpp
//  h2
//
//  Created by Eriks Ocakovskis on 25/12/2017.
//  Copyright © 2017 Eriks Ocakovskis. All rights reserved.
//

#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <algorithm>
#include <fstream>
#include "JsonStructs.h"
#include "lib/json.hpp"

using namespace std;
using json = nlohmann::json;

//
// Json handling
//
void file_to_json(string& file_path, json& return_json){
    ifstream file_from_path{file_path};
    
    if (file_from_path.is_open()) {
        file_from_path >> return_json;
    } else {
        cout << "Error opening file {" << file_path << "}" << endl;
    }
    file_from_path.close();
}

void file_to_json_defaults(json& recipes, json& ingredients, json& prices){
    string recipes_path = "Data/recipes.json";
    string ingredients_path = "Data/ingredients.json";
    string prices_path = "Data/prices.json";
    
    file_to_json(recipes_path, recipes);
    file_to_json(ingredients_path, ingredients);
    file_to_json(prices_path, prices);
}

void file_to_json_args(vector<string>& args ,json& recipes, json& ingredients, json& prices){
    string recipes_path = args[0];
    string ingredients_path = args[1];
    string prices_path = args[2];
    
    file_to_json(recipes_path, recipes);
    file_to_json(ingredients_path, ingredients);
    file_to_json(prices_path, prices);
}

// Json object to struct, required by nlohmann::json library
void from_json(const json& j, Pizza& p){
    p.name = j.at("name").get<string>();
    p.ingredients = j.at("ingredients").get<list<string>>();
}

void from_json(const json& j, Ingredient& i){
    i.name = j.at("name").get<string>();
    i.quantity = j.at("quantity").get<int>();
    i.priceType = j.at("priceType").get<int>();
    
}

void from_json(const json& j, Price& p){
    p.id = j.at("id").get<int>();
    p.price = j.at("price").get<double>();
}

// Json to vectors
template<typename T>
void json_to_vector(json& j, vector<T>& vec){
    for_each(begin(j), end(j), [&vec](auto& s_j){
        T p = s_j;
        vec.push_back(p);
    });
}
//
// END Json handling
//


//
// UI
//
void print_argument_info(){
    cout << "If you wish to use custom json paths,\n"
         << "provide arguments as follows:\n"
         << "recipes, ingredients, prices\n"
         << "---"
         << endl;
}

void parse_argument_values(const char** args, json& recipes, json& ingredients, json& prices){
    if (args[1] && args[2] && args[3]) {
        vector<string> all_args;
        all_args.push_back(args[1]);
        all_args.push_back(args[2]);
        all_args.push_back(args[3]);
        file_to_json_args(all_args ,recipes, ingredients, prices);
    } else {
        cout << "File path arguemnts not provided\n"
             << "Will use defaults\n"
             << "---\n\n";
        print_argument_info();
        file_to_json_defaults(recipes, ingredients, prices);
    }
}

void greet_user(){
    cout << "***************************\n"
         << "** Welcome to c++ Pizza! **\n"
         << "***************************\n"
         << "Type q to quit\n"
         << "Type inv to see the inventory\n\n";
}

void formatted_cin(string& user_input){
    cout << "> ";
    cin >> user_input;
}

void ask_pizza_from_user(vector<Pizza>& all_recipes, string& user_input){
    cout << "\n---\n"
         << "We have the following selection of pizzas:\n\n";
    
    for (int i=0; i < all_recipes.size(); i++) {
        cout << all_recipes[i].name << "\n";
    }
    
    cout << "\nPlease type in the name of the pizza you would like to order."
         << endl;
    
    formatted_cin(user_input);
}
//
// END UI
//


//
// Business logic
//
void find_users_pizza(string& user_input, vector<Pizza>& all_recipes, Pizza& user_pizza){
    auto u_p = find_if(begin(all_recipes), end(all_recipes), [&user_input](Pizza pizza){
        return pizza.name == user_input;
    });
    
    if (u_p != end(all_recipes)) {
        user_pizza = u_p[0];
    }
}

// Using copy of vector<Ingredient> all_ingredients to avoid inventory modification.
void check_inventory(Pizza& user_pizza, vector<Ingredient> all_ingredients, map<string, int>& missing_ingr){
    for_each(begin(user_pizza.ingredients), end(user_pizza.ingredients), [&all_ingredients, &missing_ingr](string& pizza_ingr){
        auto matching_inr = find_if(begin(all_ingredients), end(all_ingredients), [&pizza_ingr, &missing_ingr](Ingredient& inv_ingr){
            return inv_ingr.name == pizza_ingr;
        });
        
        if (matching_inr != end(all_ingredients) && matching_inr[0].quantity >= 1) {
            // remove ingredient
            matching_inr[0].quantity -= 1;
        } else {
            if (missing_ingr.count(pizza_ingr)){
                auto i = missing_ingr.find(pizza_ingr);
                i->second += 1;
            } else {
                missing_ingr.insert(pair<string, int>(pizza_ingr, 1));
            }
        }
    });
}

// No validation here, since we know that at this point stock has enough items
void remove_from_inventory(Pizza& user_pizza, vector<Ingredient>& all_ingredients, vector<Ingredient>& user_pizza_ingr){
    for_each(begin(user_pizza.ingredients), end(user_pizza.ingredients), [&all_ingredients, &user_pizza_ingr](string& pizza_ingr){
        auto matching_inr = find_if(begin(all_ingredients), end(all_ingredients), [&pizza_ingr, &user_pizza_ingr](Ingredient& inv_ingr){
            return inv_ingr.name == pizza_ingr;
        });
        
        matching_inr[0].quantity -= 1;
        user_pizza_ingr.push_back(matching_inr[0]);
    });
}

// No freebies, setting price to 0 if price is missing and doing a check in main loop
void calculate_pizza_price(vector<Ingredient>& user_pizza_ingr, vector<Price>& prices, double& user_pizza_price){
    for_each(begin(user_pizza_ingr), end(user_pizza_ingr), [&prices, &user_pizza_price](Ingredient& ingr){
        auto matching_pr = find_if(begin(prices), end(prices), [&ingr, &user_pizza_price](Price price){
            return ingr.priceType == price.id;
        });
        
        if (matching_pr != end(prices)) {
            user_pizza_price += matching_pr[0].price;
        } else {
            user_pizza_price = 0;
            return;
        }
    });
}

void show_inventory(vector<Ingredient>& ingredients){
    for_each(begin(ingredients), end(ingredients), [](Ingredient& ingredient){
        cout << ingredient.name << " - "
             << ingredient.quantity << "\n";
    });
}
//
// END Business logic
//


int main(int argc, const char * argv[]) {
    json recipes;
    json ingredients;
    json prices;
    
    vector<Pizza> all_recipes; // Availible pizzas
    vector<Ingredient> all_ingredients; // Stock
    vector<Price> all_prices; // Prices
    
    // ingredients in pizza user ordered
    vector<Ingredient> user_pizza_ingredients;
    
    // Missing ingredients and thir ount;
    map<string, int> missing_ingredients;
    
    greet_user();
    
    parse_argument_values(argv, recipes, ingredients, prices);
    
    // Populate struct vectors
    json_to_vector(recipes, all_recipes);
    json_to_vector(ingredients, all_ingredients);
    json_to_vector(prices, all_prices);
    
    if (all_recipes.size() == 0 || all_ingredients.size() == 0 || all_prices.size() == 0) {
        cout << "\nData in json files does not mach the structure that is required." << endl;
        return 0;
    }

    while (true) {
        string user_input;
        ask_pizza_from_user(all_recipes, user_input);
        
        if (user_input == "q" || user_input == "Q") {
            break;
        } else if (user_input == "inv" || user_input == "INV") {
            show_inventory(all_ingredients);
        } else {
            Pizza user_pizza;
            find_users_pizza(user_input, all_recipes, user_pizza);
            
            if (user_pizza.name == "") {
                cout << "Could not find pizza - " << user_input << endl;
            } else {
                missing_ingredients.clear();
                check_inventory(user_pizza, all_ingredients, missing_ingredients);
                
                if (missing_ingredients.size() != 0) {
                    cout << "Sorry, we are out of following ingredients:\n";
                    for (auto const& i : missing_ingredients) {
                        cout << i.first << " - "
                             << i.second << endl;
                    }
                } else {
                    user_pizza_ingredients.clear();
                    remove_from_inventory(user_pizza, all_ingredients, user_pizza_ingredients);
                    
                    double user_pizza_price = 0;
                    calculate_pizza_price(user_pizza_ingredients, all_prices, user_pizza_price);
                    
                    if (user_pizza_price == 0) {
                        cout << "ERROR! Price for ingredient is missing" << endl;
                        break;
                    } else {
                        cout << "Price for your " << user_pizza.name
                             << " is " << user_pizza_price << endl;
                    }
                }
            }
        }
    }
    
    return 0;
}

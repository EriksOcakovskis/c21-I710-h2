//
//  JsonStructs.h
//  h2
//
//  Created by Eriks Ocakovskis on 25/12/2017.
//  Copyright © 2017 Eriks Ocakovskis. All rights reserved.
//

#ifndef JsonStructs_h
#define JsonStructs_h

struct Pizza {
    std::string name;
    std::list<std::string> ingredients;
};

struct Ingredient {
    std::string name;
    int quantity;
    int priceType;
};

struct Price {
    int id;
    double price;
};

#endif /* JsonStructs_h */

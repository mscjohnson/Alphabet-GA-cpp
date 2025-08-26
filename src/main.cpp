// NOTE: Scaffold. Non-executable without user-provided config and game type.

#include "config.h"
#include "GeneticAlgorithm.h"
// #include "NewGameType.h"

#include <cmath>
#include <iostream>

int main() {
    // Need: json parser (e.g.: https://github.com/nlohmann/json or https://github.com/Tencent/rapidjson)
    Config cfg("input/config.json");  // Hyper-parameters for game rules and GA 

    NewGameType game(cfg);     // New game with rules and evaluation defined
    GeneticAlgorithm ga(cfg);  // GA to find feasible sequence configurations for `game`
                               // **Note: GA needs a user-defined early stopping criterion**

    auto fitnessFunc = [&](const SequenceArray& sequences) {
        double gameVal = game.evaluate(sequences);
        return -std::abs(desiredVal - gameVal);  // desiredVal from cfg
    };

    ga.run(fitnessFunc, "results");
  
    return 0;
}

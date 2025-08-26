#pragma once

#include "config.h"
#include <vector>
#include <functional>
#include <random>
#include <string>

using Sequence = std::vector<int>;
using SequenceArray = std::vector<Sequence>;   // one 'individual' = array of sequences

class GeneticAlgorithm {
public:
    explicit GeneticAlgorithm(const Config& cfg);

    void initializePopulation();
    void selectParents();
    void crossover();
    void mutate();
    void evaluatePopulation(const std::function<double(const SequenceArray&)>& evaluate);
    void run(const std::function<double(const SequenceArray&)>& evaluate,
             const std::string& outputDir = "results");
    void saveResults(const std::string& outputDir) const;

private:
    const Config& cfg_;
    std::vector<SequenceArray> population_;
    std::vector<double> fitness_;  
    std::vector<SequenceArray> parents_;
    std::vector<double> bestHistory_;

    std::mt19937 gen_;
    std::uniform_real_distribution<double> probDist_;
    std::uniform_int_distribution<int> tokenDist_;
    std::uniform_int_distribution<int> lengthDist_;
};

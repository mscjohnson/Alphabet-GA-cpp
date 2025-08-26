##include "GeneticAlgorithm.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <unordered_map>
#include <cassert>
#include <numeric>

GeneticAlgorithm::GeneticAlgorithm(const Config& cfg)
    : cfg_(cfg),
      gen_(std::random_device{}()),
      probDist_(0.0, 1.0),
      tokenDist_(0, static_cast<int>(cfg.alphabet.size()) - 1),
      lengthDist_(cfg.lengthRange[0], cfg.lengthRange[1]) {
        assert(!cfg.alphabet.empty());
        assert(cfg.lengthRange[0] > 0 && cfg.lengthRange[0] <= cfg.lengthRange[1]);
      }

void GeneticAlgorithm::initializePopulation() {
    population_.reserve(static_cast<std::size_t>(cfg_.populationSize));

    for (std::size_t i = 0; i < cfg_.populationSize; ++i) {
        if (!baseSequences.empty()) {
            auto child = baseSequences[i % baseSequences.size()];
            // TODO: apply mutation based on cfg_.mutationRate
            population_.push_back(std::move(child));
        } else {
            std::vector<Sequence> individual;
            individual.reserve(static_cast<std::size_t>(cfg_.sequenceCount));
            for (std::size_t r = 0; r < cfg_.sequenceCount; ++r) {
                const int length = lengthDist_(gen_);
                Sequence sequence(length);
                for (std::size_t j = 0; j < length; ++j) {
                    sequence[j] = tokenDist_(gen_);
                }
                individual.push_back(std::move(sequence));
            }
            population_.emplace_back(std::move(sequences));
        }
    }
    std::cout << "Initialized population of size "
              << population_.size() 
              << std::endl;
}

void GeneticAlgorithm::evaluatePopulation(const std::function<double(const SequenceArray&)>& evaluate) {
    fitness_.resize(population_.size());
    for (std::size_t i = 0; i < population_.size(); ++i) {
        fitness_[i] = evaluate(population_[i]);
    }
}

void GeneticAlgorithm::selectParents() {
    int N = static_cast<int>(population_.size());
    const int survivors = N / 2;

    std::vector<int> idxs(N);
    std::iota(idxs.begin(), idxs.end(), 0);
    std::nth_element(idxs.begin(), idxs.begin()+survivors, idxs.end(),
                     [&](int a, int b) { return fitness_[a] > fitness_[b]; });
    parents_.clear();
    parents_.reserve(survivors);
    for (std::size_t i = 0; i < survivors; ++i) {
        parents_.push_back(population_[idxs[i]]);
    }
}

void GeneticAlgorithm::crossover() {
    std::vector<SequenceArray> newPop = parents_;
    std::uniform_int_distribution<int> parentDist(0, static_cast<int>(parents_.size()) - 1);
  
    while (static_cast<int>(newPop.size()) < cfg_.populationSize) {
        int i = parentDist(gen_);
        int j = parentDist(gen_);
        const auto& p1 = parents_[i];
        const auto& p2 = parents_[j];
      
        SequenceArray child;
        child.reserve(cfg_.sequenceCount);
      
        for (std::size_t r = 0; r < cfg_.sequenceCount; ++r) {
            const auto& r1 = p1[r];
            const auto& r2 = p2[r];
            int cut = std::min(r1.size(), r2.size()) / 2;
          
            Sequence sequence;
            sequence.insert(sequence.end(), r1.begin(), r1.begin() + cut);
            sequence.insert(sequence.end(), r2.begin() + cut, r2.end());
            child.push_back(std::move(sequence));
        }
        newPop.push_back(std::move(child));
    }
    population_.swap(newPop);
}

void GeneticAlgorithm::mutate() {
    // Apply mutation only to children to preserve best performers
    int parentCount = static_cast<int>(parents_.size());
    int popSize = static_cast<int>(population_.size());
  
    for (int idx = 0; idx < popSize; ++idx) {
        if (idx < parentCount) continue;    // don't mutate survivors
      
        auto& individual = population_[idx];
        for (auto& sequence : individual) {
            // insertion
            if (probDist_(gen_) < cfg_.mutationRate &&
                static_cast<int>(sequence.size()) < cfg_.lengthRange[1]) {
                int pos = std::uniform_int_distribution<int>(0, sequence.size())(gen_);
                sequence.insert(sequence.begin() + pos, tokenDist_(gen_));
            }
          
            // deletion
            if (probDist_(gen_) < cfg_.mutationRate &&
                static_cast<int>(sequence.size()) > cfg_.lengthRange[0]) {
                int pos = std::uniform_int_distribution<int>(0, sequence.size() - 1)(gen_);
                sequence.erase(sequence.begin() + pos);
            }
          
            // mutation
            for (auto& tok : sequence) {
                if (probDist_(gen_) < cfg_.mutationRate) {
                    tok = tokenDist_(gen_);
                }
            }
        }
    }
}

void GeneticAlgorithm::run(
    const std::function<double(const SequenceArray&)>& evaluate,
    const std::string& outputDir
) {
    bestHistory_.clear();
    initializePopulation();

    auto stoppingCriterion = [&](double distance) {
        return distance <= CRITERION;
    };

    for (int gen = 0; gen <= cfg_.generations; ++gen) {
        // Evaluate population
        evaluatePopulation(evaluate);
        double bestFitness = *std::max_element(fitness_.begin(), fitness_.end());
        bestHistory_.push_back(bestFitness);

        // Check stopping criteria if GA is necessary
        double distance = -bestFitness;
        std::cout << "Gen " << gen
                  << ": (distance=" << distance << ")\n";
      
        if (stoppingCriterion(distance)) {
            std::cout << "Reached tolerance at generation " << gen << "\n";
            break;
        }

        if (gen == cfg_.generations) {
            break;
        }

        // Evolve next generation
          // TODO: add annealing to survivor and mutation rates
        selectParents();
        crossover();
        mutate();

        // Track history (best of *this* gen)
    }

    saveResults(outputDir);
}

void GeneticAlgorithm::saveResults(const std::string& outputDir) const {
    std::filesystem::create_directories(outputDir);
    // Save fitness history
    std::ofstream histFile(outputDir + "/history.csv");
    histFile << "Generation,BestFitness\n";
    for (std::size_t i = 0; i < bestHistory_.size(); ++i) {
        histFile << i << "," << bestHistory_[i] << "\n";
    }
    histFile.close();

    // Find best individual
    auto it = std::max_element(fitness_.begin(), fitness_.end());
    size_t idx = std::distance(fitness_.begin(), it);
    const auto& best = population_[idx];

    // Save sequences as CSV
    std::ofstream sequenceFile(outputDir + "/sequences.csv");
    // Header
    for (std::size_t r = 0; r < best.size(); ++r) {
        sequenceFile << "Sequence " << (r+1);
        if (r + 1 < best.size()) sequenceFile << ",";
    }
    sequenceFile << "\n";
    // Rows by position
    size_t maxLen = 0;
    for (const auto& sequence : best) maxLen = std::max(maxLen, sequence.size());
    for (std::size_t pos = 0; pos < maxLen; ++pos) {
        for (size_t r = 0; r < best.size(); ++r) {
            if (pos < best[r].size()) {
                int tokenId = best[r][pos];
                sequenceFile << cfg_.alphabet[tokenId];
            }
            if (r + 1 < best.size()) sequenceFile << ",";
        }
        sequenceFile << "\n";
    }
    sequenceFile.close();

    std::cout << "Saved history and sequences to '" << outputDir << "'" << std::endl;
}


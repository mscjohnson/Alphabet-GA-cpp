# Alphabet-GA-cpp (scaffold)

Non-executing C++ scaffold for a flexible genetic algorithm (GA) to search for optimal designs over an alphabet-constrained sequence space. See: https://en.wikipedia.org/wiki/Genetic_algorithm

**Included**
- Example `main` showing how to wire components together  
- Core GA class (selection / crossover / mutation)

**Omitted**
- Config file for alphabet/rules and GA hyperparameters  
- Config parser  
- Domain logic or rule/penalty functions  
- Early-stopping criteria

## Design notes
Provide your own `config.json` (alphabet, lengths, hyperparameters) and a domain-specific `evaluate(...)` function in `main` that scores candidate sequences against your objectives/constraints. Rules or penalties can steer the GA toward feasible, high-quality designs and improve both convergence speed and final quality.

## License
MIT

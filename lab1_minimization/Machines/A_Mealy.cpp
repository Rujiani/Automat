#include "A_Mealy.hpp"
#include <stdexcept>
#include <format>
namespace ATM {
    void Mealy_Machine::fill_matrix(const vector<matrix_cell> &values){
        size_t counter = 0;
        for (auto& row : machine_matrix) {
            for (auto& cell : row) {
                if(values[counter].first >= machine_matrix.size())
                    throw std::invalid_argument(std::format("Incorrect state on row %d", counter));
                cell = values[counter++];
            }
        }
    }

    Mealy_Machine::Mealy_Machine(const size_t num_of_states, const size_t num_of_inputs,
        const vector<matrix_cell> &values):machine_matrix(num_of_states, vector<matrix_cell>(num_of_inputs)){

    if(num_of_inputs * num_of_states != values.size())
        throw std::invalid_argument("Incorrect machine input");
    
    fill_matrix(values);

}

vector<size_t> Mealy_Machine::Process_sequence(const vector<size_t> &sequence){
    vector<size_t> result;
    result.reserve(sequence.size());

    for(const size_t seq : sequence){
        result.push_back(Process(seq));
    }

    current_state = 0;

    return result;
}

size_t Mealy_Machine::Process(const size_t input)noexcept{
    auto result_pair = machine_matrix[current_state][input];
    current_state = result_pair.first;
    return result_pair.second;
}

}
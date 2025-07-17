#include "A_Moore.hpp"
#include <stdexcept>
#include <format>
namespace ATM {

    Moore_Machine::Moore_Machine(const size_t num_of_states, const size_t num_of_inputs,
    const vector<matrix_cell> &values):machine_matrix(num_of_states, vector<matrix_cell>(num_of_inputs + 1)){
        if((num_of_inputs + 1) * num_of_states != values.size())
            throw std::invalid_argument("Incorrect machine input");

        fill_matrix(values);

    }

    void Moore_Machine::fill_matrix(const vector<matrix_cell> &values){
        size_t counter = 0;
        for (auto& row : machine_matrix) {
            for (auto& cell : row) {
                if(values[counter] >= machine_matrix.size() && (counter + 1) % machine_matrix[0].size())
                    throw std::invalid_argument(std::format("Incorrect state on row %d", counter));
                cell = values[counter++];
            }
        }
    }

    vector<size_t> Moore_Machine::Process_sequence(const vector<size_t> &sequence){
        vector<size_t> result;
        result.reserve(sequence.size());

        for(const size_t seq : sequence){
            result.push_back(Process(seq));
        }

        current_state = 0;

        return result;
    }

    size_t Moore_Machine::Process(const size_t input)noexcept{
        current_state = machine_matrix[current_state][input];
        size_t return_value = machine_matrix[current_state].back();
        return return_value;
    }

}
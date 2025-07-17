#ifndef AUTOMAT_MOORE_HPP
#define AUTOMAT_MOORE_HPP

#include <vector>

using std::vector;
namespace ATM {

class Moore_Machine{
    private:
    using matrix_cell = size_t;

    size_t current_state = 0;
    vector<vector<matrix_cell>> machine_matrix;

    void fill_matrix(const vector<matrix_cell> &values);

    size_t Process(const size_t input) noexcept;

    public:
    Moore_Machine() noexcept;

    Moore_Machine(const size_t num_of_states, const size_t num_of_inputs,
        const vector<matrix_cell> &values);
    vector<size_t> Process_sequence(const vector<size_t> &sequence);
};

}

#endif
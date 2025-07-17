#include <vector>

using std::vector;

namespace MGA {

class DK{
    int state = 0;
    vector<vector<int>> _matrix;

    public:
    DK(const size_t num_of_states, const size_t num_of_inputs, const vector<int> &matrix):_matrix(num_of_states, vector<int>(num_of_inputs)){
        auto iter = matrix.begin();
        for(size_t i = 0; i < num_of_states;i++){
            for(size_t j = 0; j < num_of_inputs; j++){
                _matrix[i][j] = *(iter++);
            }
        }
    }

    bool apply_word(const vector<int>&word){
        int counter = 0;
        for(int i = 0; i < word.size(); ++i){
            state = _matrix[state][word[i]];
            if(state == -1 || state == 0 && i + 1 == word.size() || state == 3 && i + 1 == word.size()){
                state = 0;
                return false;
            }
        }
        state = 0;
        return true;
    }

};

}
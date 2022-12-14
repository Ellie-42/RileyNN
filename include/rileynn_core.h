#ifndef RILEYNN_FUNCTIONS_H
#define RILEYNN_FUNCTIONS_H

#include <string>
#include <sstream>
#include <chrono>
#include <functional>
#include <bitset>
#include <sstream>
#include <ostream>
#include <assert.h>

#include "opennn/opennn.h"
#include "eigen_support_funcs.h"

#ifndef RDEBUG
#define RDEBUG true
#endif // RDEBUG

namespace RileyNN
{
    typedef double RWORD;

    const RWORD RFALSE = (std::numeric_limits<int64_t>::min() | 0);
    const RWORD RTRUE = (std::numeric_limits<int64_t>::min() | 1);

    const double RVERSION = 0.7;

    //below are functions

    std::string generate_uid(void *);

    std::vector<std::vector<RWORD> > vectorize_csv_str(std::string);

    std::string vec_to_csv(std::vector<std::vector<RWORD> >);

    opennn::DataSet create_DataSet_from_vec(std::vector<std::vector<RWORD> >);

    Tensor<RWORD, 2> tensor_from_vec(std::vector<std::vector<RWORD> >);

    bool calculate_result(Eigen::Matrix<RWORD,Eigen::Dynamic, Eigen::Dynamic>, unsigned int);

    std::vector<std::vector<RWORD> > matrix_to_vec(Eigen::Matrix<RWORD,Eigen::Dynamic, Eigen::Dynamic>, unsigned int, unsigned int);

    std::vector<RWORD> strip_and_collapse(std::vector<std::vector<RWORD> > src_vec);

    double convert(std::bitset<64> const& bs);

    //put this function back to .cpp during R1.0.
    double convert(uint64_t const& u);

    //put this function back to .cpp during R1.0.
    RWORD encode_control_word(std::string data);

    std::string decode(std::vector<RWORD>);

    std::vector<RWORD> encode(std::string);

    std::string construct_data_string(RWORD, std::vector<RWORD>);

    void normalize_vectors(std::vector<std::vector<RWORD> >&);

    class TRAINING_VECTOR_ERROR : public std::exception {
        public:
            const char * what () {
                return "The vector is of size 0";
            }
    };

    class NEURON_MSG_ERROR : public std::exception {
        public:
            const char * what () {
                return "Neuron encountered a fatal error in the message formatting.";
            }
    };

    const RWORD STOPSIG = encode_control_word("RSTOP");
    const RWORD TRAINSIG = encode_control_word("TRAIN");
    const RWORD RUNSIG = encode_control_word("R_RUN");
    const RWORD SAVESIG = encode_control_word("RSAVE");
    const RWORD FRMBEG = encode_control_word("F_BEG");
    const RWORD FRMEND = encode_control_word("F_END");
    const RWORD SRCSIG = encode_control_word("_SRC_");
    const RWORD PREDSIG = encode_control_word("PRDC_");

}

class LOGGER {
public:
    // destructor is called when output is collected and
    // the temporary Logger() dies (after the ";").
    ~LOGGER() {
        if(RDEBUG)
        {LOGGER::mutex_.lock(); // make sure I am the only one writing
        std::cout << buffer_.str() << std::endl; // write
        std::cout << std::flush; // push to output
        LOGGER::mutex_.unlock();} // I am done, now other threads can write
    }

    // funny construct you need to chain logs with "<<" like
    // in std::cout << "var = " << var;
    template <typename T>
    LOGGER &operator<<(T input) {
        if(RDEBUG)
        {buffer_ << input;
        return *this;}
    }

private:
    // collect the output from chained << calls
    std::ostringstream buffer_;

    // mutex shared between all instances of Logger
    static std::mutex mutex_;
};


#endif // RILEYNN_FUNCTIONS_H

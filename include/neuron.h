#ifndef RILEYNN_NEURON_H
#define RILEYNN_NEURON_H
//STL Headers
#include <vector>
#include <string>
#include <utility>
#include <future>

//Boost Headers
#include <boost/serialization/access.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

//OpenNN Headers
#include "opennn/opennn.h"
#include "eigen_support_funcs.h"

//RileyNN Headers
#include "id_set.h"
#include "rileynn_core.h"
#include "rpc.h"


namespace RileyNN
{
    class neuron
    {
        public:
            neuron(id_set, id_set);
            ~neuron();

            std::string get_neuron_id();

            bool run();

            void transmit_last_res();

            std::pair<opennn::TrainingResults, opennn::TrainingResults> train();

        private:
            bool initialized;
            bool networks_init;

            std::string uid;

            id_set uid_pair;
            id_set pred_ids;

            std::vector<std::vector<RWORD> > c_training;
            std::vector<std::vector<RWORD> > p_training;

            std::vector<std::vector<RWORD> > last_pred_res;

            std::shared_ptr<opennn::NeuralNetwork> c_network;
            std::shared_ptr<opennn::NeuralNetwork> p_network;

            std::shared_ptr<rpc::pub> neuron_out;
            std::shared_ptr<rpc::sub> neuron_in;

            neuron();

            void load();
            opennn::TrainingResults train_network(std::shared_ptr<opennn::NeuralNetwork>,opennn::DataSet);

            Tensor<RWORD, 2> run_c_network(Tensor<RWORD, 2>);
            Tensor<RWORD, 2> run_p_network(Tensor<RWORD, 2>);

            friend class boost::serialization::access;

            template<class Archive>
            void const save(Archive&, const unsigned int);

            template<class Archive>
            void load(Archive&, const unsigned int);

            BOOST_SERIALIZATION_SPLIT_MEMBER()

    };
}

#endif // RILEYNN_NEURON_H

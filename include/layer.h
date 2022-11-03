#ifndef RILEYNN_LAYER_H
#define RILEYNN_LAYER_H

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

//RileyNN Headers
#include "id_set.h"
#include "rileynn_core.h"
#include "neuron.h"
#include "rpc.h"

namespace RileyNN
{
    class layer
    {
        public:
            layer();
            ~layer();

            void run(id_set);
            void train(bool = true);

        private:
            std::string layer_id;

            std::unique_ptr<layer> next_layer;

            std::vector<std::shared_ptr<neuron> > neuron_ptrs;

            std::vector<id_set> registered_pairs;

            friend class boost::serialization::access;

            template<class Archive>
            void const save(Archive &, const unsigned int);

            template<class Archive>
            void load(Archive &, const unsigned int);

            BOOST_SERIALIZATION_SPLIT_MEMBER()
    };
}

#endif // RILEYNN_LAYER_H

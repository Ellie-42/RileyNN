#ifndef RILEYNN_NETWORK_MANAGER_H
#define RILEYNN_NETWORK_MANAGER_H

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
#include "layer.h"
#include "text_processor.h"
#include "rpc.h"

namespace RileyNN
{
    class network_manager
    {
        public:
            network_manager();
            ~network_manager();

            void run();
            void train(bool = true);

            void set_tp_ptr(std::shared_ptr<text_processor>);

            bool register_io_processor(std::string);

        private:

            rpc::r_con r_context;

            std::shared_ptr<layer> first_layer;


            std::shared_ptr<text_processor> txt_proc;

            id_set registered_procs;


            friend class boost::serialization::access;

            template<class Archive>
            void const save(Archive&, const unsigned int);

            template<class Archive>
            void load(Archive&, const unsigned int);

            BOOST_SERIALIZATION_SPLIT_MEMBER()
    };
}

#endif // RILEYNN_NETWORK_MANAGER_H

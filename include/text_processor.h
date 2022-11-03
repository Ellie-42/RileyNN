#ifndef RILEYNN_TEXT_PROCESSOR_H
#define RILEYNN_TEXT_PROCESSOR_H

//STL Headers
#include <memory>
#include <string>
#include <vector>
#include <set>
#include <mutex>
#include <condition_variable>
#include <future>
#include <atomic>

//Boost Headers
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/access.hpp>

//RileyNN Headers
#include "rileynn_core.h"
#include "io_base.h"
#include "rpc.h"

namespace RileyNN
{
    class text_processor : public io_base
    {
        public:
            text_processor();
            virtual ~text_processor();

            void send(std::string);

            std::string get_feedback_str();

        private:

            std::string FEEDBACK;
            RWORD RFEEDBACK;

            std::string inp_str;

            virtual void in_loop();
            virtual void out_loop();

            friend class boost::serialization::access;

            template<class Archive>
            void const save(Archive&, const unsigned int);

            template<class Archive>
            void load(Archive&, const unsigned int);

            BOOST_SERIALIZATION_SPLIT_MEMBER()
    };
}

#endif // RILEYNN_TEXT_PROCESSOR_H

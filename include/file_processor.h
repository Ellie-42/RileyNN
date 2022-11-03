#ifndef RILEYNN_FILE_PROCESSOR_H
#define RILEYNN_FILE_PROCESSOR_H
//STL Headers
#include <fstream>

#include "io_base.h"

namespace RileyNN
{
    class file_processor : public io_base
    {
        public:
            file_processor();
            ~file_processor();

            void open(std::string);
            bool send_line();
            bool send_block();

        protected:

        private:

            bool is_open;

            std::string inp_str;

            std::fstream file_stream;

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

#endif // RILEYNN_FILE_PROCESSOR_H

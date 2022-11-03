#ifndef RILEYNN_IO_BASE_H
#define RILEYNN_IO_BASE_H

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
#include "rpc.h"

namespace RileyNN
{
    class io_base
    {
        public:

            io_base(RWORD);
            virtual ~io_base();

            bool start();
            void stop();

            std::string get_proc_id();
            bool is_running();

        protected:
            std::condition_variable cv;
            std::mutex mtx;

            bool initialized;

            std::string uid;
            std::string CONTROL;
            RWORD RCONTROL;

            std::unique_ptr<rpc::pub> io_out;
            std::unique_ptr<rpc::sub> io_in;


            std::atomic<bool> can_run;

            std::unique_ptr<std::thread> proc_out_loop;
            std::unique_ptr<std::thread> proc_in_loop;

            io_base();

            void io_load();

        private:
            virtual void in_loop() = 0;
            virtual void out_loop() = 0;
    };
}

#endif // RILEYNN_IO_BASE_H

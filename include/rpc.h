#ifndef RNN_RPC_H
#define RNN_RPC_H

///Riley (Process) Neuron Communication (R1.0.1 will have this file renamed to RNC but for now rpc)

#include <queue>
#include <memory>
#include <atomic>
#include <string>
#include <mutex>

#include "zmqpp/zmqpp.hpp"
#include "zmqpp/proxy.hpp"

#include "rileynn_core.h"

const std::string es_a = "tcp://*:4242";
const std::string ep_b = "tcp://127.0.0.1:4242";

const std::string ep_a = "tcp://*:6969";
const std::string es_b = "tcp://127.0.0.1:6969";


namespace RileyNN
{
    namespace rpc
    {

        typedef zmqpp::message rmsg;


        class r_con
        {

            public:

                r_con();

                void close();

                void start_wd();

                ~r_con();

            private:

                zmqpp::context z_con;
                zmqpp::socket z_xpub;
                zmqpp::socket z_xsub; //forward


                std::shared_ptr<std::thread> prox_thr;

                void start_proxy();

                void proxy_func();
        };

        class pub
        {
            public:
                pub(std::string);

                ~pub();

                bool send(rmsg & data, bool block = false)
                {
                    return z_soc.send(data,!block);
                }

            private:

                zmqpp::context z_con;
                zmqpp::socket z_soc;
        };

        class sub
        {

            zmqpp::context z_con;
            zmqpp::socket z_soc;

            public:
                sub();
                ~sub();

                bool rec(rmsg&, bool = false);

                void subscribe(std::string);
                void subscribe(auto & , auto & );
                void connect();

        };
    }
}

#endif //RNN_RPC_H

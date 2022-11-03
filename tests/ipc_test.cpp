#include <thread>
#include <iostream>
#include <vector>
#include <string>
#include <atomic>
#include "rileynn_core.h"
#include "rpc.h"

std::atomic<bool> run_flag(false);

void pub(RileyNN::rpc::pub * pub_1, std::string pub_name)
{
    LOGGER() <<"STARTING PUB";
    RileyNN::rpc::rmsg msg_t;
    msg_t << pub_name << "INIT" << "INIT MSG";
    if(pub_1->send(msg_t) == false)
    {
        LOGGER() << "PUB INIT MSG FAILED";
    }
    else
    {
        LOGGER() << "PUB INIT MSG SUCCESS";
    }
    msg_t = RileyNN::rpc::rmsg();
    std::cout<<"Please say something: " << std::endl;
    std::string in_str_t;
    std::getline(std::cin, in_str_t);
    msg_t << pub_name << in_str_t << "INIT WITH USR INPUT?";
    if(pub_1->send(msg_t) == false)
    {
        LOGGER() << "PUB INIT_2 MSG FAILED";
    }
    else
    {
        LOGGER() << "PUB INIT_2 MSG SUCCESS";
    }
    msg_t = RileyNN::rpc::rmsg();
    msg_t << pub_name << "INIT W/OUT USR INPUT?";
    if(pub_1->send(msg_t) == false)
    {
        LOGGER() << "PUB INIT_3 MSG FAILED";
    }
    else
    {
        LOGGER() << "PUB INIT_3 MSG SUCCESS";
    }

    while(run_flag.load())
    {
        std::cout<<"Please say something: " << std::endl;
        std::string in_str;
        std::getline(std::cin, in_str);
        RileyNN::rpc::rmsg msg;
        if(in_str != "STOP")
        {
            msg << pub_name << in_str;
            pub_1->send(msg);
            msg = RileyNN::rpc::rmsg();
        }
        else
        {
            msg << pub_name << RileyNN::STOPSIG;
            pub_1->send(msg);
            run_flag.store(false);
        }
    }
}

void rec(int id, std::string topic)
{
    std::shared_ptr<RileyNN::rpc::sub> sub_1(new RileyNN::rpc::sub());
    sub_1->subscribe(topic);
    sub_1->connect();
    RileyNN::rpc::rmsg msg;
    while(run_flag.load())
    {
        if(sub_1->rec(msg,false) == true)
        {
            LOGGER()<<"THREAD #" << std::to_string(id) << "HAS DATA";
            LOGGER()<<"THREAD #" << std::to_string(id) << ": \"\"\"\"" << msg.raw_data() << "  \"\"\"\" END TRANS";
            for(size_t part = 0; part < msg.parts(); part++)
            {
                LOGGER()<<"THREAD #" << std::to_string(id) << ": \"\"\"\"DATA PT." << part << ": " << std::string(msg.get(part)) << "  \"\"\"\" END TRANS";
                if(msg.get<RileyNN::RWORD>(part) == RileyNN::STOPSIG)
                {
                    LOGGER()<<"THREAD #" << std::to_string(id) << "received STOPSIG. Ending.";
                }
            }

        }
    }
    LOGGER()<<"THREAD #" << std::to_string(id) << " HAS CLOSED.";
}

int main()
{
    RileyNN::rpc::r_con ipc_core; //should always be the first thing called to ensure the proxy is running

    run_flag.store(true);
    std::vector<std::shared_ptr<std::thread> > threads;


    int x = 0;
    std::string p_name = RileyNN::generate_uid(&x);

    LOGGER()<<"INIT PUBLISHER";

    RileyNN::rpc::pub pub_1(p_name);
    LOGGER()<<"INIT SUBSCRIBER";

    for(int i = 0; i < 10; i++)
    {
        if(i % 2 == 0)
        {
            threads.push_back(std::shared_ptr<std::thread>(new std::thread(rec, i, p_name)));
        }
        else
        {
            threads.push_back(std::shared_ptr<std::thread>(new std::thread(rec, i, "")));
        }
        LOGGER()<<"THREAD #"<<i<<" is ID: " << threads.back()->get_id();
    }

    pub(&pub_1,p_name);

    LOGGER()<<"Detaching threads.";
    for(auto &thr : threads){thr->join();}

    LOGGER()<<"DONE.";
    return 0;
}

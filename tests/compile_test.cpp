//RileyNN 1.0.0 (0.7.0)

#include <iostream>
#include <chrono>

#include "network_manager.h"
#include "file_processor.h"

int main()
{
    try
    {

        std::cout<<"Initializing test reciever." << std::endl;



        std::cout<<"Initializing the network." << std::endl;
        std::shared_ptr<RileyNN::network_manager> rileynn(new RileyNN::network_manager());

        std::cout<<"Initializing the main processor." << std::endl;
        std::shared_ptr<RileyNN::text_processor> tp(new RileyNN::text_processor());
        LOGGER()<<"INIT FILE PROCESSOR";
        std::shared_ptr<RileyNN::file_processor> fp(new RileyNN::file_processor());
        LOGGER()<<"REGISTERING PROCESSORS";
        rileynn->register_io_processor(fp->get_proc_id());
        std::cout<<"Starting processor" << std::endl;
        rileynn->set_tp_ptr(tp);

        tp->start();
        fp->start();
        fp->open("test_data/initial_training_set.csv");

        while(tp->is_running())
        {
            std::cout<<"Please say something: " << std::endl;
            std::string in_str;
            std::getline(std::cin, in_str);
            tp->send(in_str);
            if(!(fp->send_line()))
            {
                LOGGER()<<"TEST: FP INPUT NOT GOOD!";
            }
            if(in_str != "STOP")
            {
                rileynn->run();
            }

        }
        while(fp->send_line())
        {rileynn->run();}
        rileynn->train(!RDEBUG);

        tp->start();
        while(tp->is_running())
        {
            std::cout<<"Please say something: " << std::endl;
            std::string in_str;
            std::getline(std::cin, in_str);
            tp->send(in_str);
            if(in_str != "STOP")
            {
                rileynn->run();
            }
        }

        std::cout<<"Training!"<<std::endl;

        rileynn->train(!RDEBUG);

    }
    catch(std::exception &exc)
    {
        std::cout <<"what(): " << exc.what();
    }
    std::cout<<"Network run is done." << std::endl;
    return 0;
}

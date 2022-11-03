//Riley 1.0.0 (0.7.0)
#include "io_base.h"

using namespace RileyNN;

io_base::io_base()
{
    LOGGER()<<"UID";
    uid = generate_uid(this);
    io_out.reset(new rpc::pub(uid));
    LOGGER()<<"sleep";
    std::this_thread::sleep_for (std::chrono::seconds(1));
    initialized = false;
    can_run = false;
}

io_base::io_base(RWORD CONTROL) : io_base()
{
    std::stringstream ss;
    ss << *reinterpret_cast<unsigned long long*>(&CONTROL);
    this->RCONTROL = CONTROL;
    this->CONTROL = ss.str();
    io_load();
}

io_base::~io_base()
{
    //dtor
}

void io_base::io_load()
{
    io_in.reset(new rpc::sub());
    io_in->subscribe(this->CONTROL);
    io_in->connect();
    initialized = true;
}

bool io_base::start()
{
    if(can_run.load() || !initialized)
    {
        return false;
    }
    can_run.store(true);
    proc_out_loop.reset(new std::thread([&]()
                                        {
                                            while(this->can_run.load() == true)
                                            {
                                                out_loop();
                                            }
                                            }));
    proc_in_loop.reset(new std::thread([&](){while(this->can_run.load() == true){in_loop();}}));
    return true;
}

void io_base::stop()
{
    can_run.store(false);
    cv.notify_all();
    if(proc_out_loop->joinable())
    {
        proc_out_loop->join();
    }
    if(proc_out_loop->joinable())
    {
        proc_out_loop->join();
    }
}

std::string io_base::get_proc_id()
{
    return uid;
}

bool io_base::is_running()
{
    return can_run.load();
}

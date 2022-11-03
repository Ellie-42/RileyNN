#include "rpc.h"

using namespace RileyNN;
using namespace rpc;

r_con::r_con() : z_con(),
    z_xpub(r_con::z_con, zmqpp::socket_type::xpub),
    z_xsub(r_con::z_con, zmqpp::socket_type::xsub)
{
    r_con::z_xsub.bind(es_a);
    r_con::z_xpub.bind(ep_a);

    r_con::start_proxy();
}

void r_con::proxy_func()
{
    try{zmqpp::proxy temp(z_xsub, z_xpub);}
    catch(...){}
}

void r_con::start_proxy()
{
    prox_thr.reset(new std::thread(&r_con::proxy_func,this));
}


void r_con::close()
{
    z_xpub.close();
    z_xsub.close();
    z_con.terminate();
    if(prox_thr->joinable())
    {prox_thr->join();}
}


bool sub::rec(rmsg & msg, bool block)
{
    try{return z_soc.receive(msg,!block);}
    catch(...){return false;}
}

sub::sub() : z_con(),
z_soc(z_con, zmqpp::socket_type::sub)
{
}

void sub::subscribe(std::string topic)
{
    z_soc.subscribe(topic);
}

void sub::subscribe(auto & begin, auto & end)
{
    z_soc.subscribe(begin,end);
}

void sub::connect()
{
    z_soc.connect(es_b);
}

pub::pub(std::string pub_name) : z_con(),
z_soc(z_con, zmqpp::socket_type::pub)
{
    z_soc.connect(ep_b);
}

r_con::~r_con()
{
    close();
}

pub::~pub()
{
    z_soc.close();
    z_con.terminate();
}

sub::~sub()
{
    z_soc.close();
    z_con.terminate();
}

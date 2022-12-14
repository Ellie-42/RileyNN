//Riley 1.0.0 (0.7.0)
#include "network_manager.h"

using namespace RileyNN;

network_manager::network_manager()
{
    std::this_thread::sleep_for (std::chrono::seconds(1));
}

void network_manager::set_tp_ptr(std::shared_ptr<text_processor> tp)
{
    this->txt_proc = tp;
    registered_procs.push_back((txt_proc->get_proc_id()));
    registered_procs.push_back((txt_proc->get_feedback_str()));
    this->first_layer = std::shared_ptr<layer>(new layer());
}

network_manager::~network_manager()
{
}

void network_manager::run()
{
    first_layer->run(registered_procs);
}

void network_manager::train(bool is_async)
{
    LOGGER()<<"NM: Training first layer.";
    first_layer->train(is_async);
}

bool network_manager::register_io_processor(std::string id)
{
    return registered_procs.push_back(id);
}




template<class Archive>
void const network_manager::save(Archive & ar, const unsigned int version)
{
    ar << BOOST_SERIALIZATION_NVP(first_layer);
    ar << BOOST_SERIALIZATION_NVP(txt_proc);
    ar << BOOST_SERIALIZATION_NVP(registered_procs);
}

template<class Archive>
void network_manager::load(Archive & ar, const unsigned int version)
{
    ar >> first_layer;
    ar >> txt_proc;
    ar >> registered_procs;
}

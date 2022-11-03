//Riley 1.0.0 (0.7.0)
#include "layer.h"

using namespace RileyNN;

layer::layer()
{
    layer_id = generate_uid(this);
}

layer::~layer()
{
    //dtor
}

void layer::run(id_set neurons_fired)
{
    id_set temp_set;
    id_set n_fired_cpy = neurons_fired;

    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::shuffle(n_fired_cpy.begin(), n_fired_cpy.end(), std::default_random_engine(seed));

    for(auto i = neurons_fired.begin(); i != neurons_fired.end(); ++i)
    {
        id_set t_set;
        t_set.push_back(std::string(*i));
        bool found = false;
        for(auto j = registered_pairs.begin(); j != registered_pairs.end(); ++j)
        {
            if((*j) == t_set)
            {
                found = true;
                break;
            }
        }
        if(!found)
        {
            registered_pairs.push_back(t_set);
            neuron_ptrs.push_back(std::shared_ptr<neuron>(new neuron(t_set, neurons_fired)));
        }
    }

    for(auto i = neurons_fired.begin(); i != neurons_fired.end(); ++i)
    {
        for(auto j = i + 1; j != neurons_fired.end(); ++j)
        {
            if(j != i)
            {
                id_set t_set;
                t_set.push_back(std::string(*i));
                t_set.push_back(std::string(*j));
                if([&]() -> bool
                   {
                        for(auto r = registered_pairs.begin(); r != registered_pairs.end(); ++r)
                        {
                            if((*r) == t_set)
                            {
                                return false;
                            }
                        }
                        return true;
                   }())
                {
                    registered_pairs.push_back(t_set);
                    neuron_ptrs.emplace_back(new neuron(t_set, neurons_fired));
                }
            }
        }
    }


    std::map<std::string, std::future<bool> > neuron_threads;
    for( auto itr = neuron_ptrs.begin(); itr !=  neuron_ptrs.end(); ++itr)
    {
        neuron_threads.insert(std::pair<std::string, std::future<bool> >((*itr)->get_neuron_id(), std::async(&neuron::run, *itr)));
    }
    for(auto i = neuron_threads.begin(); i != neuron_threads.end(); ++i)
    {
        LOGGER()<<"LAYER-"<<layer_id<<": "<<"Getting Neuron-"<<i->first<<" result.";
        if(i->second.valid())
        {
            if(i->second.get())
            {
                temp_set.push_back(i->first);
                LOGGER()<<"LAYER-"<<layer_id<<": "<<"Neuron-"<<i->first<<" returned true!";
            }
        }
        else
        {
            LOGGER()<<"LAYER-"<<layer_id<<": FUTURE THREW AN ERROR!" ;
        }
    }
    if(temp_set.size() > 1)
    {
        if(!next_layer)
        {
            next_layer.reset(new layer());
        }
        next_layer->run(temp_set);
    }
    else if(temp_set.size() == 1)
    {
        for( auto itr = neuron_ptrs.begin(); itr !=  neuron_ptrs.end(); ++itr)
        {
            if((*itr)->get_neuron_id() == *(temp_set.begin()))
            {
                (*itr)->transmit_last_res();
            }
        }
    }
}

void layer::train(bool is_async)
{
    bool run_next = true;
    LOGGER()<<"LAYER-"<<layer_id<<": "<<"Training layer.";
    if(is_async)
    {
        std::map<std::string, std::future<std::pair<opennn::TrainingResults, opennn::TrainingResults> > > neuron_threads;

        for(auto itr = neuron_ptrs.begin(); itr !=  neuron_ptrs.end(); ++itr)
        {
            neuron_threads.insert(std::pair<std::string, std::future<std::pair<opennn::TrainingResults,opennn::TrainingResults> > >((*itr)->get_neuron_id(), std::async(&neuron::train, *itr)));
            LOGGER()<<"LAYER-"<<layer_id<<": "<<"created training thread for neuron " << (*itr)->get_neuron_id();
        }
        for(auto i = neuron_threads.begin(); i != neuron_threads.end(); ++i)
        {
            LOGGER()<<"LAYER-"<<layer_id<<": "<<"Waiting for results!";
            try
            {
                i->second.get();
            }
            catch(std::exception &exc)
            {
                LOGGER()<<"LAYER-"<<layer_id<<": "<<"NEURON-"<<i->first<<" THREW AN ERROR: "<<"RNN TRAINING ERROR: " << exc.what();
                run_next = false;
            }
            catch(...)
            {
                LOGGER()<<"LAYER-"<<layer_id<<": "<<"NEURON-"<<i->first<<" THREW AN ERROR: "<<"Unknown RNN training error!";
                run_next = false;
            }
            //There needs top be more done with this but this is the lesat concerning at this point
        }
    }
    else
    {
        for(auto itr = neuron_ptrs.begin(); itr !=  neuron_ptrs.end(); ++itr)
        {
            try
            {
                LOGGER()<<"LAYER-"<<layer_id<<": "<<"NEURON-"<<(*itr)->get_neuron_id()<<" TRAINING NOW";
                (*itr)->train();
                LOGGER()<<"LAYER-"<<layer_id<<": "<<"NEURON-"<<(*itr)->get_neuron_id()<<" HAS BEEN TRAINED SUCCESSFULLY!";
            }
            catch(std::exception &exc)
            {
                LOGGER()<<"LAYER-"<<layer_id<<": "<<"NEURON-"<<(*itr)->get_neuron_id()<<" THREW AN ERROR: "<<"RNN TRAINING ERROR: " << exc.what();
                run_next = false;
            }
            catch(...)
            {LOGGER()<<"LAYER-"<<layer_id<<": "<<"NEURON-"<<(*itr)->get_neuron_id()<<" THREW AN ERROR: Unknown RNN training error!";
                run_next = false;
            }

        }
    }
    if(next_layer && run_next)
    {
        next_layer->train(is_async);
    }
}

template<class Archive>
void const layer::save(Archive & ar, const unsigned int version)
{
    ar << BOOST_SERIALIZATION_NVP(layer_id);
    ar << BOOST_SERIALIZATION_NVP(next_layer);
    ar << BOOST_SERIALIZATION_NVP(neuron_ptrs);
}

template<class Archive>
void layer::load(Archive & ar, const unsigned int version)
{
    ar >> layer_id;
    ar >> next_layer;
    ar >> neuron_ptrs;
}


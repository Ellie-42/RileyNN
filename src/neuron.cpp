//RileyNN 1.0.0 (0.7.0)

#include "neuron.h"


using namespace RileyNN;

neuron::neuron()
{
    this->uid = generate_uid(this);
    LOGGER()<<"NEURON-"<<uid<<": "<<"Connecting sockets!" ;
    neuron_out.reset(new rpc::pub(uid));
    neuron_in.reset(new rpc::sub());
    rpc::rmsg i_msg;
    i_msg << '\0';
    neuron_out->send(i_msg);
    LOGGER()<<"NEURON-"<<uid<<": "<<"N Out connected" ;
    initialized = false;
    networks_init = false;
}

neuron::neuron(id_set c_ids, id_set p_ids) : neuron()
{
    this->uid_pair = c_ids;
    this->pred_ids = p_ids;
    load();
}

neuron::~neuron()
{
    //dtor
}

std::string neuron::get_neuron_id()
{
    return uid;
}

std::pair<opennn::TrainingResults, opennn::TrainingResults> neuron::train()
{
    if(p_training.size() == 0 || c_training.size() == 0)
    {
        LOGGER()<<"VECTOR SIZE ERROR!";
        throw TRAINING_VECTOR_ERROR();
    }
    LOGGER()<<"NEURON-"<<uid<<": "<<"Training neuron: "<<uid ;
    opennn::DataSet c_ds = create_DataSet_from_vec(c_training);
    opennn::DataSet p_ds = create_DataSet_from_vec(p_training);
    const Index hidden_neurons_number = pred_ids.size();

    LOGGER()<<"NEURON-"<<uid<<": "<<"Got hidden neurons count.";

    c_ds.set_column_type(0, opennn::DataSet::ColumnType::Binary);
    c_ds.set_column_use(0, opennn::DataSet::VariableUse::Target);

    c_ds.set_column_type(1, opennn::DataSet::ColumnType::Categorical);
    c_ds.set_column_use(1, opennn::DataSet::VariableUse::Input);

    p_ds.set_column_type(0, opennn::DataSet::ColumnType::Categorical);
    p_ds.set_column_use(0, opennn::DataSet::VariableUse::Input);

    LOGGER()<<"NEURON-"<<uid<<": "<<"Providing definitions to the datasets now!";

    for(Index i = 2; i < c_ds.get_columns_number(); i++)
    {
        c_ds.set_column_type(i, opennn::DataSet::ColumnType::Numeric);
        c_ds.set_column_use(i, opennn::DataSet::VariableUse::Input);
    }

    LOGGER()<<"NEURON-"<<uid<<": "<<"C Data has been defined!";

    for(Index i = 1; i < c_ds.get_columns_number()-1; i++)
    {
        p_ds.set_column_type(i, opennn::DataSet::ColumnType::Numeric);
        p_ds.set_column_use(i, opennn::DataSet::VariableUse::Input);
    }

    LOGGER()<<"NEURON-"<<uid<<": "<<"P Data Pt 1 has been defined!";

    p_ds.set_column_type(c_ds.get_columns_number()-1, opennn::DataSet::ColumnType::Categorical);
    p_ds.set_column_use(c_ds.get_columns_number()-1, opennn::DataSet::VariableUse::Target);

    LOGGER()<<"NEURON-"<<uid<<": "<<"P Data Pt 2 has been defined!";

    for(Index i = c_ds.get_columns_number(); i < p_ds.get_columns_number() - 1; i++)
    {
        p_ds.set_column_type(i, opennn::DataSet::ColumnType::Numeric);
        p_ds.set_column_use(i, opennn::DataSet::VariableUse::Target);
    }

    LOGGER()<<"NEURON-"<<uid<<": "<<"P Data Pt 3 has been defined!";

    LOGGER()<<"NEURON-"<<uid<<": "<<"Defining architecture now";

    p_ds.print();
    c_ds.print();

    Tensor<Index, 1> p_architecture(3);
    p_architecture.setValues({p_ds.get_input_columns_number(), hidden_neurons_number, p_ds.get_target_columns_number()});
    Tensor<Index, 1> c_architecture(3);
    c_architecture.setValues({c_ds.get_input_columns_number(), hidden_neurons_number, c_ds.get_target_columns_number()});

    LOGGER()<<"NEURON-"<<uid<<": "<<"Check if network exists";

    if(!networks_init)
    {
        LOGGER()<<"NEURON-"<<uid<<": "<<"Creating new networks!";
        p_network.reset(new opennn::NeuralNetwork(opennn::NeuralNetwork::ProjectType::Approximation, p_architecture));
        LOGGER()<<"NEURON-"<<uid<<": "<<"And now the C Network";
        c_network.reset(new opennn::NeuralNetwork(opennn::NeuralNetwork::ProjectType::Classification, c_architecture));

        LOGGER()<<"NEURON-"<<uid<<": "<<"networks made";
        networks_init = true;
    }
    if(RDEBUG)
    {
        LOGGER()<<"NEURON-"<<uid<<": C DATA SET";
        c_ds.print_data();
        LOGGER()<<"NEURON-"<<uid<<": C DATA SET HAS NAN: " << c_ds.has_nan();
        LOGGER()<<"NEURON-"<<uid<<": P DATA SET";
        p_ds.print_data();
        LOGGER()<<"NEURON-"<<uid<<": P DATA SET HAS NAN: " << p_ds.has_nan();

    }
    LOGGER()<<"NEURON-"<<uid<<": "<<"Training." ;
    std::pair<opennn::TrainingResults, opennn::TrainingResults> ret_pair;
    auto c_thr = std::async(std::launch::async, &neuron::train_network, this, c_network, c_ds);
    auto p_thr = std::async(std::launch::async, &neuron::train_network, this, p_network, p_ds);
    LOGGER()<<"NEURON-"<<uid<<": "<<"Alright, now collect the data." ;
    ret_pair.first = c_thr.get();
    ret_pair.second = p_thr.get();
    LOGGER()<<"NEURON-"<<uid<<": "<<"And return it!!" ;
    return ret_pair;
}

bool neuron::run()
{
    std::vector<std::vector<RWORD> > c_input_vec;
    std::vector<std::vector<RWORD> > p_input_vec;

    bool result;
    std::string src_str;
    std::string pred_str;

    rpc::rmsg in_msg;
    LOGGER() << "NEURON " << uid << ": read msg stream.";
    while(neuron_in->rec(in_msg))
    {
        LOGGER() << "NEURON " << uid << ": MSG STREAM NOT EMPTY!";
        LOGGER() << "NEURON " << uid << ": MSG FROM UID: " << in_msg.get<std::string>(0);

        std::vector<RWORD> temp;
        int state = -1;
        for(size_t i = 1; i < in_msg.parts(); i++)
        {
            if(in_msg.get<RWORD>(i) == RileyNN::FRMBEG)
            {
                state = 0;
                LOGGER()<< "NEURON " << uid << ": STATE 0";
            }
            else if(in_msg.get<RWORD>(i) == RileyNN::SRCSIG)
            {
                state = 1;
                LOGGER()<< "NEURON " << uid << ": STATE 1";
                i++;
            }
            else if(in_msg.get<RWORD>(i) == RileyNN::PREDSIG)
            {
                state = 2;
                LOGGER()<< "NEURON " << uid << ": STATE 2";
            }
            else if(in_msg.get<RWORD>(i) == RileyNN::FRMEND)
            {
                state = 4;
                LOGGER()<< "NEURON " << uid << ": STATE 4";
            }
            switch(state)
            {
                case -1:
                    throw NEURON_MSG_ERROR();
                    break;
                case 1:
                    temp.push_back(in_msg.get<RWORD>(i));
                    LOGGER()<< "NEURON " << uid << ": ADDED VAL " << in_msg.get<RWORD>(i) << " TO TEMP VEC";
                    break;
                case 2:
                    if(temp.size() > 0)
                    {
                        c_input_vec.push_back(temp);
                        LOGGER()<< "NEURON " << uid << ": ADDED TEMP VEC TO C_INPUT_VEC";
                    }
                    temp = std::vector<RWORD>();
                    state = 3;
                    LOGGER()<< "NEURON " << uid << ": STATE 3";
                    break;
                case 3:
                    temp.push_back(in_msg.get<RWORD>(i));
                    LOGGER()<< "NEURON " << uid << ": ADDED VAL " << in_msg.get<RWORD>(i) << " TO TEMP VEC";
                    break;
                case 4:
                    if(temp.size() > 0)
                    {
                        p_input_vec.push_back(temp);
                        LOGGER()<< "NEURON " << uid << ": ADDED TEMP VEC TO P_INPUT_VEC";
                        temp = std::vector<RWORD>();
                    }
                    state = 0;
                    LOGGER()<< "NEURON " << uid << ": STATE 0";
                    break;
            }
        }
    }

    LOGGER()<<"NEURON " << uid << ": CREATING P_TRAINING VEC";

    normalize_vectors(c_input_vec);
    normalize_vectors(p_input_vec);
    auto tmp_vec = c_input_vec;
    {
        assert(tmp_vec.size() == p_input_vec.size());
        int i = 0;
        for(auto vec = tmp_vec.begin(); vec < tmp_vec.end(); vec++)
        {
            auto tc = p_input_vec.at(i);
            vec->insert(vec->end(),tc.begin(),tc.end());
            i++;
        }
    }

    LOGGER()<<"NEURON " << uid << ": ADDING TRAINING VEC";
    p_training.insert(p_training.end(), tmp_vec.begin(), tmp_vec.end());
    c_training.insert(c_training.end(), c_input_vec.begin(), c_input_vec.end());

    result = false;

    rpc::rmsg msg_out;

    if(initialized && c_input_vec.size() > 0 && networks_init)
    {
        auto temp_1 = std::async(std::launch::async, &neuron::run_c_network, this, tensor_from_vec(c_input_vec));
        auto temp_2 = std::async(std::launch::async, &neuron::run_p_network, this, tensor_from_vec(p_input_vec));

        Tensor< RWORD, 2 > c_out = temp_1.get();
        Tensor< RWORD, 2 > p_out = temp_2.get();

        Tensor<RWORD, 2>::Dimensions c_out_dims = c_out.dimensions();
        Tensor<RWORD, 2>::Dimensions p_out_dims = p_out.dimensions();

        result = calculate_result(MatrixCast(c_out, c_out_dims[0], c_out_dims[1]), c_out_dims[1]);

        auto pred_vec = matrix_to_vec(MatrixCast(p_out, p_out_dims[0], p_out_dims[1]), p_out_dims[0], p_out_dims[1]);

        for(size_t v = 0; v < c_input_vec.size(); v++)
        {
            if(v < pred_vec.size())
            {
                msg_out << uid;
                msg_out << RileyNN::FRMBEG;
                msg_out << RileyNN::SRCSIG;
                msg_out << (result ? RTRUE : RFALSE);
                for(auto & rwrd : c_input_vec.at(v))
                {
                    msg_out << rwrd;
                }
                msg_out << RileyNN::PREDSIG;
                for(auto & rwrd : pred_vec.at(v))
                {
                    msg_out << rwrd;
                }
                msg_out << RileyNN::FRMEND;
                neuron_out->send(msg_out);
            }
            else
            {
                v = c_input_vec.size();
            }
        }

        last_pred_res = pred_vec;
    }


    return result;
}

opennn::TrainingResults neuron::train_network(std::shared_ptr<opennn::NeuralNetwork> nn, opennn::DataSet data_set)
{
    LOGGER()<<"NEURON-"<<uid<<": "<<"TRAIN NOW" ;
    opennn::TrainingStrategy training_strategy(nn.get(), &data_set);
    LOGGER()<<"NEURON-"<<uid<<": "<<"Made training strategy for neuron: " << uid  ;
    training_strategy.set_loss_method(opennn::TrainingStrategy::LossMethod::NORMALIZED_SQUARED_ERROR);
    training_strategy.set_optimization_method(opennn::TrainingStrategy::OptimizationMethod::QUASI_NEWTON_METHOD);
    LOGGER()<<"NEURON-"<<uid<<": "<<"perform training now !" ;
    training_strategy.set_maximum_epochs_number(1000);
    training_strategy.set_maximum_selection_failures(500);

    return training_strategy.perform_training();
}

Tensor<RWORD, 2> neuron::run_c_network(Tensor<RWORD, 2> inputs)
{
    return c_network->calculate_outputs(inputs);
}

Tensor<RWORD, 2> neuron::run_p_network(Tensor<RWORD, 2> inputs)
{
    return p_network->calculate_outputs(inputs);
}

void neuron::load()
{
    for(auto str : pred_ids)
    {
        LOGGER() << "NEURON " << uid << ": REGISTERING ID: " << str;
        neuron_in->subscribe(str); //eventually once debugging is done this can be refactored to iterator format
    }
    neuron_in->connect();
    initialized = true;
}

void neuron::transmit_last_res()
{
    for(auto & vec : last_pred_res )
    {
        std::stringstream ss;
        ss << *reinterpret_cast<unsigned long long*>(&(*(vec.begin())));
        std::string CODE_OUT = ss.str();
        rpc::rmsg msg_out;
        msg_out << CODE_OUT;
        vec.erase(vec.begin());
        for(RWORD & rwrd : vec)
        {
            msg_out << rwrd;
        }
        neuron_out->send(msg_out);
    }
}


template<class Archive>
void const neuron::save(Archive & ar, const unsigned int version)
{
    ar << BOOST_SERIALIZATION_NVP(uid);
    ar << BOOST_SERIALIZATION_NVP(uid_pair);
    ar << BOOST_SERIALIZATION_NVP(pred_ids);
    ar << BOOST_SERIALIZATION_NVP(networks_init);
    FILE * pFile;
    pFile = std::tmpfile();
    FILE * cFile;
    cFile = std::tmpfile();
    tinyxml2::XMLPrinter c_xml_out(cFile);
    tinyxml2::XMLPrinter p_xml_out(pFile);
    p_network->write_XML(p_xml_out);
    c_network->write_XML(c_xml_out);
    std::string t1 = "";
    std::string t2 = "";
    char c;
    do
    {
        c = fgetc(cFile);
        t1 += c;

    }while(c != EOF);

    do
    {
        c = fgetc(pFile);
        t2 += c;

    }while(c != EOF);

    ar << BOOST_SERIALIZATION_NVP(t1);
    ar << BOOST_SERIALIZATION_NVP(t2);
}

template<class Archive>
void neuron::load(Archive & ar, const unsigned int version)
{
    LOGGER()<<"NEURON-"<<uid<<": "<<"NEURON IS LOADING" ;
    ar >> uid;
    ar >> uid_pair;
    ar >> pred_ids;
    ar >> networks_init;

    std::string t1;
    std::string t2;

    ar >> t1;
    ar >> t2;

    FILE * pFile;
    pFile = std::tmpfile();
    FILE * cFile;
    cFile = std::tmpfile();

    std::fputs(t1.c_str(), cFile);
    std::fputs(t2.c_str(), pFile);

    tinyxml2::XMLDocument c_xml_in(cFile);
    tinyxml2::XMLDocument p_xml_in(pFile);

    p_network->from_XML(p_xml_in);
    c_network->from_XML(c_xml_in);
    networks_init = true;
    load();
}

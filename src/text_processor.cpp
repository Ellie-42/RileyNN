//Riley 1.0.0 (0.7.0)
#include "text_processor.h"

using namespace RileyNN;

text_processor::text_processor() : io_base(encode_control_word("UTF-8"))
{
    RFEEDBACK = encode_control_word("!ADA!");
    std::stringstream ss;
    ss << *reinterpret_cast<unsigned long long*>(&RFEEDBACK);
    FEEDBACK = ss.str();
}

text_processor::~text_processor()
{
    //dtor
}

void text_processor::send(std::string input)
{
    inp_str = input;
    if(inp_str == "STOP")
    {
        LOGGER()<<"TP-"<<uid<<": "<<"Stopping TP";
        stop();
    }
    this->cv.notify_all();
}

void text_processor::in_loop()
{
    rpc::rmsg msg_in;
    while(io_in->rec(msg_in,true) && can_run.load() == true)
    {
        std::string control_word;
        std::vector<RWORD> in_data;

        msg_in >> control_word;
        for(size_t parts = 1; parts < msg_in.parts(); parts++)
        {
            in_data.push_back(msg_in.get<RWORD>(parts));
        }

        rpc::rmsg msg_out;

        msg_out << uid;
        msg_out << RileyNN::FRMBEG;
        msg_out << RileyNN::SRCSIG;
        msg_out << RTRUE;
        msg_out << RFEEDBACK;
        for(auto &val : in_data)
        {
            msg_out << val;
        }
        msg_out << RileyNN::PREDSIG;
        msg_out << RCONTROL;
        for(auto &val : in_data)
        {
            msg_out << val;
        }
        msg_out << RileyNN::FRMEND;

        io_out->send(msg_out);

        LOGGER()<<"TP-"<<uid<<": "<<"Data has been processed!\n"<<decode(in_data);
    }
}

std::string text_processor::get_feedback_str()
{
    return FEEDBACK;
}

void text_processor::out_loop()
{
    std::unique_lock<std::mutex> lck(this->mtx);
    this->cv.wait(lck);
    if(this->can_run.load() == false)
    {
        return;
    }
    rpc::rmsg msg_out;
    msg_out << uid;
    msg_out << RileyNN::FRMBEG;
    msg_out << RileyNN::SRCSIG;
    msg_out << RTRUE;
    msg_out << RCONTROL;
    auto tmp = encode(inp_str);
    for(auto &val : tmp)
    {
        msg_out << val;
    }
    msg_out << RileyNN::PREDSIG;
    msg_out << RCONTROL;
    for(auto &val : tmp)
    {
        msg_out << val;
    }
    msg_out << RileyNN::FRMEND;
    io_out->send(msg_out);
    LOGGER()<<"TP-"<<uid<<": "<<"Message sent!";
}

template<class Archive>
void const text_processor::save(Archive & ar, const unsigned int version)
{
    ar << BOOST_SERIALIZATION_NVP(uid);
    ar << BOOST_SERIALIZATION_NVP(RCONTROL);
    ar << BOOST_SERIALIZATION_NVP(FEEDBACK);
}

template<class Archive>
void text_processor::load(Archive & ar, const unsigned int version)
{
    ar >> uid;
    ar >> RCONTROL;
    ar >> FEEDBACK;
}

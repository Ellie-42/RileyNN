#include "file_processor.h"

using namespace RileyNN;

file_processor::file_processor() : io_base(encode_control_word("FILE!"))
{

}

file_processor::~file_processor()
{
    //dtor
}

void file_processor::open(std::string file)
{
    file_stream = std::fstream(file, std::fstream::in);
    is_open = file_stream.good();
}


bool file_processor::send_block()
{
    if(!is_open)
        return false;

    inp_str = "";
    inp_str.reserve(127*8);
    file_stream.read(&inp_str[0], 127*8);
    inp_str.shrink_to_fit();
    is_open = file_stream.is_open();
    this->cv.notify_all();
    return is_open;
}

bool file_processor::send_line()
{
    std::unique_lock<std::mutex> lck(this->mtx);
    if(!is_open)
        return false;

    std::getline(file_stream,inp_str);
    file_stream.ignore(inp_str.size());
    is_open = file_stream.good();
    this->cv.notify_all();
    this->cv.wait(lck);
    return is_open;
}

void file_processor::in_loop()
{
    rpc::rmsg msg_in;

    while(io_in->rec(msg_in,true) && can_run.load() == true)
    {
        std::string control_word;
        std::vector<RWORD> in_data;

        msg_in >> control_word;
        for(size_t i = 0; i < msg_in.parts(); i++)
        {
            in_data.push_back(msg_in.get<RWORD>(i));
        }

        LOGGER()<<"FP-"<<uid<<": "<<"Data has been processed!\n"<<decode(in_data);

    }
}


void file_processor::out_loop()
{
    std::unique_lock<std::mutex> lck(this->mtx);
    this->cv.wait(lck);
    if(this->can_run.load() == false)
    {
        this->cv.notify_all();
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
    this->cv.notify_all();
    LOGGER()<<"FP-"<<uid<<": "<<"Message sent!";
}



template<class Archive>
void const file_processor::save(Archive & ar, const unsigned int version)
{
    ar << BOOST_SERIALIZATION_NVP(uid);
    ar << BOOST_SERIALIZATION_NVP(RCONTROL);
}

template<class Archive>
void file_processor::load(Archive & ar, const unsigned int version)
{
    ar >> uid;
    ar >> RCONTROL;
}


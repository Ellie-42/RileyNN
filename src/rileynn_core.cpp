//RileyNN 1.0.0 (0.7.0)
#include "rileynn_core.h"

std::mutex LOGGER::mutex_;

std::string RileyNN::generate_uid(void * buf)
{
    int i = 0;
    uint64_t t_addr = (uint64_t)((uintptr_t) buf);
    uint64_t t_addr_2 = (uint64_t)((uintptr_t) &i);
    std::stringstream uid_stream;
    uid_stream << t_addr;
    std::string s1 = uid_stream.str();
    int timestamp = std::chrono::milliseconds(std::time(NULL)).count();
    t_addr_2 = (t_addr_2 | timestamp);
    uid_stream << t_addr_2;
    std::string s2 = uid_stream.str();
    std::hash<std::string> str_hash;
    unsigned long ui1 = (str_hash(s1) ^ (str_hash(s2) << 1));
    return std::to_string(ui1);
}

std::vector<std::vector<RileyNN::RWORD> > RileyNN::vectorize_csv_str(std::string str)
{
    std::vector<std::vector<RileyNN::RWORD> > ret_vec;
    std::vector<RileyNN::RWORD> temp_vec;
    std::string temp_str;
    for(char c : str)
    {
        if(c == ',')
        {
            uint64_t temp_1 = std::stoull(temp_str);
            RWORD TEMP = convert(temp_1);
            temp_vec.push_back(TEMP);
            temp_str = "";
        }
        else if(c == '&')
        {
            if(temp_str.size() > 0)
            {
                temp_vec.push_back(convert(static_cast<uint64_t>(std::stoll(temp_str))));
                ret_vec.push_back(temp_vec);
                temp_str = "";
            }
            ret_vec.push_back(temp_vec);
            temp_vec = std::vector<RileyNN::RWORD>();

        }
        else
        {
            temp_str += c;
        }
    }
    if(temp_str.size() > 0)
    {
        temp_vec.push_back(convert(static_cast<uint64_t>(std::stoll(temp_str))));
    }
    if(temp_vec.size() > 0)
    {
        ret_vec.push_back(temp_vec);
    }
    return ret_vec;
}

std::string RileyNN::vec_to_csv(std::vector<std::vector<RileyNN::RWORD> > vec )
{
    std::stringstream data;
    for(auto itr = vec.begin(); itr != vec.end(); ++itr)
    {
        for(auto it = vec.begin(); it != vec.end(); ++it)
        {
            data << *reinterpret_cast<unsigned long long*>(&(*it));
            data << ",";
        }
        data << "&";
    }
    return data.str();
}

Tensor<RileyNN::RWORD, 2> RileyNN::tensor_from_vec(std::vector<std::vector<RileyNN::RWORD> > data)
{
    Eigen::MatrixXd mat(data.size(), data.at(0).size());
    int i = 0;
    for(auto itr = data.begin(); itr != data.end(); ++itr)
    {
        mat.row(i) = Eigen::Map<Eigen::VectorXd>(itr->data(), itr->size());
        i++;
    }
    std::array<size_t, 2> dims;
    dims[0] = data.size();
    dims[1] = data.at(0).size();
    return TensorCast(mat, dims);
}

opennn::DataSet RileyNN::create_DataSet_from_vec(std::vector<std::vector<RileyNN::RWORD> > vec)
{
    normalize_vectors(vec);
    auto tens = tensor_from_vec(vec);
    return opennn::DataSet(tens);
}

bool RileyNN::calculate_result(Eigen::Matrix<RileyNN::RWORD,Eigen::Dynamic, Eigen::Dynamic> matr, unsigned int rows)
{
    uint64_t res = 0;
    for(unsigned int r = 0; r < rows; r++)
    {
        res = (*reinterpret_cast<unsigned long long*>(&(matr(0,r))) ^ std::numeric_limits<int64_t>::min()) + res;
    }
    double d = res/rows;
    if(d > 0.5)
    {
        return true;
    }
    else
    {
        return false;
    }
}

std::vector<std::vector<RileyNN::RWORD> > RileyNN::matrix_to_vec(Eigen::Matrix<RileyNN::RWORD,Eigen::Dynamic, Eigen::Dynamic> matr, unsigned int cols, unsigned int rows)
{
    std::vector<std::vector<RileyNN::RWORD> > data;
    for(unsigned int r = 0; r < rows; r++)
    {
        std::vector<RileyNN::RWORD> temp_vec;
        for(unsigned int c = 0; c < cols; c++)
        {
            temp_vec.push_back(matr(c,r));
        }
        data.push_back(temp_vec);
    }
    return data;
}

std::vector<RileyNN::RWORD> RileyNN::strip_and_collapse(std::vector<std::vector<RileyNN::RWORD> > src_vec)
{
    std::vector<RileyNN::RWORD> ret_vec;
    for(auto i = src_vec.begin(); i != src_vec.end(); ++i)
    {
        for(auto j = *i->begin() + 1; j != *i->end(); ++j)
        {
            if(j != 0)
            {
                ret_vec.push_back(j);
            }
        }
    }
    return ret_vec;
}

double RileyNN::convert(std::bitset<64> const& bs)
{
    static_assert(sizeof(uint64_t) == sizeof(double), "Cannot use this!");

    uint64_t const u = bs.to_ullong();
    double d;

    char const* cu = reinterpret_cast<char const*>(&u);
    char* cd = reinterpret_cast<char*>(&d);

    memcpy(cd, cu, sizeof(u));

    return d;
}

double RileyNN::convert(uint64_t const& u)
{
    static_assert(sizeof(uint64_t) == sizeof(double), "Cannot use this!");

    double d = 0;

    // Aliases to `char*` are explicitly allowed in the Standard (and only them)
    char const * cu = reinterpret_cast<char const *>(&u);
    char* cd = reinterpret_cast<char*>(&d);

    // Copy the bitwise representation from u to d
    memcpy(cd, cu, sizeof(u));
    return d;
}


std::string RileyNN::decode(std::vector<RileyNN::RWORD> vec)
{
    std::string ret_str;
    for(auto code = vec.begin(); code != vec.end(); ++code)
    {
        std::bitset<64> temp_bitset(*reinterpret_cast<unsigned long long*>(&(*code)));
        int counter = 0;
        for(int i = 0; i < 8; i++)
        {
            bitset<8> temp;
            temp[0] = temp_bitset[(counter*8)];
            temp[1] = temp_bitset[(counter*8) + 1];
            temp[2] = temp_bitset[(counter*8) + 2];
            temp[3] = temp_bitset[(counter*8) + 3];
            temp[4] = temp_bitset[(counter*8) + 4];
            temp[5] = temp_bitset[(counter*8) + 5];
            temp[6] = temp_bitset[(counter*8) + 6];
            temp[7] = temp_bitset[(counter*8) + 7];
            if(temp.to_ullong() != 0)
            {
                ret_str += static_cast<char>(temp.to_ullong());
            }
            counter++;
        }
    }
    return ret_str;
}

std::vector<RileyNN::RWORD> RileyNN::encode(std::string data)
{
    std::string newData;
    std::vector<RileyNN::RWORD> ret_vec;
    int counter = 0;
    std::bitset<64> temp_bitset;
    for(std::size_t i = 0; i < data.size(); i++)
    {
        bitset<8> temp(data.at(i));
        temp_bitset[(counter*8)] = temp[0];
        temp_bitset[(counter*8) + 1] = temp[1];
        temp_bitset[(counter*8) + 2] = temp[2];
        temp_bitset[(counter*8) + 3] = temp[3];
        temp_bitset[(counter*8) + 4] = temp[4];
        temp_bitset[(counter*8) + 5] = temp[5];
        temp_bitset[(counter*8) + 6] = temp[6];
        temp_bitset[(counter*8) + 7] = temp[7];
        if(counter >= 7 || (i+1) >= data.size())
        {
            ret_vec.push_back(RileyNN::convert(temp_bitset));
            temp_bitset.reset();
            counter = 0;
        }
        counter++;
    }
    return ret_vec;
}

std::string RileyNN::construct_data_string(RileyNN::RWORD data_word, std::vector<RileyNN::RWORD> encoded_data)
{
    std::stringstream data;
    data << *reinterpret_cast<unsigned long long*>(&data_word) << ",";
    int counter = 0;
    for(auto itr = encoded_data.begin(); itr != encoded_data.end(); ++itr)
    {
        if(counter >= 126)
        {
            data << "&";
            data << *reinterpret_cast<unsigned long long*>(&data_word);
            data << ",";
            counter = 0;
        }
        data << *reinterpret_cast<unsigned long long*>(&(*itr)) << ",";
        counter++;
    }
    return data.str();
}

void RileyNN::normalize_vectors(std::vector<std::vector<RileyNN::RWORD> > &vec)
{
    size_t l = 0;
    for(auto itr = vec.begin(); itr != vec.end(); ++itr)
    {
        if(l < itr->size())
        {
            l = itr->size();
            itr = vec.begin();
        }
        while(itr->size() < l)
        {
            itr->push_back(0);
        }
    }
}

RileyNN::RWORD RileyNN::encode_control_word(std::string data) //returns the total size of the data, and the individual numbers. Throws an error if the submitted code is longer than four bytes
{
    assert(data.size() == 5);
    std::string newData;
    for(std::size_t i = 0; i < data.size(); i++)
    {
        newData += std::bitset<8>(data.c_str()[i]).to_string<char,std::string::traits_type,std::string::allocator_type>();
    }
    std::reverse(newData.begin(), newData.end());

    return convert(static_cast<uint64_t>((std::stoll(newData,nullptr, 2) |  std::numeric_limits<int64_t>::min())));
}

//RileyNN::RWORD RileyNN::STOPSIG = RileyNN:://encode_control_word("RSTOP");
//RileyNN::RWORD RileyNN::TRAINSIG = RileyNN:://encode_control_word("TRAIN");
//RileyNN::RWORD RileyNN::RUNSIG = RileyNN:://encode_control_word("R_RUN");
//RileyNN::RWORD RileyNN::SAVESIG = RileyNN:://encode_control_word("RSAVE");
//RileyNN::RWORD RileyNN::FRMBEG = RileyNN:://encode_control_word("F_BEG");
//RileyNN::RWORD RileyNN::FRMEND = RileyNN:://encode_control_word("F_END");
//RileyNN::RWORD RileyNN::SRCSIG = RileyNN:://encode_control_word("_SRC_");
//RileyNN::RWORD RileyNN::PREDSIG = RileyNN:://encode_control_word("PRDC_");

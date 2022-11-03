//RileyNN 1.0.0 (0.7.0)
#include "id_set.h"

using namespace RileyNN;

id_set::id_set()
{
}

id_set::~id_set()
{
}

id_set::id_set(const id_set& other)
{
    this->uid_list = other.uid_list;
}

bool id_set::push_back(std::string str)
{
    if(!(this->find(str)))
    {
        this->uid_list.push_back(str);
        return true;
    }
    return false;
}

bool id_set::find(std::string str)
{
    for(auto itr = this->uid_list.begin(); itr != this->uid_list.end(); ++itr)
    {
        if(str == *itr)
        {
            return true;
        }
    }
    return false;
}

size_t id_set::size()
{
    return uid_list.size();
}

std::vector<std::string>::iterator id_set::begin()
{
    return this->uid_list.begin();
}

std::vector<std::string>::iterator id_set::end()
{
    return this->uid_list.end();
}

std::vector<std::string>::iterator id_set::erase(std::vector<std::string>::iterator itr)
{
    return this->uid_list.erase(itr);
}

bool id_set::operator==(id_set & rhs)
{
    for(std::vector<std::string>::iterator rit = rhs.begin(); rit != rhs.end(); ++rit)
    {
        if(!find(*rit))
            return false;
    }
    return true;
}

template<class Archive>
void id_set::serialize(Archive & ar, const unsigned int version)
{
    ar & BOOST_SERIALIZATION_NVP(this->uid_list);
}

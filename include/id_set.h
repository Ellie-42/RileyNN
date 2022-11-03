#ifndef RILEYNN_ID_SET_H
#define RILEYNN_ID_SET_H
//STL Headers
#include <vector>
#include <string>

//Boost Headers
#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>

namespace RileyNN
{
    class id_set
    {
        public:
            id_set();
            ~id_set();
            id_set(const id_set&);

            bool push_back(std::string);
            bool find(std::string);

            std::vector<std::string>::iterator begin();
            std::vector<std::string>::iterator end();
            std::vector<std::string>::iterator erase(std::vector<std::string>::iterator);

            size_t size();

            bool operator==(id_set&);


        private:
            std::vector<std::string> uid_list;

            friend class boost::serialization::access;
            template<class Archive>
            void serialize(Archive&, const unsigned int);

    };
}

#endif // RILEYNN_ID_SET_H

#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>
#include <string>

using namespace eosio;



class random : public eosio::contract
{
    public :
        using eosio::contract::contract;

//********************************
    char char_to_symbol( char c ) {
      if( c >= 'a' && c <= 'z' )
         return (c - 'a') + 6;
      if( c >= '1' && c <= '5' )
         return (c - '1') + 1;
      return 0;
   }
   uint64_t string_to_name( const char* str ) {
      uint32_t len = 0;
      while( str[len] ) ++len;
      uint64_t value = 0;
      for( uint32_t i = 0; i <= 12; ++i ) {
         uint64_t c = 0;
         if( i < len && i <= 12 ) c = uint64_t(char_to_symbol( str[i] ));
         if( i < 12 ) {
            c &= 0x1f;
            c <<= 64-5*(i+1);
         }
         else {
            c &= 0x0f;
         }
         value |= c;
      }
      return value;
   }
   #define N(X) ::eosio::string_to_name(#X)

//********************************

#define PRODUCER_NUM 3
std::vector<account_name> producers = {N(useraaaaaaaa), N(useraaaaaaab), N(useraaaaaaac)};


        struct hash_info
        {
            account_name producer;
            std::string hash = "";
            std::string value = "";
            uint8_t flag = 0;
        };

        // @abi table randoms i64
        struct random_info
        {
            uint64_t term;
            std::vector<hash_info> producers_hash;
            uint64_t hash_count = 0;
            uint64_t value_count = 0;
            
            uint64_t primary_key() const { return term; }
            EOSLIB_SERIALIZE( random_info, (term)(producers_hash)(hash_count)(value_count) )
        };

    typedef eosio::multi_index<N(randoms), random_info> random_table;

    bool is_producer(account_name producer) {
        for(auto i = 0; i < producers.size(); ++i) {
            if(producer == producers.at(i))
                return true;
        }
        return false;
    }


    // @abi action
    void pushhash(account_name producer, const uint64_t term, const std::string hash)
    {
        require_auth(producer); //TODO:认证是否在生产者列表内
        hash_info cur_hash_info;
        if(!is_producer(producer)) {
            eosio::print("you are not producer");
            return;
        }
        cur_hash_info.producer = producer;
        cur_hash_info.hash = hash;
        cur_hash_info.flag = 1;

        random_table randoms(_self, term);
        auto random_term = randoms.find(term);  //找到该轮次

        if(random_term == randoms.end()) {
            
            std::vector<hash_info> producers_hash;
            producers_hash.push_back(cur_hash_info);

            randoms.emplace(producer, [&](auto &new_term) {
                new_term.term = term;
                new_term.hash_count = 1;
                new_term.producers_hash = producers_hash;
            });
        } else {
            if(random_term->hash_count == PRODUCER_NUM) { //判断是否是多余提交，加入检测是否是bp的功能后不再需要
                eosio::print("need not more hash");
                return;
            }
            std::vector<hash_info> producers_hash = random_term->producers_hash;
            //for( const auto& ph : producers_hash) {
            for(size_t i = 0; i < producers_hash.size(); ++i) {
                hash_info& ph = producers_hash.at(i);
                if(ph.producer == producer) {
                    eosio::print("you have push hash ", ph.hash, ", can not change");
                    return;
                }
            }
            producers_hash.push_back(cur_hash_info);
            randoms.modify(random_term, producer, [&](auto &new_term) { //这里的payer不能是0,不然会有错3090004
                
                new_term.hash_count += 1;
                new_term.producers_hash = producers_hash;
            });
        }
        eosio::print("pushhash #", term, " success" );
    }

    bool check_value(std::string value, std::string hash) {
        if(HASH_STRING_PIECE(value) == hash)
            return true;
        return false;
    }


    // @abi action
    void pushvalue(account_name producer, const uint64_t term, const std::string value)
    {
        require_auth(producer); //TODO:认证是否在生产者列表内
        random_table randoms(_self, term);
        auto random_term = randoms.find(term);  //找到该轮次
        if(random_term == randoms.end()) {
            eosio::print("term #", term, " not start yet");
            return;
        } else {
            if(random_term->hash_count < PRODUCER_NUM) {
                eosio::print("there are some hash have not pushed, please wait");
                return;
            }

            std::vector<hash_info> producers_hash = random_term->producers_hash;
            //for( auto& ph : producers_hash) {
            for(size_t i = 0; i < producers_hash.size(); ++i) {
                bool valid_value = false;

                hash_info& ph = producers_hash.at(i);
                if(ph.producer == producer) {

                    switch( ph.flag ) {
                        case 0 : { // 未提交hash
                            eosio::print("you must push hash before push value");
                            return;
                        }
                        break;
                        case 1 : { //提交了hash, 但还没有提交value
                            if( check_value(value, ph.hash) ) {
                                ph.value = value;
                                ph.flag = 2;
                                valid_value = true;
                                eosio::print("success set value ", ph.value);
                            } else {
                                eosio::print("please push right value");
                            }
                        }
                        break;
                        case 2 : { //已提交了value
                            eosio::print("you have push value ", ph.value, ", can not change");
                            return;
                        }
                        break;
                        default :
                            eosio::print("unknow status");
                            return;
                        break;
                    }

                    randoms.modify(random_term, producer, [&](auto &new_term) {
                        new_term.producers_hash = producers_hash;
                        new_term.value_count += (valid_value ? 1 : 0);
                    });

                    return;
                }
            }
            eosio::print("you have to push term #", term, " hash before push value");
            return;
        }
    }

std::string HASH_STRING_PIECE(std::string string_piece) {
    int64_t result = 0;
    for (auto it = string_piece.cbegin(); it != string_piece.cend(); ++it) {  
        result = (result * 131) + *it;
    }                                                                         
    return std::to_string(result);
}

    // @abi action
    void testhash(const std::string value)
    {
        std::string  n = HASH_STRING_PIECE(value);
        eosio:print("result: ", n);
        return;
    }
};

EOSIO_ABI(random, (pushhash)(pushvalue)(testhash))
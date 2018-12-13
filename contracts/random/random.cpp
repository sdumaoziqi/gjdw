#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>

using namespace eosio;

class random : public eosio::contract
{
    public :
        using eosio::contract::contract;

        struct hash_info
        {
            account_name producer;
            uint64_t hash;
            uint64_t value = 0;
            uint8_t flag = 0;
        };

        // @abi table randoms i64
        struct random_info
        {
            uint64_t term;
            std::vector<hash_info> producers_hash;
            uint64_t hash_count = 0;
            
            uint64_t primary_key() const { return term; }
            EOSLIB_SERIALIZE( random_info, (term)(producers_hash)(hash_count) )
        };

    typedef eosio::multi_index<N(randoms), random_info> random_table;

    // @abi action
    void pushhash(account_name producer, const uint64_t term, const uint64_t hash)
    {
        require_auth(producer); //TODO:认证是否在生产者列表内
        hash_info cur_hash_info;
        cur_hash_info.producer = producer;
        cur_hash_info.hash = hash;
        cur_hash_info.flag = 1;

        random_table randoms(_self, _self);
        auto random_term = randoms.find(term);  //找到该轮次

        if(random_term == randoms.end()) {
            randoms.emplace(producer, [&](auto &new_term) {
                new_term.term = term;
                new_term.hash_count = 1;
                std::vector<hash_info> producers_hash;
                producers_hash.push_back(cur_hash_info);
                new_term.producers_hash = producers_hash;
            });
        } else {
            std::vector<hash_info> producers_hash = random_term->producers_hash;
            for( const auto& ph : producers_hash) {
                if(ph.producer == producer) {
                    eosio::print("you have push hash ", ph.hash, ", can not push change");
                    return;
                }
            }
            producers_hash.push_back(cur_hash_info);
            randoms.modify(random_term, 0, [&](auto &new_term) {
                
                new_term.hash_count += 1;
                new_term.producers_hash = producers_hash;
            });
        }
        eosio::print("pushhash#", term, " success" );
    }


};

EOSIO_ABI(random, (pushhash))
#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>

using namespace eosio;

#define PRODUCER_NUM 3

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
        //require_auth(producer); //TODO:认证是否在生产者列表内
        hash_info cur_hash_info;
        cur_hash_info.producer = producer;
        cur_hash_info.hash = hash;
        cur_hash_info.flag = 1;

        random_table randoms(_self, _self);
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
                hash_info& ph = producers_hash[i];
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

    bool check_value(uint64_t value, uint64_t hash) {
        if(value == hash + 1)
            return true;
        return false;
    }


    // @abi action
    void pushvalue(account_name producer, const uint64_t term, const uint64_t value)
    {
        //require_auth(producer); //TODO:认证是否在生产者列表内
        random_table randoms(_self, _self);
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
                hash_info& ph = producers_hash[i];
                if(ph.producer == producer) {
                    if(ph.flag == 2) {  //已提交了value
                        eosio::print("you have push value ", ph.value, ", can not change");
                    } else if(ph.flag == 1) {  //提交了hash, 但还没有提交value
                        if( check_value(value, ph.hash) ) {
                            ph.value = value;
                            ph.flag = 2;
                            eosio::print("success set value ", ph.value);
                        } else {
                            eosio::print("please push right value");
                        }
                    } else if(ph.flag == 0) {  // 未提交hash
                        eosio::print("you must push hash before push value");
                    } else {
                        eosio::print("unknow status");
                    }

                    randoms.modify(random_term, producer, [&](auto &new_term) {
                        new_term.producers_hash = producers_hash;
                    });

                    return;
                }
            }
            eosio::print("you have to push term #", term, " hash before push value");
            return;
        }
    }


};

EOSIO_ABI(random, (pushhash)(pushvalue))
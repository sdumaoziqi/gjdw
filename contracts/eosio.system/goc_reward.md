# GOC 节点投票奖励

### 介绍

现在一个账号可以有三种奖励

* 抵押GOC为net cpu 资源，**并且有对bp节点投票**
* 锁定GOC为net cpu 资源，相对上一种方式可以额外获取一定奖励
* 成为gn节点，对提案投票

后续分别叫做`活期奖励`  `定期奖励`  `提案奖励`

前两种奖励保存在`goc_vote_reward_info` 第三种奖励保存在`goc_reward_info`

```
   struct goc_vote_reward_info {
      uint64_t      reward_id = 0; //id==0 表示第一种奖励。id!=0 是第二种
      time          reward_time;
      int64_t       rewards = 0; // 奖励值，为了避免数额太小归0，乘了系数1'000'000'000

      uint64_t primary_key() const { return reward_id;}

      EOSLIB_SERIALIZE( goc_vote_reward_info, (reward_id)(reward_time)(rewards) ) 
   };

   struct goc_reward_info { //对应第三种奖励
      time          reward_time;
      uint64_t      proposal_id; //投票的提案id
      eosio::asset  rewards = asset(0);
      time          settle_time = 0;

      uint64_t  primary_key()const { return proposal_id; }

      EOSLIB_SERIALIZE( goc_reward_info, (reward_time)(proposal_id)(rewards)(settle_time) )
   };
   
   typedef eosio::multi_index< N(gocrewards), goc_reward_info>      goc_rewards_table;
   typedef eosio::multi_index< N(gocvrewards), goc_vote_reward_info>      goc_vote_rewards_table;
   
   goc_vote_rewards_table vrewards(_self, account_name); // code("gocio") scope(account_name) table("gocvrewards")
   goc_rewards_table rewards(_self, account_name); // code("gocio") scope(account_name) table("gocrewards")
   
   
```
-------------------------

### 计算奖励

`活期奖励`参考[文档](https://shimo.im/docs/Rco86YV7X8oQwhJp/read)

#### 活期奖励

计算的触发时机是`delegatebw` `undelegatebw` `calcvrewards`

```
         void delegatebw( account_name from, account_name receiver,
                          asset stake_net_quantity, asset stake_cpu_quantity, bool transfer ); // transfer为true，表示将抵押资源的投票权也转让给receiver，触发计算的账号是receiver;false是from
                          
         void undelegatebw( account_name from, account_name receiver,
                            asset unstake_net_quantity, asset unstake_cpu_quantity ); // 触发from的计算
              
         void calcvrewards( account_name owner); // 账号主动触发计算
```

两次触发之间应有的奖励计算算法参考[文档](https://shimo.im/docs/Rco86YV7X8oQwhJp/read)

```

// eosio_global_state.cur_point指向可以插入下一个快照的id。每次计算的最大长度由eosio_global_state中的max_record设置，当前为365。
   struct goc_vote_reward_info {
      uint64_t      reward_id = 0;
      time          reward_time;
      int64_t       rewards = 0;

      uint64_t primary_key() const { return reward_id;}

      EOSLIB_SERIALIZE( goc_vote_reward_info, (reward_id)(reward_time)(rewards) ) 
   };

   typedef eosio::multi_index<N(perrewards), goc_per_reward_info>  per_rewards_table;

   per_rewards_table      _perrewards(_self, _self); // code("gocio") scope("gocio") table("perrewards")
```

`goc_per_reward_info`每一次插入元素由`claimrewards()`触发，每次插入间隔至少为一天。累加claim_time大于上次计算时间且小于当前时间的per_reward，累加值乘上抵押资源就是奖励值。

####  定期奖励



####  提案奖励






























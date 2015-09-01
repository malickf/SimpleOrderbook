/*
Copyright (C) 2015 Jonathon Ogden     < jeog.dev@gmail.com >

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see http://www.gnu.org/licenses.
*/

#ifndef JO_0815_MARKET_MAKER
#define JO_0815_MARKET_MAKER

#include "types.hpp"
#include <random>
#include <vector>
#include <functional>
#include <memory>
#include <map>
#include <mutex>


namespace NativeLayer{

namespace SimpleOrderbook{
class LimitInterface;
}

class MarketMaker;
typedef std::unique_ptr<MarketMaker> pMarketMaker;
typedef std::vector<pMarketMaker> market_makers_type;

market_makers_type operator+(market_makers_type& l, market_makers_type& r);


class MarketMaker{
  SimpleOrderbook::LimitInterface *_book;
  callback_type _callback;
  bool _is_running;

  void _base_callback(callback_msg msg,id_type id, price_type price,
                      size_type size);
  std::mutex _my_lock;

protected:
  typedef MarketMaker my_base_type;
  typedef std::tuple<bool,price_type,size_type> order_bndl_type;
  typedef std::map<id_type,order_bndl_type> orders_map_type;
  typedef orders_map_type::value_type orders_value_type;

  price_type tick;
  orders_map_type my_orders;

  bool last_fill_was_buy;
  price_type last_fill_price;
  size_type last_fill_size;
  id_type last_fill_id;

  /* this still allows a situation for pos > max */
  size_type bid_out;
  size_type offer_out;
  long long pos;

public:
  MarketMaker(callback_type callback);
  MarketMaker();
  // how to handle copy constr ??
  virtual ~MarketMaker()
    {
    };

  /* derived need to call down to start / stop */
  virtual void start(SimpleOrderbook::LimitInterface *book, price_type implied,
                     price_type tick);
  virtual void stop();

  template<bool BuyNotSell>
  void insert(price_type price, size_type size);

  static market_makers_type Factory(std::initializer_list<callback_type> il);
  static market_makers_type Factory(unsigned int n);
};


class MarketMaker_Simple1
    : public MarketMaker{

  size_type _sz, _max_pos;
  void _callback(callback_msg msg, id_type id, price_type price, size_type size);

public:
  MarketMaker_Simple1(size_type sz, size_type max_pos);
  virtual void start(SimpleOrderbook::LimitInterface *book, price_type implied,
                     price_type tick);

  static market_makers_type
  Factory(std::initializer_list<std::pair<size_type,size_type>> il);
  static market_makers_type
  Factory(unsigned int n, size_type sz, size_type max_pos);
};


class MarketMaker_Random
    : public MarketMaker{

  size_type _sz_low, _sz_high, _max_pos;
  std::default_random_engine _rand_engine;
  std::uniform_int_distribution<size_type> _distr, _distr2;
  void _callback(callback_msg msg, id_type id, price_type price, size_type size);
  unsigned long long _gen_seed();

public:
  MarketMaker_Random(size_type sz_low, size_type sz_high, size_type max_pos);
  MarketMaker_Random(const MarketMaker_Random& mm);
  virtual void start(SimpleOrderbook::LimitInterface *book, price_type implied,
                  price_type tick);

  static market_makers_type
  Factory(std::initializer_list<std::tuple<size_type,size_type,size_type>> il);
  static market_makers_type
  Factory(unsigned int n, size_type sz_low, size_type sz_high, size_type max_pos);

private:
  static const clock_type::time_point seedtp;
};



};
#endif

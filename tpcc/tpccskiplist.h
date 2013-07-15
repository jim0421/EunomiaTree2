// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.



#include "db/dbtx.h"

#include "db/memstore_skiplist.h"


#include "tpccdb.h"


namespace leveldb {

typedef int64_t Key;
typedef uint64_t Value;


class TPCCSkiplist : public TPCCDB {
 public :
  
  MemStoreSkipList* store;

  uint64_t abort;
  uint64_t conflict;
  uint64_t capacity;

  TPCCSkiplist();
  ~TPCCSkiplist();
  
  void insertWarehouse(const Warehouse & warehouse);
  void insertItem(const Item& item);
  void insertStock(const Stock & stock);
  void insertDistrict(const District & district);
  void insertCustomer(const Customer & customer);
  Order* insertOrder(const Order & order);
  NewOrder* insertNewOrder(int32_t w_id,int32_t d_id,int32_t o_id);
  History* insertHistory(const History & history);
  OrderLine* insertOrderLine(const OrderLine & orderline);
  
	
  virtual bool newOrder(int32_t warehouse_id, int32_t district_id, int32_t customer_id,
			  const std::vector<NewOrderItem>& items, const char* now,
			  NewOrderOutput* output, TPCCUndo** undo);
  virtual bool newOrderHome(int32_t warehouse_id, int32_t district_id, int32_t customer_id,
			  const std::vector<NewOrderItem>& items, const char* now,
			  NewOrderOutput* output, TPCCUndo** undo);


  //not used yet
  virtual bool newOrderRemote(int32_t home_warehouse, int32_t remote_warehouse,
  		  const std::vector<NewOrderItem>& items, std::vector<int32_t>* out_quantities,
  		  TPCCUndo** undo);
  virtual int32_t stockLevel(int32_t warehouse_id, int32_t district_id, int32_t threshold);
  virtual void orderStatus(int32_t warehouse_id, int32_t district_id, int32_t customer_id, OrderStatusOutput* output);
  virtual void orderStatus(int32_t warehouse_id, int32_t district_id, const char* c_last, OrderStatusOutput* output);
  
  virtual void payment(int32_t warehouse_id, int32_t district_id, int32_t c_warehouse_id,
  		  int32_t c_district_id, int32_t customer_id, float h_amount, const char* now,
  		  PaymentOutput* output, TPCCUndo** undo);
  virtual void payment(int32_t warehouse_id, int32_t district_id, int32_t c_warehouse_id,
  		  int32_t c_district_id, const char* c_last, float h_amount, const char* now,
  		  PaymentOutput* output, TPCCUndo** undo);
  virtual void paymentHome(int32_t warehouse_id, int32_t district_id, int32_t c_warehouse_id,
  		  int32_t c_district_id, int32_t c_id, float h_amount, const char* now,
  		  PaymentOutput* output, TPCCUndo** undo);
  virtual void paymentRemote(int32_t warehouse_id, int32_t district_id, int32_t c_warehouse_id,
  		  int32_t c_district_id, int32_t c_id, float h_amount, PaymentOutput* output,
  		  TPCCUndo** undo);
  virtual void paymentRemote(int32_t warehouse_id, int32_t district_id, int32_t c_warehouse_id,
  		  int32_t c_district_id, const char* c_last, float h_amount, PaymentOutput* output,
  		  TPCCUndo** undo);
  virtual void delivery(int32_t warehouse_id, int32_t carrier_id, const char* now,
  		  std::vector<DeliveryOrderInfo>* orders, TPCCUndo** undo);
  virtual bool hasWarehouse(int32_t warehouse_id);
  virtual void applyUndo(TPCCUndo* undo);
  virtual void freeUndo(TPCCUndo* undo);

  
};
}  
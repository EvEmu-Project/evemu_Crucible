/* 
 * File:   NPCMarket.h
 * Author: Rob R
 *
 * Created on June 10, 2014, 5:13 AM
 * 
 * example usage:
 *     NPCMarket::CreateNPCMarketFromFile("/etc/npcMarket.xml");
 * 
 * format for the xml file as follows.
 * 
 * <npcMarketOrders>
 *   <Station>
 *     <StationID>60000004</StationID>
 *     <Order>
 *       <TypeID>41</TypeID>
 *       <Qty>23572</Qty>
 *       <Price>25.52</Price>
 *       <Bid>0</Bid>
 *     </Order>
 *     <Order>
 *       <TypeID>43</TypeID>
 *       <Qty>59656</Qty>
 *       <Price>333.96</Price>
 *       <Bid>1</Bid>
 *     </Order>
 *     ....
 *   </station>
 *   <station>
 *     ....
 *   </station>
 * </npcMarketOrders>
 */

#ifndef NPCMARKET_H
#define	NPCMARKET_H

#include "MarketDB.h"


class NPCMarket
: public XMLParserEx
{
public:
  NPCMarket();

  /**
   * Process an xml file to create NPC market orders.
   * @param filename the file to process, path relative to the server root directory.
   */
  static void CreateNPCMarketFromFile(std::string filename);

private:
  /**
   * Access to the market database information.
   */
  MarketDB m_db;
  /**
   * Container for the retrieved station ID.
   */
  uint32 StationID;
  
  /**
   * Container for the retrieve market order data.
   */
  struct stationOrder {
    /**
     * The item type.
     */
    uint32 TypeID;
    /**
     * The quantity of items.
     */
    uint32 Qty;
    /**
     * The price of the item.
     */
    double Price;
    /**
     * Flag indicating a buy / sell order.
     */
    uint32 Bid;
  } order;
  /**
   * A list of orders retrieved for the current station.
   */
  std::list<stationOrder> newOrders;

  /**
   * Process the NPC Orders from the xml document.
   * @param ele the xml element to process.
   * @return true if successful, false on error.
   */
  bool ProcessNPCOrders( const TiXmlElement* ele );
  /**
   * Process the station entries from the xml document.
   * @param ele the xml element to process.
   * @return true if successful, false on error.
   */
  bool ProcessStation( const TiXmlElement* ele );
  /**
   * Process the order entries from the xml document.
   * @param ele the xml element to process.
   * @return true if successful, false on error.
   */
  bool ProcessOrder( const TiXmlElement* ele );

};

#endif	/* NPCMARKET_H */


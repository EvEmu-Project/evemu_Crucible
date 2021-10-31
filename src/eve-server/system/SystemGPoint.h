
 /**
  * @name SystemGPoint.h
  *   a group of methods and functions to get random points in a given solarsystem
  * @Author:         Allan
  * @date:   31 July 2014
  */


#ifndef _EVE_SYSTEM_GPOINT_H__
#define _EVE_SYSTEM_GPOINT_H__


class SystemManager;

class SystemGPoint {
  public:
      void GetMoons(uint32 systemID);   // incomplete
      void GetPlanets(uint32 systemID); // incomplete
      uint32 GetRandMoon(uint32 systemID);
      uint32 GetRandPlanet(uint32 systemID);
      const GPoint GetAnomalyPoint(SystemManager* pSys);
      const GPoint Get2RandPlanets(uint32 systemID);// incomplete
      const GPoint Get3RandPlanets(uint32 systemID);// incomplete
      const GPoint GetRandPointOnMoon(uint32 systemID);
      static const GPoint GetRandPointOnPlanet(uint32 systemID);
      const GPoint GetRandPointInSystem(uint32 systemID, int64 distance);// incomplete
};

#endif  // _EVE_SYSTEM_GPOINT_H__








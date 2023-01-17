
 /**
  * @name PlanetMgr.h
  *   Specific Class for managing planet resources
  * @Author:         Allan
  * @date:   30 April 2016
  */


#ifndef EVEMU_PLANET_PLANETMGR_H_
#define EVEMU_PLANET_PLANETMGR_H_

#include "planet/PlanetDB.h"
#include "planet/PlanetDataMgr.h"


class Client;
class Colony;
class PlanetSE;
class UUNCommand;
class UUNCommandList;
class PlanetMgr
{
public:
    PlanetMgr(Client* pClient, PlanetSE* pPlanet, Colony* pColony);
    virtual ~PlanetMgr()    { /* do nothing here */ }

    PyRep* UpdateNetwork(PyList* commandList);

protected:
    bool CreatePin(UUNCommand& nc);
    void RemovePin(UUNCommand& nc);
    void CreateLink(UUNCommand& nc);
    void RemoveLink(UUNCommand& nc);
    void SetLinkLevel(UUNCommand& nc);
    void CreateRoute(UUNCommand& nc);
    void RemoveRoute(UUNCommand& nc);
    void SetSchematic(UUNCommand& nc);
    bool UpgradeCommandCenter(UUNCommand& nc);
    void AddExtractorHead(UUNCommand& nc);
    void KillExtractorHead(UUNCommand& nc);
    void MoveExtractorHead(UUNCommand& nc);
    void InstallProgram(UUNCommand& nc);
    void PrioritizeRoute(UUNCommand& nc);

private:
    Client* m_client;
    Colony* m_colony;
    PlanetSE* m_planet;

    PlanetDB m_db;
};


#endif  // EVEMU_PLANET_PLANETMGR_H_

/*
 *     qtyToDistribute, cycleTime, numCycles = self.remoteHandler.GetProgramResultInfo(pinID, typeID, pin.heads, headRadius)
 *     qtyToRoute = pin.GetMaxOutput(qtyToDistribute, cycleTime)
 *
 *    def GetMaxOutput(self, baseOutput = None, cycleTime = None):
 *        if baseOutput is None:
 *            baseOutput = self.qtyPerCycle
 *        if cycleTime is None:
 *            cycleTime = self.cycleTime
 *        scalar = self.GetAttribute(const.attributeEcuNoiseFactor) + 1
 *        return int(scalar * baseOutput) * cycleTime / SEC / 900.0
 *
 *
 *
        outputVals = GetProgramOutputPrediction(maxValue, cycleTime, numCycles)
        totalOutput = sum(outputVals)
        perHourOutput = float(totalOutput) * HOUR / (numCycles * cycleTime)

 *    def GetProgramOutput(self, currentTime, baseValue = None, cycleTime = None, startTime = None):
 *        if baseValue is None:
 *            baseValue = self.qtyPerCycle
 *        if cycleTime is None:
 *            cycleTime = self.cycleTime
 *        if startTime is None:
 *            startTime = self.installTime
 *        decayFactor = self.GetAttribute(const.attributeEcuDecayFactor)
 *        noiseFactor = self.GetAttribute(const.attributeEcuNoiseFactor)
 *        timeDiff = currentTime - startTime
 *        cycleNum = max((timeDiff + const.SEC) / cycleTime - 1, 0)
 *        barWidth = cycleTime / SEC / 900.0
 *        t = (cycleNum + 0.5) * barWidth
 *        decayValue = baseValue / (1 + t * decayFactor)
 *        f1 = 1.0 / 12
 *        f2 = 1.0 / 5
 *        f3 = 1.0 / 2
 *        phaseShift = baseValue ** 0.7
 *        sinA = math.cos(phaseShift + t * f1)
 *        sinB = math.cos(phaseShift / 2 + t * f2)
 *        sinC = math.cos(t * f3)
 *        sinStuff = (sinA + sinB + sinC) / 3
 *        sinStuff = max(0, sinStuff)
 *        barHeight = decayValue * (1 + noiseFactor * sinStuff)
 *        return int(barWidth * barHeight)
 *
 *    def GetProgramOutputPrediction(self, baseValue, cycleTime, length):
 *        vals = [] (python list)
 *        startTime = 0
 *        for i in xrange(length):
 *            currentTime = (i + 1) * cycleTime
 *            vals.append(self.GetProgramOutput(currentTime, baseValue, cycleTime, startTime))
 *
 *        return vals
 */
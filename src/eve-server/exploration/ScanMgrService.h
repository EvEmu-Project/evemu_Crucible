/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://evemu.dev
    ------------------------------------------------------------------------------------
    This program is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by the Free Software
    Foundation; either version 2 of the License, or (at your option) any later
    version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License along with
    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
    http://www.gnu.org/copyleft/lesser.txt.
    ------------------------------------------------------------------------------------
    Author:        Allan
*/


#ifndef EVEMU_SCANING_SCANMGR_H_
#define EVEMU_SCANING_SCANMGR_H_

#include "services/BoundService.h"
#include "exploration/Scan.h"
#include "exploration/ScanningDB.h"

class ScanBound;

class ScanMgrService : public Service <ScanMgrService>, public BoundServiceParent<ScanBound> {
public:
    ScanMgrService(EVEServiceManager& mgr);

    void BoundReleased (ScanBound* bound) override;
protected:
    PyResult GetSystemScanMgr(PyCallArgs& call);

private:
    EVEServiceManager& m_manager;
    std::map <Client*, ScanBound*> m_instances;
};

class ScanBound : public EVEBoundObject <ScanBound>
{
public:
    ScanBound(EVEServiceManager& mgr, ScanMgrService& parent, Client* client);

    Client* GetClient () { return this->m_client; }

protected:
    PyResult ConeScan(PyCallArgs& call, PyRep* ignored1, PyRep* ignored2, PyRep* ignored3, PyRep* ignored4, PyRep* ignored5);
    PyResult RequestScans(PyCallArgs& call, std::optional <PyDict*> probes);
    PyResult RecoverProbes(PyCallArgs& call, PyList* probeIDs);
    PyResult DestroyProbe(PyCallArgs& call, PyInt* probeID);
    PyResult ReconnectToLostProbes(PyCallArgs& call);

private:
    Client* m_client;
};

#endif  // EVEMU_SCANING_SCANMGR_H_
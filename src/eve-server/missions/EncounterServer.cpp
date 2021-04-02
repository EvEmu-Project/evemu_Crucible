



21:31:36 [SvcError] PyServiceMgr::LookupService() - Service encounterSpawnServer not found in list.
21:31:36 E Client: Unable to find service to handle call to: encounterSpawnServer




def _StartSelectedEncounter(self, *args):
encounterList = [ node.encounterID for node in self.encounterScroll.GetSelected() ]
logResults = self.encounterSpawnServer.RequestActivateEncounters(encounterList, logResults=True)
self.AppendResult(logResults)

def _StopSelectedEncounter(self, *args):
encounterList = [ node.encounterID for node in self.encounterScroll.GetSelected() ]
logResults = self.encounterSpawnServer.RequestDeactivateEncounters(encounterList, logResults=True)
self.AppendResult(logResults)

def _GetAvailableEncounters(self):
encounters = self.encounterSpawnServer.GetMyEncounters()
contentList = []
for encounter in encounters:
    data = {'text': encounter['encounterName'],
        'label': encounter['encounterName'],
        'id': encounter['encounterID'],
        'encounterID': encounter['encounterID'],
        'disableToggle': True,
        'hint': encounter['encounterName']}
        contentList.append(uicls.ScrollEntryNode(**data))

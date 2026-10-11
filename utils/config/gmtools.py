# -*- coding: utf-8 -*-
"""EVEmu GM tools for the Crucible client.

The server sends this file to clients with elevated roles (GM, WM, programmer...) through
devToolsProvider.GetScript(), and the client runs it after loading devtools.raw.  It is Python 2.7,
compiled and run inside the client.  It can be edited without rebuilding the server; clients pick up
changes on their next login, or straight away with "Reload Insider" on the neocom right-click menu.

What it adds:
  - svc.slash: the client-side slash service.  CCP shipped this with their dev tools; without it, typing
    /commands in chat and several built-in GM menu entries (/load, /create, /spawn, /fit...) fail.
  - form.AttributeInspector, used by the built-in "GM: Inspect Attributes" menu entry.
  - svc.insider: an "EVEmu Insider" window with common GM actions (neocom: Toggle Insider).
  - an "EVEmu GM Tools" submenu next to "GM / WM Extras" on characters, items and objects in space.

Everything here ends up as a slash command handled by the server (see src/eve-server/admin).
"""

import types
import blue
import service
import uicls
import uiconst
import uiutil
import uix
import util
import listentry
import nasty
import svc
import log

SOURCE_NAME = 'evemu_gmtools.py'
GM_ROLES = service.ROLE_GML | service.ROLE_WORLDMOD
MENU_LABEL = 'EVEmu GM Tools'


# ------------------------------------------------------------------------------------------------
#  helpers
# ------------------------------------------------------------------------------------------------

def Slash(cmd):
    """Sends a slash command to the server and shows what it returns."""
    res = sm.RemoteSvc('slash').SlashCmd(cmd)
    if res is not None and res != '':
        try:
            eve.Message('CustomNotify', {'notify': u'%s' % (res,)})
        except Exception:
            log.LogInfo('EVEmu GM tools:', cmd, '->', res)
    return res


def AskAmount(caption, label, setvalue=1, maxvalue=2147483647, minvalue=1):
    ret = uix.QtyPopup(maxvalue=maxvalue, minvalue=minvalue, setvalue=setvalue, caption=caption, label=label, hint='')
    if ret:
        return ret['qty']
    return None


def AskText(caption, label, setvalue=''):
    ret = uiutil.NamePopup(caption, label, setvalue)
    if ret:
        return ret.strip()
    return None


def Quote(text):
    return '"%s"' % text.replace('"', '')


def IsGM():
    return bool(session.role & GM_ROLES)


def TypeName(typeID):
    try:
        return cfg.invtypes.Get(typeID).name
    except Exception:
        return str(typeID)


def OwnerName(ownerID):
    try:
        return cfg.eveowners.Get(ownerID).name
    except Exception:
        return str(ownerID)


# ------------------------------------------------------------------------------------------------
#  actions
# ------------------------------------------------------------------------------------------------

def GiveISK(charID):
    amount = AskAmount('Give ISK', 'ISK to give %s' % OwnerName(charID), setvalue=1000000, maxvalue=2000000000)
    if amount:
        Slash('/giveisk %d %d' % (charID, amount))


def CreateType(typeID):
    qty = AskAmount('Create %s' % TypeName(typeID), 'Quantity', setvalue=1)
    if qty:
        Slash('/create %d %d' % (typeID, qty))


def CreateByName():
    name = AskText('Create Item', 'Type name or typeID')
    if not name:
        return
    qty = AskAmount('Create %s' % name, 'Quantity', setvalue=1)
    if not qty:
        return
    if name.isdigit():
        Slash('/create %s %d' % (name, qty))
    else:
        Slash('/create %s %d' % (Quote(name), qty))


def GiveSkill(charID):
    name = AskText('Give Skill', 'Skill typeID')
    if not name or not name.isdigit():
        return
    level = AskAmount('Give Skill', 'Level', setvalue=5, maxvalue=5, minvalue=0)
    if level is None:
        return
    target = 'me' if charID == session.charid else str(charID)
    Slash('/giveskill %s %s %d' % (target, name, level))


def SpawnType(typeID):
    qty = AskAmount('Spawn %s' % TypeName(typeID), 'How many', setvalue=1, maxvalue=50)
    if qty:
        Slash('/spawnn %d 4000 %d' % (qty, typeID))


def DeployEntity(typeID):
    qty = AskAmount('Deploy %s' % TypeName(typeID), 'How many', setvalue=1, maxvalue=100)
    if qty:
        Slash('/entity deploy %d %d' % (qty, typeID))


def GoToSystem():
    name = AskText('Translocate', 'Solar system or station name, or locationID')
    if name:
        Slash('/tr me %s' % (name if name.isdigit() else Quote(name)))


def Confirm(text):
    return eve.Message('CustomQuestion', {'header': 'EVEmu GM Tools', 'question': text}, uiconst.YESNO) == uiconst.ID_YES


def Kick(charID):
    if Confirm('Kick %s from the server?' % OwnerName(charID)):
        Slash('/kick %d' % charID)


def Ban(charID):
    if Confirm('Ban %s? This bans their account.' % OwnerName(charID)):
        Slash('/ban %d' % charID)


def KillShip(charID):
    if charID == session.charid:
        if Confirm('Destroy your own ship?'):
            Slash('/kill me')
    elif Confirm("Destroy %s's ship?" % OwnerName(charID)):
        Slash('/kill %d' % charID)


def KillItem(itemID, slimItem):
    pilot = getattr(slimItem, 'charID', None)
    if pilot and not util.IsNPC(pilot) and not Confirm("Destroy %s's ship?" % OwnerName(pilot)):
        return
    Slash('/kill %d' % itemID)


def UnkillableMenu(target):
    return ('Unkillable', [
        ('On', Slash, ('/unkillable %s on' % target,)),
        ('Off', Slash, ('/unkillable %s off' % target,)),
    ])


def Killable(slimItem):
    """Ships, NPCs, drones and structures.  Celestials, stations and gates can't be destroyed."""
    cats = [const.categoryShip, const.categoryEntity, const.categoryDrone]
    for name in ('categoryStructure', 'categoryOrbital', 'categorySovereigntyStructure', 'categoryDeployable'):
        cat = getattr(const, name, None)
        if cat is not None:
            cats.append(cat)
    return getattr(slimItem, 'categoryID', None) in cats


# ------------------------------------------------------------------------------------------------
#  context menus
# ------------------------------------------------------------------------------------------------

def CharacterEntries(charID):
    if util.IsNPC(charID):
        return []
    me = (charID == session.charid)
    who = 'me' if me else str(charID)
    m = [
        ('Give ISK...', GiveISK, (charID,)),
        ('Give skill...', GiveSkill, (charID,)),
        ('Give all skills (V)', Slash, ('/giveallskills %s' % who,)),
        None,
        ('Heal ship', Slash, ('/heal %s' % ('' if me else charID),)),
        ('Repair modules', Slash, ('/repairmodules %s' % ('' if me else charID),)),
        ('Online all modules', Slash, ('/online %s' % who,)),
        None,
        ('Destroy ship...', KillShip, (charID,)),
        UnkillableMenu(who),
    ]
    if not me:
        m += [
            None,
            ('Bring to me', Slash, ('/tr %d me' % charID,)),
            ('Go to them', Slash, ('/tr me %d' % charID,)),
            None,
            ('Gag', [
                ('30 minutes', Slash, ('/gag %d "Gagged by a GM" 30' % charID,)),
                ('1 hour', Slash, ('/gag %d "Gagged by a GM" 60' % charID,)),
                ('24 hours', Slash, ('/gag %d "Gagged by a GM" 1440' % charID,)),
                None,
                ('Ungag', Slash, ('/ungag %d' % charID,)),
            ]),
            ('Kick...', Kick, (charID,)),
            ('Ban...', Ban, (charID,)),
        ]
    else:
        m += [
            None,
            ('Create item...', CreateByName, ()),
            ('Translocate...', GoToSystem, ()),
        ]
    return m


def TypeEntries(typeID, itemID=None):
    m = []
    try:
        invType = cfg.invtypes.Get(typeID)
    except Exception:
        return m
    cat = invType.categoryID
    if cat not in (const.categoryStation, const.categoryOwner, const.categoryCelestial) and typeID != const.typeSolarSystem:
        m.append(('Create %s...' % invType.name, CreateType, (typeID,)))
    if not session.stationid:
        if cat == const.categoryShip:
            m.append(('Spawn %s...' % invType.name, SpawnType, (typeID,)))
        elif cat == const.categoryEntity:
            m.append(('Deploy %s...' % invType.name, DeployEntity, (typeID,)))
    if itemID:
        m.append(('Inspect attributes', OpenInspector, (itemID, typeID)))
    return m


def ItemEntries(itemID, typeID, slimItem=None):
    m = TypeEntries(typeID, itemID)
    if slimItem is not None:
        m.append(None)
        cat = getattr(slimItem, 'categoryID', None)
        if Killable(slimItem):
            m.append(('Kill', KillItem, (itemID, slimItem)))
            m.append(UnkillableMenu(itemID))
        if cat in (const.categoryShip, const.categoryEntity, const.categoryDrone):
            m.append(('Unspawn', Slash, ('/unspawn %d' % itemID,)))
        m.append(('Go here', Slash, ('/tr me %d' % itemID,)))
    elif itemID:
        m.append(('Repair', Slash, ('/heal %d' % itemID,)))
    return m


def BuildMenu(kind, *args):
    try:
        if kind == 'char':
            m = CharacterEntries(*args)
        elif kind == 'item':
            m = ItemEntries(*args)
        else:
            m = TypeEntries(*args)
    except Exception:
        log.LogException('EVEmu GM tools: menu failed')
        m = []
    return m or [('(nothing here)', lambda *x: None, ())]


def AddToMenu(menu, kind, *args):
    """Puts our submenu just after GM / WM Extras."""
    if not IsGM() or not isinstance(menu, list):
        return menu
    entry = (MENU_LABEL, ('isDynamic', BuildMenu, (kind,) + args))
    for i, e in enumerate(menu):
        if isinstance(e, tuple) and len(e) and e[0] == 'GM / WM Extras':
            menu.insert(i + 1, entry)
            return menu
    menu.insert(0, entry)
    return menu


def _Wrap(cls, name, wrapper):
    original = getattr(cls, '_evemu_' + name, None) or getattr(cls, name).im_func
    setattr(cls, '_evemu_' + name, original)

    def patched(self, *args, **kwargs):
        res = original(self, *args, **kwargs)
        try:
            return wrapper(self, res, *args, **kwargs)
        except Exception:
            log.LogException('EVEmu GM tools: %s wrapper failed' % name)
            return res

    patched.__name__ = name
    setattr(cls, name, patched)


def _InvItemMenu(self, res, invItem, viewOnly, voucher, unparsed=0, multi=0, filterFunc=None, allInvItems=None):
    if unparsed or multi or invItem is None:
        return res
    return AddToMenu(res, 'item', invItem.itemID, invItem.typeID, None)


def _CharacterMenu(self, res, charid, corpid, unparsed=0, filterFunc=None, multi=0):
    if unparsed or multi or type(charid) not in (types.IntType, types.LongType):
        return res
    return AddToMenu(res, 'char', charid)


def _CelestialMenu(self, res, data, *args, **kwargs):
    # data is (itemID, mapItem, slimItem, noTrace, typeID, parentID, bookmark)
    if kwargs.get('multi', 0) or not isinstance(data, (tuple, list)) or len(data) < 5:
        return res
    itemID, mapItem, slimItem, typeID = data[0], data[1], data[2], data[4]
    if slimItem is None and itemID:
        bp = sm.GetService('michelle').GetBallpark()
        if bp:
            slimItem = bp.GetInvItem(itemID)
    if slimItem is not None:
        return AddToMenu(res, 'item', itemID, slimItem.typeID, slimItem)
    if mapItem is not None:
        typeID = typeID or mapItem.typeID
    if not typeID:
        return res
    return AddToMenu(res, 'type', typeID, None)


def PatchMenus():
    menu = svc.menu
    _Wrap(menu, '_InvItemMenu', _InvItemMenu)
    _Wrap(menu, '_CharacterMenu', _CharacterMenu)
    _Wrap(menu, '_CelestialMenu', _CelestialMenu)


# ------------------------------------------------------------------------------------------------
#  client slash service
# ------------------------------------------------------------------------------------------------

class EvemuSlash(service.Service):
    __guid__ = 'svc.slash'
    __displayname__ = 'EVEmu Slash Service'

    def SlashCmd(self, cmd):
        # typed commands and the built-in GM menus come through here.  the server does the work
        return sm.RemoteSvc('slash').SlashCmd(cmd)


# ------------------------------------------------------------------------------------------------
#  attribute inspector
# ------------------------------------------------------------------------------------------------

def OpenInspector(itemID, typeID):
    form.AttributeInspector.Open(itemID=itemID, typeID=typeID)


class AttributeInspector(uicls.Window):
    __guid__ = 'form.AttributeInspector'
    default_windowID = 'EvemuAttributeInspector'
    default_width = 420
    default_height = 500
    default_minSize = (300, 200)

    def ApplyAttributes(self, attributes):
        uicls.Window.ApplyAttributes(self, attributes)
        self.itemID = attributes.get('itemID', None)
        self.typeID = attributes.get('typeID', None)
        self.SetCaption('Attributes: %s (%s)' % (TypeName(self.typeID), self.itemID))
        self.sr.scroll = uicls.Scroll(parent=self.sr.maincontainer, padding=(4, 4, 4, 4))
        self.Refresh()

    def Refresh(self):
        statemgr = sm.GetService('godma').GetStateManager()
        attrs = sm.StartService('info').GetAttrDict(self.typeID)
        rows = []
        for attributeID, baseValue in attrs.iteritems():
            try:
                name = statemgr.attributesByID[attributeID].attributeName
            except Exception:
                name = str(attributeID)
            try:
                current = statemgr.GetAttribute(self.itemID, name)
            except Exception:
                current = baseValue
            label = '%s (%d)<t>%s<t>%s' % (name, attributeID, baseValue, current)
            rows.append((name.lower(), listentry.Get('Generic', {'label': label, 'OnDblClick': (self.Edit, name, current)})))
        rows.sort()
        self.sr.scroll.Load(contentList=[r[1] for r in rows], headers=['Attribute', 'Base', 'Current'])

    def Edit(self, entry, name, current):
        ret = uix.QtyPopup(None, None, current, 'New value for %s' % name, 'Set attribute', digits=5)
        if ret:
            Slash('/dogma %s %s = %s' % (self.itemID, name, ret['qty']))
            self.Refresh()


# ------------------------------------------------------------------------------------------------
#  insider
# ------------------------------------------------------------------------------------------------

class EvemuInsider(service.Service):
    __guid__ = 'svc.insider'
    __displayname__ = 'EVEmu Insider Service'
    __notifyevents__ = ['OnSessionChanged']

    def OnSessionChanged(self, isRemote, sess, change):
        pass

    def Show(self, *args, **kwargs):
        # called by the neocom at login.  only open if asked to (neocom: Toggle Insider)
        if kwargs.get('forceShow', False):
            form.EvemuInsiderWindow.Open()

    def Toggle(self, forceShow=False, *args):
        wnd = form.EvemuInsiderWindow.GetIfOpen()
        if wnd and not forceShow:
            wnd.CloseByUser()
        elif wnd:
            wnd.Maximize()
        else:
            form.EvemuInsiderWindow.Open()

    def Reload(self, *args):
        Load()

    def HealRemove(self, groupID=None, *args):
        # the benchmark script uses this to clear wrecks.  not supported
        pass


class EvemuInsiderWindow(uicls.Window):
    __guid__ = 'form.EvemuInsiderWindow'
    default_windowID = 'EvemuInsiderWindow'
    default_width = 260
    default_height = 330
    default_minSize = (220, 200)

    def ApplyAttributes(self, attributes):
        uicls.Window.ApplyAttributes(self, attributes)
        self.SetCaption('EVEmu Insider')
        buttons = [
            ('Create item...', CreateByName),
            ('Give me ISK...', lambda: GiveISK(session.charid)),
            ('Give me a skill...', lambda: GiveSkill(session.charid)),
            ('Give me all skills (V)', lambda: Slash('/giveallskills me')),
            ('Heal my ship', lambda: Slash('/heal')),
            ('Repair my modules', lambda: Slash('/repairmodules')),
            ('Online my modules', lambda: Slash('/online me')),
            ('Translocate...', GoToSystem),
            ('Kill NPCs in bubble', lambda: Slash('/killallnpcs')),
            ('Dev window', lambda: form.DevWindow.Open()),
            ('Reload GM tools', Load),
        ]
        for label, func in buttons:
            uicls.Button(parent=self.sr.maincontainer, align=uiconst.TOTOP, label=label, func=self._MakeHandler(func), padding=(4, 2, 4, 0))

    def _MakeHandler(self, func):
        def handler(*args):
            func()
        return handler


# ------------------------------------------------------------------------------------------------
#  registration
# ------------------------------------------------------------------------------------------------

def Install():
    nasty.nasty.RegisterNamedObject(AttributeInspector, 'form', 'AttributeInspector', SOURCE_NAME, globals())
    nasty.nasty.RegisterNamedObject(EvemuInsiderWindow, 'form', 'EvemuInsiderWindow', SOURCE_NAME, globals())
    for name, cls in (('slash', EvemuSlash), ('insider', EvemuInsider)):
        setattr(svc, name, cls)
        try:
            sm.StopService(name)
        except Exception:
            pass
        sm.StartService(name)
    PatchMenus()
    log.LogInfo('EVEmu GM tools installed')


def Load():
    """Fetches the current version of this file from the server and runs it again."""
    src = sm.RemoteSvc('devToolsProvider').GetScript()
    if src:
        import __builtin__
        ns = {'__name__': 'evemu_gmtools', '__builtins__': __builtin__}
        exec compile(src, SOURCE_NAME, 'exec') in ns


import form
Install()

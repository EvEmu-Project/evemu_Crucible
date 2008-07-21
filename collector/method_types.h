
#ifndef CALL_REQ_HANDLER
#define CALL_REQ_HANDLER(c)
#endif
#ifndef CALL_RSP_HANDLER
#define CALL_RSP_HANDLER(c)
#endif
#ifndef BOUND_CALL_RSP_HANDLER
#define BOUND_CALL_RSP_HANDLER(c)
#endif
#ifndef NOTIFY_HANDLER
#define NOTIFY_HANDLER(c)
#endif

	CALL_REQ_HANDLER(MachoBindObject)
	
//	CALL_RSP_HANDLER(GetPostAuthenticationMessage)
	CALL_RSP_HANDLER(GetCharactersToSelect)
	CALL_RSP_HANDLER(MachoBindObject)
	CALL_RSP_HANDLER(GetCachableObject)
//  CALL_RSP_HANDLER(GetChannels)
/*	CALL_RSP_HANDLER(Ping)
	CALL_RSP_HANDLER(GetCharCreationInfo)
	CALL_RSP_HANDLER(GetCharacterToSelect)
	CALL_RSP_HANDLER(SelectCharacterID)
	CALL_RSP_HANDLER(GetAttributeTypes)
	CALL_RSP_HANDLER(MachoResolveObject)
	CALL_RSP_HANDLER(GetAppearanceInfo)
	CALL_RSP_HANDLER(ValidateName)
	CALL_RSP_HANDLER(CreateCharacter)*/
	
	BOUND_CALL_RSP_HANDLER(List)
//  BOUND_CALL_RSP_HANDLER(GetInventory)


	NOTIFY_HANDLER(DoDestinyUpdate)
//	NOTIFY_HANDLER(DoDestinyUpdates)
	NOTIFY_HANDLER(OnLSC)
/*	NOTIFY_HANDLER(OnAggressionChange)
	NOTIFY_HANDLER(OnMultiEvent)
	NOTIFY_HANDLER(OnRemoteExecute)
	NOTIFY_HANDLER(OnCharNoLongerInStation)
	NOTIFY_HANDLER(OnCharNowInStation)*/

#undef CALL_REQ_HANDLER
#undef CALL_RSP_HANDLER
#undef BOUND_CALL_RSP_HANDLER
#undef NOTIFY_HANDLER


#include "Generator.h"
#include "../common/logsys.h"


void Generator::Generate(FILE *into, TiXmlElement *root) {
	TiXmlNode *main_element = NULL;
	while( (main_element = root->IterateChildren( main_element )) ) {
		//pass through comments.
		if(main_element->Type() == TiXmlNode::COMMENT) {
			TiXmlComment *com = main_element->ToComment();
			fprintf(into, "\t/* %s */\n", com->Value());
			continue;
		}
		if(main_element->Type() != TiXmlNode::ELEMENT)
			continue;	//skip crap we dont care about
		
		TiXmlElement *ele = main_element->ToElement();
		if(std::string("include") == ele->Value()) {
			Process_include(into, ele);
			continue;
		} else if(std::string("element") != ele->Value()) {
			_log(COMMON__ERROR, "Unexpected element '%s' in <Elements> on line %d. Ignore.", ele->Value(), ele->Row());
			continue;
		}
		
		Process_root(into, ele);
	}
}

void Generator::Process_include(FILE *into, TiXmlElement *element) {
}

const char *Generator::GetEncodeType(TiXmlElement *element) const {
	TiXmlElement * main = element->FirstChildElement();
	if(main == NULL)
		return("PyRep");
	std::map<std::string, std::string>::const_iterator res;
	res = m_eleTypes.find(main->Value());
	if(res == m_eleTypes.end())
		return("PyRep");
	return(res->second.c_str());
}


















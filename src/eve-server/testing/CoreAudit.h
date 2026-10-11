#pragma once

bool IsCoreAuditMode(const char* mode);
bool CoreAuditEnvironmentValid();
class EVEServiceManager;
int RunCoreAudit(const char* mode, EVEServiceManager& services);

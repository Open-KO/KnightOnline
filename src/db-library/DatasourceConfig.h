#ifndef DBLIBRARY_DATASOURCECONFIG_H
#define DBLIBRARY_DATASOURCECONFIG_H

#pragma once

#include <string>
#include <string_view>

namespace db
{

struct DatasourceConfig
{
	std::string DatasourceName;
	std::string DatasourceUsername;
	std::string DatasourcePassword;

	// NOTE: This is only used internally, so no reason not to forward along the views here.
	DatasourceConfig(const std::string_view& name, const std::string_view& username,
		const std::string_view& password) :
		DatasourceName(name), DatasourceUsername(username), DatasourcePassword(password)
	{
	}
};

} // namespace db

#endif // DBLIBRARY_DATASOURCECONFIG_H

#ifndef SERVER_EBENEZER_BOTTARGETSELECTOR_H
#define SERVER_EBENEZER_BOTTARGETSELECTOR_H

#pragma once

namespace Ebenezer
{

class CBotUser;
class EbenezerApp;

class BotTargetSelector
{
public:
	explicit BotTargetSelector(EbenezerApp& app);
	int SelectNearestEnemy(const CBotUser& source) const;

private:
	EbenezerApp& _app;
};

} // namespace Ebenezer

#endif // SERVER_EBENEZER_BOTTARGETSELECTOR_H

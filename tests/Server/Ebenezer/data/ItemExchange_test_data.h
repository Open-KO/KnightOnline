#ifndef TESTS_SERVER_EBENEZER_ITEMEXCHANGE_TEST_DATA_H
#define TESTS_SERVER_EBENEZER_ITEMEXCHANGE_TEST_DATA_H

#pragma once

#include <Ebenezer/GameDefine.h>

// Generated from the following:
/*
	SELECT
		CONCAT('{ .Index = ', nIndex,
			   ', .NpcNumber = ', nNpcNum,
			   ', .RandomFlag = ', bRandomFlag,
			   ', .OriginItemNumber = {',
				nOriginItemNum1, ', ', nOriginItemNum2, ', ', nOriginItemNum3, ', ',
				nOriginItemNum4, ', ', nOriginItemNum5, '}',
			   ', .OriginItemCount = {',
				nOriginItemCount1, ', ', nOriginItemCount2, ', ', nOriginItemCount3, ', ',
				nOriginItemCount4, ', ', nOriginItemCount5, '}',
			   ', .ExchangeItemNumber = {',
				nExchangeItemNum1, ', ', nExchangeItemNum2, ', ', nExchangeItemNum3, ', ',
				nExchangeItemNum4, ', ', nExchangeItemNum5, '}',
			   ', .ExchangeItemCount = {',
				nExchangeItemCount1, ', ', nExchangeItemCount2, ', ', nExchangeItemCount3, ', ',
				nExchangeItemCount4, ', ', nExchangeItemCount5, '}',
			   ' }, ')
	FROM ITEM_EXCHANGE
	WHERE nIndex IN(
		-- EXCHANGE_TYPE_ALL_ITEMS
		1, -- Proconsul beginners warrior
		-- EXCHANGE_TYPE_ONE_OF_EQUAL
		5, -- Isaac Kiss exchange
		-- EXCHANGE_TYPE_ONE_OF_WEIGHTED
		35271 -- Abyss gem
		)
	ORDER BY nIndex ASC
*/

static Ebenezer::model::ItemExchange itemExchangeData[] =
{
	{ .Index = 1, .NpcNumber = 14301, .RandomFlag = 0, .OriginItemNumber = {379048000, 0, 0, 0, 0}, .OriginItemCount = {5, 0, 0, 0, 0}, .ExchangeItemNumber = {120110992, 389011000, 389017000, 0, 0}, .ExchangeItemCount = {1, 10, 10, 0, 0} },
	{ .Index = 5, .NpcNumber = 26011, .RandomFlag = 5, .OriginItemNumber = {910014000, 0, 0, 0, 0}, .OriginItemCount = {5, 0, 0, 0, 0}, .ExchangeItemNumber = {389014000, 389019000, 389035000, 379016000, 379020000}, .ExchangeItemCount = {10, 10, 10, 1, 1} },
	{ .Index = 35271, .NpcNumber = 14301, .RandomFlag = 101, .OriginItemNumber = {379106000, 0, 0, 0, 0}, .OriginItemCount = {1, 0, 0, 0, 0}, .ExchangeItemNumber = {156210001, 156110001, 205001001, 204001001, 389020000}, .ExchangeItemCount = {10, 50, 500, 2000, 7440} }
};
#endif // TESTS_SERVER_EBENEZER_ITEMEXCHANGE_TEST_DATA_H

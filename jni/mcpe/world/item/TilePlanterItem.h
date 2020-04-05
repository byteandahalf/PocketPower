#pragma once

#include "Item.h"

class TilePlanterItem : public Item {
public:
	TilePlanterItem(int, Tile*);
	virtual ~TilePlanterItem();
	virtual bool useOn(ItemInstance*, Player*, int, int, int, signed char, float, float, float);
	virtual bool isEmissive(int) const;
};

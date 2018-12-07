
#include "ResourceItem.h"

FResourceItem::FResourceItem()
	:FResourceItem(NULL)
{
}

FResourceItem::FResourceItem(UResource *InResource)
	: bLoaded(0)
	, bLoading(0)
	, bDepLoading(0)
	, bCorrupt(0)
	, bHasChekUpdated(0)
	, bNeedUpdate(0)
	, Resource(InResource)
{
	if (Resource)
	{
		bLoaded = 1;
		Skuid = Resource->GetResID();
	}
}


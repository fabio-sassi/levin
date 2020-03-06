/*
 *      PRIVATE METHODS
 */

static int ab_nodeFrom(ab_Node *node)
{
	assert(node->size > 0);
	return node->items[0].letter;
}

static int ab_nodeTo(ab_Node *node)
{
	assert(node->size > 0);
	return node->items[0].letter + node->size - 1;
}

static void ab_nodeGrow(ab_Node *node, int from, int to)
{
	int offset = ab_nodeFrom(node) - from;
	int size0 = node->size;
	ab_NodeItem *src = node->items;

	node->size = to - from + 1;
	AB_D printf("ab_nodeGrow: size from %d to %d\n", size0, node->size);

	// TODO
	// node->items = ab_nodeItemsNewFrom(node->size, src, offset, size0);

	node->items = ab_newItems(node->size);
	memcpy(node->items + offset, src, size0 * sizeof(ab_NodeItem));

	ea_freeArray(ab_NodeItem, size0, src);
}



/*
 *     NODE INTERFACE
 */


static void ab_delItem(ab_Node *node, ab_NodeItem *item)
{
	int from = ab_nodeFrom(node);
	int to = ab_nodeTo(node);
	int i, size;
	ab_NodeItem *items;

	AB_D printf("nodeShrink - f='%c'%d to ='%c'%d\n", from, from, to, to);

#if 0
	AB_D {
		ab_printNode(node, 4, false);
		printf("nodeShrink items =\n");
		for (i = 0; i < node->size; i++)
			ab_printNodeItem(node->items + i, 4);
	}
#endif

	if (item->letter == from) {
		for (i = from + 1; i < to; i++)
			if (node->items[i - from].flag)
				break;

		from = i;
	} else if (item->letter == to) {
		for (i = to - 1; i > from; i--)
			if (node->items[i - from].flag)
				break;

		to = i;
	} else {
		AB_D printf("nodeShrink...nothing to do\n");
		return;
	}


	AB_D printf("nodeShrink -GO- f='%c'%d t='%c'%d\n", from, from, to, to);

	size = to - from + 1;
	i = from - ab_nodeFrom(node);

	// TODO
	//items = ab_nodeItemsNewFrom(size, node->items, i, size);
	AB_D printf("nodeShrink copy from %d size=%d\n", i, size);

	items = ab_newItems(size);
	memcpy(items, node->items + i, size * sizeof(ab_NodeItem)) ;


	ea_freeArray(ab_NodeItem, node->size, node->items);

	node->items = items;
	node->size = size;

#if 0
	AB_D {
		ab_printNode(node, 4, false);
		printf("nodeShrink new items =\n");
		for (i = 0; i < node->size; i++)
			ab_printNodeItem(node->items + i, 4);
	}
#endif
}


static void ab_addItem(ab_Node *node, int c, ab_Wood *sub, int haveval,
                                                             void *val)
{
	ab_NodeItem *item;

	AB_D printf("nodeIns: char=%d'%c' size0 = %d\n", c, c, node->size);

	if (node->items == NULL) {
		node->size = 1;
		node->items = ab_newItems(1);
		item = node->items;
	} else {
		int from = ab_nodeFrom(node);
		int to = ab_nodeTo(node);

		AB_D printf("nodeIns: range0 %d'%c' - %d'%c'\n", from, from,
		            to, to);

		if (c < from) {
			ab_nodeGrow(node, c, to);
			from = c;
		} else if (c > to) {
			ab_nodeGrow(node, from, c);
			to = c;
		}

		AB_D printf("nodeIns: range %d'%c' - %d'%c'\n", from, from,
		            to, to);
		item = node->items + (c - from);
	}
	AB_D printf("nodeIns: size = %d\n", node->size);

	item->flag = AB_ITEM_ON;
	item->letter = c;

	if (sub)
		ab_setItemWood(node, item, sub);

	if (haveval)
		ab_setItemValue(node, item, val);
}


/* item lookup */
static ab_NodeItem* ab_lookupItem(ab_Node *node, int c)
{
	int i, from;

	if (!node->items)
		return NULL;

	from = ab_nodeFrom(node);

	if (c < from)
		return NULL;

	i = c - from;

	if (i >= node->size)
		return NULL;

	return node->items + i;
}


int ab_getKeys(ab_Node *node, char *array)
{
	int i, n = 0;
	for (i = 0; i < node->size; i++) {
		ab_NodeItem* item = node->items + i;

		if (item->flag & AB_ITEM_ON) {
			if (array)
				array[n] = item->letter;
			n++;
		}
	}

	return n;
}


ab_NodeItem *ab_getItem(ab_Node *node, int index)
{
	assert(index >= 0);
	assert(index < node->size);
	return node->items + index;
}


int ab_getIndex(ab_Node *node, ab_NodeItem* item)
{
	return item->letter - ab_nodeFrom(node);
}

